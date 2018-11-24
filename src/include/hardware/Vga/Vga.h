/******************************************************************************/
/* src/kernel/include/hardware/Vga/Vga.h                                      */
/*                                                                 2018/11/24 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef VGA_H
#define VGA_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/*------------------------------------*/
/* ビデオモード0x03（テキストモード） */
/*------------------------------------*/
/* VRAM */
#define VGA_M3_VRAM_ADDR      ( 0x000B8000 )/**< VRAM先頭アドレス */
#define VGA_M3_VRAM_SIZE      ( 0x00008000 )/**< VRAM領域サイズ   */

/* 画面文字数 */
#define VGA_M3_COLUMN         ( 80 )        /**< 最大列数 */
#define VGA_M3_ROW            ( 25 )        /**< 最大行数 */

/* 文字属性 */
#define VGA_M3_ATTR_FG_BLACK  ( 0x00 )      /**< 黒色文字属性 */
#define VGA_M3_ATTR_FG_BLUE   ( 0x01 )      /**< 青色文字属性 */
#define VGA_M3_ATTR_FG_GREEN  ( 0x02 )      /**< 緑色文字属性 */
#define VGA_M3_ATTR_FG_CYAN   ( 0x03 )      /**< 水色文字属性 */
#define VGA_M3_ATTR_FG_RED    ( 0x04 )      /**< 赤色文字属性 */
#define VGA_M3_ATTR_FG_PURPLE ( 0x05 )      /**< 紫色文字属性 */
#define VGA_M3_ATTR_FG_BROWN  ( 0x06 )      /**< 茶色文字属性 */
#define VGA_M3_ATTR_FG_WHITE  ( 0x07 )      /**< 白色文字属性 */
#define VGA_M3_ATTR_FG_BRIGHT ( 0x08 )      /**< 明色文字属性 */
#define VGA_M3_ATTR_BG_BLACK  ( 0x00 )      /**< 黒色背景属性 */
#define VGA_M3_ATTR_BG_BLUE   ( 0x10 )      /**< 青色背景属性 */
#define VGA_M3_ATTR_BG_GREEN  ( 0x20 )      /**< 緑色背景属性 */
#define VGA_M3_ATTR_BG_CYAN   ( 0x30 )      /**< 水色背景属性 */
#define VGA_M3_ATTR_BG_RED    ( 0x40 )      /**< 赤色背景属性 */
#define VGA_M3_ATTR_BG_PURPLE ( 0x50 )      /**< 紫色背景属性 */
#define VGA_M3_ATTR_BG_BROWN  ( 0x60 )      /**< 茶色背景属性 */
#define VGA_M3_ATTR_BG_WHITE  ( 0x70 )      /**< 白色背景属性 */
#define VGA_M3_ATTR_BLINK     ( 0x80 )      /**< 点滅文字属性 */


/******************************************************************************/
#endif
