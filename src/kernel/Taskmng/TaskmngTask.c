/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngTask.c                                           */
/*                                                                 2021/06/15 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/config.h>
#include <kernel/task.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <Memmng.h>
#include <Taskmng.h>

/* 内部モジュールヘッダ */
#include "TaskmngSched.h"
#include "TaskmngTask.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_TASK,    \
                    __LINE__,                   \
                    __VA_ARGS__              )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* タスクID取得 */
static void DoGetId( MkTaskParam_t *pParam );
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context );
/* タスク起動 */
static void Start( void );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスク存在確認
 * @details     指定したタスクIDのタスクが存在するか確認する。
 *
 * @param[in]   taskId タスクID
 *
 * @return      タスク有無を返す。
 * @retval      true  タスク有
 * @retval      false タスク無
 */
/******************************************************************************/
bool TaskmngTaskCheckExist( MkTaskId_t taskId )
{
    return ( TaskGetInfo( taskId ) != NULL );
}


/******************************************************************************/
/**
 * @brief       プロセスタイプ取得
 * @details     タスクID taskId のタスクのプロセスタイプを取得する。
 *
 * @param[in]   taskId タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 *
 * @return      プロセスタイプを返す。
 * @retval      TASKMNG_PROC_TYPE_KERNEL カーネル
 * @retval      TASKMNG_PROC_TYPE_DRIVER ドライバ
 * @retval      TASKMNG_PROC_TYPE_SERVER サーバ
 * @retval      TASKMNG_PROC_TYPE_USER   ユーザ
 */
/******************************************************************************/
uint8_t TaskmngTaskGetType( MkTaskId_t taskId )
{
    ProcInfo_t *pProcInfo;  /* プロセス管理情報 */

    /* プロセス管理情報取得 */
    pProcInfo = ProcGetInfo( MK_TASKID_TO_PID( taskId ) );

    /* 取得結果判定 */
    if ( pProcInfo == NULL ) {
        /* 失敗 */

        /* TODO */
        return TASKMNG_PROC_TYPE_KERNEL;
    }

    return pProcInfo->type;
}


/******************************************************************************/
/**
 * @brief       プロセスタイプ差取得
 * @details     タスクID taskId1 と taskId2 のタスクのプロセスタイプを比較し階
 *              層差を取得する。
 *
 * @param[in]   taskId1 タスクID
 * @param[in]   taskId2 タスクID
 *
 * @return      プロセスタイプ差を返す。
 * @retval      0 差0
 * @retval      1 差1
 * @retval      2 差2
 * @retval      3 差3
 */
/******************************************************************************/
uint8_t TaskmngTaskGetTypeDiff( MkTaskId_t taskId1,
                                MkTaskId_t taskId2  )
{
    uint8_t type1;  /* プロセスタイプ */
    uint8_t type2;  /* プロセスタイプ */

    /* プロセスタイプ取得 */
    type1 = TaskmngTaskGetType( taskId1 );
    type2 = TaskmngTaskGetType( taskId2 );

    return MLIB_UTIL_MAX( type1, type2 ) - MLIB_UTIL_MIN( type1, type2 );
}


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスク追加
 * @details     タスクのカーネルスタック領域を割当てて、タスク管理情報pTaskInfo
 *              に必要な情報を設定し、タスクをスケジューラに登録する。
 *
 * @param[in]   *pTaskInfo   タスク管理情報
 *
 * @return      追加時したタスクIDを返す。
 * @retval      MK_TASKID_NULL 失敗
 * @retval      MK_TASKID_MIN  タスクID最小値
 * @retval      MK_TASKID_MAX  タスクID最大値
 */
/******************************************************************************/
MkTaskId_t TaskAdd( TaskInfo_t *pTaskInfo )
{
    CmnRet_t ret;   /* 関数戻り値 */

    /* 初期化 */
    ret = CMN_FAILURE;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    DEBUG_LOG( "  pid=%u, tid=%u, pAddr=%010p",
               pTaskInfo->pProcInfo->pid,
               pTaskInfo->tid,
               pTaskInfo->startInfo.pEntryPoint );

    /* タスク管理情報設定 */
    pTaskInfo->taskId      = MK_TASKID_MAKE( pTaskInfo->pProcInfo->pid,
                                             pTaskInfo->tid             );
    pTaskInfo->context.eip = ( uint32_t ) &Start;
    pTaskInfo->context.esp = ( uint32_t ) pTaskInfo->kernelStack.pBottomAddr;

    /* スケジューラ追加 */
    ret = SchedAdd( pTaskInfo );

    /* 追加結果判定 */
    if ( ret == CMN_FAILURE ) {
        /* 失敗 */

        return MK_TASKID_NULL;
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%u", __func__, pTaskInfo->taskId );

    return pTaskInfo->taskId;
}


/******************************************************************************/
/**
 * @brief       タスク管理情報取得
 * @details     タスクIDに該当するタスク管理情報を取得する。
 *
 * @param[in]   taskId タスクID
 *
 * @return      タスク管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(タスク管理情報)
 */
/******************************************************************************/
TaskInfo_t *TaskGetInfo( MkTaskId_t taskId )
{
    ProcInfo_t   *pProcInfo;    /* プロセス管理情報 */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報 */

    /* 初期化 */
    pProcInfo   = NULL;
    pThreadInfo = NULL;

    /* プロセス管理情報取得 */
    pProcInfo = ProcGetInfo( MK_TASKID_TO_PID( taskId ) );

    /* 取得結果判定 */
    if ( pProcInfo == NULL ) {
        /* 失敗 */

        return NULL;
    }

    /* スレッド管理情報取得 */
    pThreadInfo = ThreadGetInfo( pProcInfo, MK_TASKID_TO_TID( taskId ) );

    return ( TaskInfo_t * ) pThreadInfo;
}


/******************************************************************************/
/**
 * @brief       タスク制御初期化
 * @details     タスク制御提供カーネルコール用の割込みハンドラを設定する。
 */
/******************************************************************************/
void TaskInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_CONFIG_INTNO_TASK,     /* 割込み番号     */
                  HdlInt,                   /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3 );  /* 特権レベル     */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスクID取得
 * @details     実行中タスクのタスクIDを取得する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void DoGetId( MkTaskParam_t *pParam )
{
    /* タスクID取得 */
    pParam->taskId = TaskmngSchedGetTaskId();

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


/******************************************************************************/
/**
 * @brief       割込みハンドラ
 * @details     機能IDから該当する機能を呼び出す。
 *
 * @param[in]   intNo   割込み番号
 * @param[in]   context 割込み発生時コンテキスト
 */
/******************************************************************************/
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context )
{
    MkTaskParam_t *pParam;  /* パラメータ */

    /* 初期化 */
    pParam = ( MkTaskParam_t * ) context.genReg.esi;

    DEBUG_LOG( "%s(): start. pParam=%p", __func__, pParam );

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        DEBUG_LOG( "%s(): end.", __func__ );

        return;
    }

    /* 機能ID判定 */
    switch ( pParam->funcId ) {
        case MK_TASK_FUNCID_GET_ID:
            /* タスクID取得 */

            DEBUG_LOG( "%s(): get.", __func__ );
            DoGetId( pParam );
            break;

        default:
            /* 不正 */

            /* エラー設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_PARAM;
    }

    DEBUG_LOG( "%s(): end. ret=%d, err=%u",
               __func__,
               pParam->ret,
               pParam->err                   );
    return;
}


/******************************************************************************/
/**
 * @brief       タスク起動開始
 * @details     タスクの起動を開始する。
 */
/******************************************************************************/
static void Start( void )
{
    void       *pEntryPoint;    /* エントリポイント         */
    void       *pStackPointer;  /* スタックポインタ         */
    uint32_t   codeSegSel;      /* コードセグメントセレクタ */
    uint32_t   dataSegSel;      /* データセグメントセレクタ */
    ProcInfo_t *pProcInfo;      /* プロセス管理情報         */
    TaskInfo_t *pTaskInfo;      /* タスク管理情報           */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 初期化 */
    pTaskInfo     = SchedGetTaskInfo();
    pProcInfo     = pTaskInfo->pProcInfo;
    pEntryPoint   = pTaskInfo->startInfo.pEntryPoint;
    pStackPointer = pTaskInfo->startInfo.pStackPointer;
    codeSegSel    = 0;
    dataSegSel    = 0;

    /* デバッグトレースログ出力 */
    DEBUG_LOG(
        "pid=%u, pEntryPoint=%p, pStackPointer=%p",
        pProcInfo->pid,
        pEntryPoint,
        pStackPointer
    );

    /* タスクタイプ判定 */
    if ( pProcInfo->type == TASKMNG_PROC_TYPE_KERNEL ) {
        /* カーネル */

        /* セグメントセレクタ設定 */
        codeSegSel = MEMMNG_SEGSEL_KERNEL_CODE;     /* コード */
        dataSegSel = MEMMNG_SEGSEL_KERNEL_DATA;     /* データ */

    } else {
        /* アプリ */

        codeSegSel = MEMMNG_SEGSEL_APL_CODE;        /* コード */
        dataSegSel = MEMMNG_SEGSEL_APL_DATA;        /* データ */
    }

    /* iretd命令用スタック設定 */
    __asm__ __volatile__ (
        "push %0\n"              /* ss     */
        "push %1\n"              /* esp    */
        "push 0x00003202\n"      /* eflags */
        "push %2\n"              /* cs     */
        "push %3\n"              /* eip    */
        :
        : "a" ( dataSegSel ),
          "b" ( ( uint32_t ) pStackPointer - 16 ),
          "c" ( codeSegSel ),
          "d" ( ( uint32_t ) pEntryPoint )
    );

    /* セグメントレジスタ設定用スタック設定 */
    __asm__ __volatile__ (
        "push eax\n"    /* gs */
        "push eax\n"    /* fs */
        "push eax\n"    /* es */
        "push eax\n"    /* ds */
    );

    /* 汎用レジスタ設定用スタック設定 */
    IA32InstructionPush( 0 );   /* eax           */
    IA32InstructionPush( 0 );   /* ecx           */
    IA32InstructionPush( 0 );   /* edx           */
    IA32InstructionPush( 0 );   /* ebx           */
    IA32InstructionPush( 0 );   /* esp( 未使用 ) */
    IA32InstructionPush( 0 );   /* ebp           */
    IA32InstructionPush( 0 );   /* esi           */
    IA32InstructionPush( 0 );   /* edi           */

    /* 汎用レジスタ設定 */
    IA32InstructionPopad();

    /* セグメントレジスタ設定 */
    IA32InstructionPopDs();
    IA32InstructionPopEs();
    IA32InstructionPopFs();
    IA32InstructionPopGs();

    /* タスクエントリポイントへ移行 */
    IA32InstructionIretd();

    /* not return */
}


/******************************************************************************/
