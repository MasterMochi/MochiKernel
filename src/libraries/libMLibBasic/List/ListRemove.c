/******************************************************************************/
/* src/libraries/libMLibBasic/List/ListRemove.c                               */
/*                                                                 2017/02/05 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stddef.h>
#include <MLib/MLib.h>
#include <MLib/Basic/MLibBasicList.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       指定ノード削除
 * @details     連結リストから指定したノードを削除する。
 * 
 * @param[in]   *pList 連結リスト
 * @param[in]   *pNode ノード
 * 
 * @retval      MLIB_SUCCESS 正常終了
 * @retval      MLIB_FAILURE 異常終了
 */
/******************************************************************************/
MLibRet_t MLibBasicListRemove( MLibBasicList_t     *pList,
                               MLibBasicListNode_t *pNode  )
{
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
    
    /* 連結リストサイズチェック */
    if ( pList->size == 0 ) {
        /* サイズ不正 */
        
        return MLIB_FAILURE;
    }
    
    /* ノード位置判定 */
    if ( ( pNode->pPrev == NULL ) && ( pNode->pNext == NULL ) ) {
        /* 先頭かつ最後尾ノード */
        
        /* 連結リスト先頭・最後尾ノード設定 */
        pList->pHead = NULL;
        pList->pTail = NULL;
        
    } else if ( pNode->pPrev == NULL ) {
        /* 先頭ノード */
        
        /* 連結リスト先頭ノード設定 */
        pList->pHead = pNode->pNext;
        
        /* 後ノード設定 */
        pList->pHead->pPrev = NULL;
        
    } else if ( pNode->pNext == NULL ) {
        /* 最後尾ノード */
        
        /* 連結リスト最後尾ノード設定 */
        pList->pTail = pNode->pPrev;
        
        /* 前ノード設定 */
        pList->pTail->pNext = NULL;
        
    } else {
        /* 中間ノード */
        
        /* 前後ノード設定 */
        pNode->pNext->pPrev = pNode->pPrev;
        pNode->pPrev->pNext = pNode->pNext;
    }
    
    /* 削除ノード設定 */
    pNode->pNext = NULL;
    pNode->pPrev = NULL;
    
    /* 連結リストサイズ設定 */
    pList->size--;
    
    return MLIB_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       先頭ノード削除
 * @details     連結リストの先頭ノードを削除する。
 * 
 * @param[in]   *pList 連結リスト
 * 
 * @retval      NULL     正常終了（先頭ノード）
 * @retval      NULL以外 異常終了
 */
/******************************************************************************/
MLibBasicListNode_t *MLibBasicListRemoveHead( MLibBasicList_t *pList )
{
    MLibBasicListNode_t *pNode;     /* 先頭ノード（戻り値） */
    
    /* 初期化 */
    pNode = NULL;
    
    /* 引数pListチェック */
    if ( pList == NULL ) {
        /* 不正値 */
        
        return NULL;
    }
    
    /* 引数pNodeチェック */
    if ( pNode == NULL ) {
        /* 不正値 */
        
        return NULL;
    }
    
    /* 連結リストサイズチェック */
    if ( pList->size == 0 ) {
        /* サイズ不正 */
        
        return NULL;
    }
    
    /* 先頭ノード有無判定 */
    if ( pList->pHead == NULL ) {
        /* 先頭ノード無 */
        
        return NULL;
    }
    
    /* 先頭ノード取得 */
    pNode = pList->pHead;
    
    /* 連結リスト先頭ノード設定 */
    pList->pHead = pNode->pNext;
    
    /* 次ノード有無判定 */
    if ( pNode->pNext == NULL ) {
        /* 次ノード無 */
        
        /* 連結リスト最後尾ノード設定 */
        pList->pTail = NULL;
        
    } else {
        /* 次ノード有 */
        
        /* 次ノード設定 */
        pNode->pNext->pPrev = NULL;
    }
    
    /* 連結リストサイズ設定 */
    pList->size--;
    
    /* 先頭ノード設定 */
    pNode->pNext = NULL;
    pNode->pPrev = NULL;
    
    return pNode;
}


/******************************************************************************/
/**
 * @brief       最後尾ノード削除
 * @details     連結リストの先頭ノードを削除する。
 * 
 * @param[in]   *pList 連結リスト
 * 
 * @retval      NULL     正常終了（先頭ノード）
 * @retval      NULL以外 異常終了
 */
/******************************************************************************/
MLibBasicListNode_t *MLibBasicListRemoveTail( MLibBasicList_t *pList )
{
    MLibBasicListNode_t *pNode;     /* 先頭ノード（戻り値） */
    
    /* 初期化 */
    pNode = NULL;
    
    /* 引数pListチェック */
    if ( pList == NULL ) {
        /* 不正値 */
        
        return NULL;
    }
    
    /* 引数pNodeチェック */
    if ( pNode == NULL ) {
        /* 不正値 */
        
        return NULL;
    }
    
    /* 連結リストサイズチェック */
    if ( pList->size == 0 ) {
        /* サイズ不正 */
        
        return NULL;
    }
    
    /* 最後尾ノード有無判定 */
    if ( pList->pTail == NULL ) {
        /* 最後尾ノード無 */
        
        return NULL;
    }
    
    /* 最後尾ノード取得 */
    pNode = pList->pTail;
    
    /* 連結リスト最後尾ノード設定 */
    pList->pTail = pNode->pPrev;
    
    /* 前ノード有無判定 */
    if ( pNode->pPrev == NULL ) {
        /* 前ノード無 */
        
        /* 連結リスト先頭ノード設定 */
        pList->pHead = NULL;
        
    } else {
        /* 前ノード有 */
        
        /* 前ノード設定 */
        pNode->pPrev->pNext = NULL;
    }
    
    /* 連結リストサイズ設定 */
    pList->size--;
    
    /* 最後尾ノード設定 */
    pNode->pNext = NULL;
    pNode->pPrev = NULL;
    
    return pNode;
}


/******************************************************************************/
