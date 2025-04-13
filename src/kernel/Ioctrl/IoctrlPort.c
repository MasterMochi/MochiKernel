
/*                                                                            */
/* src/kernel/Ioctrl/IoctrlPort.c                                             */
/*                                                                 2025/04/02 */
/* Copyright (C) 2018-2025 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* カーネルヘッダ */
#include <kernel/ioport.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>
#include <Taskmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_IOCTRL_PORT

/** 機能ID毎関数テーブル型 */
typedef struct {
    uint32_t funcId;                                /**< 機能ID */
    void     ( *pFunc )( MkIoPortParam_t *pParam ); /**< 関数   */
} FuncTbl_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* I/Oポートバルク入出力 */
static void Bulk( MkIoPortParam_t *pParam );

/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntmngContext_t context );

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
        { MK_IOPORT_FUNCID_BULK,      &Bulk     },  /* I/Oポートバルク入出力      */
        { 0,                          NULL      },  /* 終端                       */
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
    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* 割込みハンドラ設定 */
    IntmngHdlSet( MK_CONFIG_INTNO_IOPORT,       /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3   );    /* 特権レベル     */

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oポートバルク入出力
 * @details     バルクデータに従ってI/Oポートにデータを入出力する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Bulk( MkIoPortParam_t *pParam )
{
    uint8_t            idx;     /* インデックス         */
    uint8_t            type;    /* プロセスタイプ       */
    MkTaskId_t         taskId;  /* タスクID             */
    MkIoPortBulk_t     *pBulk;  /* バルクデータ         */
    MkIoPortBulkData_t *pData;  /* バルクデータエントリ */

    /* 初期化 */
    pBulk = ( MkIoPortBulk_t * ) pParam->pData;
    pData = NULL;

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* 入出力数チェック */
    if ( pBulk->size > MK_IOPORT_BULK_SIZE_MAX ) {
        /* 上限超過 */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;

        return;
    }

    /* エントリ毎に繰り返し */
    for ( idx = 0; idx < pBulk->size; idx++ ) {
        pData = &( pBulk->data[ idx ] );

        /* [TODO]アクセス許可チェック */

        /* I/Oポート制御方法判定 */
        if ( pData->method == MK_IOPORT_BULK_METHOD_IN_8 ) {
            /* I/Oポート入力(8bit) */

            IA32InstructionInByte( ( uint8_t * ) pData->pData,
                                                 pData->portNo );

        } else if ( pData->method == MK_IOPORT_BULK_METHOD_IN_16 ) {
            /* I/Oポート入力(16bit) */

            IA32InstructionInWord( ( uint16_t * ) pData->pData,
                                                  pData->portNo );

        } else if ( pData->method == MK_IOPORT_BULK_METHOD_IN_32 ) {
            /* I/Oポート入力(32bit) */

            IA32InstructionInDWord( ( uint32_t * ) pData->pData,
                                                   pData->portNo );

        } else if ( pData->method == MK_IOPORT_BULK_METHOD_OUT_8 ) {
            /* I/Oポート出力(8bit) */

            IA32InstructionOutByte( pData->portNo,
                                    *( ( uint8_t * ) pData->pData ) );

        } else if ( pData->method == MK_IOPORT_BULK_METHOD_OUT_16 ) {
            /* I/Oポート出力(16bit) */

            IA32InstructionOutWord( pData->portNo,
                                    *( ( uint16_t * ) pData->pData ) );

        } else if ( pData->method == MK_IOPORT_BULK_METHOD_OUT_32 ) {
            /* I/Oポート出力(32bit) */

            IA32InstructionOutDWord( pData->portNo,
                                    *( ( uint32_t * ) pData->pData ) );
        } else {
            /* 未定義 */

            /* エラー設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_PARAM;

            return;
        }
    }

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
                    IntmngContext_t context )
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
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

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
    pParam->ret = MK_RET_FAILURE;
    pParam->err = MK_ERR_PARAM;

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

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート入力 */
    IA32InstructionRepInsb( pParam->pData, pParam->portNo, pParam->count );

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

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート入力 */
    IA32InstructionRepInsw( pParam->pData, pParam->portNo, pParam->count );

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

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート入力 */
    IA32InstructionRepInsd( pParam->pData, pParam->portNo, pParam->count );

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

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート出力 */
    IA32InstructionRepOutsb( pParam->pData, pParam->portNo, pParam->count );

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

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート出力 */
    IA32InstructionRepOutsw( pParam->pData, pParam->portNo, pParam->count );

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

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* [TODO]アクセス許可チェック */

    /* I/Oポート出力 */
    IA32InstructionRepOutsd( pParam->pData, pParam->portNo, pParam->count );

    return;
}


/******************************************************************************/
