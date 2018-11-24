/******************************************************************************/
/* src/kernel/include/IntMng.h                                                */
/*                                                                 2018/11/24 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef INTMNG_H
#define INTMNG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>
#include <hardware/IA32/IA32.h>
#include <hardware/IA32/IA32Descriptor.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 割込み番号定義 */
#define INTMNG_INT_NO_MIN    (   0 )                    /**< 割込み番号最小値 */
#define INTMNG_INT_NO_MAX    ( 255 )                    /**< 割込み番号最大値 */
#define INTMNG_INT_NO_NUM    ( INTMNG_INT_NO_MAX + 1 )  /**< 割込み番号数     */

/** PICベクタ番号ベース */
#define INTMNG_PIC_VCTR_BASE ( 0x20 )

/** セグメントレジスタ情報 */
typedef struct {
    uint32_t        ds;         /**< DSレジスタ */
    uint32_t        es;         /**< ESレジスタ */
    uint32_t        fs;         /**< FSレジスタ */
    uint32_t        gs;         /**< GSレジスタ */
} IntMngSegReg_t;

/** 割込み発生時コンテキスト */
typedef struct {
    IA32Pushad_t    genReg;     /**< 汎用レジスタ       */
    IntMngSegReg_t  segReg;     /**< セグメントレジスタ */
    IA32ErrCode_t   errCode;    /**< エラーコード       */
    IA32IretdInfo_t iretdInfo;  /**< 割込みリターン情報 */
} IntMngContext_t;

/** 割込みハンドラ関数型 */
typedef void ( *IntMngHdl_t )( uint32_t        intNo,
                               IntMngContext_t context );


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
                          IntMngHdl_t func,
                          uint8_t     level  );

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
