/******************************************************************************/
/* src/include/kernel/iomem.h                                                 */
/*                                                                 2018/11/24 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef _MK_IOMEM_H_
#define _MK_IOMEM_H_
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
#define MK_IOMEM_FUNCID_ALLOC     ( 0x00000001 )    /**< I/Oメモリ領域割当 */

/* エラー番号 */
#define MK_IOMEM_ERR_NONE         ( 0x00000000 )    /**< エラー無し             */
#define MK_IOMEM_ERR_PARAM_FUNCID ( 0x00000001 )    /**< 機能ID不正             */
#define MK_IOMEM_ERR_UNAUTHORIZED ( 0x00000002 )    /**< 権限無し               */
#define MK_IOMEM_ERR_IO_ALLOC     ( 0x00000003 )    /**< I/Oメモリ領域割当失敗  */
#define MK_IOMEM_ERR_VIRT_ALLOC   ( 0x00000004 )    /**< 仮想メモリ領域割当失敗 */
#define MK_IOMEM_ERR_PAGE_SET     ( 0x00000005 )    /**< ページ設定失敗         */

/** I/Oメモリ制御機能パラメータ */
typedef struct {
    uint32_t funcId;    /**< 機能ID                    */
    uint32_t errNo;     /**< エラー番号                */
    void     *pRet;     /**< 戻り値                    */
    void     *pAddr;    /**< I/Oメモリ領域先頭アドレス */
    size_t   size;      /**< I/Oメモリ領域サイズ       */
} MkIoMemParam_t;


/******************************************************************************/
#endif
