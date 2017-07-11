/******************************************************************************/
/* src/include/kernel/MochiKernel.h                                           */
/*                                                                 2017/07/11 */
/* Copyright (C) 2017 Mochi                                                   */
/******************************************************************************/
#ifndef _MOCHI_KERNEL_H_
#define _MOCHI_KERNEL_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stddef.h>
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* カーネル位置 */
#define MOCHIKERNEL_ADDR_ENTRY ( 0x00100000 )   /** エントリポイント */
#define MOCHIKERNEL_ADDR_STACK ( 0x04000000 )   /** スタックアドレス */

/* メモリ領域タイプ */
#define MOCHIKERNEL_MEMORY_TYPE_AVAILABLE ( 0x01 )  /** 使用可能メモリ領域 */
#define MOCHIKERNEL_MEMORY_TYPE_RESERVED  ( 0x02 )  /** 使用不可メモリ領域 */
#define MOCHIKERNEL_MEMORY_TYPE_ACPI      ( 0x03 )  /** ACPIメモリ領域     */
#define MOCHIKERNEL_MEMORY_TYPE_ACPI_NVS  ( 0x04 )  /** ACPI NVSメモリ領域 */
#define MOCHIKERNEL_MEMORY_TYPE_KERNEL    ( 0x05 )  /** MochiKernel領域    */

/** メモリマップ */
typedef struct {
    void     *pAddr;    /**< 先頭アドレス */
    size_t   size;      /**< メモリサイズ */
    uint32_t type;      /**< メモリタイプ */
} MochiKernelMemoryMap_t;


/******************************************************************************/
#endif
