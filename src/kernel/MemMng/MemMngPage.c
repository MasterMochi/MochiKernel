/******************************************************************************/
/*                                                                            */
/* src/kernel/MemMng/MemMngPage.c                                             */
/*                                                                 2020/05/22 */
/* Copyright (C) 2017-2020 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <string.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32.h>
#include <hardware/IA32/IA32Instruction.h>
#include <hardware/IA32/IA32Paging.h>
#include <hardware/Vga/Vga.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>


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

/** ページ管理テーブル構造体 */
typedef struct {
    uint32_t nowDirId;                      /**< 現ページディレクトリID       */
    uint8_t  usedDir[ MEMMNG_PAGE_DIR_NUM ];/**< ページディレクトリ使用フラグ */
    uint8_t  usedTbl[ MEMMNG_PAGE_TBL_NUM ];/**< ページテーブル使用フラグ     */
} PageTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** ページ管理テーブル */
static PageTbl_t gPageMngTbl;

/** ページディレクトリ管理テーブル */
static IA32PagingDir_t *pgPageDir;

/** ページディレクトリ管理テーブルサイズ */
static size_t gPageDirSize;

/** ページテーブル管理テーブル */
static IA32PagingTbl_t *pgPageTbl;

/** ページテーブル管理テーブルサイズ */
static size_t gPageTblSize;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* ページテーブル割当 */
static uint32_t PageAllocTbl( void );

/* ページテーブル解放 */
static CmnRet_t PageFreeTbl( uint32_t id );

/* 4KiBページマッピング */
static CmnRet_t PageSet( uint32_t dirId,
                         void     *pVAddr,
                         void     *pPAddr,
                         uint32_t attrGlobal,
                         uint32_t attrUs,
                         uint32_t attrRw      );

/* 全ページディレクトリ設定（カーネル領域） */
static void SetAllKernelPageDir( void );

/* ページマッピングデフォルト設定 */
static CmnRet_t PageSetDefault( uint32_t dirId );

/* ページディレクトリ設定（カーネル領域） */
static void SetKernelPageDir( uint32_t dirId );

/* 4KiBページマッピング解除 */
static void PageUnset( uint32_t dirId,
                       void     *pVAddr );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ページディレクトリ割当
 * @details     未使用のページディレクトリを割り当てる。
 *
 * @return      割り当てたページディレクトリIDを返す。
 * @retval      MEMMNG_PAGE_DIR_FULL以外 正常終了
 * @retval      MEMMNG_PAGE_DIR_FULL     異常終了
 */
/******************************************************************************/
uint32_t MemMngPageAllocDir( void )
{
    uint32_t id;    /* ページディレクトリID */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 1ページディレクトリ毎に繰り返し */
    for ( id = MEMMNG_PAGE_DIR_ID_MIN; id < MEMMNG_PAGE_DIR_NUM; id++ ) {
        /* 使用中判定 */
        if ( gPageMngTbl.usedDir[ id ] == CMN_UNUSED ) {
            /* 未使用 */

            /* ページディレクトリ初期化 */
            memset( &pgPageDir[ id ], 0, sizeof ( IA32PagingDir_t ) );

            /* カーネル領域設定 */
            SetKernelPageDir( id );

            /* ページディレクトリ使用中設定 */
            gPageMngTbl.usedDir[ id ] = CMN_USED;

            break;
        }
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%u", __func__, id );

    return id;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ解放
 * @details     使用済みページディレクトリを解放する。
 *
 * @param[in]   id ページディレクトリID
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngPageFreeDir( uint32_t id )
{
    uint32_t idx;   /* インデックス */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. id=%u", __func__, id );

    /* 識別子チェック */
    if ( id >= MEMMNG_PAGE_DIR_NUM ) {
        /* 範囲異常 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* 使用中判定 */
    if ( gPageMngTbl.usedDir[ id ] == CMN_UNUSED ) {
        /* 未使用 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* 1ページディレクトリエントリ毎に繰り返し */
    for ( idx = 0; idx < IA32_PAGING_PDE_NUM; idx++ ) {
        /* 存在フラグ判定 */
        if ( pgPageDir[ id ].entry[ idx ].attr_p == IA32_PAGING_P_YES ) {
            /* 存在 */

            /* [TODO]ページテーブル解放 */
        }
    }

    /* ページディレクトリ未使用設定 */
    gPageMngTbl.usedDir[ id ] = CMN_UNUSED;

    /* ページテーブル初期化 */
    memset( &pgPageDir[ id ], 0, sizeof ( IA32PagingDir_t ) );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );

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
uint32_t MemMngPageGetDirId( void )
{
    return gPageMngTbl.nowDirId;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ切替
 * @details     指定したページディレクトリに切り替える。
 *
 * @param[in]   pageDirId ページディレクトリID
 *
 * @return      ページディレクトリベースレジスタを返す。
 */
/******************************************************************************/
IA32PagingPDBR_t MemMngPageSwitchDir( uint32_t pageDirId )
{
    IA32PagingPDBR_t pdbr;  /* 戻り値 */

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start. pageDirId=%u", __func__, pageDirId );*/

    /* 現ページディレクトリID設定 */
    gPageMngTbl.nowDirId = pageDirId;

    /* PDBR作成 */
    IA32_PAGING_SET_PDBR( pdbr,
                          &pgPageDir[ pageDirId ],
                          IA32_PAGING_PCD_ENABLE,
                          IA32_PAGING_PWT_WB );

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/

    return pdbr;
}


/******************************************************************************/
/**
 * @brief       ページ管理初期化
 * @details     ページ管理を初期化しページングを有効化する。
 */
/******************************************************************************/
void MemMngPageInit( void )
{
    CmnRet_t         ret;   /* 戻り値 */
    IA32PagingPDBR_t pdbr;  /* PDBR   */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 初期化 */
    ret = CMN_FAILURE;

    /* ページディレクトリ管理テーブルサイズ設定 */
    gPageDirSize = MEMMNG_PAGE_DIR_NUM * sizeof ( IA32PagingDir_t );
    gPageDirSize = MLIB_ALIGN( gPageDirSize, IA32_PAGING_PAGE_SIZE );

    /* ページディレクトリ管理テーブル割当て */
    pgPageDir = ( IA32PagingDir_t * ) MemMngPhysAlloc( gPageDirSize );

    /* 割当て結果判定 */
    if ( pgPageDir == NULL ) {
        /* 失敗 */

        /* [TODO] */
    }

    /* ページテーブル管理テーブルサイズ設定 */
    gPageTblSize = MEMMNG_PAGE_TBL_NUM * sizeof ( IA32PagingTbl_t );
    gPageTblSize = MLIB_ALIGN( gPageTblSize, IA32_PAGING_PAGE_SIZE );

    /* ページテーブル管理テーブル割当て */
    pgPageTbl = ( IA32PagingTbl_t * ) MemMngPhysAlloc( gPageTblSize );

    /* 割当て結果判定 */
    if ( pgPageTbl == NULL ) {
        /* 失敗 */

        /* [TODO] */
    }

    DEBUG_LOG( "pgPageDir=%p, pgPageTbl=%p", pgPageDir, pgPageTbl );

    /* テーブル初期化 */
    memset( &gPageMngTbl, 0, sizeof ( gPageMngTbl ) );
    memset( pgPageDir,    0, gPageDirSize           );
    memset( pgPageTbl,    0, gPageTblSize           );

    /* アイドルプロセス用ページディレクトリ設定 */
    gPageMngTbl.usedDir[ MEMMNG_PAGE_DIR_ID_IDLE ] = CMN_USED;

    /* ページマッピングデフォルト設定 */
    ret = PageSetDefault( MEMMNG_PAGE_DIR_ID_IDLE );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* [TODO] */
    }

    /* ページディレクトリ切替 */
    pdbr = MemMngPageSwitchDir( MEMMNG_PAGE_DIR_ID_IDLE );

    /* ページディレクトリ設定 */
    IA32InstructionSetCr3( pdbr );

    /* ページング有効化 */
    IA32InstructionSetCr0( IA32_CR0_PG, IA32_CR0_PG );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       ページマッピング設定
 * @details     物理メモリをページにマッピングする。
 *
 * @param[in]   dirId      ページディレクトリID
 * @param[in]   *pVAddr    仮想アドレス
 * @param[in]   *pPAddr    物理アドレス
 * @param[in]   size       サイズ
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
 * @attention   引数pVAddr、pPAddr、sizeは4KiBアライメントであること。
 */
/******************************************************************************/
CmnRet_t MemMngPageSet( uint32_t dirId,
                        void     *pVAddr,
                        void     *pPAddr,
                        size_t   size,
                        uint32_t attrGlobal,
                        uint32_t attrUs,
                        uint32_t attrRw      )
{
    CmnRet_t ret;   /* 関数戻り値 */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    DEBUG_LOG( " dirId=%u, pVAddr=%010p, pPAddr=%010p,",
               dirId,
               pVAddr,
               pPAddr );
    DEBUG_LOG( " size=%#x, attrGlobal=%u, attrUs=%u, attrRw=%u",
               size,
               attrGlobal,
               attrUs,
               attrRw );

    /* 初期化 */
    ret = CMN_FAILURE;

    /* 使用中判定 */
    if ( gPageMngTbl.usedDir[ dirId ] == CMN_UNUSED ) {
        /* 未使用 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* 4KiB毎に繰り返し */
    while ( size >= IA32_PAGING_PAGE_SIZE ) {
        /* 4KiBページマッピング設定 */
        ret = PageSet( dirId, pVAddr, pPAddr, attrGlobal, attrUs, attrRw );

        /* ページマッピング結果判定 */
        if ( ret == CMN_FAILURE ) {
            /* 失敗 */

            break;
        }

        /* アドレス更新 */
        pVAddr += IA32_PAGING_PAGE_SIZE;
        pPAddr += IA32_PAGING_PAGE_SIZE;

        /* サイズ更新 */
        size -= IA32_PAGING_PAGE_SIZE;
    }

    /* ページディレクトリ判定 */
    if ( dirId == MEMMNG_PAGE_DIR_ID_IDLE ) {
        /* アイドルプロセス */

        /* 全ページディレクトリ設定 */
        SetAllKernelPageDir();
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, ret );

    return ret;
}


/******************************************************************************/
/**
 * @brief       ページマッピング設定
 * @details     物理メモリとページのマッピングを解除する。
 *
 * @param[in]   dirId  ページディレクトリID
 * @param[in]   pVAddr 仮想アドレス
 * @param[in]   size   サイズ
 *
 * @attention   引数pVAddr、sizeは4KiBアライメントであること。
 */
/******************************************************************************/
void MemMngPageUnset( uint32_t dirId,
                      void     *pVAddr,
                      size_t   size     )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    DEBUG_LOG( " dirId=%u, pVAddr=%010p, size=%#X",
               dirId,
               pVAddr,
               size );

    /* 使用中判定 */
    if ( gPageMngTbl.usedDir[ dirId ] == CMN_UNUSED ) {
        /* 未使用 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end.", __func__ );

        return;
    }

    /* 4KiB毎に繰り返し */
    while ( size >= IA32_PAGING_PAGE_SIZE ) {
        /* 4KiBページマッピング */
        PageUnset( dirId, pVAddr );

        /* アドレス更新 */
        pVAddr += IA32_PAGING_PAGE_SIZE;

        /* サイズ更新 */
        size -= IA32_PAGING_PAGE_SIZE;
    }

    /* ページディレクトリ判定 */
    if ( dirId == MEMMNG_PAGE_DIR_ID_IDLE ) {
        /* アイドルプロセス */

        /* 全ページディレクトリ設定 */
        SetAllKernelPageDir();
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ページテーブル割当
 * @details     未使用のページテーブルを割り当てる。
 *
 * @return      ページテーブルIDを返す。
 * @retval      MEMMNG_PAGE_TBL_FULL以外 正常終了
 * @retval      MEMMNG_PAGE_TBL_FULL     異常終了
 */
/******************************************************************************/
static uint32_t PageAllocTbl( void )
{
    uint32_t id;    /* ページテーブルID */

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start.", __func__ );*/

    /* 1ページテーブル毎に繰り返し */
    for ( id = 0; id < MEMMNG_PAGE_TBL_NUM; id++ ) {
        /* 使用中判定 */
        if ( gPageMngTbl.usedTbl[ id ] == CMN_UNUSED ) {
            /* 未使用 */

            /* ページテーブル初期化 */
            memset( &pgPageTbl[ id ], 0, sizeof ( IA32PagingTbl_t ) );

            /* ページテーブル使用中設定 */
            gPageMngTbl.usedTbl[ id ] = CMN_USED;

            break;
        }
    }

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end. ret=%u", __func__, id );*/

    return id;
}


/******************************************************************************/
/**
 * @brief       ページテーブル解放
 * @details     指定したページテーブルを解放する。
 *
 * @param[in]   id ページテーブルID
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t PageFreeTbl( uint32_t id )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. id=%u", __func__, id );

    /* 識別子チェック */
    if ( id >= MEMMNG_PAGE_TBL_NUM ) {
        /* 範囲異常 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* 使用中判定 */
    if ( gPageMngTbl.usedTbl[ id ] == CMN_UNUSED ) {
        /* 未使用 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* ページテーブル未使用設定 */
    gPageMngTbl.usedTbl[ id ] = CMN_UNUSED;

    /* ページテーブル初期化 */
    memset( &pgPageTbl[ id ], 0, sizeof ( IA32PagingTbl_t ) );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       4KiBページマッピング設定
 * @details     4KiBの物理メモリをページにマッピングする。
 *
 * @param[in]   dirId      ページディレクトリID
 * @param[in]   *pVAddr    仮想アドレス
 * @param[in]   *pPAddr    物理アドレス
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
 * @attention   引数pVAddr、pPAddr、sizeは4KiBアライメントであること。
 */
/******************************************************************************/
static CmnRet_t PageSet( uint32_t dirId,
                         void     *pVAddr,
                         void     *pPAddr,
                         uint32_t attrGlobal,
                         uint32_t attrUs,
                         uint32_t attrRw      )
{
    uint32_t        pdeIdx; /* PDEインデックス      */
    uint32_t        pteIdx; /* PTEインデックス      */
    uint32_t        tblId;  /* ページテーブル識別子 */
    IA32PagingPDE_t *pPde;  /* PDE参照              */
    IA32PagingPTE_t *pPte;  /* PTE参照              */

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start.", __func__ );
    DEBUG_LOG( " dirId=%u, pVAddr=%010p, pPAddr=%010p,",
               dirId,
               pVAddr,
               pPAddr );
    DEBUG_LOG( " attrGlobal=%u, attrUs=%u, attrRw=%u",
               attrGlobal,
               attrUs,
               attrRw );*/

    /* 初期化 */
    pdeIdx = IA32_PAGING_GET_PDE_IDX( pVAddr );         /* PDEインデックス */
    pteIdx = IA32_PAGING_GET_PTE_IDX( pVAddr );         /* PTEインデックス */
    pPde   = &( pgPageDir[ dirId ].entry[ pdeIdx ] );   /* PDE参照         */

    /* ページテーブル存在チェック */
    if ( pPde->attr_p == IA32_PAGING_P_NO ) {
        /* 存在しない */

        /* ページテーブル割当 */
        tblId = PageAllocTbl();

        /* 割当結果判定 */
        if ( tblId == MEMMNG_PAGE_TBL_FULL ) {
            /* 失敗 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

            return CMN_FAILURE;
        }

        /* PDE設定 */
        memset( pPde, 0, sizeof ( IA32PagingPDE_t ) );
        IA32_PAGING_SET_BASE( pPde, &pgPageTbl[ tblId ] );
        pPde->attr_g   = IA32_PAGING_G_NO;
        pPde->attr_ps  = IA32_PAGING_PS_4K;
        pPde->attr_a   = IA32_PAGING_A_NO;
        pPde->attr_pcd = IA32_PAGING_PCD_ENABLE;
        pPde->attr_pwt = IA32_PAGING_PWT_WB;
        pPde->attr_us  = IA32_PAGING_US_USER;
        pPde->attr_rw  = IA32_PAGING_RW_RW;
        pPde->attr_p   = IA32_PAGING_P_YES;

        /* PTE参照変数設定 */
        pPte = &( pgPageTbl[ tblId ].entry[ pteIdx ] );

    } else {
        /* 存在する */

        /* TLBフラッシュ */
        IA32InstructionInvlpg( pVAddr );

        /* PTE参照変数設定 */
        pPte = &( ( ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde ) )->
                  entry[ pteIdx ] );
    }

    /* PTE設定 */
    memset( pPte, 0, sizeof ( IA32PagingPTE_t ) );
    IA32_PAGING_SET_BASE( pPte, pPAddr );
    pPte->attr_g   = attrGlobal;
    pPte->attr_a   = IA32_PAGING_A_NO;
    pPte->attr_pcd = IA32_PAGING_PCD_ENABLE;
    pPte->attr_pwt = IA32_PAGING_PWT_WB;
    pPte->attr_us  = attrUs;
    pPte->attr_rw  = attrRw;
    pPte->attr_p   = IA32_PAGING_P_YES;

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );*/

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       全ページディレクトリ設定（カーネル領域）
 * @details     全ページディレクトリにアイドルプロセス用ページディレクトリの一
 *              部をコピーすることで、カーネルが使用するメモリ空間を設定する。
 */
/******************************************************************************/
static void SetAllKernelPageDir( void )
{
    uint32_t dirId;

    /* 全ページディレクトリ毎に繰り返す */
    for ( dirId = MEMMNG_PAGE_DIR_ID_MIN;
          dirId < MEMMNG_PAGE_DIR_NUM;
          dirId++                         ) {
        /* 使用中判定 */
        if ( gPageMngTbl.usedDir[ dirId ] == CMN_UNUSED ) {
            /* 未使用 */

            continue;
        }

        /* カーネル領域ページディレクトリ設定 */
        SetKernelPageDir( dirId );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ページマッピングデフォルト設定
 * @details     カーネル領域をページにマッピングする。
 *
 * @param[in]   dirId ページディレクトリID
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t PageSetDefault( uint32_t dirId )
{
    CmnRet_t        ret;    /* 戻り値               */
    MkMemMapEntry_t info;   /* メモリマップ領域情報 */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. dirId=%u", __func__, dirId );

    /* 初期化 */
    ret = CMN_FAILURE;
    memset( &info, 0, sizeof ( MkMemMapEntry_t ) );

    /* ページテーブル存在チェック */
    if ( gPageMngTbl.usedDir[ dirId ] == CMN_UNUSED ) {
        /* 存在しない */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

#ifdef DEBUG_LOG_ENABLE
    /* デバッグ用VRAM領域マッピング */
    ret = MemMngPageSet(
              dirId,                            /* ページディレクトリID    */
              ( void * ) VGA_M3_VRAM_ADDR,      /* 仮想アドレス            */
              ( void * ) VGA_M3_VRAM_ADDR,      /* 物理アドレス            */
              VGA_M3_VRAM_SIZE,                 /* サイズ                  */
              IA32_PAGING_G_YES,                /* グローバルページ属性    */
              IA32_PAGING_US_USER,              /* ユーザ/スーパバイザ属性 */
              IA32_PAGING_RW_RW            );   /* 読込/書込許可属性       */

    /* マッピング結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }
#endif

    /* カーネル領域情報取得 */
    ret = MemMngMapGetInfo( &info, MK_MEM_TYPE_KERNEL );

    /* 取得結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* カーネル領域マッピング */
    ret = MemMngPageSet( dirId,                 /* ページディレクトリID    */
                         info.pAddr,            /* 仮想アドレス            */
                         info.pAddr,            /* 物理アドレス            */
                         info.size,             /* サイズ                  */
                         IA32_PAGING_G_YES,     /* グローバルページ属性    */
                         IA32_PAGING_US_SV,     /* ユーザ/スーパバイザ属性 */
                         IA32_PAGING_RW_RW  );  /* 読込/書込許可属性       */

    /* マッピング結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* ページディレクトリ管理テーブルマッピング */
    ret = MemMngPageSet( dirId,                 /* ページディレクトリID    */
                         pgPageDir,             /* 仮想アドレス            */
                         pgPageDir,             /* 物理アドレス            */
                         gPageDirSize,          /* サイズ                  */
                         IA32_PAGING_G_NO,      /* グローバルページ属性    */
                         IA32_PAGING_US_SV,     /* ユーザ/スーパバイザ属性 */
                         IA32_PAGING_RW_RW  );  /* 読込/書込許可属性       */

    /* マッピング結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* ページテーブル管理テーブルマッピング */
    ret = MemMngPageSet( dirId,                 /* ページディレクトリID    */
                         pgPageTbl,             /* 仮想アドレス            */
                         pgPageTbl,             /* 物理アドレス            */
                         gPageTblSize,          /* サイズ                  */
                         IA32_PAGING_G_NO,      /* グローバルページ属性    */
                         IA32_PAGING_US_SV,     /* ユーザ/スーパバイザ属性 */
                         IA32_PAGING_RW_RW  );  /* 読込/書込許可属性       */

    /* マッピング結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* プロセスイメージ領域情報取得 */
    ret = MemMngMapGetInfo( &info, MK_MEM_TYPE_PROCIMG );

    /* 取得結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* プロセスイメージ領域マッピング */
    ret = MemMngPageSet( dirId,                 /* ページディレクトリID    */
                         info.pAddr,            /* 仮想アドレス            */
                         info.pAddr,            /* 物理アドレス            */
                         info.size,             /* サイズ                  */
                         IA32_PAGING_G_YES,     /* グローバルページ属性    */
                         IA32_PAGING_US_SV,     /* ユーザ/スーパバイザ属性 */
                         IA32_PAGING_RW_RW  );  /* 読込/書込許可属性       */

    /* マッピング結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリ設定（カーネル領域）
 * @details     引数dirIdのページディレクトリにアイドルプロセス用ページディレク
 *              トリの一部をコピーすることで、カーネルが使用するメモリ空間を設
 *              定する。
 *
 * @param[in]   dirId ページディレクトリID
 */
/******************************************************************************/
static void SetKernelPageDir( uint32_t dirId )
{
    memcpy( &pgPageDir[ dirId                   ],
            &pgPageDir[ MEMMNG_PAGE_DIR_ID_IDLE ],
            IA32_PAGING_GET_PDE_IDX( MK_CONFIG_SIZE_USER ) *
                sizeof( IA32PagingPDE_t )                    );

    return;
}


/******************************************************************************/
/**
 * @brief       4KiBページマッピング解除
 * @details     4KiB物理メモリのページマッピングを解除する。
 *
 * @param[in]   dirId   ページディレクトリID
 * @param[in]   *pVAddr 仮想アドレス
 *
 * @attention   引数pVAddrは4KiBアライメントであること。
 */
/******************************************************************************/
static void PageUnset( uint32_t dirId,
                       void     *pVAddr )
{
    uint32_t        id;         /* ページテーブルID   */
    uint32_t        pdeIdx;     /* PDEインデックス    */
    uint32_t        pteIdx;     /* PTEインデックス    */
    IA32PagingPDE_t *pPde;      /* PDE参照            */
    IA32PagingPTE_t *pPte;      /* PTE参照            */
    IA32PagingTbl_t *pPageTbl;  /* ページテーブル参照 */

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start. dirId=%u, pVAddr=%010p",
               __func__,
               dirId,
               pVAddr );*/

    /* 初期化 */
    pdeIdx   = IA32_PAGING_GET_PDE_IDX( pVAddr );
    pteIdx   = IA32_PAGING_GET_PTE_IDX( pVAddr );
    pPde     = &( pgPageDir[ dirId ].entry[ pdeIdx ] );
    pPageTbl = ( IA32PagingTbl_t * ) IA32_PAGING_GET_BASE( pPde );
    pPte     = &( pPageTbl->entry[ pteIdx ] );

    /* ページテーブル存在チェック */
    if ( pPde->attr_p == IA32_PAGING_P_NO ) {
        /* 存在しない */

        /* デバッグトレースログ出力 *//*
        DEBUG_LOG( "%s() end.", __func__ );*/

        return;
    }

    /* ページ存在チェック */
    if ( pPte->attr_p == IA32_PAGING_P_NO ) {
        /* 存在しない */

        /* デバッグトレースログ出力 *//*
        DEBUG_LOG( "%s() end.", __func__ );*/

        return;
    }

    /* TLBフラッシュ */
    IA32InstructionInvlpg( pVAddr );

    /* PTE初期化 */
    memset( pPte, 0, sizeof ( IA32PagingPTE_t ) );

    /* ページテーブル内全エントリ繰り返し */
    for ( pteIdx = 0; pteIdx < IA32_PAGING_PTE_NUM; pteIdx++ ) {
        /* PTE参照変数設定 */
        pPte = &( pPageTbl->entry[ pteIdx ] );

        /* 使用中判定 */
        if ( pPte->attr_p == IA32_PAGING_P_YES ) {
            /* 使用中 */

            /* デバッグトレースログ出力 *//*
            DEBUG_LOG( "%s() end.", __func__ );*/

            return;
        }
    }

    /* 1ページテーブル毎に繰り返し */
    for ( id = 0; id < MEMMNG_PAGE_TBL_NUM; id++ ) {
        /* ページテーブルアドレス比較 */
        if ( &pgPageTbl[ id ] == pPageTbl ) {
            /* 一致 */

            /* ページテーブル解放 */
            PageFreeTbl( id );

            break;
        }
    }

    /* PDE初期化 */
    memset( pPde, 0, sizeof ( IA32PagingPDE_t ) );

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/

    return;
}


/******************************************************************************/
