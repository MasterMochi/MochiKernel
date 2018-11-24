/******************************************************************************/
/* src/kernel/TaskMng/TaskMngInit.c                                           */
/*                                                                 2018/11/24 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "TaskMngProc.h"
#include "TaskMngSched.h"
#include "TaskMngTask.h"
#include "TaskMngTss.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_INIT,    \
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
 * @brief       タスク管理初期化
 * @details     タスク管理内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void TaskMngInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* TSS管理サブモジュール初期化 */
    TaskMngTssInit();
    
    /* タスク管理サブモジュール初期化 */
    TaskMngTaskInit();
    
    /* プロセス管理サブモジュール初期化 */
    TaskMngProcInit();
    
    /* スケジューラサブモジュール初期化 */
    TaskMngSchedInit();
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
