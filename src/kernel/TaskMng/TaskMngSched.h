/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngSched.h                                          */
/*                                                                 2019/08/09 */
/* Copyright (C) 2017-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_SCHED_H
#define TASKMNG_SCHED_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 外部モジュールヘッダ */
#include <Cmn.h>

/* 内部モジュールヘッダ */
#include "TaskMngTbl.h"


/******************************************************************************/
/* モジュール内グローバル関数宣言                                             */
/******************************************************************************/
/* スケジュール追加 */
extern CmnRet_t SchedAdd( TblTaskInfo_t *pTaskInfo );
/* 実行中プロセス管理情報取得 */
extern TblProcInfo_t *SchedGetProcInfo( void );
/* 実行中タスク管理情報取得 */
extern TblTaskInfo_t *SchedGetTaskInfo( void );
/* 実行中スレッド管理情報取得 */
extern TblThreadInfo_t *SchedGetThreadInfo( void );
/* スケジューラ初期化 */
extern void SchedInit( void );


/******************************************************************************/
#endif
