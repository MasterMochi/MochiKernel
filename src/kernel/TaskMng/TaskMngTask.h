/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngTask.h                                           */
/*                                                                 2019/07/23 */
/* Copyright (C) 2017-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_TASK_H
#define TASKMNG_TASK_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Paging.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** コンテキスト構造体 */
typedef struct {
    uint32_t eip;   /**< eipレジスタ */
    uint32_t esp;   /**< espレジスタ */
    uint32_t ebp;   /**< ebpレジスタ */
} TaskMngTaskContext_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* アプリスタックアドレス取得 */
extern void *TaskMngTaskGetAplStack( MkTaskId_t taskId );

/* コンテキスト取得 */
extern TaskMngTaskContext_t TaskMngTaskGetContext( MkTaskId_t taskId );

/* カーネルスタックアドレス取得 */
extern void *TaskMngTaskGetKernelStack( MkTaskId_t taskId );

/* ページディレクトリID取得 */
extern uint32_t TaskMngTaskGetPageDirId( MkTaskId_t taskId );

/* タスク管理初期化 */
extern void TaskMngTaskInit( void );

/* コンテキスト設定 */
extern void TaskMngTaskSetContext( MkTaskId_t           taskId,
                                   TaskMngTaskContext_t *pContext );


/******************************************************************************/
#endif
