/******************************************************************************/
/*                                                                            */
/* src/booter/Intmng/IntmngHdl.c                                              */
/*                                                                 2021/11/27 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Descriptor.h>
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>

/* 内部モジュールヘッダ */
#include "IntmngIdt.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_INTMNG_HDL,  \
                    __LINE__,               \
                    __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif

/** 割込みハンドラ共通関数定義マクロ */
#define HDL_CMN_PROC( _INT_NO )                             \
    static void HdlCmnProc##_INT_NO( void )                 \
    {                                                       \
        /* コンテキスト保存 */                              \
        IA32InstructionPushDs();                            \
        IA32InstructionPushEs();                            \
        IA32InstructionPushFs();                            \
        IA32InstructionPushGs();                            \
        IA32InstructionPushad();                            \
                                                            \
        /* 割込みハンドラ呼出し */                          \
        IA32InstructionPush( _INT_NO );                     \
        IA32InstructionCall( gHdlIntProcTbl[ _INT_NO ] );   \
        IA32InstructionAddEsp( 4 );                         \
                                                            \
        /* コンテキスト復帰 */                              \
        IA32InstructionPopad();                             \
        IA32InstructionPopGs();                             \
        IA32InstructionPopFs();                             \
        IA32InstructionPopEs();                             \
        IA32InstructionPopDs();                             \
                                                            \
        /* return */                                        \
        IA32InstructionIretd();                             \
    }

/** 割込みハンドラ共通関数連続定義マクロ */
#define HDL_CMN_PROC_16X( _X )  \
    HDL_CMN_PROC( _X##0 )       \
    HDL_CMN_PROC( _X##1 )       \
    HDL_CMN_PROC( _X##2 )       \
    HDL_CMN_PROC( _X##3 )       \
    HDL_CMN_PROC( _X##4 )       \
    HDL_CMN_PROC( _X##5 )       \
    HDL_CMN_PROC( _X##6 )       \
    HDL_CMN_PROC( _X##7 )       \
    HDL_CMN_PROC( _X##8 )       \
    HDL_CMN_PROC( _X##9 )       \
    HDL_CMN_PROC( _X##A )       \
    HDL_CMN_PROC( _X##B )       \
    HDL_CMN_PROC( _X##C )       \
    HDL_CMN_PROC( _X##D )       \
    HDL_CMN_PROC( _X##E )       \
    HDL_CMN_PROC( _X##F )

/** 割込みハンドラ共通関数宣言連続定義マクロ */
#define HDL_CMN_PROC_PROTOTYPE_16X( _X )    \
    static void HdlCmnProc##_X##0( void );  \
    static void HdlCmnProc##_X##1( void );  \
    static void HdlCmnProc##_X##2( void );  \
    static void HdlCmnProc##_X##3( void );  \
    static void HdlCmnProc##_X##4( void );  \
    static void HdlCmnProc##_X##5( void );  \
    static void HdlCmnProc##_X##6( void );  \
    static void HdlCmnProc##_X##7( void );  \
    static void HdlCmnProc##_X##8( void );  \
    static void HdlCmnProc##_X##9( void );  \
    static void HdlCmnProc##_X##A( void );  \
    static void HdlCmnProc##_X##B( void );  \
    static void HdlCmnProc##_X##C( void );  \
    static void HdlCmnProc##_X##D( void );  \
    static void HdlCmnProc##_X##E( void );  \
    static void HdlCmnProc##_X##F( void );


/** 割込みハンドラ共通関数リスト連続定義マクロ */
#define HDL_CMN_PROC_LIST_16X( _X ) \
    HdlCmnProc##_X##0,              \
    HdlCmnProc##_X##1,              \
    HdlCmnProc##_X##2,              \
    HdlCmnProc##_X##3,              \
    HdlCmnProc##_X##4,              \
    HdlCmnProc##_X##5,              \
    HdlCmnProc##_X##6,              \
    HdlCmnProc##_X##7,              \
    HdlCmnProc##_X##8,              \
    HdlCmnProc##_X##9,              \
    HdlCmnProc##_X##A,              \
    HdlCmnProc##_X##B,              \
    HdlCmnProc##_X##C,              \
    HdlCmnProc##_X##D,              \
    HdlCmnProc##_X##E,              \
    HdlCmnProc##_X##F

/** 割込みハンドラ共通関数型 */
typedef void ( *hdlCmnProc_t )( void );


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* 割込みハンドラ共通関数 */
HDL_CMN_PROC_PROTOTYPE_16X( 0x0 )
HDL_CMN_PROC_PROTOTYPE_16X( 0x1 )
HDL_CMN_PROC_PROTOTYPE_16X( 0x2 )
HDL_CMN_PROC_PROTOTYPE_16X( 0x3 )
HDL_CMN_PROC_PROTOTYPE_16X( 0x4 )

/* 無視割込みハンドラ */
static void HdlIgnore( uint32_t intNo );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 割込みハンドラ共通関数テーブル */
const hdlCmnProc_t gHdlCmnProcTbl[ INTMNG_INT_NO_NUM ] = {
    HDL_CMN_PROC_LIST_16X( 0x0 ),
    HDL_CMN_PROC_LIST_16X( 0x1 ),
    HDL_CMN_PROC_LIST_16X( 0x2 ),
    HDL_CMN_PROC_LIST_16X( 0x3 ),
    HDL_CMN_PROC_LIST_16X( 0x4 )  };

/** 割込みハンドラ管理テーブル */
static IntmngHdl_t gHdlIntProcTbl[ INTMNG_INT_NO_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ハンドラ管理初期化
 * @details     ハンドラ管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IntmngHdlInit( void )
{
    uint32_t intNo;     /* 割込み番号 */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 全割込み番号毎に繰り返し */
    for ( intNo =  INTMNG_INT_NO_MIN;
          intNo <= INTMNG_INT_NO_MAX;
          intNo++ ) {
        /* 割込みハンドラ管理テーブル設定 */
        gHdlIntProcTbl[ intNo ] = HdlIgnore;

        /* IDT登録 */
        IntmngIdtSet(
            intNo,                              /* IDTエントリ番号    */
            1 * 8,                              /* セレクタ           */
            gHdlCmnProcTbl[ intNo ],            /* オフセット         */
            0,                                  /* 引数コピーカウント */
            IA32_DESCRIPTOR_TYPE_GATE32_INT,    /* タイプ             */
            IA32_DESCRIPTOR_DPL_0            ); /* 特権レベル         */
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       割込みハンドラ登録
 * @details     割込みハンドラを登録する。
 *
 * @param[in]   intNo 割込み番号
 *                  - INTMNG_INT_NO_MIN 割込み番号最小値
 *                  - INTMNG_INT_NO_MAX 割込み番号最大値
 * @param[in]   func  割込みハンドラ
 */
/******************************************************************************/
void IntmngHdlSet( uint32_t    intNo,
                   IntmngHdl_t func )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. intNo=%#X, func=%p", __func__, intNo, func );

    /* 割込みハンドラ管理テーブル設定 */
    gHdlIntProcTbl[ intNo ] = func;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/* 割込みハンドラ共通関数 */
HDL_CMN_PROC_16X( 0x0 )
HDL_CMN_PROC_16X( 0x1 )
HDL_CMN_PROC_16X( 0x2 )
HDL_CMN_PROC_16X( 0x3 )
HDL_CMN_PROC_16X( 0x4 )


/******************************************************************************/
/**
 * @brief       無視割込みハンドラ
 * @details     割込み処理として何もしない割込みハンドラ。
 *
 * @param[in]   intNo 割込み番号
 */
/******************************************************************************/
static void HdlIgnore( uint32_t intNo )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() intNo=%#X", __func__, intNo );

    while ( true ) {
        IA32InstructionHlt();
    }

    /* 何もしない */
    return;
}


/******************************************************************************/
