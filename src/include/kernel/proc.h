/******************************************************************************/
/* src/include/kernel/proc.h                                                  */
/*                                                                 2018/12/31 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef _MK_PROC_H_
#define _MK_PROC_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 機能ID */
#define MK_PROC_FUNCID_SET_BREAKPOINT ( 0x00000001 )    /**< ブレイクポイント設定 */

/* エラー番号 */
#define MK_PROC_ERR_NONE         ( 0x00000000 ) /**< エラー無し */
#define MK_PROC_ERR_PARAM_FUNCID ( 0x00000001 ) /**< 機能ID不正 */
#define MK_PROC_ERR_NO_MEMORY    ( 0x00000002 ) /**< メモリ不足 */

/* 戻り値 */
#define MK_PROC_RET_FAILURE ( -1 )  /**< 失敗 */
#define MK_PROC_RET_SUCCESS (  0 )  /**< 成功 */

/** プロセス管理パラメータ */
typedef struct {
    uint32_t funcId;        /**< 機能ID           */
    uint32_t errNo;         /**< エラー番号       */
    int32_t  ret;           /**< 戻り値           */
    void     *pBreakPoint;  /**< ブレイクポイント */
    int32_t  quantity;      /**< 増減量           */
} MkProcParam_t;


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/* ブレイクポイント設定 */
extern void *MkProcSetBreakPoint( int32_t  quantity,
                                  uint32_t *pErrNo   );


/******************************************************************************/
#endif
