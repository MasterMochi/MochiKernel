/******************************************************************************/
/* src/kernel/TimerMng/TimerMngPit.c                                          */
/*                                                                 2018/11/24 */
/* Copyright (C) 2016-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdint.h>
#include <kernel/config.h>
#include <hardware/I8254/I8254.h>
#include <hardware/I8259A/I8259A.h>
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <TaskMng.h>
#include <TimerMng.h>

/* 内部モジュールヘッダ */
#include "TimerMngCtrl.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TIMERMNG_PIT,    \
                    __LINE__,                   \
                    __VA_ARGS__              )
#else
#define DEBUG_LOG( ... )
#endif

/** PIT（カウンタ0）カウンタ設定値 */
#define PIT_CYCLE ( I8254_CLOCK / MK_CONFIG_TICK_HZ )


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       PIT割込みハンドラ
 * @details     PITからの割込み処理を行う。
 * 
 * @param[in]   intNo   割込み番号
 * @param[in]   context 割込み発生時コンテキスト情報(未使用)
 */
/******************************************************************************/
void TimerMngPitHdlInt( uint32_t        intNo,
                        IntMngContext_t context )
{
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start. intNo=%#x", __func__, intNo );*/
    
    /* 割込み処理終了通知 */
    IntMngPicEoi( I8259A_IRQ0 );
    
    /* タイマ制御実行 */
    CtrlRun();
    
    /* スケジューラ実行 */
    TaskMngSchedExec();
    
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/
    
    return;
}


/******************************************************************************/
/**
 * @brief       PIT管理初期化
 * @details     PIT管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void TimerMngPitInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* PIT（カウンタ0）初期化 */
    IA32InstructionOutByte( I8254_PORT_CTRLW,
                            ( I8254_CTRLW_SC_CNTR0 |
                              I8254_CTRLW_RW_BOTH  |
                              I8254_CTRLW_M_MODE2  |
                              I8254_CTRLW_BCD_BIN    ) );
    IA32InstructionOutByte( I8254_PORT_CNTR0, I8254_CNTR_LOW(  PIT_CYCLE ) );
    IA32InstructionOutByte( I8254_PORT_CNTR0, I8254_CNTR_HIGH( PIT_CYCLE ) );
    
    /* 割込みハンドラ設定 */
    IntMngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ0,       /* 割込み番号     */
                  TimerMngPitHdlInt,                        /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_0               );    /* 特権レベル     */
    
    /* 割込み許可設定 */
    IntMngPicAllowIrq( I8259A_IRQ0 );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
