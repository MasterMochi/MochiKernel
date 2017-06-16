/******************************************************************************/
/* src/kernel/ProcMng/ProcMngTask.h                                           */
/*                                                                 2017/06/15 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef PROCMNG_TASK_H
#define PROCMNG_TASK_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <hardware/IA32/IA32Paging.h>

/* 外部モジュールヘッダ */
#include <ProcMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** コンテキスト構造体 */
typedef struct {
    uint32_t               eip;             /**< eipレジスタ */
    uint32_t               esp;             /**< espレジスタ */
    uint32_t               ebp;             /**< ebpレジスタ */
} ProcMngTaskContext_t;

/** タスクスタック情報 */
typedef struct {
    void                   *pTopAddr;       /**< 先頭アドレス */
    void                   *pBottomAddr;    /**< 後尾アドレス */
    size_t                 size;            /**< サイズ       */
} ProcMngTaskStackInfo_t;

/** タスク管理テーブル構造体 */
typedef struct {
    uint8_t                used;            /**< 使用フラグ               */
    uint8_t                type;            /**< タスクタイプ             */
    uint8_t                state;           /**< タスク状態               */
    uint8_t                reserved;        /**< パディング               */
    ProcMngTaskContext_t   context;         /**< コンテキスト             */
    uint32_t               pageDirId;       /**< ページディレクトリID     */
    void                   *pEntryPoint;    /**< エントリポイント         */
    ProcMngTaskStackInfo_t kernelStackInfo; /**< カーネルスタックアドレス */
    ProcMngTaskStackInfo_t stackInfo;       /**< スタックアドレス         */
} ProcMngTaskTbl_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* コンテキスト取得 */
extern ProcMngTaskContext_t ProcMngTaskGetContext( uint32_t taskId );

/* カーネルスタックアドレス取得 */
extern void *ProcMngTaskGetKernelStack( uint32_t taskId );

/* ページディレクトリID取得 */
extern uint32_t ProcMngTaskGetPageDirId( uint32_t taskId );

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
