/******************************************************************************/
/* src/kernel/IntMng/IntMngPic.c                                              */
/*                                                                 2017/03/11 */
/* Copyright (C) 2016-2017 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <hardware/IA32/IA32Instruction.h>
#include <hardware/I8259A/I8259A.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "IntMng.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_INI_INIT,    \
                    __LINE__,               \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/* 割込みマスク状態定義 */
#define PIC_MASK_STATE_DISABLE ( 0 )    /**< マスク無効状態 */
#define PIC_MASK_STATE_ENABLE  ( 1 )    /**< マスク有効状態 */

/* PIC定義 */
#define PIC_NUM                ( 2 )    /**< PIC数        */
#define PIC_MASTER             ( 0 )    /**< PIC1マスタ   */
#define PIC_SLAVE              ( 1 )    /**< PIC2スレーブ */

/** PIC管理テーブル型 */
typedef struct {
    uint8_t maskState;          /**< PICマスク状態 */
    uint8_t mask[ PIC_NUM ];    /**< PICマスク値   */
    uint8_t reserved;           /**< パディング    */
} picTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** PIC管理テーブル */
static picTbl_t gPicTbl;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       PIC管理初期化
 * @details     PIC管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IntMngPicInit( void )
{
    /* PIC2（スレーブ）設定 */
    IA32InstructionOutByte( I8259A_S_PORT_ICW1, 0x11 );
    IA32InstructionOutByte( I8259A_S_PORT_ICW2, 0x28 );
    IA32InstructionOutByte( I8259A_S_PORT_ICW3, 0x02 );
    IA32InstructionOutByte( I8259A_S_PORT_ICW4, 0x01 );
    
    /* PIC1（マスタ）設定 */
    IA32InstructionOutByte( I8259A_M_PORT_ICW1, 0x11 );
    IA32InstructionOutByte( I8259A_M_PORT_ICW2, 0x20 );
    IA32InstructionOutByte( I8259A_M_PORT_ICW3, 0x04 );
    IA32InstructionOutByte( I8259A_M_PORT_ICW4, 0x01 );
    
    /* PIC管理テーブル初期化 */
    gPicTbl.mask[ PIC_MASTER ] = 0xFF;
    gPicTbl.mask[ PIC_SLAVE  ] = 0xFF;
    
    /* 割込み無効化 */
    IntMngPicDisable();
    
    return;
}


/******************************************************************************/
/**
 * @brief       PIC割込み許可
 * @details     PICに指定したIRQ番号の割込み許可設定を行う。
 * 
 * @param[in]   irqNo IRQ番号
 *                  - I8259A_IRQ0  IRQ0
 *                  - I8259A_IRQ1  IRQ1
 *                  - I8259A_IRQ2  IRQ2
 *                  - I8259A_IRQ3  IRQ3
 *                  - I8259A_IRQ4  IRQ4
 *                  - I8259A_IRQ5  IRQ5
 *                  - I8259A_IRQ6  IRQ6
 *                  - I8259A_IRQ7  IRQ7
 *                  - I8259A_IRQ8  IRQ8
 *                  - I8259A_IRQ9  IRQ9
 *                  - I8259A_IRQ10 IRQ10
 *                  - I8259A_IRQ11 IRQ11
 *                  - I8259A_IRQ12 IRQ12
 *                  - I8259A_IRQ13 IRQ13
 *                  - I8259A_IRQ14 IRQ14
 *                  - I8259A_IRQ15 IRQ15
 */
/******************************************************************************/
void IntMngPicAllowIrq( uint8_t irqNo )
{
    /* PIC割込み番号判定 */
    if ( ( irqNo >= I8259A_IRQ0 ) &&
         ( irqNo <= I8259A_IRQ7 )    ) {
        /* PIC1（マスタ）向け割込み番号 */
        
        /* PIC1（マスタ）用マスク設定 */
        gPicTbl.mask[ PIC_MASTER ] &= ~( 0x01 << irqNo );
        
    } else {
        /* PIC2（スレーブ）向け割込み番号 */
        
        /* PIC1（マスタ）用マスク設定 */
        gPicTbl.mask[ PIC_MASTER ] &= ~I8259A_OCW1_M2;
        
        /* PIC2（スレーブ）用マスク設定 */
        gPicTbl.mask[ PIC_SLAVE ]  &= ~( 0x01 << ( irqNo - I8259A_IRQ8 ) );
    }
    
    /* PIC割込みマスク状態判定 */
    if ( gPicTbl.maskState == PIC_MASK_STATE_ENABLE ) {
        /* 割込み有効 */
        
        /* PIC割込みマスク設定 */
        IntMngPicEnable();
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       PIC割込み拒否
 * @details     PICに指定したIRQ番号の割込み拒否設定を行う。
 * 
 * @param[in]   irqNo IRQ番号
 *                  - I8259A_IRQ0 IRQ0
 *                  - I8259A_IRQ1 IRQ1
 *                  - I8259A_IRQ2 IRQ2
 *                  - I8259A_IRQ3 IRQ3
 *                  - I8259A_IRQ4 IRQ4
 *                  - I8259A_IRQ5 IRQ5
 *                  - I8259A_IRQ6 IRQ6
 *                  - I8259A_IRQ7 IRQ7
 *                  - I8259A_IRQ8 IRQ8
 *                  - I8259A_IRQ9 IRQ9
 *                  - I8259A_IRQ10 IRQ10
 *                  - I8259A_IRQ11 IRQ11
 *                  - I8259A_IRQ12 IRQ12
 *                  - I8259A_IRQ13 IRQ13
 *                  - I8259A_IRQ14 IRQ14
 *                  - I8259A_IRQ15 IRQ15
 */
/******************************************************************************/
void IntMngPicDenyIrq( uint8_t irqNo )
{
    /* PIC割込み番号判定 */
    if ( ( irqNo >= I8259A_IRQ0 ) &&
         ( irqNo <= I8259A_IRQ7 )    ) {
        /* PIC1（マスタ）向け割込み番号 */
        
        /* PIC1（マスタ）用マスク設定 */
        gPicTbl.mask[ PIC_MASTER ] |= 0x01 << irqNo;
        
    } else {
        /* PIC2（スレーブ）向け割込み番号 */
        
        /* PIC2（スレーブ）用マスク設定 */
        gPicTbl.mask[ PIC_SLAVE ]  |= ( 0x01 << ( irqNo - I8259A_IRQ8 ) );
        
        /* PIC2（スレーブ）の全マスク判定 */
        if ( gPicTbl.mask[ PIC_SLAVE ] == 0xFF ) {
            /* 全マスク */
            
            /* PIC1（マスタ）用マスク設定 */
            gPicTbl.mask[ PIC_MASTER ] |= I8259A_OCW1_M2;
        }
    }
    
    /* PIC割込みマスク状態判定 */
    if ( gPicTbl.maskState == PIC_MASK_STATE_ENABLE ) {
        /* 割込み有効 */
        
        /* PIC割込みマスク設定 */
        IntMngPicEnable();
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       PIC割込み無効化
 * @details     PIC割込みを無効化する。
 */
/******************************************************************************/
void IntMngPicDisable( void )
{
    /* 割込みマスク状態変更 */
    gPicTbl.maskState = PIC_MASK_STATE_DISABLE;
    
    /* PIC1（マスタ）割込みマスク設定 */
    IA32InstructionOutByte( I8259A_M_PORT_OCW1, 0xFF );
    
    /* PIC2（スレーブ）割込みマスク設定 */
    IA32InstructionOutByte( I8259A_S_PORT_OCW1, 0xFF );
    
    return;
}


/******************************************************************************/
/**
 * @brief       PIC割込み有効化
 * @details     PIC割込みを有効化する。
 */
/******************************************************************************/
void IntMngPicEnable( void )
{
    /* 割込みマスク状態変更 */
    gPicTbl.maskState = PIC_MASK_STATE_ENABLE;
    
    /* PIC1（マスタ）割込みマスク設定 */
    IA32InstructionOutByte( I8259A_M_PORT_OCW1, gPicTbl.mask[ PIC_MASTER ] );
    
    /* PIC2（スレーブ）割込みマスク設定 */
    IA32InstructionOutByte( I8259A_S_PORT_OCW1, gPicTbl.mask[ PIC_SLAVE  ] );
    
    return;
}


/******************************************************************************/
/**
 * @brief       PIC割込みEOI通知
 * @details     PICに指定したIRQ番号のEOI通知を行う。
 * 
 * @param[in]   irqNo IRQ番号
 *                  - I8259A_IRQ0 IRQ0
 *                  - I8259A_IRQ1 IRQ1
 *                  - I8259A_IRQ2 IRQ2
 *                  - I8259A_IRQ3 IRQ3
 *                  - I8259A_IRQ4 IRQ4
 *                  - I8259A_IRQ5 IRQ5
 *                  - I8259A_IRQ6 IRQ6
 *                  - I8259A_IRQ7 IRQ7
 *                  - I8259A_IRQ8 IRQ8
 *                  - I8259A_IRQ9 IRQ9
 *                  - I8259A_IRQ10 IRQ10
 *                  - I8259A_IRQ11 IRQ11
 *                  - I8259A_IRQ12 IRQ12
 *                  - I8259A_IRQ13 IRQ13
 *                  - I8259A_IRQ14 IRQ14
 *                  - I8259A_IRQ15 IRQ15
 */
/******************************************************************************/
void IntMngPicEoi( uint8_t irqNo )
{
    /* PIC割込み番号判定 */
    if ( ( irqNo >= I8259A_IRQ0 ) &&
         ( irqNo <= I8259A_IRQ7 )    ) {
        /* PIC1（マスタ）向け割込み番号 */
        
        /* PIC1（マスタ）EOI通知 */
        IA32InstructionOutByte( I8259A_M_PORT_OCW2,
                                I8259A_OCW2_SL  | 
                                I8259A_OCW2_EOI |
                                irqNo               );
        
    } else {
        /* PIC2（スレーブ）向け割込み番号 */
        
        /* IRQ番号変換 */
        irqNo -= I8259A_IRQ8;
        
        /* PIC2（スレーブ）EOI通知 */
        IA32InstructionOutByte( I8259A_S_PORT_OCW2,
                                I8259A_OCW2_SL  |
                                I8259A_OCW2_EOI |
                                irqNo               );
        
        /* PIC2（スレーブ）EOI通知 */
        IA32InstructionOutByte( I8259A_M_PORT_OCW2,
                                I8259A_OCW2_SL  |
                                I8259A_OCW2_EOI |
                                I8259A_IRQ2         );
    }
    
    return;
}


/******************************************************************************/
