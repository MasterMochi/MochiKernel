/******************************************************************************/
/*                                                                            */
/* kernel/ioport.h                                                            */
/*                                                                 2025/03/30 */
/* Copyright (C) 2018-2025 Mochi.                                             */
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
#define MK_IOPORT_FUNCID_IN_BYTE   ( 0x00000001 )   /**< I/Oポート入力(1byte)  */
#define MK_IOPORT_FUNCID_IN_WORD   ( 0x00000002 )   /**< I/Oポート入力(2byte)  */
#define MK_IOPORT_FUNCID_IN_DWORD  ( 0x00000003 )   /**< I/Oポート入力(4byte)  */
#define MK_IOPORT_FUNCID_OUT_BYTE  ( 0x00000004 )   /**< I/Oポート出力(1byte)  */
#define MK_IOPORT_FUNCID_OUT_WORD  ( 0x00000005 )   /**< I/Oポート出力(2byte)  */
#define MK_IOPORT_FUNCID_OUT_DWORD ( 0x00000006 )   /**< I/Oポート出力(4byte)  */
#define MK_IOPORT_FUNCID_BULK      ( 0x00000007 )   /**< I/Oポートバルク入出力 */

/** I/Oポートバルク入出力エントリ上限 */
#define MK_IOPORT_BULK_SIZE_MAX ( 32 )

/* I/Oポートバルク入出力メソッド */
#define MK_IOPORT_BULK_METHOD_IN_8   ( 0x00000001 ) /**< I/Oポート入力(8bit)  */
#define MK_IOPORT_BULK_METHOD_IN_16  ( 0x00000002 ) /**< I/Oポート入力(16bit) */
#define MK_IOPORT_BULK_METHOD_IN_32  ( 0x00000003 ) /**< I/Oポート入力(32bit) */
#define MK_IOPORT_BULK_METHOD_OUT_8  ( 0x00000004 ) /**< I/Oポート出力(8bit)  */
#define MK_IOPORT_BULK_METHOD_OUT_16 ( 0x00000005 ) /**< I/Oポート出力(16bit) */
#define MK_IOPORT_BULK_METHOD_OUT_32 ( 0x00000006 ) /**< I/Oポート出力(32bit) */

/** I/Oポート制御パラメータ */
typedef struct {
    uint32_t funcId;    /**< 機能ID        */
    MkRet_t  ret;       /**< 戻り値        */
    MkErr_t  err;       /**< エラー内容    */
    uint16_t portNo;    /**< I/Oポート番号 */
    void     *pData;    /**< データ        */
    size_t   count;     /**< カウント      */
} MkIoPortParam_t;

/** I/Oポートバルクデータエントリ */
typedef struct {
    uint32_t method;    /**< I/Oポート制御方法  */
    uint16_t portNo;    /**< I/Oポート番号      */
    void     *pData;    /**< 入出力データ格納先 */
} MkIoPortBulkData_t;

/** I/Oポートバルクデータ */
typedef struct {
    size_t             size;    /**< 入出力数           */
    MkIoPortBulkData_t data[];  /**< バルクデータリスト */
} MkIoPortBulk_t;


/******************************************************************************/
#endif
