/******************************************************************************/
/*                                                                            */
/* src/kernel/IntMng/IntMngHdl.c                                              */
/*                                                                 2021/05/22 */
/* Copyright (C) 2016-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32.h>
#include <hardware/IA32/IA32Descriptor.h>
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>
#include <IntMng.h>
#include <Taskmng.h>

/* 内部モジュールヘッダ */
#include "IntMngIdt.h"


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

/** 割込みハンドラ共通関数定義マクロ(エラーコード無し) */
#define HDL_CMN_PROC( _INT_NO )                                                \
    static void HdlCmnProc##_INT_NO( void )                                    \
    {                                                                          \
        /* 空エラーコードプッシュ */                                           \
        __asm__ __volatile__ ( "push 0" );                                     \
                                                                               \
        /* コンテキスト保存 */                                                 \
        IA32InstructionPushDs();                                               \
        IA32InstructionPushEs();                                               \
        IA32InstructionPushFs();                                               \
        IA32InstructionPushGs();                                               \
        IA32InstructionPushad();                                               \
                                                                               \
        /* 割込みハンドラ呼出し */                                             \
        IA32InstructionPush( _INT_NO );                                        \
        IA32InstructionCall( gHdlIntProcTbl[ _INT_NO ] );                      \
        IA32InstructionAddEsp( 4 );                                            \
        /* [MEMO]                                                            */\
        /* コンパイラによって本関数用にスタック領域が確保されてスタックアド  */\
        /* レスがズレてしまい、iretd命令による割込み直前へのリターンが出来な */\
        /* くなってしまう為、C言語による関数呼出しは行わずに直接行う。       */\
                                                                               \
        /* コンテキスト復帰 */                                                 \
        IA32InstructionPopad();                                                \
        IA32InstructionPopGs();                                                \
        IA32InstructionPopFs();                                                \
        IA32InstructionPopEs();                                                \
        IA32InstructionPopDs();                                                \
                                                                               \
        /* エラーコード削除 */                                                 \
        __asm__ __volatile__ ( "add esp, 4" );                                 \
                                                                               \
        /* return */                                                           \
        IA32InstructionIretd();                                                \
    }

/** 割込みハンドラ共通関数定義マクロ(エラーコード有り) */
#define HDL_CMN_PROC_ERR( _INT_NO )                                            \
    static void HdlCmnProc##_INT_NO( void )                                    \
    {                                                                          \
        /* コンテキスト保存 */                                                 \
        IA32InstructionPushDs();                                               \
        IA32InstructionPushEs();                                               \
        IA32InstructionPushFs();                                               \
        IA32InstructionPushGs();                                               \
        IA32InstructionPushad();                                               \
                                                                               \
        /* 割込みハンドラ呼出し */                                             \
        IA32InstructionPush( _INT_NO );                                        \
        IA32InstructionCall( gHdlIntProcTbl[ _INT_NO ] );                      \
        IA32InstructionAddEsp( 4 );                                            \
        /* [MEMO]                                                            */\
        /* コンパイラによって本関数用にスタック領域が確保されてスタックアド  */\
        /* レスがズレてしまい、iretd命令による割込み直前へのリターンが出来な */\
        /* くなってしまう為、C言語による関数呼出しは行わずに直接行う。       */\
                                                                               \
        /* コンテキスト復帰 */                                                 \
        IA32InstructionPopad();                                                \
        IA32InstructionPopGs();                                                \
        IA32InstructionPopFs();                                                \
        IA32InstructionPopEs();                                                \
        IA32InstructionPopDs();                                                \
                                                                               \
        /* エラーコード削除 */                                                 \
        __asm__ __volatile__ ( "add esp, 4" );                                 \
                                                                               \
        /* return */                                                           \
        IA32InstructionIretd();                                                \
    }

/* 割込みハンドラ共通関数型 */
typedef void ( *hdlCmnProc_t )( void );


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* 割込みハンドラ共通関数 */
static void HdlCmnProc0x00( void );
static void HdlCmnProc0x01( void );
static void HdlCmnProc0x02( void );
static void HdlCmnProc0x03( void );
static void HdlCmnProc0x04( void );
static void HdlCmnProc0x05( void );
static void HdlCmnProc0x06( void );
static void HdlCmnProc0x07( void );
static void HdlCmnProc0x08( void );
static void HdlCmnProc0x09( void );
static void HdlCmnProc0x0A( void );
static void HdlCmnProc0x0B( void );
static void HdlCmnProc0x0C( void );
static void HdlCmnProc0x0D( void );
static void HdlCmnProc0x0E( void );
static void HdlCmnProc0x0F( void );
static void HdlCmnProc0x10( void );
static void HdlCmnProc0x11( void );
static void HdlCmnProc0x12( void );
static void HdlCmnProc0x13( void );
static void HdlCmnProc0x14( void );
static void HdlCmnProc0x15( void );
static void HdlCmnProc0x16( void );
static void HdlCmnProc0x17( void );
static void HdlCmnProc0x18( void );
static void HdlCmnProc0x19( void );
static void HdlCmnProc0x1A( void );
static void HdlCmnProc0x1B( void );
static void HdlCmnProc0x1C( void );
static void HdlCmnProc0x1D( void );
static void HdlCmnProc0x1E( void );
static void HdlCmnProc0x1F( void );
static void HdlCmnProc0x20( void );
static void HdlCmnProc0x21( void );
static void HdlCmnProc0x22( void );
static void HdlCmnProc0x23( void );
static void HdlCmnProc0x24( void );
static void HdlCmnProc0x25( void );
static void HdlCmnProc0x26( void );
static void HdlCmnProc0x27( void );
static void HdlCmnProc0x28( void );
static void HdlCmnProc0x29( void );
static void HdlCmnProc0x2A( void );
static void HdlCmnProc0x2B( void );
static void HdlCmnProc0x2C( void );
static void HdlCmnProc0x2D( void );
static void HdlCmnProc0x2E( void );
static void HdlCmnProc0x2F( void );
static void HdlCmnProc0x30( void );
static void HdlCmnProc0x31( void );
static void HdlCmnProc0x32( void );
static void HdlCmnProc0x33( void );
static void HdlCmnProc0x34( void );
static void HdlCmnProc0x35( void );
static void HdlCmnProc0x36( void );
static void HdlCmnProc0x37( void );
static void HdlCmnProc0x38( void );
static void HdlCmnProc0x39( void );
static void HdlCmnProc0x3A( void );
static void HdlCmnProc0x3B( void );
static void HdlCmnProc0x3C( void );
static void HdlCmnProc0x3D( void );
static void HdlCmnProc0x3E( void );
static void HdlCmnProc0x3F( void );
static void HdlCmnProc0x40( void );
static void HdlCmnProc0x41( void );
static void HdlCmnProc0x42( void );
static void HdlCmnProc0x43( void );
static void HdlCmnProc0x44( void );
static void HdlCmnProc0x45( void );
static void HdlCmnProc0x46( void );
static void HdlCmnProc0x47( void );
static void HdlCmnProc0x48( void );
static void HdlCmnProc0x49( void );
static void HdlCmnProc0x4A( void );
static void HdlCmnProc0x4B( void );
static void HdlCmnProc0x4C( void );
static void HdlCmnProc0x4D( void );
static void HdlCmnProc0x4E( void );
static void HdlCmnProc0x4F( void );
static void HdlCmnProc0x50( void );
static void HdlCmnProc0x51( void );
static void HdlCmnProc0x52( void );
static void HdlCmnProc0x53( void );
static void HdlCmnProc0x54( void );
static void HdlCmnProc0x55( void );
static void HdlCmnProc0x56( void );
static void HdlCmnProc0x57( void );
static void HdlCmnProc0x58( void );
static void HdlCmnProc0x59( void );
static void HdlCmnProc0x5A( void );
static void HdlCmnProc0x5B( void );
static void HdlCmnProc0x5C( void );
static void HdlCmnProc0x5D( void );
static void HdlCmnProc0x5E( void );
static void HdlCmnProc0x5F( void );
static void HdlCmnProc0x60( void );
static void HdlCmnProc0x61( void );
static void HdlCmnProc0x62( void );
static void HdlCmnProc0x63( void );
static void HdlCmnProc0x64( void );
static void HdlCmnProc0x65( void );
static void HdlCmnProc0x66( void );
static void HdlCmnProc0x67( void );
static void HdlCmnProc0x68( void );
static void HdlCmnProc0x69( void );
static void HdlCmnProc0x6A( void );
static void HdlCmnProc0x6B( void );
static void HdlCmnProc0x6C( void );
static void HdlCmnProc0x6D( void );
static void HdlCmnProc0x6E( void );
static void HdlCmnProc0x6F( void );
static void HdlCmnProc0x70( void );
static void HdlCmnProc0x71( void );
static void HdlCmnProc0x72( void );
static void HdlCmnProc0x73( void );
static void HdlCmnProc0x74( void );
static void HdlCmnProc0x75( void );
static void HdlCmnProc0x76( void );
static void HdlCmnProc0x77( void );
static void HdlCmnProc0x78( void );
static void HdlCmnProc0x79( void );
static void HdlCmnProc0x7A( void );
static void HdlCmnProc0x7B( void );
static void HdlCmnProc0x7C( void );
static void HdlCmnProc0x7D( void );
static void HdlCmnProc0x7E( void );
static void HdlCmnProc0x7F( void );
static void HdlCmnProc0x80( void );
static void HdlCmnProc0x81( void );
static void HdlCmnProc0x82( void );
static void HdlCmnProc0x83( void );
static void HdlCmnProc0x84( void );
static void HdlCmnProc0x85( void );
static void HdlCmnProc0x86( void );
static void HdlCmnProc0x87( void );
static void HdlCmnProc0x88( void );
static void HdlCmnProc0x89( void );
static void HdlCmnProc0x8A( void );
static void HdlCmnProc0x8B( void );
static void HdlCmnProc0x8C( void );
static void HdlCmnProc0x8D( void );
static void HdlCmnProc0x8E( void );
static void HdlCmnProc0x8F( void );
static void HdlCmnProc0x90( void );
static void HdlCmnProc0x91( void );
static void HdlCmnProc0x92( void );
static void HdlCmnProc0x93( void );
static void HdlCmnProc0x94( void );
static void HdlCmnProc0x95( void );
static void HdlCmnProc0x96( void );
static void HdlCmnProc0x97( void );
static void HdlCmnProc0x98( void );
static void HdlCmnProc0x99( void );
static void HdlCmnProc0x9A( void );
static void HdlCmnProc0x9B( void );
static void HdlCmnProc0x9C( void );
static void HdlCmnProc0x9D( void );
static void HdlCmnProc0x9E( void );
static void HdlCmnProc0x9F( void );
static void HdlCmnProc0xA0( void );
static void HdlCmnProc0xA1( void );
static void HdlCmnProc0xA2( void );
static void HdlCmnProc0xA3( void );
static void HdlCmnProc0xA4( void );
static void HdlCmnProc0xA5( void );
static void HdlCmnProc0xA6( void );
static void HdlCmnProc0xA7( void );
static void HdlCmnProc0xA8( void );
static void HdlCmnProc0xA9( void );
static void HdlCmnProc0xAA( void );
static void HdlCmnProc0xAB( void );
static void HdlCmnProc0xAC( void );
static void HdlCmnProc0xAD( void );
static void HdlCmnProc0xAE( void );
static void HdlCmnProc0xAF( void );
static void HdlCmnProc0xB0( void );
static void HdlCmnProc0xB1( void );
static void HdlCmnProc0xB2( void );
static void HdlCmnProc0xB3( void );
static void HdlCmnProc0xB4( void );
static void HdlCmnProc0xB5( void );
static void HdlCmnProc0xB6( void );
static void HdlCmnProc0xB7( void );
static void HdlCmnProc0xB8( void );
static void HdlCmnProc0xB9( void );
static void HdlCmnProc0xBA( void );
static void HdlCmnProc0xBB( void );
static void HdlCmnProc0xBC( void );
static void HdlCmnProc0xBD( void );
static void HdlCmnProc0xBE( void );
static void HdlCmnProc0xBF( void );
static void HdlCmnProc0xC0( void );
static void HdlCmnProc0xC1( void );
static void HdlCmnProc0xC2( void );
static void HdlCmnProc0xC3( void );
static void HdlCmnProc0xC4( void );
static void HdlCmnProc0xC5( void );
static void HdlCmnProc0xC6( void );
static void HdlCmnProc0xC7( void );
static void HdlCmnProc0xC8( void );
static void HdlCmnProc0xC9( void );
static void HdlCmnProc0xCA( void );
static void HdlCmnProc0xCB( void );
static void HdlCmnProc0xCC( void );
static void HdlCmnProc0xCD( void );
static void HdlCmnProc0xCE( void );
static void HdlCmnProc0xCF( void );
static void HdlCmnProc0xD0( void );
static void HdlCmnProc0xD1( void );
static void HdlCmnProc0xD2( void );
static void HdlCmnProc0xD3( void );
static void HdlCmnProc0xD4( void );
static void HdlCmnProc0xD5( void );
static void HdlCmnProc0xD6( void );
static void HdlCmnProc0xD7( void );
static void HdlCmnProc0xD8( void );
static void HdlCmnProc0xD9( void );
static void HdlCmnProc0xDA( void );
static void HdlCmnProc0xDB( void );
static void HdlCmnProc0xDC( void );
static void HdlCmnProc0xDD( void );
static void HdlCmnProc0xDE( void );
static void HdlCmnProc0xDF( void );
static void HdlCmnProc0xE0( void );
static void HdlCmnProc0xE1( void );
static void HdlCmnProc0xE2( void );
static void HdlCmnProc0xE3( void );
static void HdlCmnProc0xE4( void );
static void HdlCmnProc0xE5( void );
static void HdlCmnProc0xE6( void );
static void HdlCmnProc0xE7( void );
static void HdlCmnProc0xE8( void );
static void HdlCmnProc0xE9( void );
static void HdlCmnProc0xEA( void );
static void HdlCmnProc0xEB( void );
static void HdlCmnProc0xEC( void );
static void HdlCmnProc0xED( void );
static void HdlCmnProc0xEE( void );
static void HdlCmnProc0xEF( void );
static void HdlCmnProc0xF0( void );
static void HdlCmnProc0xF1( void );
static void HdlCmnProc0xF2( void );
static void HdlCmnProc0xF3( void );
static void HdlCmnProc0xF4( void );
static void HdlCmnProc0xF5( void );
static void HdlCmnProc0xF6( void );
static void HdlCmnProc0xF7( void );
static void HdlCmnProc0xF8( void );
static void HdlCmnProc0xF9( void );
static void HdlCmnProc0xFA( void );
static void HdlCmnProc0xFB( void );
static void HdlCmnProc0xFC( void );
static void HdlCmnProc0xFD( void );
static void HdlCmnProc0xFE( void );
static void HdlCmnProc0xFF( void );

/* 無視割込みハンドラ */
static void HdlIgnore( uint32_t        intNo,
                       IntMngContext_t context );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 割込みハンドラ共通関数テーブル */
const hdlCmnProc_t gHdlCmnProcTbl[ INTMNG_INT_NO_NUM ] = {
    &HdlCmnProc0x00, &HdlCmnProc0x01, &HdlCmnProc0x02, &HdlCmnProc0x03,
    &HdlCmnProc0x04, &HdlCmnProc0x05, &HdlCmnProc0x06, &HdlCmnProc0x07,
    &HdlCmnProc0x08, &HdlCmnProc0x09, &HdlCmnProc0x0A, &HdlCmnProc0x0B,
    &HdlCmnProc0x0C, &HdlCmnProc0x0D, &HdlCmnProc0x0E, &HdlCmnProc0x0F,
    &HdlCmnProc0x10, &HdlCmnProc0x11, &HdlCmnProc0x12, &HdlCmnProc0x13,
    &HdlCmnProc0x14, &HdlCmnProc0x15, &HdlCmnProc0x16, &HdlCmnProc0x17,
    &HdlCmnProc0x18, &HdlCmnProc0x19, &HdlCmnProc0x1A, &HdlCmnProc0x1B,
    &HdlCmnProc0x1C, &HdlCmnProc0x1D, &HdlCmnProc0x1E, &HdlCmnProc0x1F,
    &HdlCmnProc0x20, &HdlCmnProc0x21, &HdlCmnProc0x22, &HdlCmnProc0x23,
    &HdlCmnProc0x24, &HdlCmnProc0x25, &HdlCmnProc0x26, &HdlCmnProc0x27,
    &HdlCmnProc0x28, &HdlCmnProc0x29, &HdlCmnProc0x2A, &HdlCmnProc0x2B,
    &HdlCmnProc0x2C, &HdlCmnProc0x2D, &HdlCmnProc0x2E, &HdlCmnProc0x2F,
    &HdlCmnProc0x30, &HdlCmnProc0x31, &HdlCmnProc0x32, &HdlCmnProc0x33,
    &HdlCmnProc0x34, &HdlCmnProc0x35, &HdlCmnProc0x36, &HdlCmnProc0x37,
    &HdlCmnProc0x38, &HdlCmnProc0x39, &HdlCmnProc0x3A, &HdlCmnProc0x3B,
    &HdlCmnProc0x3C, &HdlCmnProc0x3D, &HdlCmnProc0x3E, &HdlCmnProc0x3F,
    &HdlCmnProc0x40, &HdlCmnProc0x41, &HdlCmnProc0x42, &HdlCmnProc0x43,
    &HdlCmnProc0x44, &HdlCmnProc0x45, &HdlCmnProc0x46, &HdlCmnProc0x47,
    &HdlCmnProc0x48, &HdlCmnProc0x49, &HdlCmnProc0x4A, &HdlCmnProc0x4B,
    &HdlCmnProc0x4C, &HdlCmnProc0x4D, &HdlCmnProc0x4E, &HdlCmnProc0x4F,
    &HdlCmnProc0x50, &HdlCmnProc0x51, &HdlCmnProc0x52, &HdlCmnProc0x53,
    &HdlCmnProc0x54, &HdlCmnProc0x55, &HdlCmnProc0x56, &HdlCmnProc0x57,
    &HdlCmnProc0x58, &HdlCmnProc0x59, &HdlCmnProc0x5A, &HdlCmnProc0x5B,
    &HdlCmnProc0x5C, &HdlCmnProc0x5D, &HdlCmnProc0x5E, &HdlCmnProc0x5F,
    &HdlCmnProc0x60, &HdlCmnProc0x61, &HdlCmnProc0x62, &HdlCmnProc0x63,
    &HdlCmnProc0x64, &HdlCmnProc0x65, &HdlCmnProc0x66, &HdlCmnProc0x67,
    &HdlCmnProc0x68, &HdlCmnProc0x69, &HdlCmnProc0x6A, &HdlCmnProc0x6B,
    &HdlCmnProc0x6C, &HdlCmnProc0x6D, &HdlCmnProc0x6E, &HdlCmnProc0x6F,
    &HdlCmnProc0x70, &HdlCmnProc0x71, &HdlCmnProc0x72, &HdlCmnProc0x73,
    &HdlCmnProc0x74, &HdlCmnProc0x75, &HdlCmnProc0x76, &HdlCmnProc0x77,
    &HdlCmnProc0x78, &HdlCmnProc0x79, &HdlCmnProc0x7A, &HdlCmnProc0x7B,
    &HdlCmnProc0x7C, &HdlCmnProc0x7D, &HdlCmnProc0x7E, &HdlCmnProc0x7F,
    &HdlCmnProc0x80, &HdlCmnProc0x81, &HdlCmnProc0x82, &HdlCmnProc0x83,
    &HdlCmnProc0x84, &HdlCmnProc0x85, &HdlCmnProc0x86, &HdlCmnProc0x87,
    &HdlCmnProc0x88, &HdlCmnProc0x89, &HdlCmnProc0x8A, &HdlCmnProc0x8B,
    &HdlCmnProc0x8C, &HdlCmnProc0x8D, &HdlCmnProc0x8E, &HdlCmnProc0x8F,
    &HdlCmnProc0x90, &HdlCmnProc0x91, &HdlCmnProc0x92, &HdlCmnProc0x93,
    &HdlCmnProc0x94, &HdlCmnProc0x95, &HdlCmnProc0x96, &HdlCmnProc0x97,
    &HdlCmnProc0x98, &HdlCmnProc0x99, &HdlCmnProc0x9A, &HdlCmnProc0x9B,
    &HdlCmnProc0x9C, &HdlCmnProc0x9D, &HdlCmnProc0x9E, &HdlCmnProc0x9F,
    &HdlCmnProc0xA0, &HdlCmnProc0xA1, &HdlCmnProc0xA2, &HdlCmnProc0xA3,
    &HdlCmnProc0xA4, &HdlCmnProc0xA5, &HdlCmnProc0xA6, &HdlCmnProc0xA7,
    &HdlCmnProc0xA8, &HdlCmnProc0xA9, &HdlCmnProc0xAA, &HdlCmnProc0xAB,
    &HdlCmnProc0xAC, &HdlCmnProc0xAD, &HdlCmnProc0xAE, &HdlCmnProc0xAF,
    &HdlCmnProc0xB0, &HdlCmnProc0xB1, &HdlCmnProc0xB2, &HdlCmnProc0xB3,
    &HdlCmnProc0xB4, &HdlCmnProc0xB5, &HdlCmnProc0xB6, &HdlCmnProc0xB7,
    &HdlCmnProc0xB8, &HdlCmnProc0xB9, &HdlCmnProc0xBA, &HdlCmnProc0xBB,
    &HdlCmnProc0xBC, &HdlCmnProc0xBD, &HdlCmnProc0xBE, &HdlCmnProc0xBF,
    &HdlCmnProc0xC0, &HdlCmnProc0xC1, &HdlCmnProc0xC2, &HdlCmnProc0xC3,
    &HdlCmnProc0xC4, &HdlCmnProc0xC5, &HdlCmnProc0xC6, &HdlCmnProc0xC7,
    &HdlCmnProc0xC8, &HdlCmnProc0xC9, &HdlCmnProc0xCA, &HdlCmnProc0xCB,
    &HdlCmnProc0xCC, &HdlCmnProc0xCD, &HdlCmnProc0xCE, &HdlCmnProc0xCF,
    &HdlCmnProc0xD0, &HdlCmnProc0xD1, &HdlCmnProc0xD2, &HdlCmnProc0xD3,
    &HdlCmnProc0xD4, &HdlCmnProc0xD5, &HdlCmnProc0xD6, &HdlCmnProc0xD7,
    &HdlCmnProc0xD8, &HdlCmnProc0xD9, &HdlCmnProc0xDA, &HdlCmnProc0xDB,
    &HdlCmnProc0xDC, &HdlCmnProc0xDD, &HdlCmnProc0xDE, &HdlCmnProc0xDF,
    &HdlCmnProc0xE0, &HdlCmnProc0xE1, &HdlCmnProc0xE2, &HdlCmnProc0xE3,
    &HdlCmnProc0xE4, &HdlCmnProc0xE5, &HdlCmnProc0xE6, &HdlCmnProc0xE7,
    &HdlCmnProc0xE8, &HdlCmnProc0xE9, &HdlCmnProc0xEA, &HdlCmnProc0xEB,
    &HdlCmnProc0xEC, &HdlCmnProc0xED, &HdlCmnProc0xEE, &HdlCmnProc0xEF,
    &HdlCmnProc0xF0, &HdlCmnProc0xF1, &HdlCmnProc0xF2, &HdlCmnProc0xF3,
    &HdlCmnProc0xF4, &HdlCmnProc0xF5, &HdlCmnProc0xF6, &HdlCmnProc0xF7,
    &HdlCmnProc0xF8, &HdlCmnProc0xF9, &HdlCmnProc0xFA, &HdlCmnProc0xFB,
    &HdlCmnProc0xFC, &HdlCmnProc0xFD, &HdlCmnProc0xFE, &HdlCmnProc0xFF  };

/** 割込みハンドラ管理テーブル */
static IntMngHdl_t gHdlIntProcTbl[ INTMNG_INT_NO_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ハンドラ管理初期化
 * @details     ハンドラ管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IntMngHdlInit( void )
{
    uint32_t intNo;     /* 割込み番号 */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 全割込み番号毎に繰り返し */
    for ( intNo =  INTMNG_INT_NO_MIN;
          intNo <= INTMNG_INT_NO_MAX;
          intNo++                     ) {
        /* 割込みハンドラ管理テーブル設定 */
        gHdlIntProcTbl[ intNo ] = HdlIgnore;

        /* IDT登録 */
        IntMngIdtSet(
            intNo,                              /* IDTエントリ番号    */
            MEMMNG_SEGSEL_KERNEL_CODE,          /* セレクタ           */
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
 * @param[in]   level 特権レベル
 *                  - IA32_DESCRIPTOR_DPL_0 特権レベル0
 *                  - IA32_DESCRIPTOR_DPL_1 特権レベル1
 *                  - IA32_DESCRIPTOR_DPL_2 特権レベル2
 *                  - IA32_DESCRIPTOR_DPL_3 特権レベル3
 */
/******************************************************************************/
void IntMngHdlSet( uint32_t    intNo,
                   IntMngHdl_t func,
                   uint8_t     level  )
{
    /* 割込みハンドラ管理テーブル設定 */
    gHdlIntProcTbl[ intNo ] = func;

    /* IDT設定 */
    IntMngIdtSet(
        intNo,                              /* IDTエントリ番号    */
        MEMMNG_SEGSEL_KERNEL_CODE,          /* セレクタ           */
        gHdlCmnProcTbl[ intNo ],            /* オフセット         */
        0,                                  /* 引数コピーカウント */
        IA32_DESCRIPTOR_TYPE_GATE32_INT,    /* タイプ             */
        level                            ); /* 特権レベル         */

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/* 割込みハンドラ共通関数 */
HDL_CMN_PROC( 0x00 )
HDL_CMN_PROC( 0x01 )
HDL_CMN_PROC( 0x02 )
HDL_CMN_PROC( 0x03 )
HDL_CMN_PROC( 0x04 )
HDL_CMN_PROC( 0x05 )
HDL_CMN_PROC( 0x06 )
HDL_CMN_PROC( 0x07 )
HDL_CMN_PROC_ERR( 0x08 )
HDL_CMN_PROC( 0x09 )
HDL_CMN_PROC_ERR( 0x0A )
HDL_CMN_PROC_ERR( 0x0B )
HDL_CMN_PROC_ERR( 0x0C )
HDL_CMN_PROC_ERR( 0x0D )
HDL_CMN_PROC_ERR( 0x0E )
HDL_CMN_PROC( 0x0F )
HDL_CMN_PROC( 0x10 )
HDL_CMN_PROC_ERR( 0x11 )
HDL_CMN_PROC( 0x12 )
HDL_CMN_PROC( 0x13 )
HDL_CMN_PROC( 0x14 )
HDL_CMN_PROC( 0x15 )
HDL_CMN_PROC( 0x16 )
HDL_CMN_PROC( 0x17 )
HDL_CMN_PROC( 0x18 )
HDL_CMN_PROC( 0x19 )
HDL_CMN_PROC( 0x1A )
HDL_CMN_PROC( 0x1B )
HDL_CMN_PROC( 0x1C )
HDL_CMN_PROC( 0x1D )
HDL_CMN_PROC( 0x1E )
HDL_CMN_PROC( 0x1F )
HDL_CMN_PROC( 0x20 )
HDL_CMN_PROC( 0x21 )
HDL_CMN_PROC( 0x22 )
HDL_CMN_PROC( 0x23 )
HDL_CMN_PROC( 0x24 )
HDL_CMN_PROC( 0x25 )
HDL_CMN_PROC( 0x26 )
HDL_CMN_PROC( 0x27 )
HDL_CMN_PROC( 0x28 )
HDL_CMN_PROC( 0x29 )
HDL_CMN_PROC( 0x2A )
HDL_CMN_PROC( 0x2B )
HDL_CMN_PROC( 0x2C )
HDL_CMN_PROC( 0x2D )
HDL_CMN_PROC( 0x2E )
HDL_CMN_PROC( 0x2F )
HDL_CMN_PROC( 0x30 )
HDL_CMN_PROC( 0x31 )
HDL_CMN_PROC( 0x32 )
HDL_CMN_PROC( 0x33 )
HDL_CMN_PROC( 0x34 )
HDL_CMN_PROC( 0x35 )
HDL_CMN_PROC( 0x36 )
HDL_CMN_PROC( 0x37 )
HDL_CMN_PROC( 0x38 )
HDL_CMN_PROC( 0x39 )
HDL_CMN_PROC( 0x3A )
HDL_CMN_PROC( 0x3B )
HDL_CMN_PROC( 0x3C )
HDL_CMN_PROC( 0x3D )
HDL_CMN_PROC( 0x3E )
HDL_CMN_PROC( 0x3F )
HDL_CMN_PROC( 0x40 )
HDL_CMN_PROC( 0x41 )
HDL_CMN_PROC( 0x42 )
HDL_CMN_PROC( 0x43 )
HDL_CMN_PROC( 0x44 )
HDL_CMN_PROC( 0x45 )
HDL_CMN_PROC( 0x46 )
HDL_CMN_PROC( 0x47 )
HDL_CMN_PROC( 0x48 )
HDL_CMN_PROC( 0x49 )
HDL_CMN_PROC( 0x4A )
HDL_CMN_PROC( 0x4B )
HDL_CMN_PROC( 0x4C )
HDL_CMN_PROC( 0x4D )
HDL_CMN_PROC( 0x4E )
HDL_CMN_PROC( 0x4F )
HDL_CMN_PROC( 0x50 )
HDL_CMN_PROC( 0x51 )
HDL_CMN_PROC( 0x52 )
HDL_CMN_PROC( 0x53 )
HDL_CMN_PROC( 0x54 )
HDL_CMN_PROC( 0x55 )
HDL_CMN_PROC( 0x56 )
HDL_CMN_PROC( 0x57 )
HDL_CMN_PROC( 0x58 )
HDL_CMN_PROC( 0x59 )
HDL_CMN_PROC( 0x5A )
HDL_CMN_PROC( 0x5B )
HDL_CMN_PROC( 0x5C )
HDL_CMN_PROC( 0x5D )
HDL_CMN_PROC( 0x5E )
HDL_CMN_PROC( 0x5F )
HDL_CMN_PROC( 0x60 )
HDL_CMN_PROC( 0x61 )
HDL_CMN_PROC( 0x62 )
HDL_CMN_PROC( 0x63 )
HDL_CMN_PROC( 0x64 )
HDL_CMN_PROC( 0x65 )
HDL_CMN_PROC( 0x66 )
HDL_CMN_PROC( 0x67 )
HDL_CMN_PROC( 0x68 )
HDL_CMN_PROC( 0x69 )
HDL_CMN_PROC( 0x6A )
HDL_CMN_PROC( 0x6B )
HDL_CMN_PROC( 0x6C )
HDL_CMN_PROC( 0x6D )
HDL_CMN_PROC( 0x6E )
HDL_CMN_PROC( 0x6F )
HDL_CMN_PROC( 0x70 )
HDL_CMN_PROC( 0x71 )
HDL_CMN_PROC( 0x72 )
HDL_CMN_PROC( 0x73 )
HDL_CMN_PROC( 0x74 )
HDL_CMN_PROC( 0x75 )
HDL_CMN_PROC( 0x76 )
HDL_CMN_PROC( 0x77 )
HDL_CMN_PROC( 0x78 )
HDL_CMN_PROC( 0x79 )
HDL_CMN_PROC( 0x7A )
HDL_CMN_PROC( 0x7B )
HDL_CMN_PROC( 0x7C )
HDL_CMN_PROC( 0x7D )
HDL_CMN_PROC( 0x7E )
HDL_CMN_PROC( 0x7F )
HDL_CMN_PROC( 0x80 )
HDL_CMN_PROC( 0x81 )
HDL_CMN_PROC( 0x82 )
HDL_CMN_PROC( 0x83 )
HDL_CMN_PROC( 0x84 )
HDL_CMN_PROC( 0x85 )
HDL_CMN_PROC( 0x86 )
HDL_CMN_PROC( 0x87 )
HDL_CMN_PROC( 0x88 )
HDL_CMN_PROC( 0x89 )
HDL_CMN_PROC( 0x8A )
HDL_CMN_PROC( 0x8B )
HDL_CMN_PROC( 0x8C )
HDL_CMN_PROC( 0x8D )
HDL_CMN_PROC( 0x8E )
HDL_CMN_PROC( 0x8F )
HDL_CMN_PROC( 0x90 )
HDL_CMN_PROC( 0x91 )
HDL_CMN_PROC( 0x92 )
HDL_CMN_PROC( 0x93 )
HDL_CMN_PROC( 0x94 )
HDL_CMN_PROC( 0x95 )
HDL_CMN_PROC( 0x96 )
HDL_CMN_PROC( 0x97 )
HDL_CMN_PROC( 0x98 )
HDL_CMN_PROC( 0x99 )
HDL_CMN_PROC( 0x9A )
HDL_CMN_PROC( 0x9B )
HDL_CMN_PROC( 0x9C )
HDL_CMN_PROC( 0x9D )
HDL_CMN_PROC( 0x9E )
HDL_CMN_PROC( 0x9F )
HDL_CMN_PROC( 0xA0 )
HDL_CMN_PROC( 0xA1 )
HDL_CMN_PROC( 0xA2 )
HDL_CMN_PROC( 0xA3 )
HDL_CMN_PROC( 0xA4 )
HDL_CMN_PROC( 0xA5 )
HDL_CMN_PROC( 0xA6 )
HDL_CMN_PROC( 0xA7 )
HDL_CMN_PROC( 0xA8 )
HDL_CMN_PROC( 0xA9 )
HDL_CMN_PROC( 0xAA )
HDL_CMN_PROC( 0xAB )
HDL_CMN_PROC( 0xAC )
HDL_CMN_PROC( 0xAD )
HDL_CMN_PROC( 0xAE )
HDL_CMN_PROC( 0xAF )
HDL_CMN_PROC( 0xB0 )
HDL_CMN_PROC( 0xB1 )
HDL_CMN_PROC( 0xB2 )
HDL_CMN_PROC( 0xB3 )
HDL_CMN_PROC( 0xB4 )
HDL_CMN_PROC( 0xB5 )
HDL_CMN_PROC( 0xB6 )
HDL_CMN_PROC( 0xB7 )
HDL_CMN_PROC( 0xB8 )
HDL_CMN_PROC( 0xB9 )
HDL_CMN_PROC( 0xBA )
HDL_CMN_PROC( 0xBB )
HDL_CMN_PROC( 0xBC )
HDL_CMN_PROC( 0xBD )
HDL_CMN_PROC( 0xBE )
HDL_CMN_PROC( 0xBF )
HDL_CMN_PROC( 0xC0 )
HDL_CMN_PROC( 0xC1 )
HDL_CMN_PROC( 0xC2 )
HDL_CMN_PROC( 0xC3 )
HDL_CMN_PROC( 0xC4 )
HDL_CMN_PROC( 0xC5 )
HDL_CMN_PROC( 0xC6 )
HDL_CMN_PROC( 0xC7 )
HDL_CMN_PROC( 0xC8 )
HDL_CMN_PROC( 0xC9 )
HDL_CMN_PROC( 0xCA )
HDL_CMN_PROC( 0xCB )
HDL_CMN_PROC( 0xCC )
HDL_CMN_PROC( 0xCD )
HDL_CMN_PROC( 0xCE )
HDL_CMN_PROC( 0xCF )
HDL_CMN_PROC( 0xD0 )
HDL_CMN_PROC( 0xD1 )
HDL_CMN_PROC( 0xD2 )
HDL_CMN_PROC( 0xD3 )
HDL_CMN_PROC( 0xD4 )
HDL_CMN_PROC( 0xD5 )
HDL_CMN_PROC( 0xD6 )
HDL_CMN_PROC( 0xD7 )
HDL_CMN_PROC( 0xD8 )
HDL_CMN_PROC( 0xD9 )
HDL_CMN_PROC( 0xDA )
HDL_CMN_PROC( 0xDB )
HDL_CMN_PROC( 0xDC )
HDL_CMN_PROC( 0xDD )
HDL_CMN_PROC( 0xDE )
HDL_CMN_PROC( 0xDF )
HDL_CMN_PROC( 0xE0 )
HDL_CMN_PROC( 0xE1 )
HDL_CMN_PROC( 0xE2 )
HDL_CMN_PROC( 0xE3 )
HDL_CMN_PROC( 0xE4 )
HDL_CMN_PROC( 0xE5 )
HDL_CMN_PROC( 0xE6 )
HDL_CMN_PROC( 0xE7 )
HDL_CMN_PROC( 0xE8 )
HDL_CMN_PROC( 0xE9 )
HDL_CMN_PROC( 0xEA )
HDL_CMN_PROC( 0xEB )
HDL_CMN_PROC( 0xEC )
HDL_CMN_PROC( 0xED )
HDL_CMN_PROC( 0xEE )
HDL_CMN_PROC( 0xEF )
HDL_CMN_PROC( 0xF0 )
HDL_CMN_PROC( 0xF1 )
HDL_CMN_PROC( 0xF2 )
HDL_CMN_PROC( 0xF3 )
HDL_CMN_PROC( 0xF4 )
HDL_CMN_PROC( 0xF5 )
HDL_CMN_PROC( 0xF6 )
HDL_CMN_PROC( 0xF7 )
HDL_CMN_PROC( 0xF8 )
HDL_CMN_PROC( 0xF9 )
HDL_CMN_PROC( 0xFA )
HDL_CMN_PROC( 0xFB )
HDL_CMN_PROC( 0xFC )
HDL_CMN_PROC( 0xFD )
HDL_CMN_PROC( 0xFE )
HDL_CMN_PROC( 0xFF )


/******************************************************************************/
/**
 * @brief       無視割込みハンドラ
 * @details     割込み処理として何もしない割込みハンドラ。
 *
 * @param[in]   intNo   割込み番号
 * @param[in]   context 割込み発生時コンテキスト情報
 */
/******************************************************************************/
static void HdlIgnore( uint32_t        intNo,
                       IntMngContext_t context )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() intNo=%d", __func__, intNo );

    /* [TODO]デバッグ用 */
    DEBUG_LOG( " edi    = %0#10x, esi    = %0#10x, ebp    = %0#10x",
               context.genReg.edi,
               context.genReg.esi,
               context.genReg.ebp                                    );
    DEBUG_LOG( " esp    = %0#10x, ebx    = %0#10x, edx    = %0#10x",
               context.genReg.esp,
               context.genReg.ebx,
               context.genReg.edx                                    );
    DEBUG_LOG( " ecx    = %0#10x, eax    = %0#10x, err    = %0#10x",
               context.genReg.ecx,
               context.genReg.eax,
               *( ( uint32_t * )( &context.errCode ) )               );
    DEBUG_LOG( " eip    = %0#10x, cs     = %0#10x, eflags = %0#10x",
               context.iretdInfo.eip,
               context.iretdInfo.cs,
               context.iretdInfo.eflags                              );
    DEBUG_LOG( " esp    = %0#10x, ss     = %0#10x, taskId = %d",
               context.iretdInfo.esp,
               context.iretdInfo.ss,
               TaskmngSchedGetTaskId()                               );

    while( 1 ) {
        IA32InstructionHlt();
    }

    /* 処理無し */

    return;
}


/******************************************************************************/
