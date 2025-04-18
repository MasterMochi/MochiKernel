/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/Taskmng.c                                               */
/*                                                                 2026/06/02 */
/* Copyright (C) 2017-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "TaskmngName.h"
#include "TaskmngProc.h"
#include "TaskmngSched.h"
#include "TaskmngTask.h"
#include "TaskmngThread.h"
#include "TaskmngTss.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_TASKMNG_MAIN


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスク管理初期化
 * @details     タスク管理内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void TaskmngInit( void )
{
    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* プロセス制御初期化 */
    ProcInit();

    /* スレッド制御初期化 */
    ThreadInit();

    /* タスク制御初期化 */
    TaskInit();

    /* TSS管理初期化 */
    TssInit();

    /* タスク名管理初期化 */
    NameInit();

    /* スケジューラ初期化 */
    SchedInit();

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
