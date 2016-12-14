/******************************************************************************/
/* src/kernel/include/hardware/IA32/IA32Tss.h                                 */
/*                                                                 2016/12/13 */
/* Copyright (C) 2016 Mochi.                                                  */
/******************************************************************************/
#ifndef IA32_TSS_H
#define IA32_TSS_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* TSS */
typedef struct {
    uint16_t preTaskLink;       /* 前のタスクへのリンク    */
    uint16_t reserved1;         /* 予約                    */
    uint32_t esp0;              /* ESP0                    */
    uint16_t ss0;               /* SS0                     */
    uint16_t reserved2;         /* 予約                    */
    uint32_t esp1;              /* ESP1                    */
    uint16_t ss1;               /* SS1                     */
    uint16_t reserved3;         /* 予約                    */
    uint32_t esp2;              /* ESP2                    */
    uint16_t ss2;               /* SS2                     */
    uint16_t reserved4;         /* 予約                    */
    uint32_t cr3;               /* CR0                     */
    uint32_t eip;               /* EIP                     */
    uint32_t eflags;            /* ELAGS                   */
    uint32_t eax;               /* eax                     */
    uint32_t ecx;               /* ecx                     */
    uint32_t edx;               /* edx                     */
    uint32_t ebx;               /* ebx                     */
    uint32_t esp;               /* esp                     */
    uint32_t ebp;               /* ebp                     */
    uint32_t esi;               /* esi                     */
    uint32_t edi;               /* edi                     */
    uint16_t es;                /* es                      */
    uint16_t reserved5;         /* 予約                    */
    uint16_t cs;                /* cs                      */
    uint16_t reserved6;         /* 予約                    */
    uint16_t ss;                /* ss                      */
    uint16_t reserved7;         /* 予約                    */
    uint16_t ds;                /* ds                      */
    uint16_t reserved8;         /* 予約                    */
    uint16_t fs;                /* fs                      */
    uint16_t reserved9;         /* 予約                    */
    uint16_t gs;                /* gs                      */
    uint16_t reserved10;        /* 予約                    */
    uint16_t ldt;               /* LDTセグメントセレクタ   */
    uint16_t reserved11;        /* 予約                    */
    uint16_t reserved12:15;     /* 予約                    */
    uint16_t t:1;               /* デバッグトラップフラグ  */
    uint16_t ioMapBase;         /* I/Oマップベースアドレス */
} IA32Tss_t;


/******************************************************************************/
#endif
