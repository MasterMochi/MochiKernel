/******************************************************************************/
/* src/include/MLibBasicList.h                                                */
/*                                                                 2018/12/09 */
/* Copyright (C) 2017-2018 Mochi                                              */
/* https://github.com/MasterMochi/MLib.git                                    */
/******************************************************************************/
#ifndef _MLIB_LIST_H_
#define _MLIB_LIST_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stddef.h>
#include "MLib.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** ノード構造体 */
typedef struct MLibListNode {
    struct MLibListNode *pNext;     /**< 前ノード */
    struct MLibListNode *pPrev;     /**< 次ノード */
} MLibListNode_t;

/** 連結リスト構造体 */
typedef struct {
    MLibListNode_t *pHead;  /**< 先頭ノード             */
    MLibListNode_t *pTail;  /**< 最後尾ノード           */
    size_t         size;    /**< 連結リストのノード個数 */
} MLibList_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* 次ノード取得 */
extern MLibListNode_t *MLibListGetNextNode( MLibList_t     *pList,
                                            MLibListNode_t *pNode  );

/* 前ノード取得 */
extern MLibListNode_t *MLibListGetPrevNode( MLibList_t     *pList,
                                            MLibListNode_t *pNode  );

/* 連結リスト初期化 */
extern MLibRet_t MLibListInit( MLibList_t *pList );

/* ノード先頭挿入 */
extern MLibRet_t MLibListInsertHead( MLibList_t     *pList,
                                     MLibListNode_t *pNewNode );

/* ノード次挿入 */
extern MLibRet_t MLibListInsertNext( MLibList_t     *pList,
                                     MLibListNode_t *pNode,
                                     MLibListNode_t *pNewNode );

/* ノード前挿入 */
extern MLibRet_t MLibListInsertPrev( MLibList_t     *pList,
                                     MLibListNode_t *pNode,
                                     MLibListNode_t *pNewNode );

/* ノード最後尾挿入 */
extern MLibRet_t MLibListInsertTail( MLibList_t     *pList,
                                     MLibListNode_t *pNewNode );

/* 指定ノード削除 */
extern MLibRet_t MLibListRemove( MLibList_t     *pList,
                                 MLibListNode_t *pNode  );

/* 先頭ノード削除 */
extern MLibListNode_t *MLibListRemoveHead( MLibList_t *pList );

/* 最後尾ノード削除 */
extern MLibListNode_t *MLibListRemoveTail( MLibList_t *pList );


/******************************************************************************/
#endif
