/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngTbl.h                                            */
/*                                                                 2019/08/15 */
/* Copyright (C) 2019 Mochi.                                                  */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_TBL_H
#define TASKMNG_TBL_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibList.h>

/* カーネルヘッダ */
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** コンテキスト */
typedef struct {
    uint32_t eip;   /**< eipレジスタ */
    uint32_t esp;   /**< espレジスタ */
    uint32_t ebp;   /**< ebpレジスタ */
} TblContext_t;

/** タスクスタック情報 */
typedef struct {
    void   *pTopAddr;       /**< 先頭アドレス */
    void   *pBottomAddr;    /**< 終端アドレス */
    size_t size;            /**< サイズ       */
} TblStackInfo_t;

/** プロセス管理情報 */
typedef struct {
    MkPid_t    pid;             /**< プロセスID                         */
    bool       used;            /**< 使用フラグ                         */
    uint8_t    type;            /**< プロセスタイプ                     */
    uint32_t   pageDirId;       /**< ページディレクトリID               */
    void       *pEntryPoint;    /**< エントリポイント                   */
    void       *pEndPoint;      /**< エンドポイント                     */
    void       *pBreakPoint;    /**< ブレイクポイント                   */
    MLibList_t threadChunkList; /**< スレッド管理テーブルチャンクリスト */
} TblProcInfo_t;

/** スレッド管理情報 */
typedef struct {
    MLibListNode_t nodeInfo;        /**< リスト管理情報           */
    TblProcInfo_t  *pProcInfo;      /**< プロセス管理情報         */
    MkTid_t        tid;             /**< スレッドID               */
    MkTaskId_t     taskId;          /**< タスクID                 */
    bool           used;            /**< 使用フラグ               */
    uint32_t       state;           /**< 状態                     */
    void           *pEntryPoint;    /**< エントリポイント         */
    TblContext_t   context;         /**< コンテキスト             */
    TblStackInfo_t userStack;       /**< ユーザ空間スタック情報   */
    TblStackInfo_t kernelStack;     /**< カーネル空間スタック情報 */
} TblThreadInfo_t;

/** タスク管理情報 */
typedef TblThreadInfo_t TblTaskInfo_t;

/** チャンク内スレッド管理情報数 */
#define TBL_CHUNK_THREADINFO_NUM \
    ( ( 4096 - sizeof ( MLibListNode_t ) ) / sizeof ( TblThreadInfo_t ) )
/** チャンク内スレッド管理情報インデックス最小値 */
#define TBL_CHUNK_THREADINFO_MIN ( 0 )
/** チャンク内スレッド管理情報インデックス最大値 */
#define TBL_CHUNK_THREADINFO_MAX ( TBL_CHUNK_THREADINFO_NUM - 1 )

/** スレッド管理テーブルチャンク */
typedef struct {
    MLibListNode_t  nodeInfo;                               /**< ノード情報       */
    TblThreadInfo_t threadInfo[ TBL_CHUNK_THREADINFO_NUM ]; /**< スレッド管理情報 */
} TblThreadChunk_t;


/******************************************************************************/
/* モジュール内グローバル関数宣言                                             */
/******************************************************************************/
/* プロセス管理情報割当 */
extern TblProcInfo_t *TblAllocProcInfo( void );
/* スレッド管理情報割当 */
extern TblThreadInfo_t *TblAllocThreadInfo( TblProcInfo_t *pProcInfo );
/* プロセス管理情報解放 */
extern void TblFreeProcInfo( TblProcInfo_t *pProcInfo );
/* スレッド管理情報解放 */
extern void TblFreeThreadInfo( TblThreadInfo_t *pThreadInfo );
/* プロセス管理情報取得 */
extern TblProcInfo_t *TblGetProcInfo( MkPid_t pid );
/* タスク管理情報取得 */
extern TblTaskInfo_t *TblGetTaskInfo( MkTaskId_t taskId );
/*スレッド管理情報取得 */
extern TblThreadInfo_t *TblGetThreadInfo( TblProcInfo_t *pProcInfo,
                                          MkTid_t       tid         );
/* テーブル管理モジュール初期化 */
extern void TblInit( void );


/******************************************************************************/
#endif
