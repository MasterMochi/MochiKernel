/******************************************************************************/
/* src/libraries/libMLibBasic/List/ListInsert.c                               */
/*                                                                 2017/02/05 */
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
 * @brief       ノード後挿入
 * @details     指定したノードの後ろに新しいノードを挿入する。
 * 
 * @param[in]   *pList      連結リスト
 * @param[in]   *pNode      挿入先ノード
 * @param[in]   *pNewNode   挿入ノード
 * 
 * @retval      MLIB_SUCCESS 正常終了
 * @retval      MLIB_FAILURE 異常終了
 */
/******************************************************************************/
MLibRet_t MLibBasicListInsertAfter( MLibBasicList_t     *pList,
                                    MLibBasicListNode_t *pNode,
                                    MLibBasicListNode_t *pNewNode )
{
    MLibBasicListNode_t *pNextNode; /* 挿入後ノード */
    
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
    
    /* 挿入後ノード取得 */
    pNextNode = pNode->pNext;
    
    /* 挿入前ノード設定 */
    pNode->pNext = pNewNode;
    
    /* 挿入ノード設定 */
    pNewNode->pPrev = pNode;
    pNewNode->pNext = pNextNode;
    
    /* 挿入後ノード有無判定 */
    if ( pNextNode == NULL ) {
        /* 挿入後ノード無 */
        
        /* 連結リスト最後尾ノード設定 */
        pList->pTail = pNewNode;
        
    } else {
        /* 挿入後ノード有 */
        
        /* 挿入後ノード設定 */
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
MLibRet_t MLibBasicListInsertBefore( MLibBasicList_t     *pList,
                                     MLibBasicListNode_t *pNode,
                                     MLibBasicListNode_t *pNewNode )
{
    MLibBasicListNode_t *pBeforeNode; /* 挿入前ノード */
    
    /* 初期化 */
    pBeforeNode = NULL;
    
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
    
    /* 挿入前ノード取得 */
    pBeforeNode = pNode->pPrev;
    
    /* 挿入後ノード設定 */
    pNode->pPrev = pNewNode;
    
    /* 挿入ノード設定 */
    pNewNode->pNext = pNode;
    pNewNode->pPrev = pBeforeNode;
    
    /* 挿入前ノード有無判定 */
    if ( pBeforeNode == NULL ) {
        /* 挿入前ノード無 */
        
        /* 連結リスト先頭ノード設定 */
        pList->pHead = pNewNode;
        
    } else {
        /* 挿入前ノード有 */
        
        /* 挿入前ノード設定 */
        pBeforeNode->pNext = pNewNode;
    }
    
    /* 連結リストサイズ設定 */
    pList->size++;
    
    return MLIB_SUCCESS;
}


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
