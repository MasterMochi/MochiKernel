/******************************************************************************/
/*                                                                            */
/* src/booter/Loadmng/Loadmng.h                                               */
/*                                                                 2021/11/27 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef LOADMNG_H_INTERNAL
#define LOADMNG_H_INTERNAL
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** CYLINDER取得マクロ */
#define GET_CYLINDER( _CYLSEC )     \
    ( ( ( _CYLSEC >> 6 ) & 0x0300 ) | ( _CYLSEC & 0x00FF ) )

/** SECTOR取得マクロ */
#define GET_SECTOR( _CYLSEC ) ( ( _CYLSEC >> 8 ) & 0x3F )

/** CHSアドレス */
typedef struct {
    uint16_t cylSec;            /**< シリンダ&セクタ */
    uint8_t  head;              /**< ヘッド          */
}  __attribute__( ( packed ) ) chs_t;

/** パーティションテーブル */
typedef struct {
    uint8_t  status;            /**< ステータス           */
    chs_t    chsFirstAddr;      /**< CHS先頭アドレス      */
    uint8_t  type;              /**< パーティションタイプ */
    chs_t    chsLastAddr;       /**< CHS最後尾アドレス    */
    uint32_t lbaFirstAddr;      /**< LBA先頭アドレス      */
    uint32_t lbaSize;           /**< LBAサイズ            */
} __attribute__( ( packed ) ) pt_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** パーティションテーブル */
extern pt_t gLoadmngPt[ 4 ];


/******************************************************************************/
#endif