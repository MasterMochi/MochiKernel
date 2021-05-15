/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngPage.c                                             */
/*                                                                 2021/05/05 */
/* Copyright (C) 2017-2021 Mochi.                                             */
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
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_PAGE, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/** アイドルプロセス用ページディレクトリアドレス */
#define IDLE_PAGE_DIR_ADDR ( 0x05000000 )

/** カーネル領域サイズ */
#define KERNEL_AREA_SIZE    ( 0x40000000 )
/** カーネル領域用ページディレクトリエントリ数 */
#define KERNEL_AREA_PDE_NUM ( ( ( ( KERNEL_AREA_SIZE ) - 1 ) >> 22 ) + 1 )
/** カーネル領域用ページディレクトリエントリサイズ */
#define KERNEL_AREA_PAGE_DIR_SIZE \
    ( KERNEL_AREA_PDE_NUM * sizeof ( IA32PagingPDE_t ) )

/** カーネル領域用ページテーブルアドレス */
#define KERNEL_AREA_PAGE_TBL_ADDR ( 0x05001000 )
/** カーネル領域用ページテーブルサイズ */
#define KERNEL_AREA_PAGE_TBL_SIZE \
    ( KERNEL_AREA_PDE_NUM * sizeof ( IA32PagingTbl_t ) )

/* ページング操作領域 */
#define MAP_PAGE_DIR_ADDR ( 0x3EFFE000 ) /**< ページディレクトリ操作領域 */
#define MAP_PAGE_TBL_ADDR ( 0x3EFFF000 ) /**< ページテーブル操作領域     */

/* ユーザ空間先頭アドレス */
#define USER_AREA_ADDR ( 0x40000000 )

/** 操作領域マッピング情報 */
typedef struct {
    IA32PagingDir_t *pPageDirPhys;  /**< ページディレクトリ(物理アドレス) */
    IA32PagingTbl_t *pPageTblPhys;  /**< ページテーブル(物理アドレス)     */
} mapInfo_t;

/** ページディレクトリ管理情報 */
typedef struct {
    MkPid_t         pid;            /**< ページディレクトリ使用PID      */
    IA32PagingDir_t *pPageDirPhys;  /**< ページディレクトリ物理アドレス */
} mngInfo_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 操作領域マッピング情報 */
static mapInfo_t gMapInfo;

/** ページディレクトリ管理テーブル */
static MLibDynamicArray_t gMngTbl;

/** 現ページディレクトリID */
static MemmngPageDirId_t gNowDirId;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* ページテーブル割当 */
static IA32PagingTbl_t *AllocPageTbl( IA32PagingPDE_t *pPde );
/* ページテーブル解放 */
static void FreePageTbl( IA32PagingTbl_t *pPageTblPhys );
/* アイドルプロセス用管理情報初期化 */
static void InitIdleInfo( void );
/* アイドルプロセス用ページディレクトリ初期化 */
static void InitIdlePageDir( void );
/* アイドルプロセス用ページテーブル初期化 */
static void InitIdlePageTbl( void );
/* 4KBページマッピング設定 */
static CmnRet_t Set( void     *pVirtAddr,
                     void     *pPhysAddr,
                     uint32_t attrGlobal,
                     uint32_t attrUs,
                     uint32_t attrRw      );
/* ページディレクトリ操作領域マッピング設定 */
static void SetPageDirMap( IA32PagingDir_t *pPageDirPhys );
/* ページテーブル操作領域マッピング設定 */
static void SetPageTblMap( IA32PagingTbl_t *pPageTblPhys );
/* ページテーブル解放試行 */
static void TryToFreePageTbl( IA32PagingPDE_t *pPde,
                              IA32PagingTbl_t *pPageTbl );
/* 4KBページマッピング解除 */
static void Unset( void *pVirtAddr );


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
    MLibRet_t         retMLib;          /* MLib戻り値                       */
    MLibErr_t         errMLib;          /* MLibエラー値                     */
    IA32PagingDir_t   *pPageDir;        /* ページディレクトリ               */
    IA32PagingDir_t   *pPageDirPhys;    /* ページディレクトリ(物理アドレス) */
    MemmngPageDirId_t dirId;            /* ページディレクトリID             */

    /* 初期化 */
    pMngInfo     = NULL;
    retMLib      = MLIB_RET_FAILURE;
    errMLib      = MLIB_ERR_NONE;
    pPageDir     = ( IA32PagingDir_t * ) MAP_PAGE_DIR_ADDR;
    pPageDirPhys = NULL;
    dirId        = 0;

    /* 管理情報割当 */
    retMLib = MLibDynamicArrayAlloc( &gMngTbl,
                                     &dirId,
                                     ( void ** ) &pMngInfo,
                                     &errMLib );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
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
        MLibDynamicArrayFree( &gMngTbl, dirId, &errMLib );

        return MEMMNG_PAGE_DIR_ID_NULL;
    }

    /* 管理情報設定 */
    pMngInfo->pid          = pid;
    pMngInfo->pPageDirPhys = pPageDirPhys;

    /* ページディレクトリ操作領域マッピング設定 */
    SetPageDirMap( pPageDirPhys );

    /* ページディレクトリ初期化 */
    MLibUtilSetMemory32( pPageDir, 0, sizeof ( IA32PagingDir_t ) );

    /* カーネル領域マッピング設定 */
    MLibUtilCopyMemory( pPageDir,
                        ( IA32PagingDir_t * ) IDLE_PAGE_DIR_ADDR,
                        KERNEL_AREA_PAGE_DIR_SIZE                 );

    return dirId;
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
    MLibErr_t       errMLib;        /* MLIBライブラリエラー値       */
    MLibRet_t       retMLib;        /* MLIBライブラリ戻り値         */
    IA32PagingDir_t *pPageDir;      /* ページディレクトリ           */
    IA32PagingTbl_t *pPageTblPhys;  /* ページテーブル(物理アドレス) */
    /* 初期化 */
    pMngInfo     = NULL;
    errMLib      = MLIB_ERR_NONE;
    retMLib      = MLIB_RET_FAILURE;
    pPageDir     = ( IA32PagingDir_t * ) MAP_PAGE_DIR_ADDR;
    pPageTblPhys = NULL;

    /* 管理情報取得 */
    retMLib = MLibDynamicArrayGet( &gMngTbl,
                                   dirId,
                                   ( void ** ) &pMngInfo,
                                   &errMLib               );

    /* 管理情報取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* ページディレクトリ操作領域マッピング */
    SetPageDirMap( pMngInfo->pPageDirPhys );

    /* ユーザ領域のページディレクトリエントリ毎に繰り返し */
    for ( idx = KERNEL_AREA_PDE_NUM; idx < IA32_PAGING_PDE_NUM; idx++ ) {
        /* ページテーブル存在判定 */
        if ( pPageDir->entry[ idx ].attr_p == IA32_PAGING_P_YES ) {
            /* 存在する */

            /* ページテーブル物理アドレス取得 */
            pPageTblPhys = ( IA32PagingTbl_t * )
                           IA32_PAGING_GET_BASE( &( pPageDir->entry[ idx ] ) );

            /* ページテーブル操作領域マッピング設定 */
            SetPageTblMap( pPageTblPhys );

            /* ページテーブル解放 */
            FreePageTbl( pPageTblPhys );
        }
    }

    /* ページディレクトリ初期化 */
    MLibUtilSetMemory32( pPageDir, 0, sizeof ( IA32PagingDir_t ) );

    /* ページディレクトリ解放 */
    MemmngPhysFree( pMngInfo->pPageDirPhys );

    /* 管理情報解放 */
    MLibDynamicArrayFree( &gMngTbl, dirId, &errMLib );

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
    MLibDynamicArrayGet( &gMngTbl, dirId, ( void ** ) &pMngInfo, NULL );

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
                        uint32_t          attrGlobal,
                        uint32_t          attrUs,
                        uint32_t          attrRw      )
{
    void      *pNextVirtAddr;   /* 次仮想アドレス         */
    void      *pNextPhysAddr;   /* 次物理アドレス         */
    size_t    remain;           /* 残マッピングサイズ     */
    CmnRet_t  ret;              /* 関数戻り値             */
    mngInfo_t *pMngInfo;        /* 管理情報               */
    MLibErr_t errMLib;          /* MLIBライブラリエラー値 */
    MLibRet_t retMLib;          /* MLIBライブラリ戻り値   */

    /* 初期化 */
    pNextVirtAddr = pVirtAddr;
    pNextPhysAddr = pPhysAddr;
    remain        = size;
    ret           = CMN_SUCCESS;
    pMngInfo      = NULL;
    errMLib       = MLIB_ERR_NONE;
    retMLib       = MLIB_RET_FAILURE;

    /* ページディレクトリID判定 */
    if ( dirId != MEMMNG_PAGE_DIR_ID_IDLE ) {
        /* 非アイドルプロセス用 */

        /* 管理情報取得 */
        retMLib = MLibDynamicArrayGet( &gMngTbl,
                                       dirId,
                                       ( void ** ) &pMngInfo,
                                       &errMLib               );

        /* 管理情報取得結果判定 */
        if ( retMLib != MLIB_RET_SUCCESS ) {
            /* 失敗 */

            DEBUG_LOG( "dirID( %d ) failure.", dirId );

            return CMN_FAILURE;
        }

        /* ページディレクトリ操作領域マッピング */
        SetPageDirMap( pMngInfo->pPageDirPhys );
    }

    /* 4KB毎に繰り返し */
    while ( remain >= IA32_PAGING_PAGE_SIZE ) {
        /* 4KBページマッピング設定 */
        ret = Set( pNextVirtAddr, pNextPhysAddr, attrGlobal, attrUs, attrRw );

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
        MemmngPageUnset( dirId, pVirtAddr, size - remain );
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
                          size_t            size        )
{
    mngInfo_t *pMngInfo;    /* 管理情報               */
    MLibErr_t errMLib;      /* MLIBライブラリエラー値 */
    MLibRet_t retMLib;      /* MLIBライブラリ戻り値   */

    /* 初期化 */
    pMngInfo = NULL;
    errMLib  = MLIB_ERR_NONE;
    retMLib  = MLIB_RET_FAILURE;

    /* ページディレクトリID判定 */
    if ( dirId != MEMMNG_PAGE_DIR_ID_IDLE ) {
        /* 非アイドルプロセス用 */

        /* 管理情報取得 */
        retMLib = MLibDynamicArrayGet( &gMngTbl,
                                       dirId,
                                       ( void ** ) &pMngInfo,
                                       &errMLib               );

        /* 取得結果判定 */
        if ( retMLib != MLIB_RET_SUCCESS ) {
            /* 失敗 */

            return CMN_FAILURE;
        }

        /* ページディレクトリ操作領域マッピング */
        SetPageDirMap( pMngInfo->pPageDirPhys );
    }

    /* 4KB毎に繰り返し */
    while ( size >= IA32_PAGING_PAGE_SIZE ) {
        /* 4KBページマッピング解除 */
        Unset( pVirtAddr );

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

    /* 操作領域マッピング情報初期化 */
    MLibUtilSetMemory32( &gMapInfo, 0, sizeof ( mapInfo_t ) );

    /* アイドルプロセス用ページディレクトリ初期化 */
    InitIdlePageDir();

    /* アイドルプロセス用ページテーブル初期化 */
    InitIdlePageTbl();

    /* 現ページディレクトリID設定 */
    gNowDirId = 0;

    /* ページディレクトリベースレジスタ値作成 */
    IA32_PAGING_SET_PDBR( pdbr,
                          IDLE_PAGE_DIR_ADDR,
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
 * @brief       ページテーブル割当
 * @details     ページテーブルを割り当てる。
 *
 * @param[in]   *pPde ページディレクトリエントリ
 *
 * @return      割り当てたページテーブルの物理アドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 */
/******************************************************************************/
static IA32PagingTbl_t *AllocPageTbl( IA32PagingPDE_t *pPde )
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
    IA32_PAGING_SET_BASE( pPde, pPageTblPhys );
    pPde->attr_avl = 0;
    pPde->attr_g   = IA32_PAGING_G_NO;
    pPde->attr_ps  = IA32_PAGING_PS_4K;
    pPde->attr_a   = IA32_PAGING_A_NO;
    pPde->attr_pcd = IA32_PAGING_PCD_DISABLE;
    pPde->attr_pwt = IA32_PAGING_PWT_WT;
    pPde->attr_us  = IA32_PAGING_US_USER;
    pPde->attr_rw  = IA32_PAGING_RW_RW;
    pPde->attr_p   = IA32_PAGING_P_YES;

    /* ページテーブル操作領域マッピング */
    SetPageTblMap( pPageTblPhys );

    /* ページテーブル初期化 */
    MLibUtilSetMemory32( ( IA32PagingTbl_t * ) MAP_PAGE_TBL_ADDR,
                         0,
                         sizeof ( IA32PagingTbl_t )               );

    return pPageTblPhys;
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
    MLibUtilSetMemory32( ( IA32PagingTbl_t * ) MAP_PAGE_TBL_ADDR,
                         0,
                         sizeof ( IA32PagingTbl_t )               );

    /* ページテーブル解放 */
    MemmngPhysFree( pPageTblPhys );

    return;
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
    pMngInfo->pPageDirPhys = ( IA32PagingDir_t * ) IDLE_PAGE_DIR_ADDR;

    return;
}


/******************************************************************************/
/**
 * @breif       アイドルプロセス用ページディレクトリ初期化
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
    pPageDir = ( IA32PagingDir_t * ) IDLE_PAGE_DIR_ADDR;
    pPageTbl = ( IA32PagingTbl_t * ) KERNEL_AREA_PAGE_TBL_ADDR;
    pPde     = NULL;

    /* アイドルプロセス用ページディレクトリ0初期化 */
    MLibUtilSetMemory32( pPageDir, 0, sizeof ( IA32PagingDir_t ) );

    /* カーネル領域に対応するエントリ毎に繰り返し */
    for ( idx = 0; idx < KERNEL_AREA_PDE_NUM; idx++ ) {

        /* アドレス設定 */
        pPde = &( pPageDir->entry[ idx ] );

        /* ページディレクトリエントリ設定 */
        IA32_PAGING_SET_BASE( pPde, &( pPageTbl[ idx ] ) );
        pPde->attr_avl = 0;
        pPde->attr_g   = IA32_PAGING_G_NO;
        pPde->attr_ps  = IA32_PAGING_PS_4K;
        pPde->attr_a   = IA32_PAGING_A_NO;
        pPde->attr_pcd = IA32_PAGING_PCD_DISABLE;
        pPde->attr_pwt = IA32_PAGING_PWT_WT;
        pPde->attr_us  = IA32_PAGING_US_SV;
        pPde->attr_rw  = IA32_PAGING_RW_RW;
        pPde->attr_p   = IA32_PAGING_P_YES;
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
    pPageTbl   = ( IA32PagingTbl_t * ) KERNEL_AREA_PAGE_TBL_ADDR;
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
            IA32_PAGING_SET_BASE( pPte, pPhysAddr );
            pPte->attr_avl = 0;
            pPte->attr_g   = IA32_PAGING_G_YES;
            pPte->attr_pat = IA32_PAGING_PAT_UNUSED;
            pPte->attr_d   = IA32_PAGING_D_NO;
            pPte->attr_a   = IA32_PAGING_A_NO;
            pPte->attr_pcd = IA32_PAGING_PCD_DISABLE;
            pPte->attr_pwt = IA32_PAGING_PWT_WT;
            pPte->attr_us  = IA32_PAGING_US_SV;
            pPte->attr_rw  = IA32_PAGING_RW_RW;
            pPte->attr_p   = IA32_PAGING_P_YES;
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

    /* カーネル領域判定 */
    if ( pVirtAddr < ( void * ) USER_AREA_ADDR ) {
        /* カーネル領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) IDLE_PAGE_DIR_ADDR;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );

    } else {
        /* ユーザ領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) MAP_PAGE_DIR_ADDR;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) MAP_PAGE_TBL_ADDR;


        /* ページテーブル存在チェック */
        if ( pPde->attr_p == IA32_PAGING_P_NO ) {
            /* 存在しない */

            /* ページテーブル割当 */
            pPageTblPhys = AllocPageTbl( pPde );

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
            SetPageTblMap( pPageTblPhys );
        }
    }

    /* ページテーブルエントリアドレス取得 */
    pPte = &( pPageTbl->entry[ pteIdx ] );

    /* ページテーブルエントリ初期化 */
    MLibUtilSetMemory32( pPte, 0, sizeof ( IA32PagingPTE_t ) );

    /* ページテーブルエントリ設定 */
    IA32_PAGING_SET_BASE( pPte, pPhysAddr );
    pPte->attr_avl = 0;
    pPte->attr_g   = attrGlobal;
    pPte->attr_pat = IA32_PAGING_PAT_UNUSED;
    pPte->attr_d   = IA32_PAGING_D_NO;
    pPte->attr_a   = IA32_PAGING_A_NO;
    pPte->attr_pcd = IA32_PAGING_PCD_ENABLE;
    pPte->attr_pwt = IA32_PAGING_PWT_WT;
    pPte->attr_us  = attrUs;
    pPte->attr_rw  = attrRw;
    pPte->attr_p   = IA32_PAGING_P_YES;

    /* TLBフラッシュ */
    IA32InstructionInvlpg( pVirtAddr );

    return CMN_SUCCESS;
}


/******************************************************************************/
/*
 * @brief       ページディレクトリ操作領域マッピング設定
 * @details     ページディレクトリを操作領域にマッピング設定する。
 *
 * @param[in]   *pPageDirPhys ページディレクトリ(物理アドレス)
 */
/******************************************************************************/
static void SetPageDirMap( IA32PagingDir_t *pPageDirPhys )
{
    /* マッピング中アドレス比較 */
    if ( gMapInfo.pPageDirPhys != pPageDirPhys ) {
        /* 不一致 */

        /* マッピング中アドレス設定 */
        gMapInfo.pPageDirPhys = pPageDirPhys;

        /* マッピング */
        MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                       ( IA32PagingDir_t * ) MAP_PAGE_DIR_ADDR,
                       pPageDirPhys,
                       sizeof ( IA32PagingDir_t ),
                       IA32_PAGING_G_NO,
                       IA32_PAGING_US_SV,
                       IA32_PAGING_RW_RW                        );
    }

    return;
}


/******************************************************************************/
/*
 * @brief       ページテーブル操作領域マッピング設定
 * @details     ページテーブルを操作領域にマッピング設定する。
 *
 * @param[in]   *pPageTblPhys ページテーブル(物理アドレス)
 */
/******************************************************************************/
static void SetPageTblMap( IA32PagingTbl_t *pPageTblPhys )
{
    /* マッピング中アドレス比較 */
    if ( gMapInfo.pPageTblPhys != pPageTblPhys ) {
        /* 不一致 */

        /* マッピング中アドレス設定 */
        gMapInfo.pPageTblPhys = pPageTblPhys;

        /* マッピング */
        MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                       ( IA32PagingTbl_t * ) MAP_PAGE_TBL_ADDR,
                       pPageTblPhys,
                       sizeof ( IA32PagingTbl_t ),
                       IA32_PAGING_G_NO,
                       IA32_PAGING_US_SV,
                       IA32_PAGING_RW_RW                        );
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
    pPageTbl = ( IA32PagingTbl_t * ) MAP_PAGE_TBL_ADDR;

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
 *
 * @attention   引数pVirtAddrは4KBアライメントされていること。
 */
/******************************************************************************/
static void Unset( void *pVirtAddr )
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
    if ( pVirtAddr < ( void * ) USER_AREA_ADDR ) {
        /* カーネル領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) IDLE_PAGE_DIR_ADDR;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );

    } else {
        /* ユーザ領域 */

        /* アドレス設定 */
        pPageDir = ( IA32PagingDir_t * ) MAP_PAGE_DIR_ADDR;
        pPde     = &( pPageDir->entry[ pdeIdx ] );
        pPageTbl = ( IA32PagingTbl_t * ) MAP_PAGE_TBL_ADDR;

        /* ページテーブル存在チェック */
        if ( pPde->attr_p == IA32_PAGING_P_NO ) {
            /* 存在しない */

            return;
        }

        /* ページテーブルアドレス取得 */
        pPageTblPhys = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );

        /* ページテーブル操作領域マッピング */
        SetPageTblMap( pPageTblPhys );
    }

    /* ページテーブルエントリ初期化 */
    MLibUtilSetMemory32( &( pPageTbl->entry[ pteIdx ] ),
                         0,
                         sizeof ( IA32PagingPTE_t )      );

    /* TLBフラッシュ */
    IA32InstructionInvlpg( pVirtAddr );

    /* カーネル領域判定 */
    if ( pVirtAddr >= ( void * ) USER_AREA_ADDR ) {
        /* ユーザ領域 */

        /* ページテーブル解放試行 */
        TryToFreePageTbl( pPde, pPageTblPhys );
    }

    return;
}


/******************************************************************************/
