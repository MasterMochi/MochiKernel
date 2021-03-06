/******************************************************************************/
/*                                                                            */
/* src/booter/Ipl/IplMain.s                                                   */
/*                                                                 2019/07/23 */
/* Copyright (C) 2016-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
.intel_syntax noprefix
.code16

/******************************************************************************/
/* グローバル宣言                                                             */
/******************************************************************************/
.global IplMain


/******************************************************************************/
/* TEXTセクション                                                             */
/******************************************************************************/
.section .text
IplMain:
    /* 初期化 */
    xor         ax, ax
    mov         ds, ax
    mov         es, ax
    mov         fs, ax
    mov         gs, ax

    /* IPL再配置 */
    mov         si, 0x7C00      /* コピー元（ds:si）   */
    mov         di, 0x7A00      /* コピー先（es:di）   */
    mov         cx, 0x200 / 4   /* コピー回数          */
    cld                         /* コピー方向（加算）  */
    rep movsd                   /* コピー実行（4byte） */

    /* 移行 */
    jmp         0x07A0:migrate
migrate:
    mov         ax, 0x07A0
    mov         ds, ax

    /* メインプログラム格納位置取得 */
    mov         eax, [ pt1StartLBA ]
    mov         [ dapReadAddr ], eax
    mov         ax, [ pt1Size ]
    mov         [ dapReadSize ], ax

    /* メインプログラム読み込み */
    mov         ah, 0x42        /* 機能番号（EXTENDED READ）   */
    mov         dl, 0x80        /* ドライブ番号                */
    mov         si, offset dap  /* Disk address packetアドレス */
    int         0x13

    /* メインプログラム実行 */
    jmp         0x0000:0x7C00

.align 8
/* Disk Address Packet */
dap:
    .byte       0x10            /* サイズ                          */
    .byte       0x00            /* Reserved                        */
dapReadSize:
    .word       0x0000          /* 読込み論理セクタ数              */
    .word       0x0000          /* 転送先アドレス（オフセット）    */
    .word       0x07C0          /* 転送先アドレス（セグメント）    */
dapReadAddr:
    .long       0x00000000      /* 読込み先頭論理セクタ番号（LSB） */
    .long       0x00000000      /* 読込み先頭論理セクタ番号（MSB） */


/******************************************************************************/
/* パーティションテーブル                                                     */
/******************************************************************************/
.org 0x01BE
/* 第1パーティション */
    .byte       0x00                /* ブートフラグ                      */
    .byte       0x00, 0x00, 0x00    /* パーティション先頭位置（CHS方式） */
    .byte       0x00                /* パーティション種別                */
    .byte       0x00, 0x00, 0x00    /* パーティション末尾位置（CHS方式） */
pt1StartLBA:
    .long       0x00000000          /* パーティション先頭位置（LBA方式） */
pt1Size:
    .long       0x00000000          /* パーティションサイズ（セクタ）    */


/******************************************************************************/
/* ブートシグネチャ                                                           */
/******************************************************************************/
.org 0x01FE
    .word       0xAA55


/******************************************************************************/
