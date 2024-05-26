/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/Memmng.c                                                 */
/*                                                                 2021/03/24 */
/* Copyright (C) 2016-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "MemmngArea.h"
#include "MemmngSgmt.h"
#include "MemmngHeap.h"
#include "MemmngIo.h"
#include "MemmngMap.h"
#include "MemmngPage.h"
#include "MemmngPhys.h"
#include "MemmngVirt.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                 \
    DebugOutput( CMN_MODULE_MEMMNG_MAIN, \
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
 * @brief       メモリ管理初期化
 * @details     各サブモジュールの初期化を行う。
 *
 * @param[in]   *pBiosE820  BIOS-E820メモリマップ
 * @param[in]   biosE820Num BIOS-E820メモリマップエントリ数
 * @param[in]   *pMemMap    メモリマップ
 * @param[in]   memMapNum   メモリマップエントリ数
 */
/******************************************************************************/
void MemmngInit( BiosE820Entry_t *pBiosE820,
                 size_t          biosE820Num,
                 MkMemMapEntry_t *pMemMap,
                 size_t          memMapNum    )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* メモリマップ管理サブモジュール初期化 */
    MapInit( pBiosE820, biosE820Num, pMemMap, memMapNum );

    /* セグメンテーション管理サブモジュール初期化 */
    MemmngSgmtInit();

    /* ヒープ管理サブモジュール初期化 */
    HeapInit();

    /* 物理メモリ領域管理サブモジュール初期化 */
    PhysInit( pMemMap, memMapNum );

    /* ページ管理サブモジュール初期化 */
    PageInit();

    /* I/Oメモリ領域管理サブモジュール初期化 */
    IoInit( pMemMap, memMapNum );

    /* 仮想メモリ領域管理サブモジュール初期化 */
    VirtInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
