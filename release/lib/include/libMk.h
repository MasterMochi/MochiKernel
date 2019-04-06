/******************************************************************************/
/*                                                                            */
/* libMk.h                                                                    */
/*                                                                 2019/04/06 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef __LIB_MK_H__
#define __LIB_MK_H__
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* カーネルヘッダ */
#include <kernel/config.h>
#include <kernel/interrupt.h>
#include <kernel/iomem.h>
#include <kernel/ioport.h>
#include <kernel/message.h>
#include <kernel/taskname.h>
#include <kernel/timer.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* IRQ番号 */
#define MK_INT_IRQ0  (  0 ) /**< IRQ0  */
#define MK_INT_IRQ1  (  1 ) /**< IRQ1  */
#define MK_INT_IRQ2  (  2 ) /**< IRQ2  */
#define MK_INT_IRQ3  (  3 ) /**< IRQ3  */
#define MK_INT_IRQ4  (  4 ) /**< IRQ4  */
#define MK_INT_IRQ5  (  5 ) /**< IRQ5  */
#define MK_INT_IRQ6  (  6 ) /**< IRQ6  */
#define MK_INT_IRQ7  (  7 ) /**< IRQ7  */
#define MK_INT_IRQ8  (  8 ) /**< IRQ8  */
#define MK_INT_IRQ9  (  9 ) /**< IRQ9  */
#define MK_INT_IRQ10 ( 10 ) /**< IRQ10 */
#define MK_INT_IRQ11 ( 11 ) /**< IRQ11 */
#define MK_INT_IRQ12 ( 12 ) /**< IRQ12 */
#define MK_INT_IRQ13 ( 13 ) /**< IRQ13 */
#define MK_INT_IRQ14 ( 14 ) /**< IRQ14 */
#define MK_INT_IRQ15 ( 15 ) /**< IRQ15 */


/******************************************************************************/
/* ライブラリ関数プロトタイプ宣言                                             */
/******************************************************************************/
/*--------------------*/
/* ハードウェア割込み */
/*--------------------*/
/* ハードウェア割込み処理完了 */
extern int32_t MkIntComplete( uint8_t  irqNo,
                              uint32_t *pErrNo );
/* ハードウェア割込み無効化 */
extern int32_t MkIntDisable( uint8_t  irqNo,
                             uint32_t *pErrNo );
/* ハードウェア割込み有効化 */
extern int32_t MkIntEnable( uint8_t  irqNo,
                            uint32_t *pErrNo );
/* ハードウェア割込み監視開始 */
extern int32_t MkIntStartMonitoring( uint8_t  irqNo,
                                     uint32_t *pErrNo );
/* ハードウェア割込み監視停止 */
extern int32_t MkIntStopMonitoring( uint8_t  irqNo,
                                    uint32_t *pErrNo );
/* ハードウェア割込み待ち合わせ */
extern int32_t MkIntWait( uint8_t  *pInt,
                          uint32_t *pErrNo );

/*-----------*/
/* I/Oメモリ */
/*-----------*/
/* I/Oメモリ領域割当 */
extern void *MkIoMemAlloc( void     *pAddr,
                           size_t   size,
                           uint32_t *pErrNo );

/*-----------*/
/* I/Oポート */
/*-----------*/
/* I/Oポート入力(1バイト単位) */
extern int32_t MkIoPortInByte( uint16_t portNo,
                               void     *pData,
                               size_t   count,
                               uint32_t *pErrNo );
/* I/Oポート入力(4バイト単位) */
extern int32_t MkIoPortInDWord( uint16_t portNo,
                                void     *pData,
                                size_t   count,
                                uint32_t *pErrNo );
/* I/Oポート入力(2バイト単位) */
extern int32_t MkIoPortInWord( uint16_t portNo,
                               void     *pData,
                               size_t   count,
                               uint32_t *pErrNo );
/* I/Oポート出力(1バイト単位) */
extern int32_t MkIoPortOutByte( uint16_t portNo,
                                void     *pData,
                                size_t   count,
                                uint32_t *pErrNo );
/* I/Oポート出力(4バイト単位) */
extern int32_t MkIoPortOutDWord( uint16_t portNo,
                                 void     *pData,
                                 size_t   count,
                                 uint32_t *pErrNo );
/* I/Oポート出力(2バイト単位) */
extern int32_t MkIoPortOutWord( uint16_t portNo,
                                void     *pData,
                                size_t   count,
                                uint32_t *pErrNo );

/*----------------------*/
/* メッセージパッシング */
/*----------------------*/
/* メッセージ受信 */
extern int32_t MkMsgReceive( MkTaskId_t rcvTaskId,
                             void       *pBuffer,
                             size_t     size,
                             MkTaskId_t *pSrcTaskId,
                             uint32_t   *pErrNo      );
/* メッセージ送信 */
extern int32_t MkMsgSend( MkTaskId_t dst,
                          void       *pMsg,
                          size_t     size,
                          uint32_t   *pErrNo );

/*----------*/
/* タスク名 */
/*----------*/
/* タスクID取得 */
extern int32_t MkTaskNameGet( char       *pTaskName,
                              MkTaskId_t *pTaskId,
                              uint32_t   *pErrNo     );
/* タスクID登録 */
extern int32_t MkTaskNameRegister( char     *pTaskName,
                                   uint32_t *pErrNo     );
/* タスクID登録解除 */
extern int32_t MkTaskNameUnregister( uint32_t *pErrNo );

/*--------*/
/* タイマ */
/*--------*/
/* スリープ */
extern int32_t MkTimerSleep( uint32_t usec,
                             uint32_t *pErrNo );


/******************************************************************************/
#endif
