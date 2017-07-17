/******************************************************************************/
/* src/booter/LoadMng/LoadMngKernel.c                                         */
/*                                                                 2017/07/16 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <kernel/MochiKernel.h>
#include <MLib/Basic/MLibBasic.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Driver.h>
#include <LoadMng.h>

/* 内部モジュールヘッダ */
#include "LoadMngInit.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                       \
    DebugLogOutput( CMN_MODULE_LOADMNG_KERNEL, \
                    __LINE__,                  \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       カーネル読込み
 * @details     カーネルを読み込む
 */
/******************************************************************************/
void LoadMngKernelLoad( void )
{
    uint32_t            size;   /* LBAサイズ              */
    MochiKernelImgHdr_t header; /* カーネルイメージヘッダ */
    
    /* トレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 初期化 */
    size = sizeof ( MochiKernelImgHdr_t ) / 512;
    
    /* カーネルイメージヘッダ読込み */
    DriverAtaRead( &header,
                   gLoadMngInitPt[ 1 ].lbaFirstAddr,
                   size );
    
    /* カーネル読込み */
    DriverAtaRead( ( void * ) MOCHIKERNEL_ADDR_ENTRY,
                   gLoadMngInitPt[ 1 ].lbaFirstAddr + size,
                   MLIB_BASIC_ALIGN( header.fileSize, 512 ) / 512);
    
    /* トレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
