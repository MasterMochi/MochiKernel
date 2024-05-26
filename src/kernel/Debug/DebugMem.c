/******************************************************************************/
/*                                                                            */
/* src/kernel/Debug/DebugMem.c                                                */
/*                                                                 2023/01/04 */
/* Copyright (C) 2023 Mochi.                                                  */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* 共通ヘッダ */
#include <memmap.h>
#include <hardware/IA32/IA32Instruction.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 起動フラグ */
#define MEM_BOOTFLAG_INIT ( 0x00000000 )    /* 起動フラグ初期値       */
#define MEM_BOOTFLAG_FAIL ( 0xDEADBEAF )    /* 起動フラグ異常値       */
#define MEM_BOOTFLAG_HANG ( 0xDEADDEAD )    /* 起動フラグハングアップ */

/** ログ出力領域サイズ */
#define MEM_SIZE_LOG ( MEMMAP_PSIZE_DEBUG - 4 )

/** デバッグ用メモリ領域型 */
typedef struct {
    uint32_t bootFlag;              /**< 起動フラグ   */
    uint8_t  log[ MEM_SIZE_LOG ];   /**< ログ出力領域 */
} memArea_t;

/** ログ型 */
typedef struct {
    uint32_t logNo;     /**< ログ番号     */
    uint16_t moduleId;  /**< モジュールID */
    uint16_t lineNo;    /**< 行番号       */
    char     str[ 0 ];  /**< 文字列       */
} memLog_t;

/** 管理データ型 */
typedef struct {
    uint32_t idx;   /**< 書込み先インデックス */
    uint32_t logNo; /**< ログ番号             */
} memData_t;


/******************************************************************************/
/* 静的グローバル変数定義                                                     */
/******************************************************************************/
/* デバッグ用メモリ領域 */
static memArea_t *pgMemArea = ( memArea_t * ) MEMMAP_PADDR_DEBUG;

/* 管理データ */
static memData_t gMemData;


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ出力初期化
 * @details     初期起動時または通常再起動時の場合はデバッグ出力用メモリ領域を
 *              初期化する。異常再起動時はハングアップする。
 */
/******************************************************************************/
void DebugMemInit( void )
{
    /* 起動フラグ判定 */
    if ( pgMemArea->bootFlag != MEM_BOOTFLAG_FAIL ) {
        /* 通常起動 */

        /* 起動フラグ設定 */
        pgMemArea->bootFlag = MEM_BOOTFLAG_FAIL;

        /* ログ領域0初期化 */
        memset( pgMemArea->log, 0, MEM_SIZE_LOG );

        /* 管理データ初期化 */
        memset( &gMemData, 0, sizeof ( memData_t ) );

    } else {
        /* 異常再起動 */

        /* 起動フラグ設定 */
        pgMemArea->bootFlag = MEM_BOOTFLAG_HANG;

        /* ハングアップ */
        while ( true )  {
            IA32InstructionHlt();
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ログ出力
 * @details     デバッグ用メモリ領域にログ出力を行う。
 *
 * @param[in]   moduleId モジュールID
 * @param[in]   lineNo   行番号
 * @param[in]   *pStr    文字列
 */
/******************************************************************************/
void DebugMemOutput( uint16_t   moduleId,
                     uint16_t   lineNo,
                     const char *pStr     )
{
    size_t   size;  /* 書込みサイズ */
    uint32_t i;     /* インデックス */
    memLog_t *pLog; /* ログ         */

    /* 初期化 */
    size = 0;
    i    = 0;
    pLog = ( memLog_t * ) &( pgMemArea->log[ gMemData.idx ] );

    /* 書込みサイズ計算 */
    size  = sizeof ( memLog_t );
    size += strlen( pStr ) + 1;

    /* ログ領域オーバラップ判定 */
    if ( ( gMemData.idx + size ) >= MEM_SIZE_LOG ) {
        /* オーバラップ */

        /* 残りログ領域を0埋め */
        for ( i = gMemData.idx; i < MEM_SIZE_LOG; i++ ) {
            pgMemArea->log[ i ] = 0;
        }

        /* 書込み先インデックス初期化 */
        gMemData.idx = 0;
    }

    /* メモリ書込み */
    pLog->logNo    = gMemData.logNo;
    pLog->moduleId = moduleId;
    pLog->lineNo   = lineNo;
    strcpy( pLog->str, pStr );

    /* 管理データ更新 */
    gMemData.idx += size;
    gMemData.logNo++;

    return;
}


/******************************************************************************/
