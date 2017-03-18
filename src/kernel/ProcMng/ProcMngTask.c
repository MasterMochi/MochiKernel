/******************************************************************************/
/* src/kernel/ProcMng/ProcMngTask.c                                           */
/*                                                                 2017/03/16 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>
#include <ProcMng.h>

/* 内部モジュールヘッダ */
#include "ProcMngSched.h"
#include "ProcMngTask.h"
#include "ProcMngTss.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_PROCMNG_TASK,    \
                    __LINE__,                   \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/* タスクID使用フラグ */
#define TASK_ID_UNUSED ( 0 )    /** 未使用 */
#define TASK_ID_USED   ( 1 )    /** 使用済 */

/* スタックサイズ */
#define TASK_KERNEL_STACK_SIZE ( 1024000 )  /**< カーネルスタックサイズ */
#define TASK_STACK_SIZE        ( 1024000 )  /**< スタックサイズ         */

/** タスクスタック情報 */
typedef struct {
    void   *pTopAddr;       /**< 先頭アドレス */
    void   *pBottomAddr;    /**< 後尾アドレス */
    size_t size;            /**< サイズ       */
} taskStackInfo_t;

/** タスク管理テーブル構造体 */
typedef struct {
    uint8_t              used;              /**< 使用フラグ               */
    uint8_t              type;              /**< タスクタイプ             */
    uint8_t              state;             /**< タスク状態               */
    uint8_t              reserved;          /**< パディング               */
    ProcMngTaskContext_t context;           /**< コンテキスト             */
    void                 *pEntryPoint;      /**< エントリポイント         */
    taskStackInfo_t      kernelStackInfo;   /**< カーネルスタックアドレス */
    taskStackInfo_t      stackInfo;         /**< スタックアドレス         */
} taskTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** タスク管理テーブル */
static taskTbl_t gTaskTbl[ PROCMNG_TASK_ID_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスク追加
 * @details     タスク追加を行う。
 * 
 * @param[in]   taskType     タスクタイプ
 *                  - PROCMNG_TASK_TYPE_DRIVER ドライバ
 *                  - PROCMNG_TASK_TYPE_SERVER サーバ
 *                  - PROCMNG_TASK_TYPE_USER   ユーザ
 * @param[in]   *pEntryPoint エントリポイント
 * 
 * @retval      PROCMNG_TASK_ID_NULL 失敗
 * @retval      PROCMNG_TASK_ID_MIN  タスクID最小値
 * @retval      PROCMNG_TASK_ID_MAX  タスクID最大値
 */
/******************************************************************************/
uint32_t ProcMngTaskAdd( uint8_t taskType,
                         void    *pEntryPoint )
{
    void            *pKernelStack; /* カーネルスタック */
    void            *pStack;       /* スタック         */
    int32_t         ret;           /* 関数戻り値       */
    uint32_t        taskId;        /* タスクID         */
    taskStackInfo_t *pStackInfo;   /* スタック情報     */
    
    /* 初期化 */
    pKernelStack = NULL;
    pStack       = NULL;
    ret          = CMN_FAILURE;
    taskId       = PROCMNG_TASK_ID_MIN;
    pStackInfo   = NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. taskType=%u, pEntryPoint=%010p",
               __func__,
               taskType,
               pEntryPoint );
    
    /* 空タスク検索 */
    for ( ; taskId < PROCMNG_TASK_ID_MAX; taskId++ ) {
        /* 使用フラグ判定 */
        if ( gTaskTbl[ taskId ].used == TASK_ID_UNUSED ) {
            /* 未使用 */
            
            /* カーネルスタック領域割り当て */
            pKernelStack = MemMngAreaAlloc( TASK_KERNEL_STACK_SIZE );
            
            /* スタック領域割り当て */
            pStack = MemMngAreaAlloc( TASK_STACK_SIZE );
            
            /* 割り当て結果判定 */
            if ( ( pKernelStack == NULL ) ||
                 ( pStack       == NULL )    ) {
                /* 失敗 */
                
                /* [TODO] */
                /* 設定したタスクを初期化する処理。異常処理はちょっと後回し */
                
                return PROCMNG_TASK_ID_NULL;
            }
            
            /* タスク基本設定 */
            gTaskTbl[ taskId ].used        = TASK_ID_USED;
            gTaskTbl[ taskId ].type        = taskType;
            gTaskTbl[ taskId ].state       = 0;
            gTaskTbl[ taskId ].context.eip = ( uint32_t ) ProcMngTaskStart;
            gTaskTbl[ taskId ].context.esp = ( uint32_t ) pKernelStack + TASK_KERNEL_STACK_SIZE;
            gTaskTbl[ taskId ].pEntryPoint = pEntryPoint;
            
            /* カーネルスタック情報設定 */
            pStackInfo              = &( gTaskTbl[ taskId ].kernelStackInfo );
            pStackInfo->pTopAddr    = pKernelStack;
            pStackInfo->pBottomAddr = pKernelStack + TASK_KERNEL_STACK_SIZE;
            pStackInfo->size        = TASK_KERNEL_STACK_SIZE;
            
            /* スタック情報設定 */
            pStackInfo              = &( gTaskTbl[ taskId ].stackInfo );
            pStackInfo->pTopAddr    = pStack;
            pStackInfo->pBottomAddr = pStack + TASK_STACK_SIZE;
            pStackInfo->size        = TASK_STACK_SIZE;
            
            /* スケジューラ追加 */
            ret = ProcMngSchedAdd( taskId );
            
            /* 追加結果判定 */
            if ( ret == CMN_FAILURE ) {
                /* 失敗 */
                
                /* [TODO] */
                /* 設定したタスクを初期化する処理。異常処理はちょっと後回し */
                
                return PROCMNG_TASK_ID_NULL;
            }
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%u", __func__, taskId );
            
            return taskId;
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%u", __func__, PROCMNG_TASK_ID_NULL );
    
    return PROCMNG_TASK_ID_NULL;
}


/******************************************************************************/
/**
 * @brief       コンテキスト取得
 * @details     指定したタスクIDのコンテキストを取得する。
 * 
 * @param[in]   taskId タスクID
 *                  - PROCMNG_TASK_ID_MIN タスクID最小値
 *                  - PROCMNG_TASK_ID_MAX タスクID最大値
 * 
 * @return      コンテキスト
 */
/******************************************************************************/
ProcMngTaskContext_t ProcMngTaskGetContext( uint32_t taskId )
{
    /* コンテキスト返却 */
    return gTaskTbl[ taskId ].context;
}


/******************************************************************************/
/**
 * @brief       カーネルスタックアドレス取得
 * @details     指定したタスクIDのカーネルスタックアドレスを取得する。
 * 
 * @param[in]   taskId タスクID
 *                  - PROCMNG_TASK_ID_MIN タスクID最小値
 *                  - PROCMNG_TASK_ID_MAX タスクID最大値
 * 
 * @return      カーネルスタックアドレス
 */
/******************************************************************************/
void *ProcMngTaskGetKernelStack( uint32_t taskId )
{
    /* カーネルスタックアドレス返却 */
    return gTaskTbl[ taskId ].kernelStackInfo.pBottomAddr;
}


/******************************************************************************/
/**
 * @brief       タスクタイプ取得
 * @details     指定したタスクIDのタスクタイプを取得する。
 * 
 * @param[in]   taskId タスクID
 *                  - PROCMNG_TASK_ID_MIN タスクID最小値
 *                  - PROCMNG_TASK_ID_MAX タスクID最大値
 * 
 * @retval      PROCMNG_TASK_TYPE_DRIVER ドライバ
 * @retval      PROCMNG_TASK_TYPE_SERVER サーバ
 * @retval      PROCMNG_TASK_TYPE_USER   ユーザ
 */
/******************************************************************************/
uint8_t ProcMngTaskGetType( uint32_t taskId )
{
    /* タスクタイプ返却 */
    return gTaskTbl[ taskId ].type;
}


/******************************************************************************/
/**
 * @brief       タスク管理初期化
 * @details     タスク管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void ProcMngTaskInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* タスク管理テーブル初期化 */
    memset( gTaskTbl, 0, sizeof ( gTaskTbl ) );
    
    /* アイドルタスク設定 */
    gTaskTbl[ PROCMNG_TASK_ID_IDLE ].used = TASK_ID_USED;   /* 使用フラグ */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       コンテキスト設定
 * @details     指定したタスクIDのコンテキストを設定する。
 * 
 * @param[in]   taskId    設定先タスクID
 *                  - PROCMNG_TASK_ID_MIN タスクID最小値
 *                  - PROCMNG_TASK_ID_MAX タスクID最大値
 * @param[in]   *pContext コンテキスト
 */
/******************************************************************************/
void ProcMngTaskSetContext( uint32_t             taskId,
                            ProcMngTaskContext_t *pContext )
{
    /* コンテキスト設定 */
    gTaskTbl[ taskId ].context = *pContext;
    
    return;
}


/******************************************************************************/
/**
 * @brief       タスク起動開始
 * @details     タスクの起動を開始する。
 */
/******************************************************************************/
void ProcMngTaskStart( void )
{
    void      *pEntryPoint; /* エントリポイント         */
    void      *pStack;      /* スタックアドレス         */
    uint8_t   taskType;     /* タスクタイプ             */
    uint32_t  taskId;       /* タスクID                 */
    uint32_t  codeSegSel;   /* コードセグメントセレクタ */
    uint32_t  dataSegSel;   /* データセグメントセレクタ */
    taskTbl_t *pTask;       /* タスク管理情報           */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 初期化 */
    taskId      = ProcMngSchedGetTaskId();      /* タスクID         */
    pTask       = &( gTaskTbl[ taskId ] );      /* タスク管理情報   */
    pEntryPoint = pTask->pEntryPoint;           /* エントリポイント */
    pStack      = pTask->stackInfo.pBottomAddr; /* スタックアドレス */
    taskType    = pTask->type;                  /* タスクタイプ     */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "taskId=%d, pEntryPoint=%p, pStack=%p",
               taskId,
               pEntryPoint,
               pStack );
    
    /* タスクタイプ判定 */
    if ( taskType == PROCMNG_TASK_TYPE_DRIVER ) {
        /* ドライバ */
        
        /* セグメントセレクタ設定 */
        codeSegSel = MEMMNG_SEGSEL_DRIVER_CODE;     /* コード */
        dataSegSel = MEMMNG_SEGSEL_DRIVER_DATA;     /* データ */
        
    } else if ( taskType == PROCMNG_TASK_TYPE_SERVER ) {
        /* サーバ */
        
        codeSegSel = MEMMNG_SEGSEL_SERVER_CODE;     /* コード */
        dataSegSel = MEMMNG_SEGSEL_SERVER_DATA;     /* データ */
        
    } else {
        /* ユーザ */
        
        codeSegSel = MEMMNG_SEGSEL_USER_CODE;       /* コード */
        dataSegSel = MEMMNG_SEGSEL_USER_DATA;       /* データ */
    }
    
    /* iretd命令用スタック設定 */
    IA32InstructionPush( dataSegSel );              /* ss     */
    IA32InstructionPush( ( uint32_t ) pStack );     /* esp    */
    IA32InstructionPush( 0x00003202 );              /* eflags */
    IA32InstructionPush( codeSegSel );              /* cs     */
    IA32InstructionPush( ( uint32_t ) pEntryPoint );/* eip    */
    
    /* セグメントレジスタ設定用スタック設定 */
    IA32InstructionPush( dataSegSel );              /* gs */
    IA32InstructionPush( dataSegSel );              /* fs */
    IA32InstructionPush( dataSegSel );              /* es */
    IA32InstructionPush( dataSegSel );              /* ds */
    
    /* 汎用レジスタ設定用スタック設定 */
    IA32InstructionPush( 0 );                       /* eax           */
    IA32InstructionPush( 0 );                       /* ecx           */
    IA32InstructionPush( 0 );                       /* edx           */
    IA32InstructionPush( 0 );                       /* ebx           */
    IA32InstructionPush( 0 );                       /* esp( 未使用 ) */
    IA32InstructionPush( 0 );                       /* ebp           */
    IA32InstructionPush( 0 );                       /* esi           */
    IA32InstructionPush( 0 );                       /* edi           */
    
    /* 汎用レジスタ設定 */
    IA32InstructionPopad();
    
    /* セグメントレジスタ設定 */
    IA32InstructionPopDs();
    IA32InstructionPopEs();
    IA32InstructionPopFs();
    IA32InstructionPopGs();
    
    /* タスクエントリポイントへ移行 */
    IA32InstructionIretd();
    
    /* not return */
}


/******************************************************************************/
