/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngPage.c                                             */
/*                                                                 2024/08/10 */
/* Copyright (C) 2017-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>
#include <MLib/MLibDynamicArray.h>
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <memmap.h>
#include <hardware/IA32/IA32.h>
#include <hardware/IA32/IA32Instruction.h>
#include <hardware/IA32/IA32Paging.h>
#include <hardware/Vga/Vga.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_MEMMNG_PAGE

/** カーネル領域用ページディレクトリエントリ数 */
#define KERNEL_AREA_PDE_NUM \
    ( ( ( ( MEMMAP_VSIZE_BOOTDATA + MEMMAP_VSIZE_KERNEL ) - 1 ) >> 22 ) + 1 )
/** カーネル領域用ページディレクトリエントリサイズ */
#define KERNEL_AREA_PAGE_DIR_SIZE \
    ( KERNEL_AREA_PDE_NUM * sizeof ( IA32PagingPDE_t ) )

/** カーネル領域用ページテーブルサイズ */
#define KERNEL_AREA_PAGE_TBL_SIZE \
    ( KERNEL_AREA_PDE_NUM * sizeof ( IA32PagingTbl_t ) )

/** ch毎アクセス領域マッピング情報 */
typedef struct {
    IA32PagingDir_t *pDirVirt;  /**< ページディレクトリ仮想アドレス */
    IA32PagingDir_t *pDirPhys;  /**< ページディレクトリ物理アドレス */
    IA32PagingTbl_t *pTblVirt;  /**< ページテーブル仮想アドレス     */
    IA32PagingTbl_t *pTblPhys;  /**< ページテーブル物理アドレス     */
} chInfo_t;

/** アクセス領域マッピング情報 */
typedef struct {
    chInfo_t ch1;   /**< アクセス領域ch1 */
    chInfo_t ch2;   /**< アクセス領域ch2 */
} mapInfo_t;

/** ページディレクトリ管理情報 */
typedef struct {
    MkPid_t         pid;            /**< ページディレクトリ使用PID      */
    IA32PagingDir_t *pPageDirPhys;  /**< ページディレクトリ物理アドレス */
} mngInfo_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** アクセス領域マッピング情報 */
static mapInfo_t gMapInfo;

/** ページディレクトリ管理テーブル */
static MLibDynamicArray_t gMngTbl;

/** 現ページディレクトリID */
static MemmngPageDirId_t gNowDirId;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* ページディレクトリ管理情報割当 */
static mngInfo_t *AllocMngInfo( MemmngPageDirId_t *pDirId );
/* ページテーブル割当 */
static IA32PagingTbl_t *AllocPageTbl( IA32PagingPDE_t *pPde,
                                      chInfo_t        *pChInfo );
/* ページディレクトリ複製 */
static CmnRet_t Copy( void *pVirtAddr );
/* ページ複製 */
static CmnRet_t CopyPage( IA32PagingPTE_t *pDstPte,
                          IA32PagingPTE_t *pSrcPte  );
/* ページディレクトリ管理情報解放 */
static void FreeMngInfo( MemmngPageDirId_t dirId );
/* ページテーブル解放 */
static void FreePageTbl( IA32PagingTbl_t *pPageTblPhys );
/* ページディレクトリ管理情報取得 */
static mngInfo_t *GetMngInfo( MemmngPageDirId_t dirId );
/* アイドルプロセス用管理情報初期化 */
static void InitIdleInfo( void );
/* アイドルプロセス用ページディレクトリ初期化 */
static void InitIdlePageDir( void );
/* アイドルプロセス用ページテーブル初期化 */
static void InitIdlePageTbl( void );
/* 4KBページマッピング設定 */
static CmnRet_t Set( void     *pVirtAddr,
                     void     *pPhysAddr,
                     bool     allocPhys,
                     uint32_t attrGlobal,
                     uint32_t attrUs,
                     uint32_t attrRw      );
/* ページディレクトリアクセス領域マッピング設定 */
static void SetPageDirMap( chInfo_t        *pChInfo,
                           IA32PagingDir_t *pDirPhys );
/* ページテーブルアクセス領域マッピング設定 */
static void SetPageTblMap( chInfo_t        *pChInfo,
                           IA32PagingTbl_t *pTblPhys );
/* ページテーブル解放試行 */
static void TryToFreePageTbl( IA32PagingPDE_t *pPde,
                              IA32PagingTbl_t *pPageTbl );
/* 4KBページマッピング解除 */
static void Unset( void *pVirtAddr,
                   bool freePhys    );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ページディレクトリ割当
 * @details     ページディレクトリを割り当てる。
 *
 * @param[in]   pid プロセスID
 *
 * @return      ページディレクトリIDを返す。
 * @retval      MEMMNG_PAGE_DIR_ID_NULL以外 正常終了
 * @retval      MEMMNG_PAGE_DIR_ID_NULL     異常終了
 */
/******************************************************************************/
MemmngPageDirId_t MemmngPageAllocDir( MkPid_t pid )
{
    mngInfo_t         *pMngInfo;        /* 管理情報                         */
    IA32PagingDir_t   *pPageDir;        /* ページディレクトリ               */
    IA32PagingDir_t   *pPageDirPhys;    /* ページディレクトリ(物理アドレス) */
    MemmngPageDirId_t dirId;            /* ページディレクトリID             */

    /* 初期化 */
    pMngInfo     = NULL;
    pPageDir     = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD1;
    pPageDirPhys = NULL;
    dirId        = 0;

    /* 管理情報割当 */
    pMngInfo = AllocMngInfo( &dirId );

    /* 割当結果判定 */
    if ( pMngInfo == NULL ) {
        /* 失敗 */

        return MEMMNG_PAGE_DIR_ID_NULL;
    }

    /* ページディレクトリ割当 */
    pPageDirPhys = ( IA32PagingDir_t * )
                   MemmngPhysAlloc( sizeof ( IA32PagingDir_t ) );

    /* 割当結果判定 */
    if ( pPageDirPhys == NULL ) {
        /* 失敗 */

        /* 管理情報解放 */
        FreeMngInfo( dirId );

        return MEMMNG_PAGE_DIR_ID_NULL;
    }

    /* 管理情報設定 */
    pMngInfo->pid          = pid;
    pMngInfo->pPageDirPhys = pPageDirPhys;

    /* ページディレクトリ操作領域マッピング設定 */
    SetPageDirMap( &( gMapInfo.ch1 ), pPageDirPhys );

    /* ページディレクトリ初期化 */
    MLibUtilSetMemory32( pPageDir, 0, sizeof ( IA32PagingDir_t ) );

    /* カーネル領域マッピング設定 */
    MLibUtilCopyMemory( pPageDir,
                        ( IA32PagingDir_t * ) MEMMAP_PADDR_IDLE_PD,
                        KERNEL_AREA_PAGE_DIR_SIZE                   );

    return dirId;
}


/******************************************************************************/
/**
 * @brief       ページ複製
 * @details     仮想アドレス空間の指定領域を別の仮想アドレス空間に複製する。複
 *              製先の仮想アドレス空間に物理メモリ領域の割当てが無い場合は、物
 *              理メモリ領域を割当ててから複製する。
 *
 * @param[in]   dstDirId  複製先ページディレクトリID
 * @param[in]   srcDirId  複製元ページディレクトリID
 * @param[in]   pVirtAddr 先頭仮想アドレス
 * @param[in]   size      サイズ
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 *
 * @attention   引数pVirtAddrとsizeは4KiBアライメントであること。
 */
/******************************************************************************/
CmnRet_t MemmngPageCopy( MemmngPageDirId_t dstDirId,
                         MemmngPageDirId_t srcDirId,
                         void              *pVirtAddr,
                         size_t            size        )
{
    CmnRet_t  ret;          /* 戻り値   */
    mngInfo_t *pMngInfo;    /* 管理情報 */

    /* 初期化 */
    ret      = CMN_FAILURE;
    pMngInfo = NULL;

    /* 複製元ページディレクトリ管理情報取得 */
    pMngInfo = GetMngInfo( srcDirId );

    /* 取得結果判定 */
    if ( pMngInfo == NULL ) {
        /* 失敗 */
        return CMN_FAILURE;
    }

    /* ページディレクトリch1アクセス領域設定 */
    SetPageDirMap( &( gMapInfo.ch1 ), pMngInfo->pPageDirPhys );

    /* 複製先ページディレクトリ管理情報取得 */
    pMngInfo = GetMngInfo( dstDirId );

    /* 取得結果判定 */
    if ( pMngInfo == NULL ) {
        /* 失敗 */
        return CMN_FAILURE;
    }

    /* ページディレクトリch2アクセス領域設定 */
    SetPageDirMap( &( gMapInfo.ch2 ), pMngInfo->pPageDirPhys );

    /* 4KB毎に繰り返す */
    while ( size >= IA32_PAGING_PAGE_SIZE ) {
        /* ページ複製 */
        ret = Copy( pVirtAddr );

        /* 複製結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */
            break;
        }

        /* アドレス・サイズ更新 */
        pVirtAddr += IA32_PAGING_PAGE_SIZE;
        size      -= IA32_PAGING_PAGE_SIZE;
    }

    return ret;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ解放
 * @details     ページディレクトリを解放する。
 *
 * @param[in]   dirId ページディレクトリID
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemmngPageFreeDir( MemmngPageDirId_t dirId )
{
    uint32_t        idx;            /* インデックス                 */
    mngInfo_t       *pMngInfo;      /* 管理情報                     */
    IA32PagingDir_t *pPageDir;      /* ページディレクトリ           */
    IA32PagingTbl_t *pPageTblPhys;  /* ページテーブル(物理アドレス) */

    /* 初期化 */
    pMngInfo     = NULL;
    pPageDir     = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD1;
    pPageTblPhys = NULL;

    /* 管理情報取得 */
    pMngInfo = GetMngInfo( dirId );

    /* 取得結果判定 */
    if ( pMngInfo == NULL ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* ページディレクトリ操作領域マッピング */
    SetPageDirMap( &( gMapInfo.ch1 ), pMngInfo->pPageDirPhys );

    /* ユーザ領域のページディレクトリエントリ毎に繰り返し */
    for ( idx = KERNEL_AREA_PDE_NUM; idx < IA32_PAGING_PDE_NUM; idx++ ) {
        /* ページテーブル存在判定 */
        if ( pPageDir->entry[ idx ].attr_p == IA32_PAGING_P_YES ) {
            /* 存在する */

            /* ページテーブル物理アドレス取得 */
            pPageTblPhys = ( IA32PagingTbl_t * )
                           IA32_PAGING_GET_BASE( &( pPageDir->entry[ idx ] ) );

            /* ページテーブル操作領域マッピング設定 */
            SetPageTblMap( &( gMapInfo.ch1 ), pPageTblPhys );

            /* ページテーブル解放 */
            FreePageTbl( pPageTblPhys );
        }
    }

    /* ページディレクトリ初期化 */
    MLibUtilSetMemory32( pPageDir, 0, sizeof ( IA32PagingDir_t ) );

    /* ページディレクトリ解放 */
    MemmngPhysFree( pMngInfo->pPageDirPhys );

    /* 管理情報解放 */
    FreeMngInfo( dirId );

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリID取得
 * @details     現在設定されているページディレクトリのIDを取得する。
 *
 * @return      ページディレクトリIDを返す。
 */
/******************************************************************************/
MemmngPageDirId_t MemmngPageGetDirId( void )
{
    return gNowDirId;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリベースレジスタ値取得
 * @details     ページディレクトリIDのページディレクトリベースレジスタ値を取得
 *              する。
 *
 * @param[in]   dirId ページディレクトリID
 *
 * @return      ページディレクトリベースレジスタ値を返す。
 */
/******************************************************************************/
IA32PagingPDBR_t MemmngPageGetPdbr( MemmngPageDirId_t dirId )
{
    mngInfo_t        *pMngInfo; /* 管理情報 */
    IA32PagingPDBR_t pdbr;      /* 戻り値   */

    /* 管理情報取得 */
    pMngInfo = GetMngInfo( dirId );

    /* PDBR作成 */
    IA32_PAGING_SET_PDBR( pdbr,
                          pMngInfo->pPageDirPhys,
                          IA32_PAGING_PCD_ENABLE,
                          IA32_PAGING_PWT_WT      );

    return pdbr;
}


/******************************************************************************/
/**
 * @brief       ページマッピング設定
 * @details     物理アドレスと仮想アドレスのマッピング設定を行う。
 *
 * @param[in]   dirId       ページディレクトリID
 * @param[in]   *pVirtAddr  仮想アドレス
 * @param[in]   *pPhysAddr  物理アドレス
 * @param[in]   size        マッピングサイズ
 * @param[in]   allocPhys   物理メモリ領域自動割当て
 *                  - MEMMNG_PAGE_ALLOC_PHYS_TRUE  割当有り
 *                  - MEMMNG_PAGE_ALLOC_PHYS_FALSE 割当無し
 * @param[in]   attrGlobal グローバルページ属性
 *                  - IA32_PAGING_G_NO  非グローバルページ
 *                  - IA32_PAGING_G_YES グローバルページ
 * @param[in]   attrUs     ユーザ/スーパバイザ属性
 *                  - IA32_PAGING_US_SV   スーパバイザ
 *                  - IA32_PAGING_US_USER ユーザ
 * @param[in]   attrRw     読込/書込許可属性
 *                  - IA32_PAGING_RW_R  読込専用
 *                  - IA32_PAGING_RW_RW 読込/書込可
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 *
 * @attention   引数pVirtAddr、pPhysAddr、sizeは4KiBアライメントであること。
 */
/******************************************************************************/
CmnRet_t MemmngPageSet( MemmngPageDirId_t dirId,
                        void              *pVirtAddr,
                        void              *pPhysAddr,
                        size_t            size,
                        bool              allocPhys,
                        uint32_t          attrGlobal,
                        uint32_t          attrUs,
                        uint32_t          attrRw      )
{
    void      *pNextVirtAddr;   /* 次仮想アドレス         */
    void      *pNextPhysAddr;   /* 次物理アドレス         */
    size_t    remain;           /* 残マッピングサイズ     */
    CmnRet_t  ret;              /* 関数戻り値             */
    mngInfo_t *pMngInfo;        /* 管理情報               */

    /* 初期化 */
    pNextVirtAddr = pVirtAddr;
    pNextPhysAddr = pPhysAddr;
    remain        = size;
    ret           = CMN_SUCCESS;
    pMngInfo      = NULL;

    /* ページディレクトリID判定 */
    if ( dirId != MEMMNG_PAGE_DIR_ID_IDLE ) {
        /* 非アイドルプロセス用 */

        /* 管理情報取得 */
        pMngInfo = GetMngInfo( dirId );

        /* 取得結果判定 */
        if ( pMngInfo == NULL ) {
            /* 失敗 */

            return CMN_FAILURE;
        }

        /* ページディレクトリ操作領域マッピング */
        SetPageDirMap( &( gMapInfo.ch1 ), pMngInfo->pPageDirPhys );
    }

    /* 4KB毎に繰り返し */
    while ( remain >= IA32_PAGING_PAGE_SIZE ) {

        /* 4KBページマッピング設定 */
        ret = Set( pNextVirtAddr,
                   pNextPhysAddr,
                   allocPhys,
                   attrGlobal,
                   attrUs,
                   attrRw         );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */
            break;
        }

        /* 次アドレス更新 */
        pNextVirtAddr += IA32_PAGING_PAGE_SIZE;
        pNextPhysAddr += IA32_PAGING_PAGE_SIZE;

        /* 残マッピングサイズ更新 */
        remain -= IA32_PAGING_PAGE_SIZE;
    }

    /* 結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* マッピング解除 */
        MemmngPageUnset( dirId, pVirtAddr, size - remain, allocPhys );
    }

    return ret;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ切替
 * @details     指定したページディレクトリに切り替える。
 *
 * @param[in]   dirId ページディレクトリID
 */
/******************************************************************************/
void MemmngPageSwitchDir( MemmngPageDirId_t dirId )
{
    /* 現ページディレクトリID設定 */
    gNowDirId = dirId;

    return;
}


/******************************************************************************/
/**
 * @brief       ページマッピング解除
 * @details     物理アドレスと仮想アドレスのマッピング設定を解除する。
 *
 * @param[in]   dirId      ページディレクトリID
 * @param[in]   *pVirtAddr 仮想アドレス
 * @param[in]   size       マッピングサイズ
 * @param[in]   freePhys   物理メモリ領域自動解放
 *                  - MEMMNG_PAGE_FREE_PHYS_TRUE  解放有り
 *                  - MEMMNG_PAGE_FREE_PHYS_FALSE 解放無し
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 *
 * @attention   引数pVirtAddr、sizeは4KiBアライメントであること。
 */
/******************************************************************************/
CmnRet_t MemmngPageUnset( MemmngPageDirId_t dirId,
                          void              *pVirtAddr,
                          size_t            size,
                          bool              freePhys    )
{
    mngInfo_t *pMngInfo;    /* 管理情報 */

    /* 初期化 */
    pMngInfo = NULL;

    /* ページディレクトリID判定 */
    if ( dirId != MEMMNG_PAGE_DIR_ID_IDLE ) {
        /* 非アイドルプロセス用 */

        /* 管理情報取得 */
        pMngInfo = GetMngInfo( dirId );

        /* 取得結果判定 */
        if ( pMngInfo == NULL ) {
            /* 失敗 */

            return CMN_FAILURE;
        }

        /* ページディレクトリ操作領域マッピング */
        SetPageDirMap( &( gMapInfo.ch1 ), pMngInfo->pPageDirPhys );
    }

    /* 4KB毎に繰り返し */
    while ( size >= IA32_PAGING_PAGE_SIZE ) {
        /* 4KBページマッピング解除 */
        Unset( pVirtAddr, freePhys );

        /* アドレス更新 */
        pVirtAddr += IA32_PAGING_PAGE_SIZE;

        /* サイズ更新 */
        size -= IA32_PAGING_PAGE_SIZE;
    }

    return CMN_SUCCESS;
}


/******************************************************************************/
/* モジュール内向けグローバル関数定義                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ページング管理サブモジュール初期化
 * @details     ページング管理機能を初期化してページングを有効化する。
 */
/******************************************************************************/
void PageInit( void )
{
    MLibErr_t        errMLib;   /* MLIBライブラリエラー値 */
    IA32PagingPDBR_t pdbr;      /* PDBR                   */

    /* 初期化 */
    errMLib = MLIB_ERR_NONE;

    /* アクセス領域マッピング情報初期化 */
    MLibUtilSetMemory32( &gMapInfo, 0, sizeof ( mapInfo_t ) );
    gMapInfo.ch1.pDirVirt = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD1;
    gMapInfo.ch1.pTblVirt = ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT1;
    gMapInfo.ch2.pDirVirt = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD2;
    gMapInfo.ch2.pTblVirt = ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT2;

    /* アイドルプロセス用ページディレクトリ初期化 */
    InitIdlePageDir();

    /* アイドルプロセス用ページテーブル初期化 */
    InitIdlePageTbl();

    /* 現ページディレクトリID設定 */
    gNowDirId = 0;

    /* ページディレクトリベースレジスタ値作成 */
    IA32_PAGING_SET_PDBR( pdbr,
                          MEMMAP_PADDR_IDLE_PD,
                          IA32_PAGING_PCD_ENABLE,
                          IA32_PAGING_PWT_WT      );

    /* ページディレクトリ設定 */
    IA32InstructionSetCr3( pdbr );

    /* ページング有効化 */
    IA32InstructionSetCr0( IA32_CR0_PG, IA32_CR0_PG );

    /* グローバルページ有効化 */
    /* TODO */

    /* ページディレクトリ管理テーブル初期化 */
    MLibDynamicArrayInit( &gMngTbl,
                          128,
                          sizeof ( mngInfo_t ),
                          UINT32_MAX,
                          &errMLib              );

    /* アイドルプロセス用管理情報初期化 */
    InitIdleInfo();

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ページディレクトリ管理情報割当
 * @details     ページディレクトリ管理情報を割当てる。
 *
 * @param[out]  pDirId ページディレクトリID
 *
 * @return      割当てたページディレクトリ管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 ページディレクトリ管理情報
 */
/******************************************************************************/
static mngInfo_t *AllocMngInfo( MemmngPageDirId_t *pDirId )
{
    mngInfo_t *pRet;    /* 戻り値                 */
    MLibErr_t errMLib;  /* MLibライブラリエラー値 */
    MLibRet_t retMLib;  /* MLibライブラリ戻り値   */

    /* 初期化 */
    pRet    = NULL;
    errMLib = MLIB_ERR_NONE;
    retMLib = MLIB_RET_FAILURE;

    /* 管理情報割当 */
    retMLib = MLibDynamicArrayAlloc( &gMngTbl,
                                     pDirId,
                                     ( void ** ) &pRet,
                                     &errMLib           );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* TODO */
    }

    return pRet;
}


/******************************************************************************/
/**
 * @brief       ページテーブル割当
 * @details     ページテーブルを割り当て、チャンネル情報が指すアクセス領域に
 *              マッピングする。
 *
 * @param[in]   *pPde    ページディレクトリエントリ
 * @param[in]   *pChInfo アクセス領域チャンネル情報
 *
 * @return      割り当てたページテーブルの物理アドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 */
/******************************************************************************/
static IA32PagingTbl_t *AllocPageTbl( IA32PagingPDE_t *pPde,
                                      chInfo_t        *pChInfo )
{
    IA32PagingTbl_t *pPageTblPhys; /* ページテーブル(物理アドレス) */

    /* ページテーブル割当 */
    pPageTblPhys = ( IA32PagingTbl_t * )
                   MemmngPhysAlloc( sizeof ( IA32PagingTbl_t ) );

    /* 割当結果判定 */
    if ( pPageTblPhys == NULL ) {
        /* 失敗 */

        return NULL;
    }

    /* ページディレクトリエントリ初期化 */
    MLibUtilSetMemory32( pPde, 0, sizeof ( IA32PagingPDE_t ) );

    /* ページディレクトリエントリ設定 */
    pPde->attr_p   = IA32_PAGING_P_YES;
    pPde->attr_rw  = IA32_PAGING_RW_RW;
    pPde->attr_us  = IA32_PAGING_US_USER;
    pPde->attr_pwt = IA32_PAGING_PWT_WT;
    pPde->attr_pcd = IA32_PAGING_PCD_DISABLE;
    pPde->attr_a   = IA32_PAGING_A_NO;
    pPde->attr_ps  = IA32_PAGING_PS_4K;
    pPde->attr_g   = IA32_PAGING_G_NO;
    pPde->attr_avl = 0;
    IA32_PAGING_SET_BASE( pPde, pPageTblPhys );

    /* ページテーブル操作領域マッピング */
    SetPageTblMap( pChInfo, pPageTblPhys );

    /* ページテーブル初期化 */
    MLibUtilSetMemory32( pChInfo->pTblVirt, 0, sizeof ( IA32PagingTbl_t ) );

    return pPageTblPhys;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ複製
 * @details     4KBページを複製する。
 *
 * @param[in]   *pVirtAddr 仮想アドレス
 */
/******************************************************************************/
static CmnRet_t Copy( void *pVirtAddr )
{
    uint32_t        pdeIdx;         /* ページディレクトリエントリインデックス */
    uint32_t        pteIdx;         /* ページテーブルエントリインデックス     */
    IA32PagingDir_t *pSrcDir;       /* 複製元ページディレクトリ               */
    IA32PagingTbl_t *pSrcTbl;       /* 複製元ページテーブル                   */
    IA32PagingPDE_t *pSrcPde;       /* 複製元ページディレクトリエントリ       */
    IA32PagingDir_t *pDstDir;       /* 複製先ページディレクトリ               */
    IA32PagingTbl_t *pDstTbl;       /* 複製先ページテーブル                   */
    IA32PagingTbl_t *pDstTblPhys;   /* 複製先ページテーブル物理アドレス       */
    IA32PagingPDE_t *pDstPde;       /* 複製先ページディレクトリエントリ       */

    /* 初期化 */
    pdeIdx      = IA32_PAGING_GET_PDE_IDX( pVirtAddr );
    pteIdx      = IA32_PAGING_GET_PTE_IDX( pVirtAddr );
    pSrcDir     = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD1;
    pSrcTbl     = ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT1;
    pSrcPde     = &( pSrcDir->entry[ pdeIdx ] );
    pDstDir     = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD2;
    pDstTbl     = ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT2;
    pDstTblPhys = NULL;
    pDstPde     = &( pDstDir->entry[ pdeIdx ] );

    /* 複製元ページテーブル存在チェック */
    if ( pSrcPde->attr_p == IA32_PAGING_P_NO ) {
        /* 存在しない */

        return CMN_SUCCESS;
    }

    /* ch1ページテーブルアクセス領域設定 */
    SetPageTblMap( &( gMapInfo.ch1 ),
                   ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pSrcPde ) );

    /* 複製先ページテーブル存在チェック */
    if ( pDstPde->attr_p == IA32_PAGING_P_NO ) {
        /* 存在しない */

        /* 複製先ページテーブル割当 */
        pDstTblPhys = AllocPageTbl( pDstPde, &( gMapInfo.ch2 ) );

        /* 割当結果判定 */
        if ( pDstTblPhys == NULL ) {
            /* 失敗 */

            return CMN_FAILURE;
        }

        /* 複製先ページディレクトリエントリ設定 */
        pDstPde->attr_rw  = pSrcPde->attr_rw;
        pDstPde->attr_us  = pSrcPde->attr_us;
        pDstPde->attr_pwt = pSrcPde->attr_pwt;
        pDstPde->attr_pcd = pSrcPde->attr_pcd;
        pDstPde->attr_g   = pSrcPde->attr_g;

    } else {
        /* 存在する */

        /* ch2ページテーブルアクセス領域設定 */
        SetPageTblMap( &( gMapInfo.ch2 ),
                       ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pDstPde ) );
    }

    /* ページ複製 */
    return CopyPage( &( pDstTbl->entry[ pteIdx ] ),
                     &( pSrcTbl->entry[ pteIdx ] )  );
}


/******************************************************************************/
/**
 * @brief       ページ複製
 * @details     複製元ページを複製先ページにコピーする。複製元ページが存在しな
 *              い場合はコピーしない。複製先ページのメモリが割当てられていない
 *              場合は物理メモリを割当てる。
 *
 * @param[in]   *pDstPte 複製元ページテーブルエントリ
 * @param[in]   *pSrcPte 複製先ページテーブルエントリ
 *
 * @return      ページ複製結果を返す。
 * @retval      CMN_FAILURE 失敗
 * @retval      CMN_SUCCESS 成功
 */
/******************************************************************************/
static CmnRet_t CopyPage( IA32PagingPTE_t *pDstPte,
                          IA32PagingPTE_t *pSrcPte  )
{
    void *pDstPagePhys; /* 複製先ページ物理アドレス */
    void *pSrcPagePhys; /* 複製元ページ物理アドレス */

    /* 初期化 */
    pDstPagePhys = NULL;
    pSrcPagePhys = IA32_PAGING_GET_BASE( pSrcPte );

    /* 複製元ページ存在チェック */
    if ( pSrcPte->attr_p == IA32_PAGING_P_NO ) {
        /* 存在しない */

        return CMN_SUCCESS;
    }

    /* 複製先ページ存在チェック */
    if ( pDstPte->attr_p == IA32_PAGING_P_NO ) {
        /* 存在しない */

        /* 複製先ページ割当 */
        pDstPagePhys = MemmngPhysAlloc( sizeof ( IA32_PAGING_PAGE_SIZE ) );

        /* 割当結果判定 */
        if ( pDstPagePhys == NULL ) {
            /* 失敗 */

            return CMN_FAILURE;
        }

        /* 複製先ページテーブルエントリ設定 */
        pDstPte->attr_p = IA32_PAGING_P_YES;
        IA32_PAGING_SET_BASE( pDstPte, pDstPagePhys );

    } else {
        /* 存在する */

        /* 複製先ページ物理アドレス取得 */
        pDstPagePhys = IA32_PAGING_GET_BASE( pDstPte );
    }

    /* 複製先ページテーブルエントリ設定 */
    pDstPte->attr_rw  = pSrcPte->attr_rw;
    pDstPte->attr_us  = pSrcPte->attr_us;
    pDstPte->attr_pwt = pSrcPte->attr_pwt;
    pDstPte->attr_pcd = pSrcPte->attr_pcd;
    pDstPte->attr_a   = IA32_PAGING_A_NO;
    pDstPte->attr_d   = IA32_PAGING_D_NO;
    pDstPte->attr_pat = IA32_PAGING_PAT_UNUSED;
    pDstPte->attr_g   = pSrcPte->attr_g;
    pDstPte->attr_avl = 0;

    /* ページ複製 */
    MemmngCtrlCopyPhysToPhys( pDstPagePhys,
                              pSrcPagePhys,
                              IA32_PAGING_PAGE_SIZE );

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ管理情報解放
 * @details     ページディレクトリIDが指すページディレクトリ管理情報を解放する。
 *
 * @param[in]   dirId ページディレクトリID
 */
/******************************************************************************/
static void FreeMngInfo( MemmngPageDirId_t dirId )
{
    MLibErr_t errMLib;  /* MLIBライブラリエラー値 */
    MLibRet_t retMLib;  /* MLIBライブラリ戻り値   */

    /* 初期化 */
    errMLib = MLIB_ERR_NONE;
    retMLib = MLIB_RET_FAILURE;

    /* 解放 */
    retMLib = MLibDynamicArrayFree( &gMngTbl, dirId, &errMLib );

    /* 解放結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* TODO */
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ページテーブル解放
 * @details     ページテーブルを解放する。
 *
 * @param[in]   *pPageTblPhys ページテーブル(物理アドレス)
 */
/******************************************************************************/
static void FreePageTbl( IA32PagingTbl_t *pPageTblPhys )
{
    /* ページテーブル初期化 */
    MLibUtilSetMemory32( ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT1,
                         0,
                         sizeof ( IA32PagingTbl_t )                     );

    /* ページテーブル解放 */
    MemmngPhysFree( pPageTblPhys );

    return;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ管理情報取得
 * @details     ページディレクトリIDからページディレクトリ管理情報を取得する。
 *
 * @param[in]   dirId ページディレクトリID
 *
 * @return      ページディレクトリ管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 ページディレクトリ管理情報
 */
/******************************************************************************/
static mngInfo_t *GetMngInfo( MemmngPageDirId_t dirId )
{
    mngInfo_t *pRet;    /* 管理情報               */
    MLibErr_t errMLib;  /* MLIBライブラリエラー値 */
    MLibRet_t retMLib;  /* MLIBライブラリ戻り値   */

    /* 初期化 */
    pRet    = NULL;
    errMLib = MLIB_ERR_NONE;
    retMLib = MLIB_RET_FAILURE;

    /* 管理情報取得 */
    retMLib = MLibDynamicArrayGet( &gMngTbl,
                                   dirId,
                                   ( void ** ) &pRet,
                                   &errMLib           );

    /* 取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG_WRN( "dirID( %d ) failure.", dirId );
    }

    return pRet;
}


/******************************************************************************/
/**
 * @brief       アイドルプロセス用管理情報初期化
 * @details     アイドルプロセス用のページング管理情報を初期化する。
 */
/******************************************************************************/
static void InitIdleInfo( void )
{
    mngInfo_t *pMngInfo;    /* アイドルプロセス用管理情報 */

    /* 初期化 */
    pMngInfo = NULL;

    /* アイドルプロセス用管理情報割当 */
    MLibDynamicArrayAllocSpec( &gMngTbl,
                               MEMMNG_PAGE_DIR_ID_IDLE,
                               ( void ** ) &pMngInfo,
                               NULL                     );

    /* アイドルプロセス用管理情報設定 */
    pMngInfo->pid          = MK_PID_IDLE;
    pMngInfo->pPageDirPhys = ( IA32PagingDir_t * ) MEMMAP_PADDR_IDLE_PD;

    return;
}


/******************************************************************************/
/**
 * @brief       アイドルプロセス用ページディレクトリ初期化
 * @details     カーネル領域に対応する全エントリをアイドルプロセス用ページテー
 *              ブルにリンクする様に初期化する。
 */
/******************************************************************************/
static void InitIdlePageDir( void )
{
    uint32_t        idx;        /* PDEインデックス            */
    IA32PagingDir_t *pPageDir;  /* ページディレクトリ         */
    IA32PagingTbl_t *pPageTbl;  /* ページテーブル             */
    IA32PagingPDE_t *pPde;      /* ページディレクトリエントリ */

    /* 初期化 */
    idx      = 0;
    pPageDir = ( IA32PagingDir_t * ) MEMMAP_PADDR_IDLE_PD;
    pPageTbl = ( IA32PagingTbl_t * ) MEMMAP_PADDR_KERNEL_PT;
    pPde     = NULL;

    /* アイドルプロセス用ページディレクトリ0初期化 */
    MLibUtilSetMemory32( pPageDir, 0, sizeof ( IA32PagingDir_t ) );

    /* カーネル領域に対応するエントリ毎に繰り返し */
    for ( idx = 0; idx < KERNEL_AREA_PDE_NUM; idx++ ) {

        /* アドレス設定 */
        pPde = &( pPageDir->entry[ idx ] );

        /* ページディレクトリエントリ設定 */
        pPde->attr_p   = IA32_PAGING_P_YES;
        pPde->attr_rw  = IA32_PAGING_RW_RW;
        pPde->attr_us  = IA32_PAGING_US_SV;
        pPde->attr_pwt = IA32_PAGING_PWT_WT;
        pPde->attr_pcd = IA32_PAGING_PCD_DISABLE;
        pPde->attr_a   = IA32_PAGING_A_NO;
        pPde->attr_ps  = IA32_PAGING_PS_4K;
        pPde->attr_g   = IA32_PAGING_G_NO;
        pPde->attr_avl = 0;
        IA32_PAGING_SET_BASE( pPde, &( pPageTbl[ idx ] ) );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       アイドルプロセス用ページテーブル初期化
 * @details     カーネル領域に対応する全エントリを物理アドレスと仮想アドレスが
 *              等しくなる様に初期化する。
 */
/******************************************************************************/
static void InitIdlePageTbl( void )
{
    void            *pPhysAddr; /* 物理アドレス           */
    uint32_t        pdeIdx;     /* PDEインデックス        */
    uint32_t        pteIdx;     /* PTEインデックス        */
    IA32PagingTbl_t *pPageTbl;  /* ページテーブル         */
    IA32PagingPTE_t *pPte;      /* ページテーブルエントリ */

    /* 初期化 */
    pPhysAddr  = NULL;
    pdeIdx     = 0;
    pteIdx     = 0;
    pPageTbl   = ( IA32PagingTbl_t * ) MEMMAP_PADDR_KERNEL_PT;
    pPte       = NULL;

    /* アイドルプロセス用ページテーブル0初期化 */
    MLibUtilSetMemory32( pPageTbl, 0, KERNEL_AREA_PAGE_TBL_SIZE );

    /* カーネル領域に対応するエントリ毎に繰り返し */
    for ( pdeIdx = 0; pdeIdx < KERNEL_AREA_PDE_NUM; pdeIdx++ ) {
        for ( pteIdx = 0; pteIdx < IA32_PAGING_PTE_NUM; pteIdx++ ) {

            /* アドレス設定 */
            pPte      = &( pPageTbl[ pdeIdx ].entry[ pteIdx ] );
            pPhysAddr =
                ( void * )
                ( pdeIdx * IA32_PAGING_PAGE_SIZE * IA32_PAGING_PTE_NUM +
                  pteIdx * IA32_PAGING_PAGE_SIZE                         );

            /* ページテーブルエントリ設定 */
            pPte->attr_p   = IA32_PAGING_P_YES;
            pPte->attr_rw  = IA32_PAGING_RW_RW;
            pPte->attr_us  = IA32_PAGING_US_SV;
            pPte->attr_pwt = IA32_PAGING_PWT_WT;
            pPte->attr_pcd = IA32_PAGING_PCD_DISABLE;
            pPte->attr_a   = IA32_PAGING_A_NO;
            pPte->attr_d   = IA32_PAGING_D_NO;
            pPte->attr_pat = IA32_PAGING_PAT_UNUSED;
            pPte->attr_g   = IA32_PAGING_G_YES;
            pPte->attr_avl = 0;
            IA32_PAGING_SET_BASE( pPte, pPhysAddr );
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief       4KBページマッピング設定
 * @details     物理アドレスを仮想アドレスの4KBページにマッピングする。
 *
 * @param[in]   *pVirtAddr 仮想アドレス
 * @param[in]   *pPhysAddr 物理アドレス
 * @param[in]   allocPhys  物理メモリ領域自動割当
 *                  - MEMMNG_PAGE_ALLOC_PHYS_TRUE  割当有り
 *                  - MEMMNG_PAGE_ALLOC_PHYS_FALSE 割当無し
 * @param[in]   attrGlobal グローバルページ属性
 *                  - IA32_PAGING_G_NO  非グローバルページ
 *                  - IA32_PAGING_G_YES グローバルページ
 * @param[in]   attrUs     ユーザ/スーパバイザ属性
 *                  - IA32_PAGING_US_SV   スーパバイザ
 *                  - IA32_PAGING_US_USER ユーザ
 * @param[in]   attrRw     読込/書込許可属性
 *                  - IA32_PAGING_RW_R  読込専用
 *                  - IA32_PAGING_RW_RW 読込/書込可
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 *
 * @attention   引数pVirtAddr、pPhysAddrは4KBアライメントされていること。
 */
/******************************************************************************/
static CmnRet_t Set( void     *pVirtAddr,
                     void     *pPhysAddr,
                     bool     allocPhys,
                     uint32_t attrGlobal,
                     uint32_t attrUs,
                     uint32_t attrRw      )
{
    uint32_t        pdeIdx;         /* ページディレクトリエントリインデックス */
    uint32_t        pteIdx;         /* ページテーブルエントリインデックス     */
    IA32PagingDir_t *pPageDir;      /* ページディレクトリ                     */
    IA32PagingTbl_t *pPageTbl;      /* ページテーブル                         */
    IA32PagingTbl_t *pPageTblPhys;  /* ページテーブル(物理アドレス)           */
    IA32PagingPDE_t *pPde;          /* ページディレクトリエントリ             */
    IA32PagingPTE_t *pPte;          /* ページテーブルエントリ                 */

    /* 初期化 */
    pdeIdx       = IA32_PAGING_GET_PDE_IDX( pVirtAddr );
    pteIdx       = IA32_PAGING_GET_PTE_IDX( pVirtAddr );
    pPageDir     = NULL;
    pPageTbl     = NULL;
    pPageTblPhys = NULL;
    pPde         = NULL;

    /* 物理メモリ領域自動割当判定 */
    if ( allocPhys != false ) {
        /* 割当有り */

        /* 物理メモリ領域割当 */
        pPhysAddr = MemmngPhysAlloc( IA32_PAGING_PAGE_SIZE );

        /* 割当結果判定 */
        if ( pPhysAddr == NULL ) {
            /* 失敗 */
            return CMN_FAILURE;
        }
    }

    /* カーネル領域判定 */
    if ( pVirtAddr < ( void * ) MEMMAP_VADDR_USER ) {
        /* カーネル領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) MEMMAP_PADDR_IDLE_PD;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );

    } else {
        /* ユーザ領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD1;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT1;


        /* ページテーブル存在チェック */
        if ( pPde->attr_p == IA32_PAGING_P_NO ) {
            /* 存在しない */

            /* ページテーブル割当 */
            pPageTblPhys = AllocPageTbl( pPde, &( gMapInfo.ch1 ) );

            /* 割当結果判定 */
            if ( pPageTblPhys == NULL ) {
                /* 失敗 */

                return CMN_FAILURE;
            }

        } else {
            /* 存在する */

            /* ページテーブルアドレス取得 */
            pPageTblPhys = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );

            /* ページテーブル操作領域マッピング */
            SetPageTblMap( &( gMapInfo.ch1 ), pPageTblPhys );
        }
    }

    /* ページテーブルエントリアドレス取得 */
    pPte = &( pPageTbl->entry[ pteIdx ] );

    /* ページテーブルエントリ初期化 */
    MLibUtilSetMemory32( pPte, 0, sizeof ( IA32PagingPTE_t ) );

    /* ページテーブルエントリ設定 */
    pPte->attr_p   = IA32_PAGING_P_YES;
    pPte->attr_rw  = attrRw;
    pPte->attr_us  = attrUs;
    pPte->attr_pwt = IA32_PAGING_PWT_WT;
    pPte->attr_pcd = IA32_PAGING_PCD_ENABLE;
    pPte->attr_a   = IA32_PAGING_A_NO;
    pPte->attr_d   = IA32_PAGING_D_NO;
    pPte->attr_pat = IA32_PAGING_PAT_UNUSED;
    pPte->attr_g   = attrGlobal;
    pPte->attr_avl = 0;
    IA32_PAGING_SET_BASE( pPte, pPhysAddr );

    /* TLBフラッシュ */
    IA32InstructionInvlpg( pVirtAddr );

    return CMN_SUCCESS;
}


/******************************************************************************/
/*
 * @brief       ページディレクトリアクセス領域マッピング設定
 * @details     チャンネル情報が指すアクセス領域にページディレクトリをマッピン
 *              グする。
 *
 * @param[in]   *pChInfo  アクセス領域チャンネル情報
 * @param[in]   *pDirPhys ページディレクトリ物理アドレス
 */
/******************************************************************************/
static void SetPageDirMap( chInfo_t        *pChInfo,
                           IA32PagingDir_t *pDirPhys )
{
    /* マッピング中アドレス比較 */
    if ( pChInfo->pDirPhys != pDirPhys ) {
        /* 不一致 */

        /* マッピング中アドレス設定 */
        pChInfo->pDirPhys = pDirPhys;

        /* マッピング */
        MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                       pChInfo->pDirVirt,
                       pDirPhys,
                       sizeof ( IA32PagingDir_t ),
                       MEMMNG_PAGE_ALLOC_PHYS_FALSE,
                       IA32_PAGING_G_NO,
                       IA32_PAGING_US_SV,
                       IA32_PAGING_RW_RW             );
    }

    return;
}


/******************************************************************************/
/*
 * @brief       ページテーブルアクセス領域マッピング設定
 * @details     チャンネル情報が指すアクセス領域にページテーブルをマッピングす
 *              る。
 *
 * @param[in]   *pChInfo  アクセス領域チャンネル情報
 * @param[in]   *pTblPhys ページテーブル物理アドレス
 */
/******************************************************************************/
static void SetPageTblMap( chInfo_t        *pChInfo,
                           IA32PagingTbl_t *pTblPhys )
{
    /* マッピング中アドレス比較 */
    if ( pChInfo->pTblPhys != pTblPhys ) {
        /* 不一致 */

        /* マッピング中アドレス設定 */
        pChInfo->pTblPhys = pTblPhys;

        /* マッピング */
        MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                       pChInfo->pTblVirt,
                       pTblPhys,
                       sizeof ( IA32PagingTbl_t ),
                       MEMMNG_PAGE_ALLOC_PHYS_FALSE,
                       IA32_PAGING_G_NO,
                       IA32_PAGING_US_SV,
                       IA32_PAGING_RW_RW             );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ページテーブル解放試行
 * @details     ページテーブル内のエントリが全て存在しない場合は、ページテーブ
 *              ルを解放し、リンク元のページディレクトリエントリをクリアする。
 *
 * @param[in]   *pPde         ページディレクトリエントリ
 * @param[in]   *pPageTblPhys ページテーブル(物理アドレス)
 */
/******************************************************************************/
static void TryToFreePageTbl( IA32PagingPDE_t *pPde,
                              IA32PagingTbl_t *pPageTblPhys )
{
    uint32_t        idx;        /* ページテーブルエントリインデックス */
    IA32PagingTbl_t *pPageTbl;  /* ページテーブル                     */

    /* 初期化 */
    pPageTbl = ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT1;

    /* ページテーブル毎に繰り返す */
    for ( idx = 0; idx < IA32_PAGING_PTE_NUM; idx++ ) {
        /* 存在フラグ判定 */
        if ( pPageTbl->entry[ idx ].attr_p == IA32_PAGING_P_YES ) {
            /* 存在する */
            return;
        }
    }

    /* ページテーブル解放 */
    FreePageTbl( pPageTblPhys );

    /* ページディレクトリエントリ初期化 */
    MLibUtilSetMemory32( pPde, 0, sizeof ( IA32PagingTbl_t ) );

    return;
}


/******************************************************************************/
/**
 * @brief       4KBページマッピング解除
 * @details     物理アドレスと仮想アドレスの4KBページをマッピング解除する。
 *
 * @param[in]   *pVirtAddr 仮想アドレス
 * @param[in]   freePhys   物理メモリ領域自動解放
 *                  - MEMMNG_PAGE_FREE_PHYS_TRUE  解放有り
 *                  - MEMMNG_PAGE_FREE_PHYS_FALSE 解放無し
 *
 * @attention   引数pVirtAddrは4KBアライメントされていること。
 */
/******************************************************************************/
static void Unset( void *pVirtAddr,
                   bool freePhys    )
{
    uint32_t        pdeIdx;         /* ページディレクトリエントリインデックス */
    uint32_t        pteIdx;         /* ページテーブルエントリインデックス     */
    IA32PagingDir_t *pPageDir;      /* ページディレクトリ                     */
    IA32PagingTbl_t *pPageTbl;      /* ページテーブル                         */
    IA32PagingTbl_t *pPageTblPhys;  /* ページテーブル(物理アドレス)           */
    IA32PagingPDE_t *pPde;          /* ページディレクトリエントリ             */

    /* 初期化 */
    pdeIdx       = IA32_PAGING_GET_PDE_IDX( pVirtAddr );
    pteIdx       = IA32_PAGING_GET_PTE_IDX( pVirtAddr );
    pPageDir     = NULL;
    pPageTbl     = NULL;
    pPageTblPhys = NULL;
    pPde         = NULL;

    /* カーネル領域判定 */
    if ( pVirtAddr < ( void * ) MEMMAP_VADDR_USER ) {
        /* カーネル領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) MEMMAP_PADDR_IDLE_PD;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );

    } else {
        /* ユーザ領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) MEMMAP_VADDR_KERNEL_PD1;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) MEMMAP_VADDR_KERNEL_PT1;

        /* ページテーブル存在チェック */
        if ( pPde->attr_p == IA32_PAGING_P_NO ) {
            /* 存在しない */

            return;
        }

        /* ページテーブルアドレス取得 */
        pPageTblPhys = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );

        /* ページテーブル操作領域マッピング */
        SetPageTblMap( &( gMapInfo.ch1 ), pPageTblPhys );
    }

    /* 物理メモリ領域自動解放判定 */
    if ( freePhys != false ) {
        /* 解放有り */

        /* 物理メモリ領域解放 */
        MemmngPhysFree(
            IA32_PAGING_GET_BASE( &( pPageTbl->entry[ pteIdx ] ) )
        );

    }

    /* ページテーブルエントリ初期化 */
    MLibUtilSetMemory32( &( pPageTbl->entry[ pteIdx ] ),
                         0,
                         sizeof ( IA32PagingPTE_t )      );

    /* TLBフラッシュ */
    IA32InstructionInvlpg( pVirtAddr );

    /* カーネル領域判定 */
    if ( pVirtAddr >= ( void * ) MEMMAP_VADDR_USER ) {
        /* ユーザ領域 */

        /* ページテーブル解放試行 */
        TryToFreePageTbl( pPde, pPageTblPhys );
    }

    return;
}


/******************************************************************************/
