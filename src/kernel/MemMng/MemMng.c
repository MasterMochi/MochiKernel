/******************************************************************************/
/*                                                                            */
/* src/kernel/MemMng/MemMng.c                                                 */
/*                                                                 2019/07/24 */
/* Copyright (C) 2016-2019 Mochi.                                             */
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
#include "MemMngArea.h"
#include "MemMngGdt.h"
#include "MemMngIo.h"
#include "MemMngMap.h"
#include "MemMngPage.h"
#include "MemMngPhys.h"
#include "MemMngVirt.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_MAIN, \
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
void MemMngInit( BiosE820Entry_t *pBiosE820,
                 size_t          biosE820Num,
                 MkMemMapEntry_t *pMemMap,
                 size_t          memMapNum    )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* メモリマップ管理サブモジュール初期化 */
    MapInit( pBiosE820, biosE820Num, pMemMap, memMapNum );

    /* GDT管理サブモジュール初期化 */
    MemMngGdtInit();

    /* メモリ領域管理サブモジュール初期化 */
    AreaInit();

    /* 物理メモリ領域管理サブモジュール初期化 */
    PhysInit( pMemMap, memMapNum );

    /* I/Oメモリ領域管理サブモジュール初期化 */
    IoInit( pMemMap, memMapNum );

    /* 仮想メモリ領域管理サブモジュール初期化 */
    VirtInit();

    /* ページ管理サブモジュール初期化 */
    MemMngPageInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
