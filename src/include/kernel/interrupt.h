/******************************************************************************/
/* src/include/kernel/interrupt.h                                             */
/*                                                                 2018/06/12 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef _MK_INTERRUPT_H_
#define _MK_INTERRUPT_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <kernel/config.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 機能ID */
#define MK_INTERRUPT_FUNCID_START_MONITORING ( 0x00000001 ) /** ハードウェア割込み監視開始   */
#define MK_INTERRUPT_FUNCID_STOP_MONITORING  ( 0x00000002 ) /** ハードウェア割込み監視停止   */
#define MK_INTERRUPT_FUNCID_WAIT             ( 0x00000003 ) /** ハードウェア割込み待ち合わせ */
#define MK_INTERRUPT_FUNCID_COMPLETE         ( 0x00000004 ) /** ハードウェア割込み完了       */
#define MK_INTERRUPT_FUNCID_ENABLE           ( 0x00000005 ) /** ハードウェア割込み有効化     */
#define MK_INTERRUPT_FUNCID_DISABLE          ( 0x00000006 ) /** ハードウェア割込み無効化     */

/* エラー番号 */
#define MK_INTERRUPT_ERR_NONE                ( 0x00000000 ) /** エラー無し   */
#define MK_INTERRUPT_ERR_PARAM_FUNCID        ( 0x00000001 ) /** 機能ID不正   */
#define MK_INTERRUPT_ERR_PARAM_IRQNO         ( 0x00000002 ) /** IRQ番号不正  */
#define MK_INTERRUPT_ERR_UNAUTHORIZED        ( 0x00000003 ) /** 権限無し     */
#define MK_INTERRUPT_ERR_ALREADY_START       ( 0x00000004 ) /** 監視開始済み */

/* 戻り値 */
#define MK_INTERRUPT_RET_FAILURE             ( -1 )         /** 失敗 */
#define MK_INTERRUPT_RET_SUCCESS             (  0 )         /** 成功 */

/** ハードウェア割込み制御パラメータ */
typedef struct {
    uint32_t funcId;        /**< 機能ID           */
    uint32_t errNo;         /**< エラー番号       */
    int32_t  ret;           /**< 戻り値           */
    union {
        uint32_t irqNo;     /**< IRQ番号          */
        uint16_t flag;      /**< 割込み発生フラグ */
    };
} MkInterruptParam_t;


/******************************************************************************/
#endif