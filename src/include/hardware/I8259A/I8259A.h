/******************************************************************************/
/* src/kernel/include/hardware/I8259A/I8259A.h                                */
/*                                                                 2016/12/13 */
/* Copyright (C) 2016 Mochi.                                                  */
/******************************************************************************/
#ifndef I8259A_H
#define I8259A_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* PIC1マスタI/Oポート定義 */
#define I8259A_M_PORT_ICW1 ( 0x20 ) /**< PIC1マスタICW1ポート番号 */
#define I8259A_M_PORT_ICW2 ( 0x21 ) /**< PIC1マスタICW2ポート番号 */
#define I8259A_M_PORT_ICW3 ( 0x21 ) /**< PIC1マスタICW3ポート番号 */
#define I8259A_M_PORT_ICW4 ( 0x21 ) /**< PIC1マスタICW4ポート番号 */
#define I8259A_M_PORT_OCW1 ( 0x21 ) /**< PIC1マスタOCW1ポート番号 */
#define I8259A_M_PORT_OCW2 ( 0x20 ) /**< PIC1マスタOCW2ポート番号 */
#define I8259A_M_PORT_OCW3 ( 0x20 ) /**< PIC1マスタOCW3ポート番号 */

/* PIC2スレーブI/Oポート定義 */
#define I8259A_S_PORT_ICW1 ( 0xA0 ) /**< PIC2スレーブICW1ポート番号 */
#define I8259A_S_PORT_ICW2 ( 0xA1 ) /**< PIC2スレーブICW2ポート番号 */
#define I8259A_S_PORT_ICW3 ( 0xA1 ) /**< PIC2スレーブICW3ポート番号 */
#define I8259A_S_PORT_ICW4 ( 0xA1 ) /**< PIC2スレーブICW4ポート番号 */
#define I8259A_S_PORT_OCW1 ( 0xA1 ) /**< PIC2スレーブOCW1ポート番号 */
#define I8259A_S_PORT_OCW2 ( 0xA0 ) /**< PIC2スレーブOCW2ポート番号 */
#define I8259A_S_PORT_OCW3 ( 0xA0 ) /**< PIC2スレーブOCW3ポート番号 */

/* OCW1レジスタビット定義 */
#define I8259A_OCW1_M0     ( 0x01 ) /**< OCW1レジスタIR0マスク */
#define I8259A_OCW1_M1     ( 0x02 ) /**< OCW1レジスタIR1マスク */
#define I8259A_OCW1_M2     ( 0x04 ) /**< OCW1レジスタIR2マスク */
#define I8259A_OCW1_M3     ( 0x08 ) /**< OCW1レジスタIR3マスク */
#define I8259A_OCW1_M4     ( 0x10 ) /**< OCW1レジスタIR4マスク */
#define I8259A_OCW1_M5     ( 0x20 ) /**< OCW1レジスタIR5マスク */
#define I8259A_OCW1_M6     ( 0x40 ) /**< OCW1レジスタIR6マスク */
#define I8259A_OCW1_M7     ( 0x80 ) /**< OCW1レジスタIR7マスク */

/* OCW2レジスタビット定義 */
#define I8259A_OCW2_EOI    ( 0x20 ) /**< OCW2レジスタEOIモード    */
#define I8259A_OCW2_SL     ( 0x40 ) /**< OCW2レジスタIRQ指定      */
#define I8259A_OCW2_R      ( 0x80 ) /**< OCW2レジスタROTATEモード */

/* IRQ番号定義 */
#define I8259A_IRQ0        (  0 )   /**< IRQ0  */
#define I8259A_IRQ1        (  1 )   /**< IRQ1  */
#define I8259A_IRQ2        (  2 )   /**< IRQ2  */
#define I8259A_IRQ3        (  3 )   /**< IRQ3  */
#define I8259A_IRQ4        (  4 )   /**< IRQ4  */
#define I8259A_IRQ5        (  5 )   /**< IRQ5  */
#define I8259A_IRQ6        (  6 )   /**< IRQ6  */
#define I8259A_IRQ7        (  7 )   /**< IRQ7  */
#define I8259A_IRQ8        (  8 )   /**< IRQ8  */
#define I8259A_IRQ9        (  9 )   /**< IRQ9  */
#define I8259A_IRQ10       ( 10 )   /**< IRQ10 */
#define I8259A_IRQ11       ( 11 )   /**< IRQ11 */
#define I8259A_IRQ12       ( 12 )   /**< IRQ12 */
#define I8259A_IRQ13       ( 13 )   /**< IRQ13 */
#define I8259A_IRQ14       ( 14 )   /**< IRQ14 */
#define I8259A_IRQ15       ( 15 )   /**< IRQ15 */


/******************************************************************************/
#endif
