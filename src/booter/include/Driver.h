/******************************************************************************/
/* src/booter/include/Driver.h                                                */
/*                                                                 2017/06/26 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef DRIVER_H
#define DRIVER_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 外部モジュールヘッダ */
#include <Cmn.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/*--------------*
 * DriverInit.c *
 *--------------*/
/* ドライバ初期化 */
void DriverInit( void );


/*-------------*
 * DriverA20.c *
 *-------------*/
/* A20ライン有効化 */
CmnRet_t DriverA20Enable( void );


/*-------------*
 * DriverAta.c *
 *-------------*/
/* ATA割込みハンドラ */
void DriverAtaHandler( uint32_t intNo );

/* ディスク読み込み */
CmnRet_t DriverAtaRead( void     *pAddr,
                        uint32_t lba,
                        size_t   size    );


/******************************************************************************/
#endif