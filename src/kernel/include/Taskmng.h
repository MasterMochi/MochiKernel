/******************************************************************************/
/*                                                                            */
/* src/kernel/include/Taskmng.h                                               */
/*                                                                 2021/05/22 */
/* Copyright (C) 2018-2021 Mochi.                                             */
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
/* Taskmng.c */
/*-----------*/
/* タスク管理初期化 */
extern void TaskmngInit( void );

/*----------------*/
/* TaskmngSched.c */
/*----------------*/
/* スケジューラ実行 */
extern void TaskmngSchedExec( void );
/* タスクID取得 */
extern MkTaskId_t TaskmngSchedGetTaskId( void );
/* スケジュール開始 */
extern void TaskmngSchedStart( MkTaskId_t taskId );
/* スケジュール停止 */
extern void TaskmngSchedStop( MkTaskId_t taskId );

/*---------------*/
/* TaskmngProc.c */
/*---------------*/
/* プロセス追加 */
extern MkPid_t TaskmngProcAdd( uint8_t type,
                               void    *pAddr,
                               size_t  size    );

/*---------------*/
/* TaskmngTask.c */
/*---------------*/
/* タスク存在確認 */
extern bool TaskmngTaskCheckExist( MkTaskId_t taskId );
/* タスクタイプ取得 */
extern uint8_t TaskmngTaskGetType( MkTaskId_t taskId );
/* プロセス階層差取得 */
extern uint8_t TaskmngTaskGetTypeDiff( MkTaskId_t taskId1,
                                       MkTaskId_t taskId2  );


/******************************************************************************/
#endif
