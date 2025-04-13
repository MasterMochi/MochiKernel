/******************************************************************************/
/*                                                                            */
/* libmk.h                                                                    */
/*                                                                 2025/04/02 */
/* Copyright (C) 2018-2025 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef __LIBMK_H__
#define __LIBMK_H__
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
#include <kernel/task.h>
#include <kernel/taskname.h>
#include <kernel/timer.h>
#include <kernel/thread.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* IRQ番号 */
#define LIBMK_INT_IRQ0  (  0 ) /**< IRQ0  */
#define LIBMK_INT_IRQ1  (  1 ) /**< IRQ1  */
#define LIBMK_INT_IRQ2  (  2 ) /**< IRQ2  */
#define LIBMK_INT_IRQ3  (  3 ) /**< IRQ3  */
#define LIBMK_INT_IRQ4  (  4 ) /**< IRQ4  */
#define LIBMK_INT_IRQ5  (  5 ) /**< IRQ5  */
#define LIBMK_INT_IRQ6  (  6 ) /**< IRQ6  */
#define LIBMK_INT_IRQ7  (  7 ) /**< IRQ7  */
#define LIBMK_INT_IRQ8  (  8 ) /**< IRQ8  */
#define LIBMK_INT_IRQ9  (  9 ) /**< IRQ9  */
#define LIBMK_INT_IRQ10 ( 10 ) /**< IRQ10 */
#define LIBMK_INT_IRQ11 ( 11 ) /**< IRQ11 */
#define LIBMK_INT_IRQ12 ( 12 ) /**< IRQ12 */
#define LIBMK_INT_IRQ13 ( 13 ) /**< IRQ13 */
#define LIBMK_INT_IRQ14 ( 14 ) /**< IRQ14 */
#define LIBMK_INT_IRQ15 ( 15 ) /**< IRQ15 */

/** ハードウェア割込み用foreachマクロ */
#define LIBMK_INT_FOREACH( _LIST, _IRQNO )     \
    for ( ( _IRQNO )  = LIBMK_INT_IRQ0;        \
          ( _IRQNO ) <= LIBMK_INT_IRQ15;       \
          ( _IRQNO )++                   )     \
        if ( ( ( _LIST ) & ( 1 << ( _IRQNO ) ) ) != 0 )


/******************************************************************************/
/* ライブラリ関数プロトタイプ宣言                                             */
/******************************************************************************/
/*--------------------*/
/* ハードウェア割込み */
/*--------------------*/
/* ハードウェア割込み処理完了 */
extern MkRet_t LibMkIntComplete( uint8_t irqNo,
                                 MkErr_t *pErr  );
/* ハードウェア割込み無効化 */
extern MkRet_t LibMkIntDisable( uint8_t irqNo,
                                MkErr_t *pErr  );
/* ハードウェア割込み有効化 */
extern MkRet_t LibMkIntEnable( uint8_t irqNo,
                               MkErr_t *pErr  );
/* ハードウェア割込み監視開始 */
extern MkRet_t LibMkIntStartMonitoring( uint8_t irqNo,
                                        MkErr_t *pErr  );
/* ハードウェア割込み監視停止 */
extern MkRet_t LibMkIntStopMonitoring( uint8_t irqNo,
                                       MkErr_t *pErr  );
/* ハードウェア割込み待ち合わせ */
extern MkRet_t LibMkIntWait( uint32_t *pIntList,
                             MkErr_t  *pErr      );

/*-----------*/
/* I/Oメモリ */
/*-----------*/
/* I/Oメモリ領域割当 */
extern MkRet_t LibMkIoMemAlloc( void     *pIoAddr,
                                size_t   size,
                                void     *ppVirtAddr,
                                MkErr_t  *pErr        );

/*-----------*/
/* I/Oポート */
/*-----------*/
/* I/Oポートバルク入出力 */
extern MkRet_t LibMkIoPortBulk( MkIoPortBulk_t *pData,
                                MkErr_t        *pErr   );
/* I/Oポート入力(1byte) */
extern MkRet_t LibMkIoPortInByte( uint16_t portNo,
                                  void     *pData,
                                  size_t   count,
                                  MkErr_t  *pErr   );
/* I/Oポート入力(4byte) */
extern MkRet_t LibMkIoPortInDWord( uint16_t portNo,
                                   void     *pData,
                                   size_t   count,
                                   MkErr_t  *pErr   );
/* I/Oポート入力(2byte) */
extern MkRet_t LibMkIoPortInWord( uint16_t portNo,
                                  void     *pData,
                                  size_t   count,
                                  MkErr_t  *pErr   );
/* I/Oポート出力(1byte) */
extern MkRet_t LibMkIoPortOutByte( uint16_t portNo,
                                   void     *pData,
                                   size_t   count,
                                   MkErr_t *pErr   );
/* I/Oポート出力(4byte) */
extern MkRet_t LibMkIoPortOutDWord( uint16_t portNo,
                                    void     *pData,
                                    size_t   count,
                                    MkErr_t  *pErr   );
/* I/Oポート出力(2byte) */
extern MkRet_t LibMkIoPortOutWord( uint16_t portNo,
                                   void     *pData,
                                   size_t   count,
                                   MkErr_t  *pErr   );

/*----------------------*/
/* メッセージパッシング */
/*----------------------*/
/* メッセージ受信 */
extern MkRet_t LibMkMsgReceive( MkTaskId_t recvTaskId,
                                void       *pBuffer,
                                size_t     bufferSize,
                                MkTaskId_t *pSrcTaskId,
                                size_t     *pRecvSize,
                                uint32_t   timeout,
                                MkErr_t    *pErr        );
/* メッセージ送信(ブロッキング) */
extern MkRet_t LibMkMsgSend( MkTaskId_t dst,
                             void       *pMsg,
                             size_t     msgSize,
                             MkErr_t    *pErr    );
/* メッセージ送信(ノンブロッキング) */
extern MkRet_t LibMkMsgSendNB( MkTaskId_t dst,
                               void       *pMsg,
                               size_t     msgSize,
                               MkErr_t    *pErr    );

/*--------------*/
/* プロセス管理 */
/*--------------*/
/* プロセス複製 */
extern MkRet_t LibMkProcFork( MkPid_t *pPid,
                              MkErr_t *pErr  );
/* ブレイクポイント設定 */
extern MkRet_t LibMkProcSetBreakPoint( int32_t quantity,
                                       void    *ppBreakPoint,
                                       MkErr_t *pErr          );

/*------------*/
/* タスク管理 */
/*------------*/
/* タスクID取得 */
extern MkRet_t LibMkTaskGetId( MkTaskId_t *pTaskId,
                               MkErr_t    *pErr     );

/*----------*/
/* タスク名 */
/*----------*/
/* タスクID取得 */
extern MkRet_t LibMkTaskNameGet( char       *pTaskName,
                                 MkTaskId_t *pTaskId,
                                 MkErr_t    *pErr       );
/* タスクID登録 */
extern MkRet_t LibMkTaskNameRegister( char    *pTaskName,
                                      MkErr_t *pErr       );
/* タスクID登録解除 */
extern MkRet_t MkTaskNameUnregister( MkErr_t *pErr );

/*--------*/
/* タイマ */
/*--------*/
/* スリープ */
extern MkRet_t LibMkTimerSleep( uint32_t usec,
                                MkErr_t  *pErr );

/*----------*/
/* スレッド */
/*----------*/
extern MkRet_t LibMkThreadCreate( MkThreadFunc_t pFunc,
                                  void           *pArg,
                                  void           *pStackAddr,
                                  size_t         stackSize,
                                  MkTaskId_t     *pTaskId,
                                  MkErr_t        *pErr        );


/******************************************************************************/
#endif
