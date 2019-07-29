/******************************************************************************/
/*                                                                            */
/* kernel/taskname.h                                                          */
/*                                                                 2019/07/28 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef __KERNEL_TASKNAME_H__
#define __KERNEL_TASKNAME_H__
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* カーネルヘッダ */
#include "config.h"
#include "types.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* タスク名管理割込み番号 */
#define MK_TASKNAME_INTNO MK_CONFIG_INTNO_TASKNAME

/* 機能ID */
#define MK_TASKNAME_FUNCID_GET        ( 0x00000001 )    /**< タスクID取得     */
#define MK_TASKNAME_FUNCID_UNREGISTER ( 0x00000002 )    /**< タスク名登録解除 */
#define MK_TASKNAME_FUNCID_REGISTER   ( 0x00000003 )    /**< タスク名登録     */

/** タスク名管理機能パラメータ */
typedef struct {
    uint32_t   funcId;      /**< 機能ID     */
    MkRet_t    ret;         /**< 戻り値     */
    MkErr_t    err;         /**< エラー内容 */
    MkTaskId_t taskId;      /**< タスクID   */
    char       *pTaskName;  /**< タスク名　 */
} MkTaskNameParam_t;


/******************************************************************************/
#endif