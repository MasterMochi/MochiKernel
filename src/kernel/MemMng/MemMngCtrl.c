/******************************************************************************/
/* src/kernel/MemMng/MemMngCtrl.c                                             */
/*                                                                 2017/05/19 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <hardware/IA32/IA32Paging.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                     \
    DebugLogOutput( CMN_MODULE_MEMMNG_CTRL,  \
                    __LINE__,                \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/* 物理マッピング用領域定義 */
#define CTRL_MAP_ADDR1 ( 0x3F000000 )   /** 物理マッピング用領域1先頭アドレス */
#define CTRL_MAP_ADDR2 ( 0x3F800000 )   /** 物理マッピング用領域2先頭アドレス */
#define CTRL_MAP_SIZE  ( 0x01000000 )   /** 物理マッピング用領域全サイズ      */
#define CTRL_MAP_SIZE1 ( 0x00800000 )   /** 物理マッピング用領域1サイズ       */
#define CTRL_MAP_SIZE2 ( 0x00800000 )   /** 物理マッピング用領域2サイズ       */


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリコピー（仮想->物理）
 * @details     仮想アドレス空間から物理アドレス空間へメモリコピーを行う。
 * 
 * @param[in]   pPAddr コピー先物理アドレス
 * @param[in]   pVAddr コピー元仮想アドレス
 * @param[in]   size   コピーサイズ
 * 
 * @attention   引数pPhysAddrは4KiBアライメントであること。
 */
/******************************************************************************/
void MemMngCtrlCopyVirtToPhys( void   *pPAddr,
                               void   *pVAddr,
                               size_t size     )
{
    uint32_t dirId; /* ページディレクトリID */
    uint32_t idx;   /* インデックス         */
    size_t   count; /* コピーサイズ         */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    DEBUG_LOG( " pPAddr=%010p, pVAddr=%010p, size=%#X",
               pPAddr,
               pVAddr,
               size );
    
    /* ページディレクトリID取得 */
    dirId = MemMngPageGetDirId();
    
    /* 物理マッピング用領域サイズ毎に繰り返し */
    for ( idx = 0; idx < size; idx += CTRL_MAP_SIZE ) {
        /* 次コピー有無判定 */
        if ( ( size - idx ) > CTRL_MAP_SIZE ) {
            /* 次コピー有 */
            
            /* コピーサイズ設定 */
            count = CTRL_MAP_SIZE;
            
        } else {
            /* 次コピー無 */
            
            /* コピーサイズ設定 */
            count = size - idx;
        }
        
        /* ページマッピング設定 */
        MemMngPageSet( dirId,
                       ( void * ) CTRL_MAP_ADDR1,
                       pPAddr + idx,
                       count,
                       IA32_PAGING_G_NO,
                       IA32_PAGING_US_SV,
                       IA32_PAGING_RW_RW  );
        
        /* コピー */
        memcpy( ( void * ) CTRL_MAP_ADDR1, pVAddr + idx, count );
    }
    
    /* ページマッピング解除 */
    MemMngPageUnset( dirId, ( void * ) CTRL_MAP_ADDR1, CTRL_MAP_SIZE );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       メモリ設定
 * @details     物理アドレス空間のメモリ領域に指定値を設定する。
 * 
 * @param[in]   pPAddr 設定先物理アドレス
 * @param[in]   value  設定値
 * @param[in]   size   サイズ
 * 
 * @attention   引数pPhysAddrは4KiBアライメントであること。
 */
/******************************************************************************/
void MemMngCtrlSet( void    *pPAddr,
                    uint8_t value,
                    size_t  size     )
{
    uint32_t dirId; /* ページディレクトリID */
    uint32_t idx;   /* インデックス         */
    size_t   count; /* コピーサイズ         */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pPAddr=%010p, value=%0#4X, size=%#X",
               __func__,
               pPAddr,
               value,
               size );
    
    /* ページディレクトリID取得 */
    dirId = MemMngPageGetDirId();
    
    /* 物理マッピング用領域サイズ毎に繰り返し */
    for ( idx = 0; idx < size; idx += CTRL_MAP_SIZE ) {
        /* 次設定有無判定 */
        if ( ( size - idx ) > CTRL_MAP_SIZE ) {
            /* 次設定有 */
            
            /* サイズ設定 */
            count = CTRL_MAP_SIZE;
            
        } else {
            /* 次設定無 */
            
            /* サイズ設定 */
            count = size - idx;
        }
        
        /* ページマッピング設定 */
        MemMngPageSet( dirId,
                       ( void * ) CTRL_MAP_ADDR1,
                       pPAddr + idx,
                       count,
                       IA32_PAGING_G_NO,
                       IA32_PAGING_US_SV,
                       IA32_PAGING_RW_RW  );
        
        /* メモリ設定 */
        memset( ( void * ) CTRL_MAP_ADDR1, value, count );
    }
    
    /* ページマッピング解除 */
    MemMngPageUnset( dirId, ( void * ) CTRL_MAP_ADDR1, CTRL_MAP_SIZE );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
