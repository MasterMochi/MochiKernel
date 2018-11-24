/******************************************************************************/
/* src/include/kernel/ioport.h                                                */
/*                                                                 2018/11/24 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef _MK_IOPORT_H_
#define _MK_IOPORT_H_
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
#define MK_IOPORT_FUNCID_IN_BYTE   ( 0x00000001 )   /**< I/Oポート入力(1バイト単位) */
#define MK_IOPORT_FUNCID_IN_WORD   ( 0x00000002 )   /**< I/Oポート入力(2バイト単位) */
#define MK_IOPORT_FUNCID_IN_DWORD  ( 0x00000003 )   /**< I/Oポート入力(4バイト単位) */
#define MK_IOPORT_FUNCID_OUT_BYTE  ( 0x00000004 )   /**< I/Oポート出力(1バイト単位) */
#define MK_IOPORT_FUNCID_OUT_WORD  ( 0x00000005 )   /**< I/Oポート出力(2バイト単位) */
#define MK_IOPORT_FUNCID_OUT_DWORD ( 0x00000006 )   /**< I/Oポート出力(4バイト単位) */

/* エラー番号 */
#define MK_IOPORT_ERR_NONE         ( 0x00000000 )   /**< エラー無し */
#define MK_IOPORT_ERR_PARAM_FUNCID ( 0x00000001 )   /**< 機能ID不正 */
#define MK_IOPORT_ERR_UNAUTHORIZED ( 0x00000002 )   /**< 権限無し   */

/* 戻り値 */
#define MK_IOPORT_RET_FAILURE      ( -1 )           /**< 失敗 */
#define MK_IOPORT_RET_SUCCESS      (  0 )           /**< 成功 */

/** メッセージパッシングパラメータ */
typedef struct {
    uint32_t funcId;    /**< 機能ID        */
    uint32_t errNo;     /**< エラー番号    */
    int32_t  ret;       /**< 戻り値        */
    uint16_t portNo;    /**< I/Oポート番号 */
    void     *pData;    /**< データ        */
    size_t   count;     /**< カウント      */
} MkIoPortParam_t;


/******************************************************************************/
#endif
