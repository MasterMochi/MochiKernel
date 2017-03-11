/******************************************************************************/
/* src/kernel/include/ProcMng.h                                               */
/*                                                                 2017/03/03 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef PROCMNG_H
#define PROCMNG_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* タスクID */
#define PROCMNG_TASK_ID_MIN  ( 0 )                      /** タスクID最小値 */
#define PROCMNG_TASK_ID_MAX  ( 4096 )                   /** タスクID最大値 */
#define PROCMNG_TASK_ID_NULL ( PROCMNG_TASK_ID_MAX + 1 )/** タスクID無     */
#define PROCMNG_TASK_ID_NUM  ( PROCMNG_TASK_ID_MAX + 1 )/** タスクID数     */
#define PROCMNG_TASK_ID_IDLE ( 0 )                      /** アイドルタスク */

/* タスクタイプ */
#define PROCMNG_TASK_TYPE_DRIVER ( 0 )  /** ドライバ */
#define PROCMNG_TASK_TYPE_SERVER ( 1 )  /** サーバ   */
#define PROCMNG_TASK_TYPE_USER   ( 2 )  /** ユーザ   */


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/*---------------*/
/* ProcMngInit.c */
/*---------------*/
/* プロセス管理初期化 */
extern void ProcMngInit( void );

/*--------------*/
/* ProcMngSched */
/*--------------*/
/* スケジューラ実行 */
extern void ProcMngSchedExec( void );

/* タスク追加 */
extern uint32_t ProcMngTaskAdd( uint8_t taskType,
                                void    *pEntryPoint );

/******************************************************************************/
#endif