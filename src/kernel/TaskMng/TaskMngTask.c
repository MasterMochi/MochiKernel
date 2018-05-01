/******************************************************************************/
/* src/kernel/TaskMng/TaskMngTask.c                                           */
/*                                                                 2018/05/01 */
/* Copyright (C) 2017-2018 Mochi.                                             */
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
#include <Config.h>
#include <Debug.h>
#include <MemMng.h>
#include <TaskMng.h>

/* 内部モジュールヘッダ */
#include "TaskMngElf.h"
#include "TaskMngSched.h"
#include "TaskMngTask.h"
#include "TaskMngTss.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_TASK,    \
                    __LINE__,                   \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/* タスクID使用フラグ */
#define TASK_ID_UNUSED ( 0 )    /** 未使用 */
#define TASK_ID_USED   ( 1 )    /** 使用済 */


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** タスク管理テーブル */
static TaskMngTaskTbl_t gTaskTbl[ TASKMNG_TASK_ID_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスク追加
 * @details     タスク追加を行う。
 * 
 * @param[in]   taskType タスクタイプ
 *                  - TASKMNG_TASK_TYPE_DRIVER ドライバ
 *                  - TASKMNG_TASK_TYPE_SERVER サーバ
 *                  - TASKMNG_TASK_TYPE_USER   ユーザ
 * @param[in]   *pAddr   実行ファイル
 * @param[in]   size     実行ファイルサイズ
 * 
 * @retval      TASKMNG_TASK_ID_NULL 失敗
 * @retval      TASKMNG_TASK_ID_MIN  タスクID最小値
 * @retval      TASKMNG_TASK_ID_MAX  タスクID最大値
 */
/******************************************************************************/
uint32_t TaskMngTaskAdd( uint8_t taskType,
                         void    *pAddr,
                         size_t  size      )
{
    void                   *pKernelStack;   /* カーネルスタック     */
    void                   *pStack;         /* スタック             */
    CmnRet_t               ret;             /* 関数戻り値           */
    uint32_t               taskId;          /* タスクID             */
    uint32_t               pageDirId;       /* ページディレクトリID */
    TaskMngTaskStackInfo_t *pStackInfo;     /* スタック情報         */
    
    /* 初期化 */
    pKernelStack = NULL;
    pStack       = NULL;
    ret          = CMN_FAILURE;
    taskId       = TASKMNG_TASK_ID_MIN;
    pageDirId    = MEMMNG_PAGE_DIR_FULL;
    pStackInfo   = NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. taskType=%u, pAddr=%010p, size=%d",
               __func__,
               taskType,
               pAddr,
               size );
    
    /* 空タスク検索 */
    for ( ; taskId < TASKMNG_TASK_ID_MAX; taskId++ ) {
        /* 使用フラグ判定 */
        if ( gTaskTbl[ taskId ].used == TASK_ID_UNUSED ) {
            /* 未使用 */
            
            /* ページディレクトリ割当 */
            pageDirId = MemMngPageAllocDir();
            
            /* ページディレクトリ割当結果判定 */
            if ( pageDirId == MEMMNG_PAGE_DIR_FULL ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return TASKMNG_TASK_ID_NULL;
            }
            
            /* PDBR設定 */
            
            /* タスク基本設定 */
            gTaskTbl[ taskId ].used         = TASK_ID_USED;
            gTaskTbl[ taskId ].type         = taskType;
            gTaskTbl[ taskId ].state        = 0;
            gTaskTbl[ taskId ].context.eip  = ( uint32_t ) TaskMngTaskStart;
            gTaskTbl[ taskId ].context.esp  = CONFIG_MEM_KERNEL_STACK_ADDR +
                                              CONFIG_MEM_KERNEL_STACK_SIZE -
                                              sizeof ( uint32_t );
            gTaskTbl[ taskId ].pageDirId    = pageDirId;
            
            /* カーネルスタック情報設定 */
            pStackInfo              = &( gTaskTbl[ taskId ].kernelStackInfo );
            pStackInfo->pTopAddr    = ( void * ) CONFIG_MEM_KERNEL_STACK_ADDR;
            pStackInfo->pBottomAddr = ( void * )
                                      ( CONFIG_MEM_KERNEL_STACK_ADDR +
                                        CONFIG_MEM_KERNEL_STACK_SIZE -
                                        sizeof ( uint32_t )            );
            pStackInfo->size        = CONFIG_MEM_KERNEL_STACK_SIZE;
            
            /* スタック情報設定 */
            pStackInfo              = &( gTaskTbl[ taskId ].stackInfo );
            pStackInfo->pTopAddr    = ( void * ) CONFIG_MEM_TASK_STACK_ADDR;
            pStackInfo->pBottomAddr = ( void * )
                                      ( CONFIG_MEM_TASK_STACK_ADDR +
                                        CONFIG_MEM_TASK_STACK_SIZE -
                                        sizeof ( uint32_t )          );
            pStackInfo->size        = CONFIG_MEM_TASK_STACK_SIZE;
            
            /* ELFファイル読込 */
            ret = TaskMngElfLoad( pAddr, size, &gTaskTbl[ taskId ] );
            
            /* ELFファイル読込結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return TASKMNG_TASK_ID_NULL;
            }
            
            /* カーネルスタック領域割り当て */
            pKernelStack = MemMngAreaAlloc( CONFIG_MEM_KERNEL_STACK_SIZE );
            
            /* 割り当て結果判定 */
            if ( pKernelStack == NULL ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return TASKMNG_TASK_ID_NULL;
            }
            
            /* スタック領域割り当て */
            pStack = MemMngAreaAlloc( CONFIG_MEM_TASK_STACK_SIZE );
            
            /* 割り当て結果判定 */
            if ( pStack == NULL ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return TASKMNG_TASK_ID_NULL;
            }
            
            /* カーネルスタックページマップ設定 */
            ret = MemMngPageSet( pageDirId,
                                 ( void * ) CONFIG_MEM_KERNEL_STACK_ADDR,
                                 pKernelStack,
                                 CONFIG_MEM_KERNEL_STACK_SIZE,
                                 IA32_PAGING_G_NO,
                                 IA32_PAGING_US_SV,
                                 IA32_PAGING_RW_RW );
            
            /* ページマップ設定結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return TASKMNG_TASK_ID_NULL;
            }
            
            /* スタックページマップ設定 */
            ret = MemMngPageSet( pageDirId,
                                 ( void * ) CONFIG_MEM_TASK_STACK_ADDR,
                                 pStack,
                                 CONFIG_MEM_TASK_STACK_SIZE,
                                 IA32_PAGING_G_NO,
                                 IA32_PAGING_US_USER,
                                 IA32_PAGING_RW_RW );
            
            /* ページマップ設定結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return TASKMNG_TASK_ID_NULL;
            }
            
            /* スケジューラ追加 */
            ret = TaskMngSchedAdd( taskId );
            
            /* 追加結果判定 */
            if ( ret == CMN_FAILURE ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return TASKMNG_TASK_ID_NULL;
            }
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%u", __func__, taskId );
            
            return taskId;
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%u", __func__, TASKMNG_TASK_ID_NULL );
    
    return TASKMNG_TASK_ID_NULL;
}


/******************************************************************************/
/**
 * @brief       コンテキスト取得
 * @details     指定したタスクIDのコンテキストを取得する。
 * 
 * @param[in]   taskId タスクID
 *                  - TASKMNG_TASK_ID_MIN タスクID最小値
 *                  - TASKMNG_TASK_ID_MAX タスクID最大値
 * 
 * @return      コンテキスト
 */
/******************************************************************************/
TaskMngTaskContext_t TaskMngTaskGetContext( uint32_t taskId )
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
 *                  - TASKMNG_TASK_ID_MIN タスクID最小値
 *                  - TASKMNG_TASK_ID_MAX タスクID最大値
 * 
 * @return      カーネルスタックアドレス
 */
/******************************************************************************/
void *TaskMngTaskGetKernelStack( uint32_t taskId )
{
    /* カーネルスタックアドレス返却 */
    return gTaskTbl[ taskId ].kernelStackInfo.pBottomAddr;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリID取得
 * @details     指定したタスクIDのページディレクトリIDを取得する。
 * 
 * @param[in]   taskId タスクID
 *                  - TASKMNG_TASK_ID_MIN タスクID最小値
 *                  - TASKMNG_TASK_ID_MAX タスクID最大値
 * 
 * @return      ページディレクトリID
 */
/******************************************************************************/
uint32_t TaskMngTaskGetPageDirId( uint32_t taskId )
{
    /* ページディレクトリID返却 */
    return gTaskTbl[ taskId ].pageDirId;
}


/******************************************************************************/
/**
 * @brief       タスクタイプ取得
 * @details     指定したタスクIDのタスクタイプを取得する。
 * 
 * @param[in]   taskId タスクID
 *                  - TASKMNG_TASK_ID_MIN タスクID最小値
 *                  - TASKMNG_TASK_ID_MAX タスクID最大値
 * 
 * @retval      TASKMNG_TASK_TYPE_DRIVER ドライバ
 * @retval      TASKMNG_TASK_TYPE_SERVER サーバ
 * @retval      TASKMNG_TASK_TYPE_USER   ユーザ
 */
/******************************************************************************/
uint8_t TaskMngTaskGetType( uint32_t taskId )
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
void TaskMngTaskInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* タスク管理テーブル初期化 */
    memset( gTaskTbl, 0, sizeof ( gTaskTbl ) );
    
    /* アイドルタスク設定 */
    gTaskTbl[ TASKMNG_TASK_ID_IDLE ].used      = TASK_ID_USED;
    gTaskTbl[ TASKMNG_TASK_ID_IDLE ].pageDirId = MEMMNG_PAGE_DIR_ID_IDLE;
    
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
 *                  - TASKMNG_TASK_ID_MIN タスクID最小値
 *                  - TASKMNG_TASK_ID_MAX タスクID最大値
 * @param[in]   *pContext コンテキスト
 */
/******************************************************************************/
void TaskMngTaskSetContext( uint32_t             taskId,
                            TaskMngTaskContext_t *pContext )
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
void TaskMngTaskStart( void )
{
    void             *pEntryPoint;  /* エントリポイント         */
    void             *pStack;       /* スタックアドレス         */
    uint8_t          taskType;      /* タスクタイプ             */
    uint32_t         taskId;        /* タスクID                 */
    uint32_t         codeSegSel;    /* コードセグメントセレクタ */
    uint32_t         dataSegSel;    /* データセグメントセレクタ */
    TaskMngTaskTbl_t *pTask;        /* タスク管理情報           */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 初期化 */
    taskId      = TaskMngSchedGetTaskId();      /* タスクID         */
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
    if ( taskType == TASKMNG_TASK_TYPE_DRIVER ) {
        /* ドライバ */
        
        /* セグメントセレクタ設定 */
        codeSegSel = MEMMNG_SEGSEL_DRIVER_CODE;     /* コード */
        dataSegSel = MEMMNG_SEGSEL_DRIVER_DATA;     /* データ */
        
    } else if ( taskType == TASKMNG_TASK_TYPE_SERVER ) {
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
