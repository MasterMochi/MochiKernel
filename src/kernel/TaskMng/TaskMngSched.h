/******************************************************************************/
/* src/kernel/TaskMng/TaskMngSched.h                                          */
/*                                                                 2018/05/08 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef TASKMNG_SCHED_H
#define TASKMNG_SCHED_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <Cmn.h>
#include <kernel/types.h>


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* スケジュール追加 */
extern CmnRet_t TaskMngSchedAdd( MkTaskId_t taskId );

/* タスクID取得 */
extern MkTaskId_t TaskMngSchedGetTaskId( void );

/* スケジューラ初期化 */
extern void TaskMngSchedInit( void );


/******************************************************************************/
#endif
