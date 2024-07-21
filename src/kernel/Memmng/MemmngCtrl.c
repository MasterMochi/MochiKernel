/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngCtrl.c                                             */
/*                                                                 2026/07/20 */
/* Copyright (C) 2017-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <memmap.h>
#include <hardware/IA32/IA32Paging.h>
#include <kernel/config.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_MEMMNG_CTRL


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリコピー（物理->物理）
 * @details     物理アドレス空間から物理アドレス空間へメモリコピーを行う。
 *
 * @param[in]   pDst コピー先物理アドレス
 * @param[in]   pSrc コピー元物理アドレス
 * @param[in]   size コピーサイズ
 *
 * @attention   引数pDstとpSrcは4KBアライメントであること。
 */
/******************************************************************************/
void MemmngCtrlCopyPhysToPhys( void   *pDst,
                               void   *pSrc,
                               size_t size   )
{
    size_t   mapSize;   /* マッピングサイズ */
    size_t   copySize;  /* コピーサイズ     */
    uint32_t idx;       /* インデックス     */
    CmnRet_t ret;       /* 関数戻り値       */

    DEBUG_LOG_TRC(
        "%s() start. pDst=%010p, pSrc=%010p, size=%#X",
        __func__,
        pDst,
        pSrc,
        size
    );

    /* 物理マッピング用領域サイズ毎に繰り返し */
    for ( idx = 0; idx < size; idx += MEMMAP_VSIZE_KERNEL_CTRL1 ) {
        /* 次コピー有無判定 */
        if ( ( size - idx ) > MEMMAP_VSIZE_KERNEL_CTRL1 ) {
            /* 次コピー有 */

            /* コピーサイズ設定 */
            copySize = MEMMAP_VSIZE_KERNEL_CTRL1;

            /* マッピングサイズ設定 */
            mapSize = copySize;

        } else {
            /* 次コピー無 */

            /* コピーサイズ設定 */
            copySize = size - idx;

            /* マッピングサイズ設定 */
            mapSize = MLIB_UTIL_ALIGN( copySize, IA32_PAGING_PAGE_SIZE );
        }

        /* メモリ制御領域ch1マッピング設定 */
        ret = MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                             ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                             pSrc + idx,
                             mapSize,
                             IA32_PAGING_G_NO,
                             IA32_PAGING_US_SV,
                             IA32_PAGING_RW_RW                     );

        /* 設定結果判定 */
        if ( ret == CMN_FAILURE ) {
            /* 失敗 */
            /* [TODO] */
        }

        /* メモリ制御領域ch2マッピング設定 */
        ret = MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                             ( void * ) MEMMAP_VADDR_KERNEL_CTRL2,
                             pDst + idx,
                             mapSize,
                             IA32_PAGING_G_NO,
                             IA32_PAGING_US_SV,
                             IA32_PAGING_RW_RW                     );

        /* 設定結果判定 */
        if ( ret == CMN_FAILURE ) {
            /* 失敗 */
            /* [TODO] */
        }

        /* メモリコピー */
        MLibUtilCopyMemory( ( void * ) MEMMAP_VADDR_KERNEL_CTRL2,
                            ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                            copySize                              );
    }

    /* メモリ制御領域ch1マッピング解除 */
    MemmngPageUnset( MEMMNG_PAGE_DIR_ID_IDLE,
                     ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                     MEMMAP_VSIZE_KERNEL_CTRL1             );

    /* メモリ制御領域ch2マッピング解除 */
    MemmngPageUnset( MEMMNG_PAGE_DIR_ID_IDLE,
                     ( void * ) MEMMAP_VADDR_KERNEL_CTRL2,
                     MEMMAP_VSIZE_KERNEL_CTRL2             );

    return;
}


/******************************************************************************/
/**
 * @brief       メモリコピー（仮想->物理）
 * @details     仮想アドレス空間から物理アドレス空間へメモリコピーを行う。
 *
 * @param[in]   pPAddr コピー先物理アドレス
 * @param[in]   pVAddr コピー元仮想アドレス
 * @param[in]   size   コピーサイズ
 *
 * @attention   引数pPAddrは4KiBアライメントであること。
 */
/******************************************************************************/
void MemmngCtrlCopyVirtToPhys( void   *pPAddr,
                               void   *pVAddr,
                               size_t size     )
{
    size_t   mapSize;   /* マッピングサイズ */
    size_t   copySize;  /* コピーサイズ     */
    uint32_t idx;       /* インデックス     */
    CmnRet_t ret;       /* 関数戻り値       */

    DEBUG_LOG_TRC(
        "%s() start. pPAddr=%010p, pVAddr=%010p, size=%#X",
        __func__,
        pPAddr,
        pVAddr,
        size
    );

    /* 物理マッピング用領域サイズ毎に繰り返し */
    for ( idx = 0; idx < size; idx += MEMMAP_VSIZE_KERNEL_CTRL1 ) {
        /* 次コピー有無判定 */
        if ( ( size - idx ) > MEMMAP_VSIZE_KERNEL_CTRL1 ) {
            /* 次コピー有 */

            /* コピーサイズ設定 */
            copySize = MEMMAP_VSIZE_KERNEL_CTRL1;

            /* マッピングサイズ設定 */
            mapSize = copySize;

        } else {
            /* 次コピー無 */

            /* コピーサイズ設定 */
            copySize = size - idx;

            /* マッピングサイズ設定 */
            mapSize = MLIB_UTIL_ALIGN( copySize, IA32_PAGING_PAGE_SIZE );
        }

        /* ページマッピング設定 */
        ret = MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                             ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                             pPAddr + idx,
                             mapSize,
                             IA32_PAGING_G_NO,
                             IA32_PAGING_US_SV,
                             IA32_PAGING_RW_RW                     );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */

            /* [TODO] */
        }

        /* コピー */
        MLibUtilCopyMemory( ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                            pVAddr + idx,
                            copySize                              );
    }

    /* ページマッピング解除 */
    MemmngPageUnset( MEMMNG_PAGE_DIR_ID_IDLE,
                     ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                     MEMMAP_VSIZE_KERNEL_CTRL1             );

    DEBUG_LOG_TRC( "%s() end.", __func__ );

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
void MemmngCtrlSet( void    *pPAddr,
                    uint8_t value,
                    size_t  size     )
{
    size_t   mapSize;   /* マッピングサイズ */
    size_t   setSize;   /* 設定サイズ       */
    uint32_t idx;       /* インデックス     */
    CmnRet_t ret;       /* 関数戻り値       */

    /* デバッグトレースログ出力 */
    DEBUG_LOG_TRC(
        "%s() start. pPAddr=%010p, value=%0#4X, size=%#X",
        __func__,
        pPAddr,
        value,
        size
    );

    /* 物理マッピング用領域サイズ毎に繰り返し */
    for ( idx = 0; idx < size; idx += MEMMAP_VSIZE_KERNEL_CTRL1 ) {
        /* 次設定有無判定 */
        if ( ( size - idx ) > MEMMAP_VSIZE_KERNEL_CTRL1 ) {
            /* 次設定有 */

            /* サイズ設定 */
            setSize = MEMMAP_VSIZE_KERNEL_CTRL1;
            /* マッピングサイズ設定 */
            mapSize = setSize;

        } else {
            /* 次設定無 */

            /* サイズ設定 */
            setSize = size - idx;

            /* マッピングサイズ設定 */
            mapSize = MLIB_UTIL_ALIGN( setSize, IA32_PAGING_PAGE_SIZE );
        }

        /* ページマッピング設定 */
        ret = MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                             ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                             pPAddr + idx,
                             mapSize,
                             IA32_PAGING_G_NO,
                             IA32_PAGING_US_SV,
                             IA32_PAGING_RW_RW                     );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */

            /* [TODO] */
        }

        /* メモリ設定 */
        MLibUtilSetMemory32( ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                             value,
                             setSize                               );
    }

    /* ページマッピング解除 */
    MemmngPageUnset( MEMMNG_PAGE_DIR_ID_IDLE,
                     ( void * ) MEMMAP_VADDR_KERNEL_CTRL1,
                     MEMMAP_VSIZE_KERNEL_CTRL1             );

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG_TRC( "%s() end.", __func__ );*/

    return;
}


/******************************************************************************/
