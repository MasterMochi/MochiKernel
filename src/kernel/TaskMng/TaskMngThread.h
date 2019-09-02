/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngThread.h                                         */
/*                                                                 2019/08/08 */
/* Copyright (C) 2019 Mochi.                                                  */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_THREAD_H
#define TASKMNG_THREAD_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* カーネルヘッダ */
#include <kernel/types.h>

/* 内部モジュールヘッダ */
#include "TaskMngTbl.h"


/******************************************************************************/
/* モジュール内グローバル関数宣言                                             */
/******************************************************************************/
/* メインスレッド追加 */
extern MkTid_t ThreadAddMain( TblProcInfo_t *pProcInfo );
/* スレッド制御初期化 */
extern void ThreadInit( void );


/******************************************************************************/
#endif
