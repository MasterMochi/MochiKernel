/******************************************************************************/
/* src/booter/Initctrl/InitctrlMain.s                                         */
/*                                                                 2017/03/12 */
/* Copyright (C) 2016-2017 Mochi.                                             */
/******************************************************************************/
 .intel_syntax noprefix
.code16
/******************************************************************************/
/* グローバル宣言                                                             */
/******************************************************************************/
.global InitctrlMain


/******************************************************************************/
/* 外部関数宣言                                                               */
/******************************************************************************/
.extern A20Enable
.extern LoaderKernel
.extern CpuSwitchMode


/******************************************************************************/
/* TEXTセクション                                                             */
/******************************************************************************/
.section .text

InitctrlMain:
    /* ビデオモード設定 */
    mov         ax, 0x0003
    int         0x10
    
    /* A20ライン有効化 */
    call        A20Enable
    
    /* カーネル読込み */
    call        LoaderLoadKernel
    
    /* CPUモード変更 */
    call        CpuSwitchMode
    
Stop:
    hlt
    jmp         Stop

/******************************************************************************/
