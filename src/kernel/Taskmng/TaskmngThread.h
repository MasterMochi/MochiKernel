/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngThread.h                                         */
/*                                                                 2021/10/24 */
/* Copyright (C) 2019-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_THREAD_H
#define TASKMNG_THREAD_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibList.h>

/* 共通ヘッダ */
#include <kernel/types.h>

/* 内部モジュールヘッダ */
#include "TaskmngProc.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** スケジュール情報 */
typedef struct {
    MLibListNode_t nodeInfo;    /**< ノード情報 */
    uint32_t       state;       /**< 状態       */
} ThreadSchedInfo_t;

/** 起動時情報 */
typedef struct {
    void *pEntryPoint;      /**< エントリポイント */
    void *pStackPointer;    /**< スタックポインタ */
} ThreadStartInfo_t;

/** スタック情報 */
typedef struct {
    void   *pTopAddr;       /**< 先頭アドレス */
    void   *pBottomAddr;    /**< 終端アドレス */
    size_t size;            /**< サイズ       */
} ThreadStackInfo_t;

/** コンテキスト */
typedef struct {
    uint32_t eip;   /**< eipレジスタ */
    uint32_t esp;   /**< espレジスタ */
    uint32_t ebp;   /**< ebpレジスタ */
} ThreadContext_t;

/** スレッド管理情報 */
typedef struct {
    ThreadSchedInfo_t schedInfo;    /**< スケジュール情報     */
    ProcInfo_t        *pProcInfo;   /**< プロセス管理情報     */
    MkTid_t           tid;          /**< スレッドID           */
    MkTaskId_t        taskId;       /**< タスクID             */
    ThreadContext_t   context;      /**< コンテキスト         */
    ThreadStartInfo_t startInfo;    /**< 起動時情報           */
    ThreadStackInfo_t kernelStack;  /**< カーネルスタック情報 */
} ThreadInfo_t;


/******************************************************************************/
/* モジュール内グローバル関数宣言                                             */
/******************************************************************************/
/* メインスレッド追加 */
extern MkTid_t ThreadAddMain( ProcInfo_t *pProcInfo );
/* アイドルプロセス用メインスレッド追加 */
extern MkTid_t ThreadAddMainIdle( ProcInfo_t *pProcInfo );
/* スレッド複製 */
extern MkTid_t ThreadFork( ProcInfo_t *pProcInfo );
/* スレッド管理情報取得 */
extern ThreadInfo_t *ThreadGetInfo( ProcInfo_t *pProcInfo,
                                    MkTid_t    tid         );
/* スレッド制御初期化 */
extern void ThreadInit( void );


/******************************************************************************/
#endif
