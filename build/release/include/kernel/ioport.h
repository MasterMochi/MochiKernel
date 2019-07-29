/******************************************************************************/
/*                                                                            */
/* kernel/ioport.h                                                            */
/*                                                                 2019/07/27 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef __KERNEL_IOPORT_H__
#define __KERNEL_IOPORT_H__
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
/* I/Oポート制御割込み番号 */
#define MK_IOPORT_INTNO MK_CONFIG_INTNO_IOPORT

/* 機能ID */
#define MK_IOPORT_FUNCID_IN_BYTE   ( 0x00000001 )   /**< I/Oポート入力(1byte) */
#define MK_IOPORT_FUNCID_IN_WORD   ( 0x00000002 )   /**< I/Oポート入力(2byte) */
#define MK_IOPORT_FUNCID_IN_DWORD  ( 0x00000003 )   /**< I/Oポート入力(4byte) */
#define MK_IOPORT_FUNCID_OUT_BYTE  ( 0x00000004 )   /**< I/Oポート出力(1byte) */
#define MK_IOPORT_FUNCID_OUT_WORD  ( 0x00000005 )   /**< I/Oポート出力(2byte) */
#define MK_IOPORT_FUNCID_OUT_DWORD ( 0x00000006 )   /**< I/Oポート出力(4byte) */

/** I/Oポート制御パラメータ */
typedef struct {
    uint32_t funcId;    /**< 機能ID        */
    MkRet_t  ret;       /**< 戻り値        */
    MkErr_t  err;       /**< エラー内容    */
    uint16_t portNo;    /**< I/Oポート番号 */
    void     *pData;    /**< データ        */
    size_t   count;     /**< カウント      */
} MkIoPortParam_t;


/******************************************************************************/
#endif
