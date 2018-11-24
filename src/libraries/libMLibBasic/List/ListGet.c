/******************************************************************************/
/* src/libraries/libMLibBasic/List/ListGet.c                                  */
/*                                                                 2018/11/24 */
/* Copyright (C) 2017-2018 Mochi.                                             */
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
 * @brief       次ノード取得
 * @details     指定したノードの次のノードを連結リストから削除せずに取得する。
 *              ノードを指定しない場合は先頭ノードを取得する。
 * 
 * @param[in]   *pList 連結リスト
 * @param[in]   *pNode ノード
 * 
 * @return      次ノードを返す。
 * @retval      NULL     ノード無
 * @retval      NULL以外 ノード有
 */
/******************************************************************************/
MLibBasicListNode_t *MLibBasicListGetNextNode( MLibBasicList_t     *pList,
                                               MLibBasicListNode_t *pNode  )
{
    MLibBasicListNode_t *pRet;  /* 戻り値 */
    
    /* 初期化 */
    pRet = NULL;
    
    /* 引数チェック */
    if ( pList == NULL ) {
        /* 不正 */
        
        return NULL;
    }
    
    /* ノード指定判定 */
    if ( pNode == NULL ) {
        /* 指定無 */
        
        /* 先頭ノード返却 */
        pRet = pList->pHead;
        
    } else {
        /* 指定有 */
        
        /* 次ノード返却 */
        pRet = pNode->pNext;
    }
    
    return pRet;
}


/******************************************************************************/
/**
 * @brief       前ノード取得
 * @details     指定したノードの前のノードを連結リストから削除せずに取得する。
 *              ノーを指定しない場合は最後尾ノードを取得する。
 * 
 * @param[in]   *pList 連結リスト
 * @param[in]   *pNode ノード
 * 
 * @return      前ノードを返す。
 * @retval      NULL     ノード無
 * @retval      NULL以外 ノード有
 */
/******************************************************************************/
MLibBasicListNode_t *MLibBasicListGetPrevNode( MLibBasicList_t     *pList,
                                               MLibBasicListNode_t *pNode  )
{
    MLibBasicListNode_t *pRet;  /* 戻り値 */
    
    /* 初期化 */
    pRet = NULL;
    
    /* 引数チェック */
    if ( pList == NULL ) {
        /* 不正 */
        
        return NULL;
    }
    
    /* ノード指定判定 */
    if ( pNode == NULL ) {
        /* 指定無 */
        
        /* 最後尾ノード返却 */
        pRet = pList->pTail;
        
    } else {
        /* 指定有 */
        
        /* 前ノード返却 */
        pRet = pNode->pPrev;
    }
    
    return pRet;
}


/******************************************************************************/
