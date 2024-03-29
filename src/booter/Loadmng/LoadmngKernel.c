/******************************************************************************/
/*                                                                            */
/* src/booter/Loadmng/LoadmngKernel.c                                         */
/*                                                                 2021/11/27 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <elf.h>
#include <kernel/kernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <config.h>
#include <Debug.h>
#include <Driver.h>
#include <Loadmng.h>
#include <Memmng.h>

/* 内部モジュールヘッダ */
#include "Loadmng.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_LOADMNG_KERNEL,  \
                    __LINE__,                   \
                    __VA_ARGS__                )
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
void LoadmngKernelLoad( void )
{
    CmnRet_t   ret;         /* 戻り値                           */
    uint32_t   index;       /* インデックス                     */
    uint32_t   size;        /* LBAサイズ                        */
    MkImgHdr_t header;      /* カーネルイメージヘッダ           */
    Elf32_Ehdr *pElfHdr;    /* プログラムヘッダテーブル         */
    Elf32_Phdr *pPrgHdr;    /* プログラムヘッダテーブル         */
    Elf32_Phdr *pEntry;     /* プログラムヘッダテーブルエントリ */

    /* トレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 初期化 */
    size = sizeof ( MkImgHdr_t ) / 512;

    /* カーネルイメージヘッダ読込み */
    DriverAtaRead( &header,
                   gLoadmngPt[ 1 ].lbaFirstAddr,
                   size                          );

    /* カーネル読込み */
    DriverAtaRead( ( void * ) MB_CONFIG_ADDR_KERNEL,
                   gLoadmngPt[ 1 ].lbaFirstAddr + size,
                   MLIB_UTIL_ALIGN( header.fileSize, 512 ) / 512 );

    /* ELF操作用変数設定 */
    pElfHdr = ( Elf32_Ehdr * ) MB_CONFIG_ADDR_KERNEL;
    pPrgHdr = ( Elf32_Phdr * ) ( ( uint32_t ) pElfHdr + pElfHdr->e_phoff );

    /* プログラムヘッダテーブルエントリ毎に繰り返し */
    for ( index = 0; index < pElfHdr->e_phnum; index++ ) {
        /* プログラムヘッダ参照アドレス設定 */
        pEntry = ( Elf32_Phdr * ) ( ( uint32_t ) pPrgHdr +
                                    pElfHdr->e_phentsize * index );

        /* セグメントタイプ判定 */
        if ( pEntry->p_type != PT_LOAD ) {
            /* ロード可能セグメントでない */

            /* 次エントリ */
            continue;
        }

        /* セグメント初期化 */
        MLibUtilSetMemory8( ( void * ) pEntry->p_vaddr,
                            0,
                            pEntry->p_memsz             );

        /* セグメントコピー */
        MLibUtilCopyMemory(
            ( void * ) pEntry->p_vaddr,
            ( void * ) ( uint32_t ) pElfHdr + pEntry->p_offset,
            pEntry->p_filesz                                    );

        /* メモリマップリスト設定 */
        ret = MemmngMapSetList( ( void * ) pEntry->p_vaddr,
                                MK_SIZE_KERNEL,
                                MK_MEM_TYPE_KERNEL          );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "MemmngMapSetList() failed." );

            /* アボート */
            CmnAbort();
        }

        /* 現状は1つのロード可能セグメントだけに対応 */
        break;
    }

    /* トレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
