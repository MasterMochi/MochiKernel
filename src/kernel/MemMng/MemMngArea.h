/******************************************************************************/
/* src/kernel/MemMng/MemMngArea.h                                             */
/*                                                                 2018/08/15 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef MEMMNG_AREA_H
#define MEMMNG_AREA_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <kernel/kernel.h>
#include <kernel/types.h>
#include <MLib/MLib.h>
#include <MLib/Basic/MLibBasic.h>
#include <MLib/Basic/MLibBasicList.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** メモリ領域情報構造体 */
typedef struct {
    MLibBasicListNode_t node;   /**< 連結リストノード情報 */
    uint32_t            used;   /**< 使用フラグ           */
    void                *pAddr; /**< 先頭アドレス         */
    size_t              size;   /**< サイズ               */
    MkTaskId_t          taskId; /**< タスクID             */
} AreaInfo_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* メモリ領域割当 */
extern void *AreaAlloc( MLibBasicList_t *pAllocList,
                        MLibBasicList_t *pFreeList,
                        size_t          size         );

/* 指定メモリ領域割当 */
extern void *AreaAllocSpecified( MLibBasicList_t *pAllocList,
                                 MLibBasicList_t *pFreeList,
                                 void            *pAddr,
                                 size_t          size         );

/* メモリ領域解放 */
extern CmnRet_t AreaFree( MLibBasicList_t *pAllocList,
                          MLibBasicList_t *pFreeList,
                          void            *pAddr       );

/* メモリ領域管理初期化 */
extern void AreaInit( void );

/* メモリ領域情報リスト設定 */
extern AreaInfo_t *AreaSet( MLibBasicList_t *pFreeList,
                            void            *pAddr,
                            size_t          size        );

/******************************************************************************/
#endif
