/******************************************************************************/
/* src/include/kernel/taskname.h                                              */
/*                                                                 2019/01/26 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/******************************************************************************/
#ifndef _MK_TASKNAME_H_
#define _MK_TASKNAME_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* 共通ヘッダ */
#include <kernel/config.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 機能ID */
#define MK_TASKNAME_FUNCID_GET        ( 0x00000001 )    /**< タスクID取得     */
#define MK_TASKNAME_FUNCID_UNREGISTER ( 0x00000002 )    /**< タスク名登録解除 */
#define MK_TASKNAME_FUNCID_REGISTER   ( 0x00000003 )    /**< タスク名登録     */

/* 戻り値 */
#define MK_TASKNAME_RET_FAILURE ( -1 )   /**< 失敗 */
#define MK_TASKNAME_RET_SUCCESS (  0 )   /**< 成功 */

/* エラー番号 */
#define MK_TASKNAME_ERR_NONE         ( 0x00000000 )     /**< エラー無し   */
#define MK_TASKNAME_ERR_PARAM_FUNCID ( 0x00000001 )     /**< 機能ID不正   */
#define MK_TASKNAME_ERR_PARAM_NAME   ( 0x00000002 )     /**< タスク名不正 */
#define MK_TASKNAME_ERR_UNAUTHORIZED ( 0x00000003 )     /**< 権限無し     */
#define MK_TASKNAME_ERR_REGISTERED   ( 0x00000004 )     /**< 登録済み     */
#define MK_TASKNAME_ERR_NOREGISTERED ( 0x00000006 )     /**< 登録無し     */
#define MK_TASKNAME_ERR_FULL         ( 0x00000005 )     /**< 登録上限     */

/** タスク名管理機能パラメータ */
typedef struct {
    uint32_t   funcId;      /**< 機能ID     */
    uint32_t   errNo;       /**< エラー番号 */
    int32_t    ret;         /**< 戻り値     */
    MkTaskId_t taskId;      /**< タスクID   */
    char       *pTaskName;  /**< タスク名　 */
} MkTaskNameParam_t;


/******************************************************************************/
#endif
