/******************************************************************************/
/*                                                                            */
/* src/kernel/Debug/DebugVram.c                                               */
/*                                                                 2024/07/21 */
/* Copyright (C) 2022-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* 共通ヘッダ */
#include <Cmn.h>
#include <hardware/Vga/Vga.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_DEBUG_VRAM

/** 文字色取得マクロ */
#define VRAM_GET_ATTR_FGC ( gVramData.attr & 0x07 )
/** 背景色取得マクロ */
#define VRAM_GET_ATTR_BGC ( gVramData.attr & 0x70 )
/** 点滅フラグ取得マクロ */
#define VRAM_GET_ATTR_FLAG \
    ( gVramData.attr & ( VGA_M3_ATTR_BLINK | VGA_M3_ATTR_FG_BRIGHT ) )

/** 文字属性設定マクロ */
#define VRAM_SET_ATTR( __ATTR ) ( gVramData.attr = ( __ATTR ) )
/** 文字色設定マクロ */
#define VRAM_SET_ATTR_FGC( __FGC ) \
    VRAM_SET_ATTR( ( gVramData.attr & 0xF8 ) | ( __FGC ) )
/** 背景色設定マクロ */
#define VRAM_SET_ATTR_BGC( __BGC ) \
    VRAM_SET_ATTR( ( gVramData.attr & 0x8F ) | ( __BGC ) )

/** 文字属性フラグオン設定マクロ */
#define VRAM_SET_ATTR_ON( __FLAG )  ( gVramData.attr |=  ( __FLAG ) )
/** 文字属性フラグオフ設定マクロ */
#define VRAM_SET_ATTR_OFF( __FLAG ) ( gVramData.attr &= ~( __FLAG ) )

/** 管理データ型 */
typedef struct {
    uint32_t row;       /**< カーソル行 */
    uint32_t column;    /**< カーソル列 */
    uint8_t  attr;      /**< 文字属性   */
} vramData_t;

/** モジュールID変換型 */
typedef struct {
    uint16_t moduleId;  /**< モジュールID */
    char     str[ 9 ];  /**< モジュール名 */
} vramConv_t;


/******************************************************************************/
/* 静的グローバル変数定義                                                     */
/******************************************************************************/
/** 管理データ */
static vramData_t gVramData;

/** モジュールID変換テーブル */
static const vramConv_t gConvTbl[] = {
    { CMN_MODULE_INIT_MAIN,      "INI-MAIN" },   /* 初期化制御(メイン)       */
    { CMN_MODULE_DEBUG_MAIN,     "DBG-MAIN" },   /* デバッグ制御(メイン)     */
    { CMN_MODULE_DEBUG_MEM,      "DBG-MEM " },   /* デバッグ制御(メモリ)     */
    { CMN_MODULE_DEBUG_VRAM,     "DBG-VRAM" },   /* デバッグ制御(VRAM)       */
    { CMN_MODULE_MEMMNG_MAIN,    "MEM-MAIN" },   /* メモリ管理(メイン)       */
    { CMN_MODULE_MEMMNG_SGMT,    "MEM-SGMT" },   /* メモリ管理(セグメント)   */
    { CMN_MODULE_MEMMNG_AREA,    "MEM-AREA" },   /* メモリ管理(領域)         */
    { CMN_MODULE_MEMMNG_PAGE,    "MEM-PAGE" },   /* メモリ管理(ページ)       */
    { CMN_MODULE_MEMMNG_CTRL,    "MEM-CTRL" },   /* メモリ管理(制御)         */
    { CMN_MODULE_MEMMNG_MAP,     "MEM-MAP " },   /* メモリ管理(マップ)       */
    { CMN_MODULE_MEMMNG_PHYS,    "MEM-PHYS" },   /* メモリ管理(物理)         */
    { CMN_MODULE_MEMMNG_IO,      "MEM-I/O " },   /* メモリ管理(I/O)          */
    { CMN_MODULE_MEMMNG_VIRT,    "MEM-VIRT" },   /* メモリ管理(仮想)         */
    { CMN_MODULE_MEMMNG_HEAP,    "MEM-HEAP" },   /* メモリ管理(ヒープ)       */
    { CMN_MODULE_TASKMNG_MAIN,   "TSK-MAIN" },   /* タスク管理(メイン)       */
    { CMN_MODULE_TASKMNG_TSS,    "TSK-TSS " },   /* タスク管理(TSS)          */
    { CMN_MODULE_TASKMNG_SCHED,  "TSK-SCHD" },   /* タスク管理(スケジューラ) */
    { CMN_MODULE_TASKMNG_TASK,   "TSK-TASK" },   /* タスク管理(タスク)       */
    { CMN_MODULE_TASKMNG_ELF,    "TSK-ELF " },   /* タスク管理(ELFローダ)    */
    { CMN_MODULE_TASKMNG_PROC,   "TSK-PROC" },   /* タスク管理(プロセス)     */
    { CMN_MODULE_TASKMNG_NAME,   "TSK-NAME" },   /* タスク管理(名前管理)     */
    { CMN_MODULE_TASKMNG_THREAD, "TSK-THRD" },   /* タスク管理(スレッド)     */
    { CMN_MODULE_INTMNG_MAIN,    "INT-MAIN" },   /* 割込管理(メイン)         */
    { CMN_MODULE_INTMNG_PIC,     "INT-PIC " },   /* 割込管理(PIC)            */
    { CMN_MODULE_INTMNG_IDT,     "INT-IDT " },   /* 割込管理(IDT)            */
    { CMN_MODULE_INTMNG_HDL,     "INT-HDL " },   /* 割込管理(ハンドラ)       */
    { CMN_MODULE_INTMNG_CTRL,    "INT-CTRL" },   /* 割込管理(ハードウェア)   */
    { CMN_MODULE_TIMERMNG_MAIN,  "TIM-MAIN" },   /* タイマ管理(メイン)       */
    { CMN_MODULE_TIMERMNG_CTRL,  "TIM-CTRL" },   /* タイマ管理(制御)         */
    { CMN_MODULE_TIMERMNG_PIT,   "TIM-PIT " },   /* タイマ管理(PIT)          */
    { CMN_MODULE_ITCCTRL_MAIN,   "ITC-MAIN" },   /* タスク間通信制御(メイン) */
    { CMN_MODULE_ITCCTRL_MSG,    "ITC-MSG " },   /* タスク間通信制御(ﾒｯｾｰｼﾞ) */
    { CMN_MODULE_IOCTRL_MAIN,    "IOC-MAIN" },   /* 入出力制御(メイン)       */
    { CMN_MODULE_IOCTRL_PORT,    "IOC-PORT" },   /* 入出力制御(I/Oポート)    */
    { CMN_MODULE_IOCTRL_MEM,     "IOC-MEM " },   /* 入出力制御(I/Oメモリ)    */
    { 0,                         "UNKNOWN " }  };/* 終端                     */


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* モジュールID文字列変換 */
static const char *ConvModuleId( uint16_t moduleId );
/* 数値変換 */
static uint32_t ConvUint32( const char **ppChar,
                            uint32_t   value     );
/* VRAM出力 */
static void Output( const char *pStr );
/* VRAM出力（文字） */
static void OutputChar( const char **ppChar );
/* エスケープシーケンス処理 */
static void ProcEscape( const char **ppChar );
/* エスケープシーケンス処理（CSI） */
static void ProcEscapeCsi( const char **ppChar );
/* エスケープシーケンス処理（CSI-SGR） */
static void ProcEscapeCsiSgr( const char **ppChar );
/* 画面スクロール */
static void Scroll( void );


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       VRAM出力初期化
 * @details     管理データとVRAMを初期化する。
 */
/******************************************************************************/
void DebugVramInit( void )
{
    uint32_t row;       /* 行カウンタ */
    uint32_t column;    /* 列カウンタ */

    /* 管理データ初期化 */
    gVramData.row    = 0;
    gVramData.column = 0;
    gVramData.attr   = VGA_M3_ATTR_FG_WHITE  |  /* 白色文字属性 */
                       VGA_M3_ATTR_FG_BRIGHT |  /* 明色文字属性 */
                       VGA_M3_ATTR_BG_BLACK;    /* 黒色背景属性 */

    /* VRAM初期化 */
    for ( row = 0; row < VGA_M3_ROW; row++ ) {
        for ( column = 0; column < VGA_M3_COLUMN; column++ ) {
            /* 1文字出力 */
            VGA_M3_TEXT_BUFFER( row, column ).code = ' ';
            VGA_M3_TEXT_BUFFER( row, column ).attr = gVramData.attr;
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ログ出力
 * @details     VRAMにログ出力を行う。
 *
 * @param[in]   moduleId モジュールID
 * @param[in]   lineNo   行番号
 * @param[in]   lv       ログレベル
 * @param[in]   pStr     文字列
 */
/******************************************************************************/
void DebugVramOutput( uint16_t   moduleId,
                      uint16_t   lineNo,
                      uint16_t   lv,
                      const char *pStr     )
{
    char       header[ 25 ];
    const char *pModuleName;

    /* モジュールID->文字列変換 */
    pModuleName = ConvModuleId( moduleId );

    /* モジュールID・行番号出力 */
    snprintf( header, 25, "\e[32m%s:%04u \e[0m", pModuleName, lineNo );
    Output( header );

    /* 文字列出力 */
    Output( pStr );

    /* カーソル行更新 */
    gVramData.row++;
    gVramData.column = 0;

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       モジュールID文字列変換
 * @details     モジュールIDを文字列に変換する。
 *
 * @param[in]   moduleId モジュールID
 *
 * @return      文字列を返す。
 */
/******************************************************************************/
static const char *ConvModuleId( uint16_t moduleId )
{
    uint32_t idx;

    /* 初期化 */
    idx = 0;

    /* 変換テーブルエントリ毎に繰り返す */
    while ( gConvTbl[ idx ].moduleId != 0 ) {
        /* モジュールID一致確認 */
        if ( gConvTbl[ idx ].moduleId == moduleId ) {
            /* 一致 */
            break;
        }

        /* 次のエントリ */
        idx++;
    }

    return gConvTbl[ idx ].str;
}


/******************************************************************************/
/**
 * @brief       数値変換
 * @details     数文字列を値に変換する。変換出来なかった場合はデフォルト値を返
 *              す。
 *
 * @param[in]   **ppChar 文字
 * @param[in]   value    デフォルト値
 *
 * @return      変換した数値を返す。
 */
/******************************************************************************/
static uint32_t ConvUint32( const char **ppChar,
                            uint32_t   value     )
{
    uint32_t conv;

    /* 初期化 */
    conv = 0;

    /* 数文字の間繰り返す */
    while ( ( '0' <= **ppChar ) && ( **ppChar <= '9' ) ) {
        /* 数値変換 */
        conv = conv * 10 + ( uint32_t )( **ppChar - '0' );

        /* 戻り値設定 */
        value = conv;

        /* 次の文字 */
        ( *ppChar )++;
    }

    return value;
}


/******************************************************************************/
/**
 * @brief       VRAM出力
 * @details     文字列をテキストバッファに出力する。文字列中のエスケープシーケ
 *              ンスは文字属性に反映する。
 *
 * @param[in]   *pStr 文字列
 */
/******************************************************************************/
static void Output( const char *pStr )
{
    const char *pChar;

    /* 初期化 */
    pChar = pStr;

    /* 1文字毎に繰り返し */
    while ( *pChar != '\0' ) {
        /* カーソル行オーバフロー判定 */
        if ( gVramData.row >= VGA_M3_COLUMN ) {
            /* オーバフロー */

            /* 画面スクロール */
            Scroll();
        }

        /* 文字コード判定 */
        if ( *pChar == '\e' ) {
            /* エスケープシーケンス */

            /* 次の文字 */
            pChar++;

            ProcEscape( &pChar );

        } else {
            /* 文字 */
            OutputChar( &pChar );
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief       VRAM出力（文字）
 * @details     文字をテキストバッファに出力する。
 *
 * @param[in]   **ppChar 文字
 */
/******************************************************************************/
static void OutputChar( const char **ppChar )
{
    /* VRAM出力 */
    VGA_M3_TEXT_BUFFER( gVramData.row, gVramData.column ).code = **ppChar;
    VGA_M3_TEXT_BUFFER( gVramData.row, gVramData.column ).attr = gVramData.attr;

    /* 文字列インデックス更新 */
    ( *ppChar )++;

    /* カーソル更新 */
    gVramData.column++;

    /* カーソル列オーバーフロー判定 */
    if ( gVramData.column >= VGA_M3_COLUMN ) {
        /* オーバラップ */
        gVramData.row++;
        gVramData.column = 0;
    }

    return;
}


/******************************************************************************/
/**
 * @brief       エスケープシーケンス処理
 * @details     エスケープシーケンスを処理する。
 *
 * @param[in]   **ppChar 文字
 */
/******************************************************************************/
static void ProcEscape( const char **ppChar )
{
    /* 機能判定 */
    if ( **ppChar == '[' ) {
        /* CSI */

        /* 次の文字へ */
        ( *ppChar )++;

        /* CSI処理 */
        ProcEscapeCsi( ppChar );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       エスケープシーケンス処理（CSI）
 * @details     エスケープシーケンス（CSI）を処理する。
 *
 * @param[in]   **ppChar 文字
 */
/******************************************************************************/
static void ProcEscapeCsi( const char **ppChar )
{
    const char *pChar;  /* 一時的な文字参照 */

    /* 初期化 */
    pChar = *ppChar;

    /* 終端文字まで繰り返す */
    while ( *pChar != '\0' ) {
        /* 機能判定 */
        if ( *pChar == 'm' ) {
            /* SGR */
            ProcEscapeCsiSgr( ppChar );

            /* 次の文字 */
            ( *ppChar )++;

            break;
        }

        /* 次の文字 */
        pChar++;
    }

    return;
}


/******************************************************************************/
/**
 * @brief       エスケープシーケンス処理（CSI-SGR）
 * @details     エスケープシーケンス（CSI-SGR）を処理する。
 *
 * @param[in]   **ppChar 文字
 */
/******************************************************************************/
static void ProcEscapeCsiSgr( const char **ppChar )
{
    uint32_t ps;    /* パラメータ */

    /* パラメータ毎に繰り返す */
    do {
        /* パラメータ取得 */
        ps = ConvUint32( ppChar, 0 );

        /* パラメータ判定 */
        switch ( ps ) {
            case 0:
                /* 点滅オフ */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_BLINK );
                break;

            case 30:
                /* 明るい黒色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_BLACK  );
                break;

            case 31:
                /* 明るい赤色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_RED    );
                break;

            case 32:
                /* 明るい緑色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_GREEN  );
                break;

            case 33:
                /* 明るい黄色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_BROWN  );
                break;

            case 34:
                /* 明るい青色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_BLUE   );
                break;

            case 35:
                /* 明るい紫色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_PURPLE );
                break;

            case 36:
                /* 明るい水色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_CYAN   );
                break;

            case 37:
            case 39:
                /* 明るい白色文字 */
                VRAM_SET_ATTR_ON(  VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_WHITE  );
                break;

            case 40:
            case 49:
                /* 黒色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_BLACK );
                break;

            case 41:
                /* 赤色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_RED );
                break;

            case 42:
                /* 緑色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_GREEN );
                break;

            case 43:
                /* 黄色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_BROWN );
                break;

            case 44:
                /* 青色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_BLUE );
                break;

            case 45:
                /* 紫色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_PURPLE );
                break;

            case 46:
                /* 水色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_CYAN );
                break;

            case 47:
                /* 白色背景 */
                VRAM_SET_ATTR_BGC( VGA_M3_ATTR_BG_WHITE );
                break;

            case 5:
                /* 点滅オン */
                VRAM_SET_ATTR_ON( VGA_M3_ATTR_BLINK );
                break;

            case 7:
                /* 反転 */
                VRAM_SET_ATTR( ( VRAM_GET_ATTR_FLAG     ) |
                               ( VRAM_GET_ATTR_FGC << 4 ) |
                               ( VRAM_GET_ATTR_BGC >> 4 )   );
                break;

            case 90:
                /* 黒色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_BLACK  );
                break;

            case 91:
                /* 赤色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_RED    );
                break;

            case 92:
                /* 緑色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_GREEN  );
                break;

            case 93:
                /* 黄色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_BROWN  );
                break;

            case 94:
                /* 青色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_BLUE   );
                break;

            case 95:
                /* 紫色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_PURPLE );
                break;

            case 96:
                /* 水色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_CYAN   );
                break;

            case 97:
                /* 白色文字 */
                VRAM_SET_ATTR_OFF( VGA_M3_ATTR_FG_BRIGHT );
                VRAM_SET_ATTR_FGC( VGA_M3_ATTR_FG_WHITE  );
                break;

            default:
                /* 未対応 */
                break;
        }

    } while ( *( *( ppChar )++ ) == ';' );

    return;
}


/******************************************************************************/
/**
 * @brief       画面スクロール
 * @details     画面を1行上にスクロールし最下行を初期化する。
 */
/******************************************************************************/
static void Scroll( void )
{
    uint32_t row;       /* 行カウンタ */
    uint32_t column;    /* 列カウンタ */

    /* カーソル行更新 */
    gVramData.row = VGA_M3_ROW - 1;

    /* 行毎に繰り返し */
    for ( row = 0; row < ( VGA_M3_ROW - 1 ); row++ ) {
        /* 一行コピー */
        memcpy( &VGA_M3_TEXT_BUFFER( row,     0 ),
                &VGA_M3_TEXT_BUFFER( row + 1, 0 ),
                VGA_M3_COLUMN * 2                  );
    }

    /* 最下行初期化 */
    for ( column = 0; column < VGA_M3_COLUMN; column++ ) {
        /* 1文字出力 */
        VGA_M3_TEXT_BUFFER( VGA_M3_ROW - 1, column ).code = ' ';
        VGA_M3_TEXT_BUFFER( VGA_M3_ROW - 1, column ).attr = gVramData.attr;
    }

    return;
}


/******************************************************************************/
