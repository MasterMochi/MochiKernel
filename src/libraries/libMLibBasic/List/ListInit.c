/******************************************************************************/
/* src/libraries/libMLibBasic/List/ListInit.c                                 */
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
 * @brief       連結リスト初期化
 * @details     連結リスト構造体の初期化を行う。
 * 
 * @param[in]   *pList 連結リスト
 * 
 * @retval      MLIB_SUCCESS 正常終了
 * @retval      MLIB_FAILURE 異常終了
 */
/******************************************************************************/
MLibRet_t MLibBasicListInit( MLibBasicList_t *pList )
{
    /* 引数チェック */
    if ( pList == NULL ) {
        /* NULL */
        
        return MLIB_FAILURE;
    }
    
    /* 連結リスト初期化 */
    pList->pHead = NULL;
    pList->pTail = NULL;
    pList->size  = 0;
    
    return MLIB_SUCCESS;
}


/******************************************************************************/
