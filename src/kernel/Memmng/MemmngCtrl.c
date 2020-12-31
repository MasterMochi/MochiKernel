/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngCtrl.c                                             */
/*                                                                 2020/12/31 */
/* Copyright (C) 2017-2020 Mochi.                                             */
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
#include <hardware/IA32/IA32Paging.h>
#include <kernel/config.h>

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
    DebugLogOutput( CMN_MODULE_MEMMNG_CTRL, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif


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
 * @attention   引数pPAddrは4KiBアライメントであること。
 */
/******************************************************************************/
void MemmngCtrlCopyVirtToPhys( void   *pPAddr,
                               void   *pVAddr,
                               size_t size     )
{
    size_t   mapSize;   /* マッピングサイズ     */
    size_t   copySize;  /* コピーサイズ         */
    uint32_t pageDirId; /* ページディレクトリID */
    uint32_t idx;       /* インデックス         */
    CmnRet_t ret;       /* 関数戻り値           */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    DEBUG_LOG( " pPAddr=%010p, pVAddr=%010p, size=%#X",
               pPAddr,
               pVAddr,
               size );

    /* ページディレクトリID取得 */
    pageDirId = MemmngPageGetDirId();

    /* 物理マッピング用領域サイズ毎に繰り返し */
    for ( idx = 0; idx < size; idx += MK_CONFIG_SIZE_KERNEL_MAP ) {
        /* 次コピー有無判定 */
        if ( ( size - idx ) > MK_CONFIG_SIZE_KERNEL_MAP ) {
            /* 次コピー有 */

            /* コピーサイズ設定 */
            copySize = MK_CONFIG_SIZE_KERNEL_MAP;

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
        ret = MemmngPageSet( pageDirId,
                             ( void * ) MK_CONFIG_ADDR_KERNEL_MAP1,
                             pPAddr + idx,
                             mapSize,
                             IA32_PAGING_G_NO,
                             IA32_PAGING_US_SV,
                             IA32_PAGING_RW_RW );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */

            /* [TODO] */
        }

        /* コピー */
        MLibUtilCopyMemory( ( void * ) MK_CONFIG_ADDR_KERNEL_MAP1,
                            pVAddr + idx,
                            copySize                               );
    }

    /* ページマッピング解除 */
    MemmngPageUnset( pageDirId,
                     ( void * ) MK_CONFIG_ADDR_KERNEL_MAP1,
                     MK_CONFIG_SIZE_KERNEL_MAP );

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
void MemmngCtrlSet( void    *pPAddr,
                    uint8_t value,
                    size_t  size     )
{
    size_t   mapSize;   /* マッピングサイズ     */
    size_t   setSize;   /* 設定サイズ           */
    uint32_t pageDirId; /* ページディレクトリID */
    uint32_t idx;       /* インデックス         */
    CmnRet_t ret;       /* 関数戻り値           */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pPAddr=%010p, value=%0#4X, size=%#X",
               __func__,
               pPAddr,
               value,
               size );

    /* ページディレクトリID取得 */
    pageDirId = MemmngPageGetDirId();

    /* 物理マッピング用領域サイズ毎に繰り返し */
    for ( idx = 0; idx < size; idx += MK_CONFIG_SIZE_KERNEL_MAP ) {
        /* 次設定有無判定 */
        if ( ( size - idx ) > MK_CONFIG_SIZE_KERNEL_MAP ) {
            /* 次設定有 */

            /* サイズ設定 */
            setSize = MK_CONFIG_SIZE_KERNEL_MAP;
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
        ret = MemmngPageSet( pageDirId,
                             ( void * ) MK_CONFIG_ADDR_KERNEL_MAP1,
                             pPAddr + idx,
                             mapSize,
                             IA32_PAGING_G_NO,
                             IA32_PAGING_US_SV,
                             IA32_PAGING_RW_RW );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */

            /* [TODO] */
        }

        /* メモリ設定 */
        MLibUtilSetMemory8( ( void * ) MK_CONFIG_ADDR_KERNEL_MAP1, value, setSize );
    }

    /* ページマッピング解除 */
    MemmngPageUnset( pageDirId,
                     ( void * ) MK_CONFIG_ADDR_KERNEL_MAP1,
                     MK_CONFIG_SIZE_KERNEL_MAP );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
