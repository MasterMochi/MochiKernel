/******************************************************************************/
/* src/booter/Driver/DriverA20.c                                              */
/*                                                                 2017/07/10 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <hardware/IA32/IA32.h>
#include <hardware/IA32/IA32Instruction.h>
#include <MLib/Basic/MLibBasic.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                   \
    DebugLogOutput( CMN_MODULE_DRIVER_A20, \
                    __LINE__,              \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* A20ライン有効化チェック */
static CmnRet_t A20CheckEnable( void );

/* A20ライン有効化（System Port A） */
static void A20EnableBySysPortA( void );

/* A20ライン有効化（Keyboard Controller） */
static void A20EnableByKbc( void );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       A20ライン有効化
 * @details     A20ラインを有効化し、1MiB以上のメモリをアクセス可能にする。
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t DriverA20Enable( void )
{
    CmnRet_t ret;   /* 関数戻り値 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* A20ライン有効化判定 */
    ret = A20CheckEnable();
    
    /* 有効化判定 */
    if ( ret == CMN_SUCCESS ) {
        /* 有効 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );
        
        return CMN_SUCCESS;
    }
    
    /* System Port AによるA20ライン有効化 */
    A20EnableBySysPortA();
    
    /* A20ライン有効化判定 */
    ret = A20CheckEnable();
    
    /* 有効化判定 */
    if ( ret == CMN_SUCCESS ) {
        /* 有効 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );
        
        return CMN_SUCCESS;
    }
    
    /* キーボードコントローラによるA20ライン有効化 */
    A20EnableByKbc();
    
    /* A20ライン有効化判定 */
    ret = A20CheckEnable();
    
    /* 有効化判定 */
    if ( ret == CMN_SUCCESS ) {
        /* 有効 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );
        
        return CMN_SUCCESS;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
    
    return CMN_FAILURE;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       A20ライン有効化チェック
 * @details     A20ラインが有効でメモリが1MiB以上アクセス可能かチェックする。
 * 
 * @retval      CMN_SUCCESS 有効
 * @retval      CMN_FAILURE 無効
 */
/******************************************************************************/
static CmnRet_t A20CheckEnable( void )
{
    volatile uint16_t backup;    /* バックアップ値 */
    volatile uint16_t check;     /* チェック値     */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* アドレス0x0010_0000（または0x0000_0000）を2byte分バックアップ */
    backup = *( ( volatile uint16_t * ) 0x00100000 );
    
    /* CPUキャッシング無効化 */
    IA32InstructionSetCr0( IA32_CR0_CD, IA32_CR0_CD | IA32_CR0_NW );
    IA32InstructionWbinvd();
    
    /* アドレス0x0010_0000の2byte書き込み */
    *( ( volatile uint16_t * ) 0x00100000 ) = 0xC0DE;
    
    /* アドレス0x0000_0000の2byte読み込み */
    check = *( ( volatile uint16_t * ) 0x00000000 );
    
    /* アドレス0x0010_0000（または0x0000_0000）を2byte分リストア */
    *( ( volatile uint16_t * ) 0x00100000 ) = backup;
    
    /* CPUキャッシュ有効化 */
    IA32InstructionSetCr0( 0, IA32_CR0_CD | IA32_CR0_NW );
    
    /* A20ライン有効判定 */
    if ( check == 0xC0DE ) {
        /* 無効 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
        
        return CMN_FAILURE;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );
    
    /* A20ライン有効 */
    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       A20ライン有効化（System Port A）
 * @details     System Port Aを用いてA20ラインを有効化する。
 */
/******************************************************************************/
static void A20EnableBySysPortA( void )
{
    uint8_t value;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* System Port A の設定値取得 */
    IA32InstructionInByte( &value, 0x92 );
    
    /* A20ライン有効化 */
    value |= 0x02;
    
    /* システムリセット抑止 */
    value &= 0xFE;
    
    /* System Port A に設定 */
    IA32InstructionOutByte( 0x92, value );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       A20ライン有効化（Keyboard Controller）
 * @details     キーボードコントローラを用いてA20ラインを有効化する。
 */
/******************************************************************************/
static void A20EnableByKbc( void )
{
    uint8_t value;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* コマンド入力待ち */
    do {
        /* IBF設定値取得 */
        IA32InstructionInByte( &value, 0x64 );
        
    } while ( !MLIB_BASIC_HAVE_FLAG( value, 0x02 ) );
    
    /* コマンド書き込み */
    IA32InstructionOutByte( 0x64, 0xD1 );
    
    /* コマンド入力待ち */
    do {
        /* IBF設定値取得 */
        IA32InstructionInByte( &value, 0x64 );
        
    } while ( !MLIB_BASIC_HAVE_FLAG( value, 0x02 ) );
    
    /* A20ライン有効化 */
    IA32InstructionOutByte( 0x60, 0xDF );
    
    /* コマンド入力待ち */
    do {
        /* IBF設定値取得 */
        IA32InstructionInByte( &value, 0x64 );
        
    } while ( !MLIB_BASIC_HAVE_FLAG( value, 0x02 ) );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
