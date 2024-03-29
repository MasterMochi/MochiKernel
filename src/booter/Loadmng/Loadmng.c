/******************************************************************************/
/*                                                                            */
/* src/booter/Loadmng/LoadmngInit.c                                           */
/*                                                                 2021/11/27 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Driver.h>

/* 内部モジュールヘッダ */
#include "Loadmng.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_LOADMNG_MAIN,    \
                    __LINE__,                   \
                    __VA_ARGS__              )
#else
#define DEBUG_LOG( ... )
#endif

/** MBR */
typedef struct {
    uint8_t code[ 446 ];        /**< ブートストラップコード */
    pt_t    partitionTbl[ 4 ];  /**< パーティションテーブル */
    uint8_t signature[ 2 ];     /**< ブートシグネチャ       */
} __attribute__( ( packed ) ) mbr_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** パーティションテーブル */
pt_t gLoadmngPt[ 4 ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ロード管理初期化
 * @details     ロード管理モジュールを初期化する。
 */
/******************************************************************************/
void LoadmngInit( void )
{
    mbr_t mbr;  /* MBR */

    /* トレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* MBR読込み */
    DriverAtaRead( &mbr, 0, 1 );

    /* パーティションテーブル取得 */
    gLoadmngPt[ 0 ] = mbr.partitionTbl[ 0 ];
    gLoadmngPt[ 1 ] = mbr.partitionTbl[ 1 ];
    gLoadmngPt[ 2 ] = mbr.partitionTbl[ 2 ];
    gLoadmngPt[ 3 ] = mbr.partitionTbl[ 3 ];

    /* トレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
