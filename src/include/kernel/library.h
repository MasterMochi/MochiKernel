/******************************************************************************/
/* src/include/kernel/library.h                                               */
/*                                                                 2018/08/22 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef _MK_LIBRARY_H_
#define _MK_LIBRARY_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>
#include <hardware/I8259A/I8259A.h>
#include <kernel/config.h>
#include <kernel/interrupt.h>
#include <kernel/ioport.h>
#include <kernel/message.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/*---------*/
/* MkInt.c */
/*---------*/
/* ハードウェア割込み処理完了 */
extern int32_t MkIntComplete( uint8_t irqNo,
                              uint32_t *pErrNo );

/* ハードウェア割込み無効化 */
extern int32_t MkIntDisable( uint8_t irqNo,
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
/* MkIoMem.c */
/*-----------*/
/* I/Oメモリ領域割当 */
extern void *MkIoMemAlloc( void     *pAddr,
                           size_t   size,
                           uint32_t *pErrNo );

/*------------*/
/* MkIoPort.c */
/*------------*/
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

/*---------*/
/* MkMsg.c */
/*---------*/
/* メッセージ受信 */
extern int32_t MkMsgReceive( MkTaskId_t src,
                             void       *pBuffer,
                             size_t     size,
                             uint32_t   *pErrNo   );

/* メッセージ送信 */
extern int32_t MkMsgSend( MkTaskId_t dst,
                          void       *pMsg,
                          size_t     size,
                          uint32_t   *pErrNo );


/******************************************************************************/
#endif
