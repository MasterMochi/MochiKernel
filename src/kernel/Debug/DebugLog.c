/******************************************************************************/
/* src/kernel/Debug/DebugLog.c                                                */
/*                                                                 2018/06/16 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <hardware/Vga/Vga.h>

/* 内部モジュールヘッダ */
#include "DebugLog.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 長さ定義 */
#define LOG_LENGTH_ID      (  8 )   /** 識別子文字数 */
#define LOG_LENGTH_LINENUM (  4 )   /** 行番号文字数 */

/** カーソル位置アドレス計算 */
#define LOG_CURSOR_ADDR( __ROW, __COLUMN )                  \
    ( ( uint8_t * ) ( VGA_M3_VRAM_ADDR +                    \
                      ( __ROW ) * VGA_M3_COLUMN * 2 +       \
                      ( __COLUMN ) * 2                ) )

/** 文字色取得マクロ */
#define LOG_ATTR_FG( __BASE )           ( __BASE & 0x0F )

/** 背景色取得マクロ */
#define LOG_ATTR_BG( __BASE )           ( __BASE & 0xF0 )

/** 文字色変更マクロ */
#define LOG_ATTR_FG_CHG( __BASE, __FG ) ( LOG_ATTR_BG( __BASE ) | ( __FG ) )

/** 背景色変更マクロ */
#define LOG_ATTR_BG_CHG( __BASE, __BG ) ( LOG_ATTR_FG( __BASE ) | ( __BG ) )

/* 変換指定子フラグ */
#define LOG_FLAG_LEFT      ( 0x01 ) /* 左寄せ       */
#define LOG_FLAG_SIGN      ( 0x02 ) /* 符号表示     */
#define LOG_FLAG_SPACE     ( 0x04 ) /* 正数符号空白 */
#define LOG_FLAG_ALTERNATE ( 0x08 ) /* 代替形式     */
#define LOG_FLAG_ZERO      ( 0x10 ) /* 0埋め        */
#define LOG_FLAG_UPPERCASE ( 0x20 ) /* 大文字       */
#define LOG_FLAG_UNSIGNED  ( 0x40 ) /* 符号無       */

/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                \
    DebugLogOutput( CMN_MODULE_DBG_LOG, \
                    __LINE__,           \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/** 識別子変換テーブル型 */
typedef struct {
    uint32_t moduleId;                  /**< モジュール・サブモジュール識別子 */
    char     str[ LOG_LENGTH_ID + 1 ];  /**< 識別文字列                       */
} logIdTrans_t;

/** ログ管理テーブル型 */
typedef struct {
    uint32_t row;       /**< カーソル行 */
    uint32_t column;    /**< カーソル列 */
    uint8_t  attr;      /**< 文字属性   */
} logTbl_t;


/******************************************************************************/
/* ローカル変数定義                                                           */
/******************************************************************************/
#ifdef DEBUG_LOG_ENABLE
/** 識別子変換テーブル */
const static logIdTrans_t gIdTransTbl[ CMN_MODULE_NUM + 1 ] = {
    { CMN_MODULE_INIT_INIT,     "INI-INIT" },   /* 初期化制御(初期化)               */
    { CMN_MODULE_DEBUG_INIT,    "DBG-INIT" },   /* デバッグ制御(初期化)             */
    { CMN_MODULE_DEBUG_LOG,     "DBG-LOG " },   /* デバッグ制御(ログ管理)           */
    { CMN_MODULE_MEMMNG_INIT,   "MEM-INIT" },   /* メモリ管理(初期化)               */
    { CMN_MODULE_MEMMNG_GDT,    "MEM-GDT " },   /* メモリ管理(GDT管理)              */
    { CMN_MODULE_MEMMNG_AREA,   "MEM-AREA" },   /* メモリ管理(メモリ領域管理)       */
    { CMN_MODULE_MEMMNG_PAGE,   "MEM-PAGE" },   /* メモリ管理(ページ管理)           */
    { CMN_MODULE_MEMMNG_CTRL,   "MEM-CTRL" },   /* メモリ管理(メモリ制御)           */
    { CMN_MODULE_TASKMNG_INIT,  "TSK-INIT" },   /* タスク管理(初期化)               */
    { CMN_MODULE_TASKMNG_TSS,   "TSK-TSS " },   /* タスク管理(TSS管理)              */
    { CMN_MODULE_TASKMNG_SCHED, "TSK-SCHD" },   /* タスク管理(スケジューラ)         */
    { CMN_MODULE_TASKMNG_TASK,  "TSK-TASK" },   /* タスク管理(タスク管理)           */
    { CMN_MODULE_TASKMNG_ELF,   "TSK-ELF " },   /* タスク管理(ELFローダ)            */
    { CMN_MODULE_TASKMNG_PROC,  "TSK-PROC" },   /* タスク管理(プロセス管理)         */
    { CMN_MODULE_INTMNG_INIT,   "INT-INIT" },   /* 割込管理(初期化)                 */
    { CMN_MODULE_INTMNG_PIC,    "INT-PIC " },   /* 割込管理(PIC管理)                */
    { CMN_MODULE_INTMNG_IDT,    "INT-IDT " },   /* 割込管理(IDT管理)                */
    { CMN_MODULE_INTMNG_HDL,    "INT-HDL " },   /* 割込管理(ハンドラ管理)           */
    { CMN_MODULE_INTMNG_CTRL,   "INT-CTRL" },   /* 割込管理(ハードウェア割込み制御) */
    { CMN_MODULE_TIMERMNG_INIT, "TIM-INIT" },   /* タイマ管理(初期化)               */
    { CMN_MODULE_TIMERMNG_PIT,  "TIM-PIT " },   /* タイマ管理(PIT管理)              */
    { CMN_MODULE_ITCCTRL_INIT,  "ITC-INIT" },   /* タスク間通信制御(初期化)         */
    { CMN_MODULE_ITCCTRL_MSG,   "ITC-MSG " },   /* タスク間通信制御(メッセージ制御) */
    { CMN_MODULE_IOCTRL_INIT,   "IOC-INIT" },   /* 入出力制御(初期化)               */
    { CMN_MODULE_IOCTRL_PORT,   "IOC-PORT" },   /* 入出力制御(I/Oポート制御)        */
    { 0,                        "UNKNOWN " }  };/* 終端                             */
#endif

/** 数字変換表 */
static char gNumTransTbl[ 2 ][ 17 ] = { "0123456789abcdef",
                                        "0123456789ABCDEF"  };

/** ログ管理テーブル */
static logTbl_t gLogTbl;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
#ifdef DEBUG_LOG_ENABLE

/* 数値取得 */
static CmnRet_t LogGetNum( char     *pStr,
                           uint32_t *pValue,
                           uint32_t *pLength );

/* 書式付き文字列出力（可変長引数型） */
static void LogOutput( char *pFormat,
                       ...            );

/* 書式付き文字列出力（可変長引数リスト型） */
static void LogOutputByVaList( char     *pFormat,
                               va_list  vaList    );

/* 一文字出力 */
static void LogOutputChar( char c );

/* 整数出力 */
static void LogOutputNumber( uint32_t value,
                             uint32_t base,
                             uint32_t flags,
                             int32_t  width  );

/* 文字列出力 */
static void LogOutputString( char *pStr );

/* ESCコード処理 */
static uint32_t LogProcEscape( char *pStr );

/* 文字属性エスケープシーケンス処理 */
static void LogProcEscapeAttr( uint32_t funcNo );

/* 変換指定子処理 */
static uint32_t LogProcFormat( char    *pFormat,
                               va_list *pVaList  );

#endif


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ログ管理初期化
 * @details     ログ管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void DebugLogInit( void )
{
    /* ログ管理テーブル初期化 */
    memset( &gLogTbl, 0, sizeof ( logTbl_t ) );
    
    /* 文字属性設定 */
    gLogTbl.attr = VGA_M3_ATTR_FG_WHITE  |  /* 白色文字属性 */
                   VGA_M3_ATTR_FG_BRIGHT |  /* 明色文字属性 */
                   VGA_M3_ATTR_BG_BLACK;    /* 黒色背景属性 */
    
    return;
}


/******************************************************************************/
/**
 * @brief       トレースログ出力
 * @details     デバッグトレースログオプションが有効の時、画面にトレースログ出
 *              力を行う。
 * 
 * @param[in]   moduleId モジュール・サブモジュール識別子
 *                  - CMN_MODULE_INIT_INIT     初期化制御(初期化)
 *                  - CMN_MODULE_DEBUG_INIT    デバッグ制御(初期化)
 *                  - CMN_MODULE_DEBUG_LOG     デバッグ制御(ログ管理)
 *                  - CMN_MODULE_MEMMNG_INIT   メモリ管理(初期化)
 *                  - CMN_MODULE_MEMMNG_GDT    メモリ管理(GDT管理)
 *                  - CMN_MODULE_MEMMNG_AREA   メモリ管理(メモリ領域管理)
 *                  - CMN_MODULE_MEMMNG_PAGE   メモリ管理(ページ管理)
 *                  - CMN_MODULE_MEMMNG_CTRL   メモリ管理(メモリ制御)
 *                  - CMN_MODULE_TASKMNG_INIT  タスク管理(初期化)
 *                  - CMN_MODULE_TASKMNG_TSS   タスク管理(TSS管理)
 *                  - CMN_MODULE_TASKMNG_SCHED タスク管理(スケジューラ)
 *                  - CMN_MODULE_TASKMNG_TASK  タスク管理(タスク管理)
 *                  - CMN_MODULE_TASKMNG_ELF   タスク管理(ELFローダ)
 *                  - CMN_MODULE_TASKMNG_PROC  タスク管理(プロセス管理)
 *                  - CMN_MODULE_INTMNG_INIT   割込管理(初期化)
 *                  - CMN_MODULE_INTMNG_PIC    割込管理(PIC管理)
 *                  - CMN_MODULE_INTMNG_IDT    割込管理(IDT管理)
 *                  - CMN_MODULE_INTMNG_HDL    割込管理(ハンドラ管理)
 *                  - CMN_MODULE_TIMERMNG_INIT タイマ管理(初期化)
 *                  - CMN_MODULE_TIMERMNG_PIT  タイマ管理(PIT管理) *                  - CMN_MODULE_ITCCTRL_INIT  タスク間通信制御(初期化)
 *                  - CMN_MODULE_ITCCTRL_MSG   タスク間通信制御(メッセージ制御)
 * @param[in]   lineNum  行番号
 * @param[in]   *pFormat トレースログ
 * 
 * @note        デバッグトレースログオプションの有効化は、コンパイルオプション
 *              にて「DEBUG_LOG_ENABLE」マクロを定義する事で行う。
 */
/******************************************************************************/
void DebugLogOutput( uint32_t moduleId,
                     uint32_t lineNum,
                     char     *pFormat,
                     ...                )
{
#ifdef DEBUG_LOG_ENABLE
    va_list  vaList;    /* 可変長引数リスト         */
    uint32_t row;       /* カーソル行               */
    uint32_t column;    /* カーソル列               */
    uint32_t moduleIdx; /* 変換テーブルインデックス */
    
    /* 可変長引数リスト設定 */
    va_start( vaList, pFormat );
    
    /* カーソル列初期化 */
    gLogTbl.column = 0;
    
    /* カーソル行範囲判定 */
    if ( gLogTbl.row >= VGA_M3_ROW ) {
        /* 上限越え */
        
        /* カーソル行設定 */
        gLogTbl.row = VGA_M3_ROW - 1;
        
        /* 画面スクロール */
        for ( row = 0; row < ( VGA_M3_ROW - 1 ); row++ ) {
            /* 一行コピー */
            memcpy( LOG_CURSOR_ADDR( row, 0 ),
                    LOG_CURSOR_ADDR( row + 1, 0 ),
                    VGA_M3_COLUMN * 2 );
        }
        
        /* 最下行初期化 */
        for ( column = 0; column < VGA_M3_COLUMN; column++ ) {
            /* 一文字設定 */
            LOG_CURSOR_ADDR( row, column )[ 0 ] = ' ';
            LOG_CURSOR_ADDR( row, column )[ 1 ] =
                VGA_M3_ATTR_FG_WHITE  |     /* 白色文字属性 */
                VGA_M3_ATTR_FG_BRIGHT |     /* 明色文字属性 */
                VGA_M3_ATTR_BG_BLACK;       /* 黒色背景属性 */
        }
    }
    
    /* モジュール・サブモジュール識別子変換 */
    for ( moduleIdx = 0; moduleIdx < CMN_MODULE_NUM; moduleIdx++ ) {
        /* 識別子変換テーブル一致判定 */
        if ( gIdTransTbl[ moduleIdx ].moduleId == moduleId ) {
            /* 一致 */
            break;
        }
    }
    
    /* モジュール・サブモジュール識別子、行番号出力 */
    LogOutput( "\033[32m%s:%04u \033[0m",
               gIdTransTbl[ moduleIdx ].str,
               lineNum % 10000               );
    
    /* トレースログ出力 */
    LogOutputByVaList( pFormat, vaList );
    
    /* カーソル行更新 */
    gLogTbl.row++;
    
    /* 可変長引数リスト解放 */
    va_end( vaList );
    
#endif
    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       数値取得
 * @details     数字文字列から数値を取得する。
 * 
 * @param[in]   *pStr    文字列
 * @param[out]  *pVaule  数値
 * @param[out]  *pLength 数字文字列長
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了（数字文字列無し）
 * 
 * @note        「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 */
/******************************************************************************/
static CmnRet_t LogGetNum( char     *pStr,
                           uint32_t *pValue,
                           uint32_t *pLength )
{
    /* 初期化 */
    *pValue  = 0;
    *pLength = 0;
    
    /* 一文字毎に繰り返す */
    while ( pStr[ *pLength ] != '\0' ) {
        /* 数値判別 */
        if ( ( '0' <= pStr[ *pLength ] ) && ( pStr[ *pLength ] <= '9' ) ) {
            /* 数値 */
            *pValue = *pValue * 10 + pStr[ *pLength ] - '0';
            
        } else {
            /* 数値以外 */
            
            break;
        }
        
        /* 文字列長更新 */
        ( *pLength )++;
    }
    
    /* 文字列長判定 */
    if ( *pLength == 0 ) {
        /* 数値無 */
        
        return CMN_FAILURE;
    }
    
    return CMN_SUCCESS;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       書式付き文字列出力（可変長引数型）
 * @details     画面に可変長引数型書式付き文字列を出力する。
 * 
 * @param[in]   *pFormat 書式付き文字列
 * @param[in]   ...      可変長引数
 * 
 * @note        「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 */
/******************************************************************************/
static void LogOutput( char *pFormat,
                       ...            )
{
    va_list vaList; /* 可変長引数リスト */
    
    /* 可変長引数リスト設定 */
    va_start( vaList, pFormat );
    
    /* 書式付き文字列出力 */
    LogOutputByVaList( pFormat, vaList );
    
    /* 可変長引数リスト解放 */
    va_end( vaList );
    
    return;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       書式付き文字列出力（可変長引数リスト型）
 * @details     画面に可変長引数リスト型書式付き文字列を出力する。
 * 
 * @param[in]   *pFormat 書式付き文字列
 * @param[in]   vaList   可変長引数リスト
 * 
 * @note        「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 */
/******************************************************************************/
static void LogOutputByVaList( char     *pFormat,
                               va_list  vaList    )
{
    uint32_t idx;   /* インデックス */
    
    /* 初期化 */
    idx = 0;
    
    /* 一文字毎に繰り返し */
    while ( pFormat[ idx ] != '\0' ) {
        /* 文字判定 */
        switch ( pFormat[ idx ] ) {
            case '%':
                /* 変換指定子 */
                
                /* インデックス更新 */
                idx++;
                
                /* 変換指定処理 */
                idx += LogProcFormat( &pFormat[ idx ], &vaList );
                
                break;
                
            case '\033':
                /* ESCコード */
                
                /* インデックス更新 */
                idx++;
                
                /* ESCコード処理 */
                idx += LogProcEscape( &pFormat[ idx ] );
                
                break;
                
            default:
                /* その他 */
                
                /* 一文字出力 */
                LogOutputChar( pFormat[ idx ] );
                
                /* インデックス更新 */
                idx++;
                
                break;
        }
    }
    
    return;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       一文字出力
 * @details     画面に一文字出力する。
 * 
 * @param[in]   c 文字コード
 * 
 * @note        「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 */
/******************************************************************************/
static void LogOutputChar( char c )
{
    uint8_t *pCursor;   /* カーソル */
    
    /* カーソル列チェック */
    if ( gLogTbl.column >= VGA_M3_COLUMN ) {
        /* 行数超 */
        
        return;
    }
    
    /* カーソル設定 */
    pCursor = LOG_CURSOR_ADDR( gLogTbl.row, gLogTbl.column );
    
    /* 画面出力 */
    pCursor[ 0 ] = ( uint8_t ) c;   /* 文字コード */
    pCursor[ 1 ] = gLogTbl.attr;    /* 文字属性   */
    
    /* カーソル更新 */
    gLogTbl.column++;
    
    return;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       整数出力
 * @details     整数を画面に出力する。
 * 
 * @param[in]   value 数値
 * @param[in]   base  進数
 *                  - 8  8進数
 *                  - 10 10進数
 *                  - 16 16進数
 * @param[in]   flags フラグ
 *                  - LOG_FLAG_LEFT      左寄せ
 *                  - LOG_FLAG_SIGN      符号表示
 *                  - LOG_FLAG_SPACE     正数符号空白
 *                  - LOG_FLAG_ALTERNATE 代替形式
 *                  - LOG_FLAG_ZERO      0埋め
 *                  - LOG_FLAG_UNSIGNED  符号無
 * @param[in]   width 最小フィールド幅
 *                  - 0～80
 * 
 * @note        「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 */
/******************************************************************************/
static void LogOutputNumber( uint32_t value,
                             uint32_t base,
                             uint32_t flags,
                             int32_t  width  )
{
    char     buffer[ VGA_M3_COLUMN ];   /* 出力バッファ     */
    char     *pTrans;                   /* 数字変換表       */
    int32_t  length;                    /* バッファ文字列長 */
    uint32_t tmp;                       /* 一時変数         */
    uint32_t idx;                       /* インデックス     */
    
    /* 初期化 */
    memset( buffer, '0', sizeof ( buffer ) );
    length = 0;
    tmp    = value;
    
    /* 最小フィールド幅範囲チェック */
    if ( ( width < 0 ) && ( VGA_M3_COLUMN <= width ) ) {
        /* 0未満、LOG_LENGTH_COLUMN超過 */
        
        /* 最小フィールド幅初期化 */
        width = 0;
    }
    
    /* 左寄せフラグ判定 */
    if ( ( flags & LOG_FLAG_LEFT ) != 0 ) {
        /* 左寄せフラグ有 */
        
        /* 0フラグ無効化 */
        flags = flags & ~LOG_FLAG_ZERO;
    }
    
    /* 符号無フラグ判定 */
    if ( ( flags & LOG_FLAG_UNSIGNED ) == 0 ) {
        /* 符号無フラグ無 */
        
        /* 負数判定 */
        if ( ( ( int32_t ) tmp ) < 0 ) {
            /* 負数 */
            
            /* 正数化 */
            tmp = ( uint32_t ) ( ( ( int32_t ) tmp ) * -1 );
        }
        
    } else {
        /* 符号無フラグ有 */
        
        /* フラグ無効化 */
        flags &= ~LOG_FLAG_SIGN;    /* 符号表示     */
        flags &= ~LOG_FLAG_SPACE;   /* 正数符号空白 */
    }
    
    /* 大文字フラグ判定 */
    if ( ( flags & LOG_FLAG_UPPERCASE ) == 0 ) {
        /* 大文字フラグ無 */
        
        /* 数字変換表設定 */
        pTrans = gNumTransTbl[ 0 ];
        
    } else {
        /* 大文字フラグ有 */
        
        /* 数字変換表設定 */
        pTrans = gNumTransTbl[ 1 ];
    }
    
    /* 代替形式フラグ判定 */
    if ( ( flags & LOG_FLAG_ALTERNATE ) != 0 ) {
        /* 代替表示フラグ有 */
        
        /* 進数判定 */
        if ( base == 8 ) {
            /* 8進数 */
            
            /* 最小フィールド幅から代替形式出力分減算 */
            width -= 1;
            
        } else if ( base == 16 ) {
            /* 16進数 */
            
            /* 最小フィールド幅から代替形式出力分減算 */
            width -= 2;
        }
    }
    
    /* 符号表示フラグ、正数符号空白フラグ判定 */
    if ( ( ( flags & LOG_FLAG_SIGN  ) != 0 ) ||
         ( ( flags & LOG_FLAG_SPACE ) != 0 )    ) {
        /* 符号表示フラグ有、または、正数符号空白フラグ有 */
        
        /* 最小フィールド幅から符号出力分減算 */
        width -= 1;
    }
    
    /* 一桁毎に繰り返し */
    do {
        /* 一桁文字列変換 */
        buffer[ length++ ] = pTrans[ tmp % base ];
        
        /* 値更新 */
        tmp /= base;
        
    } while ( tmp != 0 );
    
    /* 0埋めフラグ、最小フィールド幅判定 */
    if ( ( ( flags & LOG_FLAG_ZERO ) != 0      ) &&
         ( width                     >  length )    ) {
        /* 0埋めフラグ有、かつ、最小フィールド幅未満 */
        
        /* 最小フィールド幅合わせ */
        length = width;
    }
    
    /* 代替形式フラグ判定 */
    if ( ( flags & LOG_FLAG_ALTERNATE ) != 0 ) {
        /* 代替表示フラグ有 */
        
        /* 進数判定 */
        if ( base == 8 ) {
            /* 8進数 */
            
            /* 0をバッファ出力 */
            buffer[ length++ ] = '0';
            
        } else if ( base == 16 ) {
            /* 16進数 */
            
            /* 0xをバッファ出力 */
            buffer[ length++ ] = 'x';
            buffer[ length++ ] = '0';
        }
    }
    
    /* 符号表示フラグ・正数判定 */
    if ( ( ( flags & LOG_FLAG_SIGN ) != 0 ) &&
         ( ( int32_t ) value         >= 0 )    ) {
        /* 符号表示フラグ有、かつ、正数 */
        buffer[ length++ ] = '+';
    }
    
    /* 正数符号空白フラグ・正数判定 */
    if ( ( ( flags & LOG_FLAG_SPACE ) != 0 ) &&
         ( ( int32_t ) value          >= 0 )    ) {
        /* 正数符号空白フラグ有、かつ、正数 */
        buffer[ length++ ] = ' ';
    }
    
    /* 符号表示フラグ・正数符号空白フラグ・負数判定 */
    if ( ( ( ( flags & LOG_FLAG_SIGN  ) != 0 ) ||
           ( ( flags & LOG_FLAG_SPACE ) != 0 )    ) &&
         ( ( int32_t ) value < 0                  )    ) {
        /* 符号表示または正数符号空白フラグ有、かつ、負数 */
        buffer[ length++ ] = '-';
    }
    
    /* 左寄せフラグ、最小フィールド幅判定 */
    if ( ( ( flags & LOG_FLAG_LEFT ) == 0      ) &&
         ( width                     >  length )    ) {
        /* 左寄せフラグ無、かつ、最小フィールド幅未満 */
        
        /* 空白挿入 */
        for ( idx = 0; idx < ( uint32_t ) ( width - length ); idx++ ) {
            /* 一文字出力 */
            LogOutputChar( ' ' );
        }
    }
    
    /* バッファ出力 */
    for ( idx = 0; idx < ( uint32_t ) length; idx++ ) {
        /* 一文字出力 */
        LogOutputChar( buffer[ ( uint32_t ) length - idx - 1 ] );
    }
    
    /* 左寄せフラグ判定 */
    if ( ( ( flags & LOG_FLAG_LEFT ) != 0      ) &&
         ( width                     >  length )    ) {
        /* 左寄せフラグ有、かつ、最小フィールド幅未満 */
        
        /* 空白挿入 */
        for ( idx = 0; idx < ( uint32_t ) ( width - length ); idx++ ) {
            /* 一文字出力 */
            LogOutputChar( ' ' );
        }
    }
    
    return;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       文字列出力
 * @details     文字列を画面に出力する。
 * 
 * @param[in]   pStr 文字列
 * 
 * @note        「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 */
/******************************************************************************/
static void LogOutputString( char *pStr )
{
    uint32_t idx;   /* インデックス */
    
    /* 初期化 */
    idx = 0;
    
    /* 一文字毎に繰り返し */
    while ( pStr[ idx ] != '\0' ) {
        /* 文字判定 */
        switch ( pStr[ idx ] ) {
            case '\033':
                /* ESCコード */
                
                /* インデックス更新 */
                idx++;
                
                /* ESCコード処理 */
                idx += LogProcEscape( &pStr[ idx ] );
                
                break;
                
            default:
                /* 他 */
                
                /* 一文字出力 */
                LogOutputChar( pStr[ idx ] );
                
                /* インデックス更新 */
                idx++;
                
                break;
        }
    }
    
    return;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       ESCコード処理
 * @details     ESCコードを処理する。
 * 
 * @param[in]   *pStr エスケープシーケンス
 * 
 * @return      エスケープシーケンス文字数
 * 
 * @note        - 「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 *              - 画面制御エスケープシーケンスには対応しない。
 */
/******************************************************************************/
static uint32_t LogProcEscape( char *pStr )
{
    CmnRet_t ret;       /* 関数戻り値   */
    uint32_t idx;       /* インデックス */
    uint32_t value;     /* 数値         */
    uint32_t length;    /* 文字列長     */
    
    /* 初期化 */
    idx    = 1;
    value  = 0;
    length = 0;
    
    /* エスケープシーケンスチェック */
    if ( pStr[ 0 ] != '['  ) {
        /* エスケープシーケンスでない */
        
        return 0;
    }
    
    /* 数値取得 */
    ret = LogGetNum( &pStr[ idx ], &value, &length );
    
    /* 取得結果判定 */
    if ( ret == CMN_SUCCESS ) {
        /* 成功 */
        
        /* インデックス更新 */
        idx += length;
    }
    
    /* エスケープシーケンス種別判定 */
    switch ( pStr[ idx ] ) {
        case 'm':
            /* 文字属性 */
            
            /* 文字属性エスケープシーケンス処理 */
            LogProcEscapeAttr( value );
            
            /* 文字列長設定 */
            length = idx + 1;
            
            break;
            
        default:
            /* 他 */
            
            /* 文字列長設定 */
            length = idx;
            
            break;
    }
    
    return length;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       文字属性エスケープシーケンス処理
 * @details     文字属性のエスケープシーケンスを処理する。
 * 
 * @param[in]   funcNo 機能番号
 *                  - 0  属性初期化
 *                  - 1  文字強調
 *                  - 4  下線
 *                  - 7  反転
 *                  - 30 黒色文字
 *                  - 31 赤色文字
 *                  - 32 緑色文字
 *                  - 33 黄色文字
 *                  - 34 青色文字
 *                  - 35 紫色文字
 *                  - 36 水色文字
 *                  - 37 白色文字
 *                  - 39 標準色文字
 *                  - 40 黒色背景
 *                  - 41 赤色背景
 *                  - 42 緑色背景
 *                  - 43 黄色背景
 *                  - 44 青色背景
 *                  - 45 紫色背景
 *                  - 46 水色背景
 *                  - 47 白色背景
 *                  - 49 標準色背景
 * 
 * @note        - 「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 *              - 文字強調、下線は対応しない。
 */
/******************************************************************************/
static void LogProcEscapeAttr( uint32_t funcNo )
{
    uint8_t attr;   /* 文字属性 */
    
    /* 機能番号判定 */
    switch ( funcNo ) {
        case 0:
            /* 属性初期化 */
            
            gLogTbl.attr = VGA_M3_ATTR_FG_WHITE  |  /* 白色文字属性 */
                           VGA_M3_ATTR_FG_BRIGHT |  /* 明色文字属性 */
                           VGA_M3_ATTR_BG_BLACK;    /* 黒色背景属性 */
            
            break;
            
        case 7:
            /* 反転 */
            
            /* 反転 */
            attr  = ( LOG_ATTR_BG( gLogTbl.attr ) >> 4 ) |
                    VGA_M3_ATTR_FG_BRIGHT;
            attr &= ( LOG_ATTR_FG( gLogTbl.attr ) << 4 ) & ~VGA_M3_ATTR_BLINK;
            
            /* 設定 */
            gLogTbl.attr = attr;
            
            break;
            
        case 30:
            /* 黒色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_BLACK | VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 31:
            /* 赤色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_RED | VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 32:
            /* 緑色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_GREEN | VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 33:
            /* 黄色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_BROWN | VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 34:
            /* 青色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_BLUE | VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 35:
            /* 紫色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_PURPLE | 
                                     VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 36:
            /* 水色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_CYAN | VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 37:
            /* 白色文字 *//* FALL THROUGH */
        case 39:
            /* 標準色文字 */
            gLogTbl.attr =
                LOG_ATTR_FG_CHG( gLogTbl.attr,
                                 VGA_M3_ATTR_FG_WHITE | VGA_M3_ATTR_FG_BRIGHT );
            break;
            
        case 40:
            /* 黒色背景 *//* FALL THROUGH */
        case 49:
            /* 標準色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr,
                                            VGA_M3_ATTR_BG_BLACK );
            break;
            
        case 41:
            /* 赤色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr, VGA_M3_ATTR_BG_RED );
            break;
            
        case 42:
            /* 緑色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr, VGA_M3_ATTR_BG_RED );
            break;
            
        case 43:
            /* 黄色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr,
                                            VGA_M3_ATTR_BG_BROWN );
            break;
            
        case 44:
            /* 青色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr, VGA_M3_ATTR_BG_BLUE );
            break;
            
        case 45:
            /* 紫色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr,
                                            VGA_M3_ATTR_BG_PURPLE );
            break;
            
        case 46:
            /* 水色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr, VGA_M3_ATTR_BG_CYAN );
            break;
            
        case 47:
            /* 白色背景 */
            gLogTbl.attr = LOG_ATTR_BG_CHG( gLogTbl.attr,
                                            VGA_M3_ATTR_BG_WHITE );
            break;
            
        default:
            /* 他 */
            break;
    }
    
    return;
}
#endif


#ifdef DEBUG_LOG_ENABLE
/******************************************************************************/
/**
 * @brief       変換指定子処理
 * @details     変換指定子を処理する。
 * 
 * @param[in]   *pFormat 変換指定子
 * @param[in]   vaList   可変長引数リスト
 * 
 * @note        - 「DEBUG_LOG_ENABLE」マクロが定義されている場合に有効となる。
 *              - 精度、長さ修飾子は未対応。
 */
/******************************************************************************/
static uint32_t LogProcFormat( char    *pFormat,
                               va_list *pVaList  )
{
    uint32_t idx;       /* インデックス     */
    uint32_t width;     /* 最小フィールド幅 */
    uint32_t flags;     /* フラグ           */
    uint32_t length;    /* 文字列長         */
    CmnRet_t ret;       /* 関数戻り値       */
    
    /* 初期化 */
    idx    = 0;
    flags  = 0;
    width  = 0;
    length = 0;
    
    /* フラグ判定 */
    while ( 1 ) {
        switch ( pFormat[ idx ] ) {
            case '-':
                /* 左寄せ */
                idx++;                      /* インデックス更新 */
                flags |= LOG_FLAG_LEFT;     /* フラグ設定       */
                continue;
                
            case '+':
                /* 符号表示 */
                idx++;                      /* インデックス更新 */
                flags |= LOG_FLAG_SIGN;     /* フラグ設定       */
                continue;
                
            case ' ':
                /* 正数符号空白 */
                idx++;                      /* インデックス更新 */
                flags |= LOG_FLAG_SPACE;    /* フラグ設定       */
                continue;
                
            case '#':
                /* 代替形式 */
                idx++;                      /* インデックス更新 */
                flags |= LOG_FLAG_ALTERNATE;/* フラグ設定       */
                continue;
                
            case '0':
                /* 0埋め */
                idx++;                      /* インデックス更新 */
                flags |= LOG_FLAG_ZERO;     /* フラグ設定       */
                continue;
                
            case '\0':
                /* 終端 */
                return 0;
                
            default:
                /* 他 */
                break;
        }
        break;
    }
    
    /* 最小フィールド幅取得 */
    ret = LogGetNum( &pFormat[ idx ], &width, &length );
    
    /* 取得結果判定 */
    if ( ret == CMN_SUCCESS ) {
        /* 最小フィールド幅有り */
        
        /* インデックス更新 */
        idx += length;
    }
    
    /* 変換指定子判定 */
    switch ( pFormat[ idx ] ) {
        case 'u':
            /* 10進符号無整数 *//* FALL THROUGH */
            
            /* フラグ設定 */
            flags |= LOG_FLAG_UNSIGNED;     /* 符号無 */
            
        case 'd':
            /* 10進符号付整数 *//* FALL THROUGH */
        case 'i':
            /* 10進符号付整数 */
            
            /* 10進整数出力 */
            LogOutputNumber( va_arg( *pVaList, uint32_t ), 10, flags, width );
            
            /* 文字列長設定 */
            length = 1;
            
            break;
            
        case 'o':
            /* 8進符号無整数 */
            
            /* フラグ設定 */
            flags |= LOG_FLAG_UNSIGNED;     /* 符号無 */
            
            /* 8進整数出力 */
            LogOutputNumber( va_arg( *pVaList, uint32_t ), 8, flags, width );
            
            /* 文字列長設定 */
            length = 1;
            
            break;
            
        case 'X':
            /* 16進符号無整数（大文字） *//* FALL THROUGH */
            
            /* フラグ設定 */
            flags |= LOG_FLAG_UNSIGNED;     /* 符号無 */
            flags |= LOG_FLAG_UPPERCASE;    /* 大文字 */
            
        case 'x':
            /* 16進符号無整数（小文字） */
            
            /* フラグ設定 */
            flags |= LOG_FLAG_UNSIGNED;     /* 符号無 */
            
            /* 16進整数出力 */
            LogOutputNumber( va_arg( *pVaList, uint32_t ), 16, flags, width );
            
            /* 文字列長設定 */
            length = 1;
            
            break;
            
        case 'c':
            /* 文字 */
            
            /* 一文字出力 */
            LogOutputChar( ( char ) va_arg( *pVaList, int ) );
            
            /* 文字列長設定 */
            length = 1;
            
            break;
            
        case 's':
            /* 文字列 */
            
            /* 文字列出力 */
            LogOutputString( va_arg( *pVaList, char * ) );
            
            /* 文字列長設定 */
            length = 1;
            
            break;
            
        case 'p':
            /* ポインタ */
            
            /* フラグ設定 */
            flags |= LOG_FLAG_ALTERNATE;    /* 代替表示 */
            flags |= LOG_FLAG_UNSIGNED;     /* 符号無   */
            
            /* 16進整数出力 */
            LogOutputNumber( va_arg( *pVaList, uint32_t ), 16, flags, width );
            
            /* 文字列長設定 */
            length = 1;
            
            break;
            
        case '%':
            /* % */
            
            /* 一文字出力 */
            LogOutputChar( '%' );
            
            /* 文字列長設定 */
            length = 1;
            
            break;
            
        default:
            /* 他 */
            
            /* 文字列長設定 */
            length = 0;
            
            break;
    }
    
    return idx + length;
}
#endif


/******************************************************************************/
