/******************************************************************************/
/* src/kernel/ProcMng/ProcMngTask.h                                           */
/*                                                                 2017/03/01 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef PROCMNG_TASK_H
#define PROCMNG_TASK_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <ProcMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** コンテキスト構造体 */
typedef struct {
    uint32_t eip;   /**< eipレジスタ */
    uint32_t esp;   /**< espレジスタ */
} ProcMngTaskContext_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* コンテキスト取得 */
extern ProcMngTaskContext_t ProcMngTaskGetContext( uint32_t taskId );

/* カーネルスタックアドレス取得 */
extern void *ProcMngTaskGetKernelStack( uint32_t taskId );

/* タスクタイプ取得 */
extern uint8_t ProcMngTaskGetType( uint32_t taskId );

/* タスク管理初期化 */
extern void ProcMngTaskInit( void );

/* コンテキスト設定 */
extern void ProcMngTaskSetContext( uint32_t             taskId,
                                   ProcMngTaskContext_t *pContext );

/* タスク起動開始 */
extern void ProcMngTaskStart( void );


/******************************************************************************/
#endif
