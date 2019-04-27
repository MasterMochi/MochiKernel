/******************************************************************************/
/*                                                                            */
/* kernel/interrupt.h                                                         */
/*                                                                 2019/04/04 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef _MK_INT_H_
#define _MK_INT_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* カーネルヘッダ */
#include "config.h"
#include "types.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 機能ID */
#define MK_INT_FUNCID_START_MONITORING ( 0x00000001 )   /**< 割込み監視開始   */
#define MK_INT_FUNCID_STOP_MONITORING  ( 0x00000002 )   /**< 割込み監視停止   */
#define MK_INT_FUNCID_WAIT             ( 0x00000003 )   /**< 割込み待ち合わせ */
#define MK_INT_FUNCID_COMPLETE         ( 0x00000004 )   /**< 割込み完了       */
#define MK_INT_FUNCID_ENABLE           ( 0x00000005 )   /**< 割込み有効化     */
#define MK_INT_FUNCID_DISABLE          ( 0x00000006 )   /**< 割込み無効化     */

/* エラー番号 */
#define MK_INT_ERR_NONE                ( 0x00000000 )   /**< エラー無し   */
#define MK_INT_ERR_PARAM_FUNCID        ( 0x00000001 )   /**< 機能ID不正   */
#define MK_INT_ERR_PARAM_IRQNO         ( 0x00000002 )   /**< IRQ番号不正  */
#define MK_INT_ERR_UNAUTHORIZED        ( 0x00000003 )   /**< 権限無し     */
#define MK_INT_ERR_ALREADY_START       ( 0x00000004 )   /**< 監視開始済み */

/* 戻り値 */
#define MK_INT_RET_FAILURE             ( -1 )           /**< 失敗 */
#define MK_INT_RET_SUCCESS             (  0 )           /**< 成功 */

/** ハードウェア割込み制御パラメータ */
typedef struct {
    uint32_t funcId;        /**< 機能ID           */
    uint32_t errNo;         /**< エラー番号       */
    int32_t  ret;           /**< 戻り値           */
    union {
        uint8_t irqNo;      /**< IRQ番号          */
        uint8_t flag;       /**< 割込み発生フラグ */
    };
} MkIntParam_t;


/******************************************************************************/
#endif
