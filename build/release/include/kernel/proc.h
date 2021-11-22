/******************************************************************************/
/*                                                                            */
/* kernel/proc.h                                                              */
/*                                                                 2021/10/25 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef __KERNEL_PROC_H__
#define __KERNEL_PROC_H__
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* カーネルヘッダ */
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** プロセス管理割込み番号 */
#define MK_PROC_INTNO MK_CONFIG_INTNO_PROC

/* 機能ID */
#define MK_PROC_FUNCID_SET_BREAKPOINT ( 0x00000001 )    /**< ブレイクポイント設定 */
#define MK_PROC_FUNCID_FORK           ( 0x00000002 )    /**< プロセス複製         */

/** プロセス管理パラメータ */
typedef struct {
    uint32_t funcId;        /**< 機能ID           */
    MkRet_t  ret;           /**< 戻り値           */
    MkErr_t  err;           /**< エラー内容       */
    void     *pBreakPoint;  /**< ブレイクポイント */
    int32_t  quantity;      /**< 増減量           */
    MkPid_t  pid;           /**< プロセスID       */
} MkProcParam_t;


/******************************************************************************/
#endif

