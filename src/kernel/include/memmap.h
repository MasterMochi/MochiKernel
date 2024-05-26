/******************************************************************************/
/*                                                                            */
/* src/kernel/include/memmap.h                                                */
/*                                                                 2023/01/04 */
/* Copyright (C) 2023 Mochi.                                                  */
/*                                                                            */
/******************************************************************************/
#ifndef MEMMAP_H
#define MEMMAP_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* カーネルヘッダ */
#include <kernel/kernel.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/*--------------------------*/
/* 物理メモリマップアドレス */
/*--------------------------*/
#define MEMMAP_PADDR_KERNEL       ( MK_ADDR_ENTRY )     /**< カーネル物理アドレス                             */
#define MEMMAP_PADDR_KERNEL_STACK ( MK_ADDR_STACK )     /**< カーネルスタック物理アドレス                     */
#define MEMMAP_PADDR_PROCIMG      ( MK_ADDR_PROCIMG )   /**< プロセスイメージ物理アドレス                     */
#define MEMMAP_PADDR_DEBUG        ( 0x05000000 )        /**< デバッグ用メモリ領域物理アドレス                 */
#define MEMMAP_PADDR_IDLE_PD      ( 0x06000000 )        /**< アイドルプロセス用ページディレクトリ物理アドレス */
#define MEMMAP_PADDR_KERNEL_PT    ( 0x06001000 )        /**< カーネル領域ページテーブル物理アドレス           */

/*------------------------*/
/* 物理メモリマップサイズ */
/*------------------------*/
#define MEMMAP_PSIZE_KERNEL       ( MK_SIZE_KERNEL )    /**< カーネル物理サイズ             */
#define MEMMAP_PSIZE_KERNEL_STACK ( MK_SIZE_STACK )     /**< カーネルスタック物理サイズ     */
#define MEMMAP_PSIZE_DEBUG        ( 0x01000000 )        /**< デバッグ用メモリ領域物理サイズ */

/*--------------------------*/
/* 仮想メモリマップアドレス */
/*--------------------------*/
#define MEMMAP_VADDR_BOOTDATA     ( 0x00000000 )    /**< ブートデータ仮想アドレス          */
#define MEMMAP_VADDR_KERNEL       ( MK_ADDR_ENTRY ) /**< カーネル領域仮想アドレス          */
#define MEMMAP_VADDR_KERNEL_STACK ( 0x3EFFBFFC )    /**< カーネルスタック仮想アドレス      */
#define MEMMAP_VADDR_KERNEL_PD1   ( 0x3EFFC000 )    /**< ページディレクトリch1仮想アドレス */
#define MEMMAP_VADDR_KERNEL_PT1   ( 0x3EFFD000 )    /**< ページテーブルch1仮想アドレス     */
#define MEMMAP_VADDR_KERNEL_PD2   ( 0x3EFFE000 )    /**< ページディレクトリch2仮想アドレス */
#define MEMMAP_VADDR_KERNEL_PT2   ( 0x3EFFF000 )    /**< ページテーブルch2仮想アドレス     */
#define MEMMAP_VADDR_KERNEL_CTRL1 ( 0x3F000000 )    /**< メモリ制御領域ch1仮想アドレス     */
#define MEMMAP_VADDR_KERNEL_CTRL2 ( 0x3F800000 )    /**< メモリ制御領域ch2仮想アドレス     */
#define MEMMAP_VADDR_USER         ( 0x40000000 )    /**< ユーザ領域仮想アドレス            */
#define MEMMAP_VADDR_USER_STACK   ( 0xBFFF8000 )    /**< ユーザスタック仮想アドレス        */

/*------------------------*/
/* 仮想メモリマップサイズ */
/*------------------------*/
#define MEMMAP_VSIZE_BOOTDATA     ( 0x00100000 )    /**< ブートデータ仮想サイズ          */
#define MEMMAP_VSIZE_KERNEL       ( 0x3FF00000 )    /**< カーネル領域仮想サイズ          */
#define MEMMAP_VSIZE_KERNEL_STACK ( 0x00002000 )    /**< カーネルスタック仮想サイズ      */
#define MEMMAP_VSIZE_KERNEL_PD1   ( 0x00001000 )    /**< ページディレクトリch1仮想サイズ */
#define MEMMAP_VSIZE_KERNEL_PT1   ( 0x00001000 )    /**< ページテーブルch1仮想サイズ     */
#define MEMMAP_VSIZE_KERNEL_PD2   ( 0x00001000 )    /**< ページディレクトリch2仮想サイズ */
#define MEMMAP_VSIZE_KERNEL_PT2   ( 0x00001000 )    /**< ページテーブルch2仮想サイズ     */
#define MEMMAP_VSIZE_KERNEL_CTRL1 ( 0x00800000 )    /**< メモリ制御領域ch1仮想サイズ     */
#define MEMMAP_VSIZE_KERNEL_CTRL2 ( 0x00800000 )    /**< メモリ制御領域ch2仮想サイズ     */
#define MEMMAP_VSIZE_USER         ( 0x80000000 )    /**< ユーザ領域仮想サイズ            */
#define MEMMAP_VSIZE_USER_STACK   ( 0x00008000 )    /**< ユーザスタック仮想アドレス      */


/******************************************************************************/
#endif
