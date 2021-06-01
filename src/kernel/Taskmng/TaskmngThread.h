/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngThread.h                                         */
/*                                                                 2021/05/29 */
/* Copyright (C) 2019-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_THREAD_H
#define TASKMNG_THREAD_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
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
/** スレッド管理情報動的配列チャンクサイズ */
#define THREAD_TBL_CHUNK_SIZE ( 4 )

/** コンテキスト */
typedef struct {
    uint32_t eip;   /**< eipレジスタ */
    uint32_t esp;   /**< espレジスタ */
    uint32_t ebp;   /**< ebpレジスタ */
} ThreadContext_t;

/** スタック情報 */
typedef struct {
    void   *pTopAddr;       /**< 先頭アドレス */
    void   *pBottomAddr;    /**< 終端アドレス */
    size_t size;            /**< サイズ       */
} ThreadStackInfo_t;

/** スレッド管理情報 */
typedef struct {
    MLibListNode_t    nodeInfo;     /**< リスト管理情報           */
    ProcInfo_t        *pProcInfo;   /**< プロセス管理情報         */
    MkTid_t           tid;          /**< スレッドID               */
    MkTaskId_t        taskId;       /**< タスクID                 */
    uint32_t          state;        /**< 状態                     */
    void              *pEntryPoint; /**< エントリポイント         */
    ThreadContext_t   context;      /**< コンテキスト             */
    ThreadStackInfo_t userStack;    /**< ユーザ空間スタック情報   */
    ThreadStackInfo_t kernelStack;  /**< カーネル空間スタック情報 */
} ThreadInfo_t;


/******************************************************************************/
/* モジュール内グローバル関数宣言                                             */
/******************************************************************************/
/* メインスレッド追加 */
extern MkTid_t ThreadAddMain( ProcInfo_t *pProcInfo );
/* アイドルプロセス用メインスレッド追加 */
extern MkTid_t ThreadAddMainIdle( ProcInfo_t *pProcInfo );
/* スレッド管理情報取得 */
extern ThreadInfo_t *ThreadGetInfo( ProcInfo_t *pProcInfo,
                                    MkTid_t    tid         );
/* スレッド制御初期化 */
extern void ThreadInit( void );


/******************************************************************************/
#endif
