/******************************************************************************/
/* src/include/kernel/kernel.h                                                */
/*                                                                 2018/11/23 */
/* Copyright (C) 2018 Mochi                                                   */
/******************************************************************************/
#ifndef _KERNEL_H_
#define _KERNEL_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* カーネル位置 */
#define MK_ADDR_ENTRY   ( 0x00100000 )      /** エントリポイント         */
#define MK_ADDR_STACK   ( 0x03F00000 )      /** スタックアドレス         */
#define MK_ADDR_PROCIMG ( 0x04000000 )      /** プロセスイメージアドレス */
#define MK_SIZE_STACK   ( 0x00100000 )      /** スタックサイズ           */
#define MK_SIZE_KERNEL  ( MK_ADDR_STACK + \
                          MK_SIZE_STACK - \
                          MK_ADDR_ENTRY   ) /** カーネル領域サイズ       */

/* メモリ領域タイプ */
#define MK_MEM_TYPE_AVAILABLE ( 0x01 )      /** 使用可能メモリ領域   */
#define MK_MEM_TYPE_RESERVED  ( 0x02 )      /** 使用不可メモリ領域   */
#define MK_MEM_TYPE_BOOTDATA  ( 0x05 )      /** ブートデータ領域     */
#define MK_MEM_TYPE_KERNEL    ( 0x06 )      /** カーネル領域         */
#define MK_MEM_TYPE_PROCIMG   ( 0x07 )      /** プロセスイメージ領域 */

/* プロセスタイプ */
#define MK_PROC_TYPE_NONE   ( 0x00 )        /** プロセスタイプ無し */
#define MK_PROC_TYPE_KERNEL ( 0x01 )        /** カーネルプロセス   */
#define MK_PROC_TYPE_DRIVER ( 0x02 )        /** ドライバプロセス   */
#define MK_PROC_TYPE_SERVER ( 0x03 )        /** サーバプロセス     */
#define MK_PROC_TYPE_USER   ( 0x04 )        /** ユーザプロセス     */

/** メモリマップエントリ型 */
typedef struct {
    void     *pAddr;    /**< 先頭アドレス */
    size_t   size;      /**< メモリサイズ */
    uint32_t type;      /**< メモリタイプ */
} MkMemMapEntry_t;

/** カーネルイメージヘッダ型 */
typedef struct {
    char     fileName[ 256 ];   /**< ファイル名     */
    uint32_t fileSize;          /**< ファイルサイズ */
    uint8_t  fileType;          /**< ファイルタイプ */
    uint8_t  reserved[ 251 ];   /**< 予約済み領域   */
} MkImgHdr_t;


/******************************************************************************/
#endif
