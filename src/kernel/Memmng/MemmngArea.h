/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngArea.h                                             */
/*                                                                 2021/01/09 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef MEMMNG_AREA_H
#define MEMMNG_AREA_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>
#include <stddef.h>

/* 共通ヘッダ */
#include <MLib/MLibList.h>

/* 共通ヘッダ */
#include <kernel/kernel.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** ブロック管理情報 */
typedef struct {
    MLibListNode_t node;    /**< 連結リストノード情報 */
    void           *pAddr;  /**< 先頭アドレス         */
    size_t         size;    /**< サイズ               */
    MkTaskId_t     taskId;  /**< タスクID             */
} AreaInfo_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* メモリ領域追加 */
extern void AreaAdd( MLibList_t *pAddList,
                     MLibList_t *pUnusedList,
                     void       *pAddr,
                     size_t     size,
                     bool       merge         );

/* メモリ領域割当 */
extern void *AreaAlloc( MLibList_t *pAllocList,
                        MLibList_t *pFreeList,
                        MLibList_t *pUnusedList,
                        size_t     size          );

/* 指定メモリ領域割当 */
extern void *AreaAllocSpec( MLibList_t *pAllocList,
                            MLibList_t *pFreeList,
                            MLibList_t *pUnusedList,
                            void       *pAddr,
                            size_t     size          );

/* メモリ領域解放 */
extern CmnRet_t AreaFree( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          MLibList_t *pUnusedList,
                          void       *pAddr        );


/******************************************************************************/
#endif
