/******************************************************************************/
/* src/kernel/IoCtrl/IoCtrlPort.c                                             */
/*                                                                 2018/11/24 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <kernel/ioport.h>
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <TaskMng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_IOCTRL_PORT, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/** 機能ID毎関数テーブル型 */
typedef struct {
    uint32_t funcId;                                /**< 機能ID */
    void     ( *pFunc )( MkIoPortParam_t *pParam ); /**< 関数   */
} FuncTbl_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context );

/* I/Oポート入力(1バイト単位) */
static void InByte( MkIoPortParam_t *pParam );

/* I/Oポート入力(2バイト単位) */
static void InWord( MkIoPortParam_t *pParam );

/* I/Oポート入力(4バイト単位) */
static void InDword( MkIoPortParam_t *pParam );

/* I/Oポート出力(1バイト単位) */
static void OutByte( MkIoPortParam_t *pParam );

/* I/Oポート出力(2バイト単位) */
static void OutWord( MkIoPortParam_t *pParam );

/* I/Oポート出力(4バイト単位) */
static void OutDword( MkIoPortParam_t *pParam );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 機能ID毎関数テーブル */
const static FuncTbl_t gFuncTbl[] =
    {
        { MK_IOPORT_FUNCID_IN_BYTE,   &InByte   },  /* I/Oポート入力(1バイト単位) */
        { MK_IOPORT_FUNCID_IN_WORD,   &InWord   },  /* I/Oポート入力(2バイト単位) */
        { MK_IOPORT_FUNCID_IN_DWORD,  &InDword  },  /* I/Oポート入力(4バイト単位) */
        { MK_IOPORT_FUNCID_OUT_BYTE,  &OutByte  },  /* I/Oポート出力(1バイト単位) */
        { MK_IOPORT_FUNCID_OUT_WORD,  &OutWord  },  /* I/Oポート出力(2バイト単位) */
        { MK_IOPORT_FUNCID_OUT_DWORD, &OutDword },  /* I/Oポート出力(4バイト単位) */
        { 0,                          NULL     },   /* 終端                       */
    };


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oポート制御初期化
 * @details     カーネルコール用の割込みハンドラを設定する。
 */
/******************************************************************************/
void PortInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_CONFIG_INTNO_IOPORT,       /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3   );    /* 特権レベル     */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
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
    uint32_t        idx;        /* インデックス */
    MkIoPortParam_t *pParam;    /* パラメータ   */

    /* 初期化 */
    pParam = ( MkIoPortParam_t * ) context.genReg.esi;

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        return;
    }

    /* エラー設定初期化 */
    pParam->ret   = MK_IOPORT_RET_SUCCESS;  /* 戻り値     */
    pParam->errNo = MK_IOPORT_ERR_NONE;     /* エラー番号 */

    /* 機能ID判定 */
    for ( idx = 0; gFuncTbl[ idx ].funcId != 0; idx++ ) {
        /* 機能ID判定 */
        if ( gFuncTbl[ idx ].funcId == pParam->funcId ) {
            /* 一致 */

            /* 機能呼出し */
            ( gFuncTbl[ idx ].pFunc )( pParam );

            return;
        }
    }

    /* エラー設定 */
    pParam->ret   = MK_IOPORT_RET_FAILURE;      /* 戻り値     */
    pParam->errNo = MK_IOPORT_ERR_PARAM_FUNCID; /* エラー番号 */

    return;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(1バイト単位)
 * @details     I/Oポートから1バイト分のデータを入力する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void InByte( MkIoPortParam_t *pParam )
{
    uint8_t    type;    /* プロセスタイプ */
    MkTaskId_t taskId;  /* タスクID       */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskMngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret   = MK_IOPORT_RET_FAILURE;      /* 戻り値     */
        pParam->errNo = MK_IOPORT_ERR_UNAUTHORIZED; /* エラー番号 */

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート入力 */
    /* [TODO]ストリング命令 */
    IA32InstructionInByte( pParam->pData, pParam->portNo );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(2バイト単位)
 * @details     I/Oポートから2バイト分のデータを入力する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void InWord( MkIoPortParam_t *pParam )
{
    uint8_t    type;    /* プロセスタイプ */
    MkTaskId_t taskId;  /* タスクID       */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskMngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret   = MK_IOPORT_RET_FAILURE;      /* 戻り値     */
        pParam->errNo = MK_IOPORT_ERR_UNAUTHORIZED; /* エラー番号 */

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート入力 */
    /* [TODO]ストリング命令 */
    IA32InstructionInWord( pParam->pData, pParam->portNo );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       I/Oポート入力(4バイト単位)
 * @details     I/Oポートから4バイト分のデータを入力する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void InDword( MkIoPortParam_t *pParam )
{
    uint8_t    type;    /* プロセスタイプ */
    MkTaskId_t taskId;  /* タスクID       */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskMngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret   = MK_IOPORT_RET_FAILURE;      /* 戻り値     */
        pParam->errNo = MK_IOPORT_ERR_UNAUTHORIZED; /* エラー番号 */

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート入力 */
    /* [TODO]ストリング命令 */
    IA32InstructionInDWord( pParam->pData, pParam->portNo );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(1バイト単位)
 * @details     I/Oポートから1バイト分のデータを出力する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void OutByte( MkIoPortParam_t *pParam )
{
    uint8_t    type;    /* プロセスタイプ */
    MkTaskId_t taskId;  /* タスクID       */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskMngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret   = MK_IOPORT_RET_FAILURE;      /* 戻り値     */
        pParam->errNo = MK_IOPORT_ERR_UNAUTHORIZED; /* エラー番号 */

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート出力 */
    /* [TODO]ストリング命令 */
    IA32InstructionOutByte( pParam->portNo, *( ( uint8_t * ) pParam->pData ) );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(2バイト単位)
 * @details     I/Oポートから2バイト分のデータを出力する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void OutWord( MkIoPortParam_t *pParam )
{
    uint8_t    type;    /* プロセスタイプ */
    MkTaskId_t taskId;  /* タスクID       */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskMngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret   = MK_IOPORT_RET_FAILURE;      /* 戻り値     */
        pParam->errNo = MK_IOPORT_ERR_UNAUTHORIZED; /* エラー番号 */

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート出力 */
    /* [TODO]ストリング命令 */
    IA32InstructionOutWord( pParam->portNo, *( ( uint16_t * ) pParam->pData ) );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       I/Oポート出力(4バイト単位)
 * @details     I/Oポートから4バイト分のデータを出力する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void OutDword( MkIoPortParam_t *pParam )
{
    uint8_t    type;    /* プロセスタイプ */
    MkTaskId_t taskId;  /* タスクID       */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskMngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret   = MK_IOPORT_RET_FAILURE;      /* 戻り値     */
        pParam->errNo = MK_IOPORT_ERR_UNAUTHORIZED; /* エラー番号 */

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート出力 */
    /* [TODO]ストリング命令 */
    IA32InstructionOutDWord( pParam->portNo, *( ( uint32_t * ) pParam->pData ) );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
