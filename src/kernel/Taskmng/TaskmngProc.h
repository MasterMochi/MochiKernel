/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngProc.h                                           */
/*                                                                 2021/10/01 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TASKMNG_PROC_H
#define TASKMNG_PROC_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibDynamicArray.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Memmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** ヒープ情報 */
typedef struct {
    void *pEndPoint;    /**< エンドポイント   */
    void *pBreakPoint;  /**< ブレイクポイント */
} ProcHeapInfo_t;

/** スタック情報 */
typedef struct {
    void   *pPhysAddr;      /**< 物理アドレス */
    void   *pTopAddr;       /**< 先頭アドレス */
    void   *pBottomAddr;    /**< 終端アドレス */
    size_t size;            /**< サイズ       */
} ProcStackInfo_t;

/** プロセス管理情報 */
typedef struct {
    MkPid_t            pid;             /**< プロセスID                       */
    MkPid_t            parent;          /**< 親プロセスID                     */
    uint8_t            type;            /**< プロセスタイプ                   */
    void               *pEntryPoint;    /**< エントリポイント                 */
    MemmngPageDirId_t  dirId;           /**< ページディレクトリID             */
    IA32PagingPDBR_t   pdbr;            /**< ページディレクトリベースレジスタ */
    ProcHeapInfo_t     userHeap;        /**< ユーザヒープ情報                 */
    ProcStackInfo_t    userStack;       /**< ユーザスタック情報               */
    MLibDynamicArray_t threadTbl;       /**< スレッド管理情報動的配列         */
} ProcInfo_t;


/******************************************************************************/
/* モジュール内グローバル関数宣言                                             */
/******************************************************************************/
/* プロセス管理情報取得 */
extern ProcInfo_t *ProcGetInfo( MkPid_t pid );
/* プロセス制御初期化 */
extern void ProcInit( void );


/******************************************************************************/
#endif
