/******************************************************************************/
/*                                                                            */
/* src/kernel/Ioctrl/Ioctrl.c                                                 */
/*                                                                 2024/06/01 */
/* Copyright (C) 2018-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "IoctrlMem.h"
#include "IoctrlPort.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_IOCTRL_MAIN


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       入出力制御初期化
 * @details     入出力制御内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IoctrlInit( void )
{
    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* I/Oポート制御サブモジュール初期化 */
    PortInit();

    /* I/Oメモリ制御サブモジュール初期化 */
    MemInit();

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
