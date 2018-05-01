/******************************************************************************/
/* src/kernel/TaskMng/TaskMngTask.h                                           */
/*                                                                 2018/05/01 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef TASKMNG_TASK_H
#define TASKMNG_TASK_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <hardware/IA32/IA32Paging.h>

/* 外部モジュールヘッダ */
#include <TaskMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** コンテキスト構造体 */
typedef struct {
    uint32_t               eip;             /**< eipレジスタ */
    uint32_t               esp;             /**< espレジスタ */
    uint32_t               ebp;             /**< ebpレジスタ */
} TaskMngTaskContext_t;

/** タスクスタック情報 */
typedef struct {
    void                   *pTopAddr;       /**< 先頭アドレス */
    void                   *pBottomAddr;    /**< 後尾アドレス */
    size_t                 size;            /**< サイズ       */
} TaskMngTaskStackInfo_t;

/** タスク管理テーブル構造体 */
typedef struct {
    uint8_t                used;            /**< 使用フラグ               */
    uint8_t                type;            /**< タスクタイプ             */
    uint8_t                state;           /**< タスク状態               */
    uint8_t                reserved;        /**< パディング               */
    TaskMngTaskContext_t   context;         /**< コンテキスト             */
    uint32_t               pageDirId;       /**< ページディレクトリID     */
    void                   *pEntryPoint;    /**< エントリポイント         */
    TaskMngTaskStackInfo_t kernelStackInfo; /**< カーネルスタックアドレス */
    TaskMngTaskStackInfo_t stackInfo;       /**< スタックアドレス         */
} TaskMngTaskTbl_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* コンテキスト取得 */
extern TaskMngTaskContext_t TaskMngTaskGetContext( uint32_t taskId );

/* カーネルスタックアドレス取得 */
extern void *TaskMngTaskGetKernelStack( uint32_t taskId );

/* ページディレクトリID取得 */
extern uint32_t TaskMngTaskGetPageDirId( uint32_t taskId );

/* タスクタイプ取得 */
extern uint8_t TaskMngTaskGetType( uint32_t taskId );

/* タスク管理初期化 */
extern void TaskMngTaskInit( void );

/* コンテキスト設定 */
extern void TaskMngTaskSetContext( uint32_t             taskId,
                                   TaskMngTaskContext_t *pContext );

/* タスク起動開始 */
extern void TaskMngTaskStart( void );


/******************************************************************************/
#endif
