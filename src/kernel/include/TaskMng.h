/******************************************************************************/
/*                                                                            */
/* src/kernel/include/TaskMng.h                                               */
/*                                                                 2019/08/09 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_H
#define TASKMNG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* 共通ヘッダ */
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* PID */
#define TASKMNG_PID_IDLE         ( 0 )  /**< アイドルプロセス */

/* タスクID */
#define TASKMNG_TASKID_IDLE      ( 0 )  /**< アイドルタスク */

/* プロセスタイプ */
#define TASKMNG_PROC_TYPE_KERNEL ( 0 )  /**< カーネル */
#define TASKMNG_PROC_TYPE_DRIVER ( 1 )  /**< ドライバ */
#define TASKMNG_PROC_TYPE_SERVER ( 2 )  /**< サーバ   */
#define TASKMNG_PROC_TYPE_USER   ( 3 )  /**< ユーザ   */


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/*-----------*/
/* TaskMng.c */
/*-----------*/
/* タスク管理初期化 */
extern void TaskMngInit( void );

/*----------------*/
/* TaskMngSched.c */
/*----------------*/
/* スケジューラ実行 */
extern void TaskMngSchedExec( void );
/* タスクID取得 */
extern MkTaskId_t TaskMngSchedGetTaskId( void );
/* スケジュール開始 */
extern void TaskMngSchedStart( MkTaskId_t taskId );
/* スケジュール停止 */
extern void TaskMngSchedStop( MkTaskId_t taskId );

/*---------------*/
/* TaskMngProc.c */
/*---------------*/
/* プロセス追加 */
extern MkPid_t TaskMngProcAdd( uint8_t type,
                               void    *pAddr,
                               size_t  size    );

/*---------------*/
/* TaskMngTask.c */
/*---------------*/
/* タスク存在確認 */
extern bool TaskMngTaskCheckExist( MkTaskId_t taskId );
/* タスクタイプ取得 */
extern uint8_t TaskMngTaskGetType( MkTaskId_t taskId );
/* プロセス階層差取得 */
extern uint8_t TaskMngTaskGetTypeDiff( MkTaskId_t taskId1,
                                       MkTaskId_t taskId2  );


/******************************************************************************/
#endif
