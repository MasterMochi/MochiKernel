/******************************************************************************/
/* src/kernel/include/IntMng.h                                                */
/*                                                                 2016/12/16 */
/* Copyright (C) 2016 Mochi.                                                  */
/******************************************************************************/
#ifndef INTMNG_H
#define INTMNG_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 割込み番号定義 */
#define INTMNG_INT_NO_MIN    (   0 )                    /** 割込み番号最小値 */
#define INTMNG_INT_NO_MAX    ( 255 )                    /** 割込み番号最大値 */
#define INTMNG_INT_NO_NUM    ( INTMNG_INT_NO_MAX + 1 )  /** 割込み番号数     */

/** PICベクタ番号ベース */
#define INTMNG_PIC_VCTR_BASE ( 0x20 )

/** 割込みハンドラ関数型 */
typedef void ( *IntMngHdl_t )( uint32_t intNo );


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/*--------------*/
/* IntMngInit.c */
/*--------------*/
/* 割込み管理初期化 */
extern void IntMngInit( void );

/*-------------*/
/* IntMngHdl.c */
/*-------------*/
/* 割込みハンドラ設定 */
extern void IntMngHdlSet( uint32_t    intNo,
                          IntMngHdl_t func   );

/*-------------*/
/* IntMngPic.c */
/*-------------*/
/* PIC割込み許可 */
extern void IntMngPicAllowIrq( uint8_t irqNo );

/* PIC割込み拒否 */
extern void IntMngPicDenyIrq( uint8_t irqNo );

/* PIC割込み無効化 */
extern void IntMngPicDisable( void );

/* PIC割込み有効化 */
extern void IntMngPicEnable( void );

/* PIC割込みEOI通知 */
extern void IntMngPicEoi( uint8_t irqNo );


/******************************************************************************/
#endif
