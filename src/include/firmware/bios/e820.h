/******************************************************************************/
/* src/kernel/include/firmware/bios/e820.h                                    */
/*                                                                 2018/11/24 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef BIOS_E820_H
#define BIOS_E820_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* メモリ領域タイプ */
#define BIOS_E820_TYPE_AVAILABLE ( 0x00000001 ) /**< 使用可能メモリ領域 */
#define BIOS_E820_TYPE_RESERVED  ( 0x00000002 ) /**< 使用不可メモリ領域 */
#define BIOS_E820_TYPE_ACPI      ( 0x00000003 ) /**< ACPIメモリ領域     */
#define BIOS_E820_TYPE_ACPI_NVS  ( 0x00000004 ) /**< ACPI NVSメモリ領域 */

/** BIOS-E820メモリマップエントリ型 */
typedef struct {
    uint32_t baseLow;       /**< メモリ領域先頭アドレス(00-31bit) */
    uint32_t baseHigh;      /**< メモリ領域先頭アドレス(32-63bit) */
    uint32_t lengthLow;     /**< メモリ領域サイズ(00-31bit)       */
    uint32_t lengthHigh;    /**< メモリ領域サイズ(32-63bit)       */
    uint32_t type;          /**< メモリ領域タイプ                 */
} BiosE820Entry_t;


/******************************************************************************/
#endif
