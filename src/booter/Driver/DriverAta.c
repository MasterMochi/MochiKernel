/******************************************************************************/
/* src/booter/Driver/DriverAta.c                                              */
/*                                                                 2018/11/24 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <hardware/ATA/ATA.h>
#include <hardware/IA32/IA32Instruction.h>
#include <hardware/I8259A/I8259A.h>
#include <MLib/Basic/MLibBasic.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Driver.h>
#include <IntMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_DRIVER_ATA,  \
                    __LINE__,               \
                    __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif

/** リトライ最大回数 */
#define ATA_RETRY        ( 50000 )

/* 割込みフラグ */
#define ATA_INT_FLAG_ON  ( 1 )      /**< 割込みフラグON  */
#define ATA_INT_FLAG_OFF ( 0 )      /**< 割込みフラグOFF */


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* 状態待ち合わせ */
static void AtaWaitStatus( uint8_t mask,
                           uint8_t status );
/* 割込み待ち合わせ */
static void AtaWaitInterrupt( void );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** 割込みフラグ */
static volatile uint32_t gInterruptFlag;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief           ATAドライバ初期化
 * @details         ATAドライバを初期化する。
 */
/******************************************************************************/
void DriverAtaInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 割込みフラグ初期化 */
    gInterruptFlag = ATA_INT_FLAG_OFF;
    
    /* 割込み許可設定 */
    IntMngPicAllowIrq( I8259A_IRQ14 );
    
    /* 割込みハンドラ登録 */
    IntMngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ14,
                  DriverAtaHandler );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       ATA割込みハンドラ
 * @details     ATAからの割込みを処理する。
 * 
 * @param[in]   intNo 割込み番号
 */
/******************************************************************************/
void DriverAtaHandler( uint32_t intNo )
{
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start. intNo=%#X", __func__, intNo );*/
    
    /* 割込みフラグ設定 */
    gInterruptFlag = ATA_INT_FLAG_ON;
    
    /* EOI命令発行 */
    IntMngPicEoi( I8259A_IRQ14 );
    
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/
    
    return;
}


/******************************************************************************/
/**
 * @brief           ディスク読み込み
 * @details         ディスクからデータを読み込む
 * 
 * @param[in,out]   *pAddr データ格納先アドレス
 * @param[in]       lba    ディスク上アドレス（LBA）
 * @param[in]       size   読み込みサイズ（セクタ）
 */
/******************************************************************************/
CmnRet_t DriverAtaRead( void     *pAddr,
                        uint32_t lba,
                        size_t   size    )
{
    uint8_t           status;        /* Status レジスタ値     */
    volatile uint32_t sectorCnt;     /* 1コマンド読込みサイズ */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. *pAddr=%p, lba=%#X, size=%u",
               __func__,
               pAddr,
               lba,
               size );
    
    /* 1コマンド毎に繰り返し */
    while ( size > 0 ) {
        /* アイドル状態待ち合わせ */
        AtaWaitStatus( ATA_STATUS_BSY | ATA_STATUS_DRQ, 0 );
        
        /* デバイスアドレス設定 */
        IA32InstructionOutByte( ATA_PORT_DEV_HEAD, 0 );
        
        /* アイドル状態待ち合わせ */
        AtaWaitStatus( ATA_STATUS_BSY | ATA_STATUS_DRQ, 0 );
        
        /* Featuresレジスタ設定 */
        IA32InstructionOutByte( ATA_PORT_FEATURES, 0 );
        
        /* LBA設定 */
        IA32InstructionOutByte( ATA_PORT_DEV_HEAD,
                                ATA_DEV_HEAD_LBA | ATA_LBA_TO_DEV_HEAD( lba ) );
        IA32InstructionOutByte( ATA_PORT_CYL_HIGH,
                                ATA_LBA_TO_CYL_HIGH( lba ) );
        IA32InstructionOutByte( ATA_PORT_CYL_LOW,
                                ATA_LBA_TO_CYL_LOW( lba ) );
        IA32InstructionOutByte( ATA_PORT_SECTOR_NUM,
                                ATA_LBA_TO_SECTOR_NUM( lba ) );
        
        /* 読込みサイズ判定 */
        if ( size >= ATA_SECTOR_CNT_MAX ) {
            /* Sector Count 最大値以上 */
            
            /* 設定 */
            sectorCnt  = ATA_SECTOR_CNT_MAX;    /* Sector Count */
            size      -= ATA_SECTOR_CNT_MAX;    /* 読込みサイズ */
            lba       += ATA_SECTOR_CNT_MAX;    /* LBA          */
            
            /* サイズ設定 */
            IA32InstructionOutByte( ATA_PORT_SECTOR_CNT, 0 );
            
        } else {
            /* Sector Count 最大値未満 */
            
            /* 設定 */
            sectorCnt = size;                   /* Sector Count */
            size      = 0;                      /* 読込みサイズ */
            
            /* サイズ設定 */
            IA32InstructionOutByte( ATA_PORT_SECTOR_CNT, sectorCnt );
        }
        
        /* 割込みフラグOFF */
        gInterruptFlag = ATA_INT_FLAG_OFF;
        
        /* コマンド書き込み */
        IA32InstructionOutByte( ATA_PORT_COMMAND, ATA_CMD_READ_SECTOR );
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() command write. cnt=%u", __func__, sectorCnt );
        
        /* 1セクタサイズ毎繰り返し */
        for ( ; sectorCnt > 0; sectorCnt-- ) {
            /* 割り込み待ち合わせ */
            AtaWaitInterrupt();
            
            /* アイドル状態待ち合わせ */
            AtaWaitStatus( ATA_STATUS_BSY, 0 );
            
            /* ステータスレジスタ取得 */
            IA32InstructionInByte( &status, ATA_PORT_STATUS );
            
            /* 割込みフラグOFF */
            gInterruptFlag = ATA_INT_FLAG_OFF;
            
            /* データ転送要求有無判定 */
            if ( MLIB_BASIC_HAVE_FLAG( status, ATA_STATUS_DRQ ) ) {
                /* データ転送要求有 */
                
                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() read. pAddr=%p, sectorCnt=%u.",
                           __func__,
                           pAddr,
                           sectorCnt );
                
                /* データ転送 */
                IA32InstructionRepInsw( pAddr,
                                        ATA_PORT_DATA,
                                        ATA_TRANSFER_COUNT );
                
                /* 格納先アドレス更新 */
                pAddr += ATA_TRANSFER_COUNT * 2;
                
            } else {
                /* データ転送要求無 */
                
                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
                
                return CMN_FAILURE;
            }
            
            /* 1PIO転送サイクル待ち */
            IA32InstructionInByte( &status, ATA_PORT_ALT_STATUS );
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief           状態待ち合わせ
 * @details         Statusレジスタが指定状態になるのを待ち合わせる。
 * 
 * @param[in]       mask   レジスタマスク値
 * @param[in]       status 状態
 */
/******************************************************************************/
static void AtaWaitStatus( uint8_t mask,
                           uint8_t status )
{
    uint8_t  value;     /* Statusレジスタ値 */
    uint32_t retry;     /* リトライ回数     */
    
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start. mask=%#X, status=%#X", __func__, mask, status );*/
    
    /* アイドル状態になるまで繰り返し */
    for ( retry = 0; retry <= ATA_RETRY; retry++ ) {
        /* Statusレジスタ読込 */
        IA32InstructionInByte( &value, ATA_PORT_STATUS );
        
        /* 状態判定 */
        if ( ( value & mask ) == status ) {
            /* 状態一致 */
            
            /* デバッグトレースログ出力 *//*
            DEBUG_LOG( "%s() end." );*/
            
            return;
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() retry out.", __func__ );
    
    /* アボート */
    CmnAbort();
}


/******************************************************************************/
/**
 * @brief           割込み待ち合わせ
 * @details         ATAデバイスから割り込みが発生するのを待ち合わせる。
 */
/******************************************************************************/
static void AtaWaitInterrupt( void )
{
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start.", __func__ );*/
    
    /* 割込みが発生するまで繰り返し */
    while ( gInterruptFlag == ATA_INT_FLAG_OFF ) {
        /* 未発生 */
        
        /* hlt命令実行 */
        IA32InstructionHlt();
    }
    
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/
    
    return;
}


/******************************************************************************/
