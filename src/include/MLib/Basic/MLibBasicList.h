/******************************************************************************/
/* src/include/MLib/Basic/MLibBasicList.h                                     */
/*                                                                 2017/02/17 */
/* Copyright (C) 2017 Mochi                                                   */
/******************************************************************************/
#ifndef _MLIB_BASIC_LIST_H_
#define _MLIB_BASIC_LIST_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stddef.h>

#include <MLib/MLib.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** ノード構造体 */
typedef struct MLibBasicListNode {
    struct MLibBasicListNode *pNext;    /**< 前ノード */
    struct MLibBasicListNode *pPrev;    /**< 次ノード */
} MLibBasicListNode_t;

/** 連結リスト構造体 */
typedef struct {
    MLibBasicListNode_t      *pHead;    /**< 先頭ノード             */
    MLibBasicListNode_t      *pTail;    /**< 最後尾ノード           */
    size_t                   size;      /**< 連結リストのノード個数 */
} MLibBasicList_t;


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/* 次ノード取得 */
extern MLibBasicListNode_t *MLibBasicListGetNextNode(
    MLibBasicList_t     *pList,
    MLibBasicListNode_t *pNode  );

/* 前ノード取得 */
extern MLibBasicListNode_t *MLibBasicListGetPrevNode(
    MLibBasicList_t     *pList,
    MLibBasicListNode_t *pNode  );

/* 連結リスト初期化 */
extern MLibRet_t MLibBasicListInit( MLibBasicList_t *pList );

/* ノード先頭挿入 */
extern MLibRet_t MLibBasicListInsertHead( MLibBasicList_t     *pList,
                                          MLibBasicListNode_t *pNewNode );

/* ノード次挿入 */
extern MLibRet_t MLibBasicListInsertNext( MLibBasicList_t     *pList,
                                          MLibBasicListNode_t *pNode,
                                          MLibBasicListNode_t *pNewNode );

/* ノード前挿入 */
extern MLibRet_t MLibBasicListInsertPrev( MLibBasicList_t     *pList,
                                          MLibBasicListNode_t *pNode,
                                          MLibBasicListNode_t *pNewNode );

/* ノード最後尾挿入 */
extern MLibRet_t MLibBasicListInsertTail( MLibBasicList_t     *pList,
                                          MLibBasicListNode_t *pNewNode );

/* 指定ノード削除 */
extern MLibRet_t MLibBasicListRemove( MLibBasicList_t     *pList,
                                      MLibBasicListNode_t *pNode  );

/* 先頭ノード削除 */
extern MLibBasicListNode_t *MLibBasicListRemoveHead( MLibBasicList_t *pList );

/* 最後尾ノード削除 */
extern MLibBasicListNode_t *MLibBasicListRemoveTail( MLibBasicList_t *pList );


/******************************************************************************/
#endif
