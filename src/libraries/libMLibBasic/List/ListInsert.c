/******************************************************************************/
/* src/libraries/libMLibBasic/List/ListInsert.c                               */
/*                                                                 2017/02/17 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stddef.h>
#include <stdint.h>

#include <MLib/MLib.h>
#include <MLib/Basic/MLibBasicList.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ノード先頭挿入
 * @details     指定した連結リストの先頭に新しいノードを挿入する。
 * 
 * @param[in]   *pList      連結リスト
 * @param[in]   *pNewNode   挿入ノード
 * 
 * @retval      MLIB_SUCCESS 正常終了
 * @retval      MLIB_FAILURE 異常終了
 */
/******************************************************************************/
MLibRet_t MLibBasicListInsertHead( MLibBasicList_t     *pList,
                                   MLibBasicListNode_t *pNewNode )
{
    MLibBasicListNode_t *pOldHead;  /* 旧先頭ノード */
    
    /* 初期化 */
    pOldHead = NULL;
    
    /* 引数pListチェック */
    if ( pList == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 引数pNewNodeチェック */
    if ( pNewNode == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 連結リストサイズチェック */
    if ( pList->size == SIZE_MAX ) {
        /* サイズ不正 */
        
        return MLIB_FAILURE;
    }
    
    /* 旧先頭ノード取得 */
    pOldHead = pList->pHead;
    
    /* 連結リスト先頭ノード設定 */
    pList->pHead = pNewNode;
    
    /* 挿入ノード設定 */
    pNewNode->pNext = pOldHead;
    pNewNode->pPrev = NULL;
    
    /* 旧先頭ノード有無判定 */
    if ( pOldHead == NULL ) {
        /* 旧先頭ノード無 */
        
        /* 連結リスト最後尾ノード設定 */
        pList->pTail = pNewNode;
        
    } else {
        /* 旧先頭ノード有 */
        
        /* 旧先頭ノード設定 */
        pOldHead->pPrev = pNewNode;
    }
    
    /* 連結リストサイズ設定 */
    pList->size++;
    
    return MLIB_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ノード次挿入
 * @details     指定したノードの次に新しいノードを挿入する。
 * 
 * @param[in]   *pList      連結リスト
 * @param[in]   *pNode      挿入先ノード
 * @param[in]   *pNewNode   挿入ノード
 * 
 * @retval      MLIB_SUCCESS 正常終了
 * @retval      MLIB_FAILURE 異常終了
 */
/******************************************************************************/
MLibRet_t MLibBasicListInsertNext( MLibBasicList_t     *pList,
                                   MLibBasicListNode_t *pNode,
                                   MLibBasicListNode_t *pNewNode )
{
    MLibBasicListNode_t *pNextNode; /* 次ノード */
    
    /* 初期化 */
    pNextNode = NULL;
    
    /* 引数pListチェック */
    if ( pList == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 引数pNodeチェック */
    if ( pNode == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 引数pNewNodeチェック */
    if ( pNewNode == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 連結リストサイズチェック */
    if ( ( pList->size == 0 ) || ( pList->size == SIZE_MAX ) ) {
        /* サイズ不正 */
        
        return MLIB_FAILURE;
    }
    
    /* 次ノード取得 */
    pNextNode = pNode->pNext;
    
    /* 前ノード設定 */
    pNode->pNext = pNewNode;
    
    /* 挿入ノード設定 */
    pNewNode->pPrev = pNode;
    pNewNode->pNext = pNextNode;
    
    /* 次ノード有無判定 */
    if ( pNextNode == NULL ) {
        /* 次ノード無 */
        
        /* 連結リスト最後尾ノード設定 */
        pList->pTail = pNewNode;
        
    } else {
        /* 次ノード有 */
        
        /* 次ノード設定 */
        pNextNode->pPrev = pNextNode;
    }
    
    /* 連結リストサイズ設定 */
    pList->size++;
    
    return MLIB_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ノード前挿入
 * @details     指定したノードの前に新しいノードを挿入する。
 * 
 * @param[in]   *pList      連結リスト
 * @param[in]   *pNode      挿入先ノード
 * @param[in]   *pNewNode   挿入ノード
 * 
 * @retval      MLIB_SUCCESS 正常終了
 * @retval      MLIB_FAILURE 異常終了
 */
/******************************************************************************/
MLibRet_t MLibBasicListInsertPrev( MLibBasicList_t     *pList,
                                   MLibBasicListNode_t *pNode,
                                   MLibBasicListNode_t *pNewNode )
{
    MLibBasicListNode_t *pPrevNode; /* 前ノード */
    
    /* 初期化 */
    pPrevNode = NULL;
    
    /* 引数pListチェック */
    if ( pList == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 引数pNodeチェック */
    if ( pNode == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 引数pNewNodeチェック */
    if ( pNewNode == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 連結リストサイズチェック */
    if ( ( pList->size == 0 ) || ( pList->size == SIZE_MAX ) ) {
        /* サイズ不正 */
        
        return MLIB_FAILURE;
    }
    
    /* 前ノード取得 */
    pPrevNode = pNode->pPrev;
    
    /* 次ノード設定 */
    pNode->pPrev = pNewNode;
    
    /* 挿入ノード設定 */
    pNewNode->pNext = pNode;
    pNewNode->pPrev = pPrevNode;
    
    /* 前ノード有無判定 */
    if ( pPrevNode == NULL ) {
        /* 前ノード無 */
        
        /* 連結リスト先頭ノード設定 */
        pList->pHead = pNewNode;
        
    } else {
        /* 前ノード有 */
        
        /* 前ノード設定 */
        pPrevNode->pNext = pNewNode;
    }
    
    /* 連結リストサイズ設定 */
    pList->size++;
    
    return MLIB_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ノード最後尾挿入
 * @details     指定した連結リストの最後尾に新しいノードを挿入する。
 * 
 * @param[in]   *pList      連結リスト
 * @param[in]   *pNewNode   挿入ノード
 * 
 * @retval      MLIB_SUCCESS 正常終了
 * @retval      MLIB_FAILURE 異常終了
 */
/******************************************************************************/
MLibRet_t MLibBasicListInsertTail( MLibBasicList_t     *pList,
                                   MLibBasicListNode_t *pNewNode )
{
    MLibBasicListNode_t *pOldTail;  /* 最後尾ノード */
    
    /* 初期化 */
    pOldTail = NULL;
    
    /* 引数pListチェック */
    if ( pList == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 引数pNewNodeチェック */
    if ( pNewNode == NULL ) {
        /* 不正値 */
        
        return MLIB_FAILURE;
    }
    
    /* 連結リストサイズチェック */
    if ( pList->size == SIZE_MAX ) {
        /* サイズ不正 */
        
        return MLIB_FAILURE;
    }
    
    /* 旧最後尾ノード取得 */
    pOldTail = pList->pTail;
    
    /* 連結リスト最後尾ノード設定 */
    pList->pTail = pNewNode;
    
    /* 挿入ノード設定 */
    pNewNode->pNext = NULL;
    pNewNode->pPrev = pOldTail;
    
    /* 旧最後尾ノード有無判定 */
    if ( pOldTail == NULL ) {
        /* 旧最後尾ノード無 */
        
        /* 連結リスト先頭ノード設定 */
        pList->pHead = pNewNode;
        
    } else {
        /* 旧最後尾ノード有 */
        
        /* 旧最後尾ノード設定 */
        pOldTail->pNext = pNewNode;
    }
    
    /* 連結リストサイズ設定 */
    pList->size++;
    
    return MLIB_SUCCESS;
}


/******************************************************************************/
