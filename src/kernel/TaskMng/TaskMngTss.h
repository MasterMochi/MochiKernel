/******************************************************************************/
/* src/kernel/TaskMng/TaskMngTss.h                                            */
/*                                                                 2018/05/01 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef TASKMNG_TSS_H
#define TASKMNG_TSS_H
/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* TSS管理初期化 */
extern void TaskMngTssInit( void );

/* ESP0設定 */
extern void TaskMngTssSetEsp0( uint32_t esp0 );


/******************************************************************************/
#endif