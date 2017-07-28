/******************************************************************************/
/* src/booter/LoadMng/LoadMngProc.c                                           */
/*                                                                 2017/07/27 */
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
#define DEBUG_LOG( ... )                     \
    DebugLogOutput( CMN_MODULE_LOADMNG_PROC, \
                    __LINE__,                \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセスファイル読込み
 * @details     プロセスファイルを読み込む
 */
/******************************************************************************/
void LoadMngProcLoad( void )
{
    void                *pDstAddr;      /* 読込み先アドレス       */
    uint32_t            srcLbaAddr;     /* 読込元LBAアドレス      */
    uint32_t            srcLbaSize;     /* 読込元LBAサイズ        */
    MochiKernelImgHdr_t *pHeader;       /* ファイルヘッダ         */
    MochiKernelImgHdr_t kernelHeader;   /* カーネルバイナリヘッダ */
    
    /* トレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 初期化 */
    pDstAddr   = ( void * ) MOCHIKERNEL_ADDR_PROCIMG;
    srcLbaAddr = gLoadMngInitPt[ 1 ].lbaFirstAddr;
    srcLbaSize = MLIB_BASIC_ALIGN( sizeof ( MochiKernelImgHdr_t ), 512 ) / 512;
    
    /* カーネルバイナリヘッダ読込み */
    DriverAtaRead( &kernelHeader,
                   srcLbaAddr,
                   srcLbaSize );
    
    /* 読込み元LBAアドレス設定 */
    srcLbaAddr += srcLbaSize;
    srcLbaAddr += MLIB_BASIC_ALIGN( kernelHeader.fileSize, 512 ) / 512;
    
    /* ファイル毎に繰り返し */
    do {
        /* アドレス設定 */
        pHeader = ( MochiKernelImgHdr_t * ) pDstAddr;
        
        /* ヘッダ読込み */
        DriverAtaRead( pHeader,
                       srcLbaAddr,
                       srcLbaSize );
        
        /* ファイルサイズチェック */
        if ( pHeader->fileSize == 0 ) {
            /* ファイル無し */
            
            break;
        }
        
        /* トレースログ出力 */
        DEBUG_LOG( "%s() read process( %s: type=%d, size=%d ).",
                   __func__,
                   pHeader->fileName,
                   pHeader->fileType,
                   pHeader->fileSize );
        
        /* アドレス・サイズ設定 */
        pDstAddr   += srcLbaSize * 512;
        srcLbaAddr += srcLbaSize;
        srcLbaSize  = MLIB_BASIC_ALIGN( pHeader->fileSize, 512 ) / 512;
        
        /* ファイル読込み */
        DriverAtaRead( pDstAddr,
                       srcLbaAddr,
                       srcLbaSize );
        
        /* アドレス・サイズ設定 */
        pDstAddr   += srcLbaSize * 512;
        srcLbaAddr += srcLbaSize;
        srcLbaSize  = MLIB_BASIC_ALIGN( sizeof ( MochiKernelImgHdr_t ), 512 ) /
                      512;
        
    } while ( true );
    
    /* トレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
