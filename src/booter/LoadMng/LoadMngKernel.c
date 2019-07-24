/******************************************************************************/
/*                                                                            */
/* src/booter/LoadMng/LoadMngKernel.c                                         */
/*                                                                 2019/07/24 */
/* Copyright (C) 2017-2019 Mochi.                                             */
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
#include <elf.h>
#include <kernel/kernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <config.h>
#include <Debug.h>
#include <Driver.h>
#include <LoadMng.h>
#include <MemMng.h>

/* 内部モジュールヘッダ */
#include "LoadMng.h"


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
void LoadMngKernelLoad( void )
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
                   gLoadMngPt[ 1 ].lbaFirstAddr,
                   size                          );

    /* カーネル読込み */
    DriverAtaRead( ( void * ) MB_CONFIG_ADDR_KERNEL,
                   gLoadMngPt[ 1 ].lbaFirstAddr + size,
                   MLIB_ALIGN( header.fileSize, 512 ) / 512 );

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
        memset( ( void * ) pEntry->p_vaddr,
                0,
                pEntry->p_memsz             );

        /* セグメントコピー */
        memcpy( ( void * ) pEntry->p_vaddr,
                ( void * ) ( uint32_t ) pElfHdr + pEntry->p_offset,
                pEntry->p_filesz                                    );

        /* メモリマップリスト設定 */
        ret = MemMngMapSetList( ( void * ) pEntry->p_vaddr,
                                MK_SIZE_KERNEL,
                                MK_MEM_TYPE_KERNEL          );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "MemMngMapSetList() failed." );

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
