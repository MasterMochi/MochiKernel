/******************************************************************************/
/* src/booter/LoadMng/LoadMngProc.c                                           */
/*                                                                 2018/12/09 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <kernel/kernel.h>
#include <MLib/MLib.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Driver.h>
#include <LoadMng.h>
#include <MemMng.h>

/* 内部モジュールヘッダ */
#include "LoadMngInit.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_LOADMNG_PROC,    \
                    __LINE__,                   \
                    __VA_ARGS__              )
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
    void       *pDstAddr;       /* 読込み先アドレス       */
    size_t     size;            /* プロセスイメージサイズ */
    CmnRet_t   ret;             /* 戻り値                 */
    uint32_t   srcLbaAddr;      /* 読込元LBAアドレス      */
    uint32_t   srcLbaSize;      /* 読込元LBAサイズ        */
    MkImgHdr_t *pHeader;        /* ファイルヘッダ         */
    MkImgHdr_t kernelHeader;    /* カーネルバイナリヘッダ */
    
    /* トレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 初期化 */
    pDstAddr   = ( void * ) MK_ADDR_PROCIMG;
    srcLbaAddr = gLoadMngInitPt[ 1 ].lbaFirstAddr;
    srcLbaSize = MLIB_ALIGN( sizeof ( MkImgHdr_t ), 512 ) / 512;
    size       = srcLbaSize;
    
    /* カーネルイメージヘッダ読込み */
    DriverAtaRead( &kernelHeader,
                   srcLbaAddr,
                   srcLbaSize );
    
    /* 読込み元LBAアドレス設定 */
    srcLbaAddr += srcLbaSize;
    srcLbaAddr += MLIB_ALIGN( kernelHeader.fileSize, 512 ) / 512;
    
    /* ファイル毎に繰り返し */
    do {
        /* アドレス設定 */
        pHeader = ( MkImgHdr_t * ) pDstAddr;
        
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
        srcLbaSize  = MLIB_ALIGN( pHeader->fileSize, 512 ) / 512;
        size       += srcLbaSize;
        
        /* ファイル読込み */
        DriverAtaRead( pDstAddr,
                       srcLbaAddr,
                       srcLbaSize );
        
        /* アドレス・サイズ設定 */
        pDstAddr   += srcLbaSize * 512;
        srcLbaAddr += srcLbaSize;
        srcLbaSize  = MLIB_ALIGN( sizeof ( MkImgHdr_t ), 512 ) / 512;
        size       += srcLbaSize;
        
    } while ( true );
    
    /* メモリマップリスト設定 */
    ret = MemMngMapSetList( ( void * ) MK_ADDR_PROCIMG,
                            MLIB_ALIGN( size * 512, 4096 ),
                            MK_MEM_TYPE_PROCIMG             );
    
    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "MemMngMapSetList() failed." );
        
        /* アボート */
        CmnAbort();
    }
    
    /* トレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
