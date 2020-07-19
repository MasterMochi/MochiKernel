/******************************************************************************/
/*                                                                            */
/* src/booter/MemMng/MemMngMap.c                                              */
/*                                                                 2020/07/19 */
/* Copyright (C) 2018-2020 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ライブラリヘッダ */
#include <MLib/MLibList.h>

/* 共通ヘッダ */
#include <firmware/bios/e820.h>
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/kernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Driver.h>
#include <IntMng.h>
#include <LoadMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_MAP,  \
                    __LINE__,               \
                    __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif

/** メモリ領域終端アドレス計算マクロ */
#define END_ADDR( __TOP, __SIZE ) \
    ( ( void * ) ( ( size_t ) ( __TOP ) + ( __SIZE ) ) )

/** メモリマップリストエントリ数 */
#define MEMMAPLIST_ENTRY_NUM ( 100 )

/** メモリマップリストエントリ型 */
typedef struct {
    MLibListNode_t  node;   /**< 連結リストノード情報 */
    MkMemMapEntry_t entry;  /**< メモリマップエントリ */
} MemMapListEntry_t;


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** メモリマップリストエントリ */
static MemMapListEntry_t gListEntry[ MEMMAPLIST_ENTRY_NUM ];

/** 空エントリリスト */
static MLibList_t gEmptyList;

/** メモリマップリスト */
static MLibList_t gMemMapList;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* BIOS-E820メモリマップエントリアライメント */
static void AlignBiosE820Entry( BiosE820Entry_t *pInfo );

/* メモリマップリスト初期化 */
static void InitMemMapList( void );

/* BIOS-E820メモリマップデバッグログ出力 */
static void OutputBiosE820( void );

/* メモリマップリストデバッグログ出力 */
static void OutputMemMapList( void );

/* メモリマップリストエントリ設定(指定エントリ後方) */
static CmnRet_t SetMemMapListBack( MemMapListEntry_t *pEntry,
                                   void              *pAddr,
                                   size_t            size,
                                   uint32_t          type     );

/* メモリマップリストエントリ設定(指定エントリ中間) */
static CmnRet_t SetMemMapListCenter( MemMapListEntry_t *pEntry,
                                     void              *pAddr,
                                     size_t            size,
                                     uint32_t          type     );

/* メモリマップリストエントリ設定(指定エントリ前方) */
static CmnRet_t SetMemMapListFront( MemMapListEntry_t *pEntry,
                                    void              *pAddr,
                                    size_t            size,
                                    uint32_t          type     );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリマップ管理初期化
 * @details     メモリマップ管理用データを初期化する。
 */
/******************************************************************************/
void MemMngMapInit( void )
{
    /* デバッグログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* BIOS-E820メモリマップデバッグログ出力 */
    OutputBiosE820();

    /* メモリマップリスト初期化 */
    InitMemMapList();

    /* メモリマップリストデバッグ出力 */
    OutputMemMapList();

    /* デバッグログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       メモリマップ設定
 * @details     メモリマップをメモリマップリストから設定する。
 */
/******************************************************************************/
void MemMngMapSet( void )
{
    uint32_t          index;    /* メモリマップインデックス   */
    MemMapListEntry_t *pEntry;  /* メモリマップリストエントリ */

    /* 初期化 */
    index = 0;

    /* 先頭エントリ取得 */
    pEntry = ( MemMapListEntry_t * )
        MLibListGetNextNode( &gMemMapList, NULL );

    /* エントリ毎に繰り返し */
    while ( pEntry != NULL ) {
        /* デバッグログ出力 */
        DEBUG_LOG( "MemMap: base=0x%08X size=0x%08X type=%d",
                   pEntry->entry.pAddr,
                   pEntry->entry.size,
                   pEntry->entry.type                         );

        /* メモリマップ設定 */
        gMemMap[ index++ ] = pEntry->entry;

        /* 次エントリ取得 */
        pEntry = ( MemMapListEntry_t * )
            MLibListGetNextNode( &gMemMapList,
                                 ( MLibListNode_t * ) pEntry );
    }

    /* メモリマップエントリ数設定 */
    gMemMapEntryNum = index;

    return;
}


/******************************************************************************/
/**
 * @brief       メモリマップリスト設定
 * @details     メモリマップリストの利用可能メモリ領域に指定したタイプのメモリ
 *              領域を設定する。
 *
 * @param[in]   *pAddr メモリ領域先頭アドレス
 * @param[in]   size   メモリ領域サイズ
 * @param[in]   type   メモリ領域タイプ
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
CmnRet_t MemMngMapSetList( void     *pAddr,
                           size_t   size,
                           uint32_t type    )
{
    void              *pAddr1;  /* エントリメモリ領域終端アドレス */
    void              *pAddr2;  /* メモリ領域終端アドレス         */
    MemMapListEntry_t *pEntry;  /* メモリマップリストエントリ     */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pAddr=%p, size=%x, type=%d",
               __func__,
               pAddr,
               size,
               type                                      );

    /* 先頭エントリ取得 */
    pEntry = ( MemMapListEntry_t * )
        MLibListGetNextNode( &gMemMapList, NULL );

    /* エントリ毎に繰り返し */
    while ( pEntry != NULL ) {
        /* デバッグトレースログ出力 */
        DEBUG_LOG( " pEntry:pAddr=%p, size=%x, type=%d",
                   pEntry->entry.pAddr,
                   pEntry->entry.size,
                   pEntry->entry.type                    );

        /* メモリ領域比較用アドレス計算 */
        pAddr1 = END_ADDR( pEntry->entry.pAddr, pEntry->entry.size );
        pAddr2 = END_ADDR( pAddr              , size               );

        /* 領域チェック */
        if ( ( pAddr < pEntry->entry.pAddr               ) ||
             ( ( pAddr < pAddr1 ) && ( pAddr1 < pAddr2 ) )    ) {
            /* 範囲外 */
            break;
        }

        /* 領域チェック */
        if ( pAddr1 <= pAddr ) {
            /* 次エントリ範囲 */

            /* 次エントリ取得 */
            pEntry = ( MemMapListEntry_t * )
                MLibListGetNextNode( &gMemMapList,
                                     ( MLibListNode_t * ) pEntry );
            continue;
        }

        /* タイプ判定 */
        if ( pEntry->entry.type != MK_MEM_TYPE_AVAILABLE ) {
            /* 利用可能メモリ領域以外 */

            break;
        }

        /* デバッグトレースログ出力 */
        DEBUG_LOG( " pEntry->entry.pAddr=%p, pEntry->entry.size=%x",
                   pEntry->entry.pAddr,
                   pEntry->entry.size                                );

        /* 先頭アドレス比較 */
        if ( pEntry->entry.pAddr == pAddr ) {
            /* 一致 */

            /* サイズ比較 */
            if ( pEntry->entry.size == size ) {
                /* 一致 */

                /* メモリマップリストエントリ設定 */
                pEntry->entry.type = type;

                return CMN_SUCCESS;

            } else if ( pEntry->entry.size > size ) {
                /* 不一致 */

                /* メモリマップリストエントリ設定 */
                return SetMemMapListFront( pEntry, pAddr, size, type );
            }

            return CMN_FAILURE;

        } else {
            /* 不一致 */

            /* メモリ領域比較 */
            if ( pAddr1 > pAddr2 ) {
                /* 中間メモリ領域 */

                /* メモリマップリストエントリ設定 */
                return SetMemMapListCenter( pEntry, pAddr, size, type );

            } else if ( pAddr1 == pAddr2 ) {
                /* 後方メモリ領域 */

                /* メモリマップリストエントリ設定 */
                return SetMemMapListBack( pEntry, pAddr, size, type );
            }
        }

        /* 次エントリ取得 */
        pEntry = ( MemMapListEntry_t * )
            MLibListGetNextNode( &gMemMapList,
                                 ( MLibListNode_t * ) pEntry );
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
 * @brief       BIOS-E820メモリマップエントリアライメント
 * @details     BIOS-E820メモリマップの1エントリが示すメモリ領域（先頭アドレス
 *              とサイズ)を4KiBアライメントに合わせる。メモリ領域タイプが使用可
 *              能メモリ領域の場合はサイズを小さく、それ以外の場合は大きくなる
 *              ようにアライメントを合わせる。
 *
 * @param[in]   *pEntry BIOS-E820メモリマップエントリ
 */
/******************************************************************************/
static void AlignBiosE820Entry( BiosE820Entry_t *pEntry )
{
    uint32_t base;      /* 先頭アドレス */
    uint32_t length;    /* サイズ       */

    /* 初期化 */
    base   = pEntry->baseLow;
    length = pEntry->lengthLow;

    /* メモリ領域タイプチェック */
    if ( pEntry->type == BIOS_E820_TYPE_AVAILABLE ) {
        /* 使用可能メモリ領域 */

        /* 先頭アドレスアライメント */
        pEntry->baseLow   += 0x00000FFF;
        pEntry->baseLow   &= 0xFFFFF000;
        pEntry->lengthLow -= ( pEntry->baseLow - base );

        /* サイズチェック */
        if ( pEntry->lengthLow > length ) {
            /* アンダーフロー */

            pEntry->lengthLow = 0;
        }

        /* サイズアライメント */
        pEntry->lengthLow &= 0xFFFFF000;


    } else {
        /* 使用不可メモリ領域 */

        /* 先頭アドレスアライメント */
        pEntry->baseLow   &= 0xFFFFF000;
        pEntry->lengthLow += ( base - pEntry->baseLow );

        /* サイズアライメント */
        pEntry->lengthLow += 0x00000FFF;
        pEntry->lengthLow &= 0xFFFFF000;
    }

    return;
}


/******************************************************************************/
/**
 * @brief       メモリマップリスト初期化
 * @details     BIOS-E820メモリマップを基にメモリマップリストを初期化する。
 */
/******************************************************************************/
static void InitMemMapList( void )
{
    uint32_t          start;    /* メモリ領域アドレス                */
    uint32_t          index;    /* BIOS-E820メモリマップインデックス */
    BiosE820Entry_t   entry;    /* BIOS-E820メモリマップエントリ     */
    MemMapListEntry_t *pEmpty;  /* 空メモリマップリストエントリ      */
    MemMapListEntry_t *pPrev;   /* 前メモリマップリストエントリ      */

    /* 初期化 */
    start  = 0;
    pEmpty = NULL;
    pPrev  = NULL;

    /* メモリマップリストエントリ初期化 */
    memset( &gListEntry, 0, sizeof ( gListEntry ) );

    /* リスト初期化 */
    MLibListInit( &gEmptyList  );
    MLibListInit( &gMemMapList );

    /* 全エントリ毎に繰り返し */
    for ( index = 0; index < MEMMAPLIST_ENTRY_NUM; index++ ) {
        /* 空リストにメモリマップリストエントリ挿入 */
        MLibListInsertTail(
            &gEmptyList,
            ( MLibListNode_t * ) &gListEntry[ index ] );
    }

    /* エントリ毎に繰り返し */
    for ( index = 0; index < gBiosE820EntryNum; ) {
        /* BIOS-E820メモリマップエントリ取得 */
        entry = gBiosE820[ index ];

        /* BIOS-E820メモリマップエントリアライメント */
        AlignBiosE820Entry( &entry );

        /* サイズチェック */
        if ( entry.lengthLow == 0 ) {
            /* サイズ無し */

            /* BIOS-E820メモリマップインデックス更新 */
            index++;
            continue;
        }

        /* 前メモリマップリストエントリ有無チェック */
        if ( pPrev != NULL ) {
            /* 有り */

            /* 結合可能チェック */
            if ( ( entry.baseLow != start                ) &&
                 ( entry.type    == MK_MEM_TYPE_RESERVED )    ) {
                /* 結合可能 */

                /* メモリマップリストエントリ設定 */
                pPrev->entry.size += entry.baseLow - start;

                /* 開始アドレス更新 */
                start = ( uint32_t ) pPrev->entry.pAddr + pPrev->entry.size;

                continue;

            } else if ( ( entry.baseLow == start             ) &&
                        ( entry.type    == pPrev->entry.type )    ) {
                /* 結合可能 */

                /* メモリマップリストエントリ設定 */
                pPrev->entry.size += entry.lengthLow;

                /* BIOS-E820メモリマップインデックス更新 */
                index++;

                /* 開始アドレス更新 */
                start = ( uint32_t ) pPrev->entry.pAddr + pPrev->entry.size;

                continue;
            }
        }

        /* 空メモリマップリストエントリ取得 */
        pEmpty = ( MemMapListEntry_t * ) MLibListRemoveTail( &gEmptyList );

        /* 取得結果判定 */
        if ( pEmpty == NULL ) {
            /* 失敗 */

            /* デバッグログ出力 */
            DEBUG_LOG( "MLibListRemoveTail() failed." );
            break;
        }

        /* 開始アドレス判定 */
        if ( start != entry.baseLow ) {
            /* 不一致 */

            /* メモリマップリストエントリ設定 */
            pEmpty->entry.pAddr = ( void * ) start;
            pEmpty->entry.size  = entry.baseLow - start;
            pEmpty->entry.type  = MK_MEM_TYPE_RESERVED;

        } else {
            /* 一致 */

            /* メモリマップリストエントリ設定 */
            pEmpty->entry.pAddr = ( void * ) entry.baseLow;
            pEmpty->entry.size  = entry.lengthLow;
            pEmpty->entry.type  = entry.type;

            /* BIOS-E820メモリマップインデックス更新 */
            index++;
        }

        /* メモリマップリスト挿入 */
        MLibListInsertTail( &gMemMapList,
                            ( MLibListNode_t * ) pEmpty );

        /* 開始アドレス更新 */
        start = ( uint32_t ) pEmpty->entry.pAddr + pEmpty->entry.size;

        /* 前メモリマップリストエントリ設定 */
        pPrev = pEmpty;
    }

    /* 4GiB終端チェック */
    if ( start != 0 ) {
        /* 非4GiB終端 */

        /* 終端タイプチェック */
        if ( pPrev->entry.type == MK_MEM_TYPE_RESERVED ) {
            /* 使用不可 */

            /* 4GiB終端化 */
            pPrev->entry.size += 0 - start;

        } else {
            /* 使用不可以外 */

            /* 空メモリマップリストエントリ取得 */
            pEmpty = ( MemMapListEntry_t * ) MLibListRemoveTail( &gEmptyList );

            /* 取得結果判定 */
            if ( pEmpty == NULL ) {
                /* 失敗 */

                /* デバッグログ出力 */
                DEBUG_LOG( "MLibListRemoveTail() failed." );
                return;
            }

            /* メモリマップリストエントリ設定 */
            pEmpty->entry.pAddr = ( void * ) start;
            pEmpty->entry.size  = 0 - start;
            pEmpty->entry.type  = MK_MEM_TYPE_RESERVED;

            /* メモリマップリスト挿入 */
            MLibListInsertTail( &gMemMapList,
                                ( MLibListNode_t * ) pEmpty );
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief       BIOS-E820メモリマップデバッグログ出力
 * @details     BIOS-E820メモリマップが示すメモリ領域を1エントリ毎にデバッグロ
 *              グ出力する。
 */
/******************************************************************************/
static void OutputBiosE820( void )
{
    uint32_t index; /* エントリインデックス */

    /* エントリ毎繰り返し */
    for ( index = 0; index < gBiosE820EntryNum; index++ ) {
        /* デバッグログ出力 */
        DEBUG_LOG(
            "BIOS-E820: "
                "base=0x%08X_%08X "
                "len=0x%08X_%08X "
                "type=%d",
            gBiosE820[ index ].baseHigh,
            gBiosE820[ index ].baseLow,
            gBiosE820[ index ].lengthHigh,
            gBiosE820[ index ].lengthLow,
            gBiosE820[ index ].type        );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       メモリマップリストデバッグログ出力
 * @details     メモリマップリストが示すメモリ領域を1エントリ毎にデバッグロ
 *              グ出力する。
 */
/******************************************************************************/
static void OutputMemMapList( void )
{
    MemMapListEntry_t *pEntry;

    /* 先頭エントリ取得 */
    pEntry = ( MemMapListEntry_t * )
        MLibListGetNextNode( &gMemMapList, NULL );

    /* エントリ毎繰り返し */
    while ( pEntry != NULL ) {
        /* デバッグログ出力 */
        DEBUG_LOG( "MemMapList: base=%010p, size=%#010X, type=%u",
                   pEntry->entry.pAddr,
                   pEntry->entry.size,
                   pEntry->entry.type                              );

        /* 次エントリ取得 */
        pEntry = ( MemMapListEntry_t * )
            MLibListGetNextNode( &gMemMapList,
                                 ( MLibListNode_t * ) pEntry );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       メモリマップリストエントリ設定(指定エントリ後方)
 * @details     指定したメモリマップリストエントリが示す利用可能メモリ領域の後
 *              方を分割してメモリマップリストエントリを設定する。
 *
 * @param[in]   *pEntry メモリマップリストエントリ
 * @param[in]   *pAddr  設定するメモリ領域先頭アドレス
 * @param[in]   size    設定するメモリ領域サイズ
 * @param[in]   type    設定するメモリ領域タイプ
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
static CmnRet_t SetMemMapListBack( MemMapListEntry_t *pEntry,
                                   void              *pAddr,
                                   size_t            size,
                                   uint32_t          type     )
{
    MLibRet_t         retMLib;  /* MLib関数戻り値               */
    MemMapListEntry_t *pEmpty;  /* 空メモリマップリストエントリ */

    /* 空メモリマップリストエントリ取得 */
    pEmpty = ( MemMapListEntry_t * ) MLibListRemoveTail( &gEmptyList );

    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* 失敗 */

        /* デバッグログ出力 */
        DEBUG_LOG( "MLibListRemoveTail() failed." );

        return CMN_FAILURE;
    }

    /* メモリマップリストエントリ設定 */
    pEntry->entry.size  -= size;
    pEmpty->entry.pAddr  = pAddr;
    pEmpty->entry.size   = size;
    pEmpty->entry.type   = type;

    /* メモリマップリスト挿入 */
    retMLib = MLibListInsertNext( &gMemMapList,
                                  ( MLibListNode_t * ) pEntry,
                                  ( MLibListNode_t * ) pEmpty  );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* デバッグログ出力 */
        DEBUG_LOG( "MLibListInsertPrev() failed." );
        return CMN_FAILURE;
    }

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       メモリマップリストエントリ設定(指定エントリ中間)
 * @details     指定したメモリマップリストエントリが示す利用可能メモリ領域の中
 *              間を分割してメモリマップリストエントリを設定する。
 *
 * @param[in]   *pEntry メモリマップリストエントリ
 * @param[in]   *pAddr  設定するメモリ領域先頭アドレス
 * @param[in]   size    設定するメモリ領域サイズ
 * @param[in]   type    設定するメモリ領域タイプ
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
static CmnRet_t SetMemMapListCenter( MemMapListEntry_t *pEntry,
                                     void              *pAddr,
                                     size_t            size,
                                     uint32_t          type     )
{
    size_t   size1;     /* 前方メモリ領域サイズ */
    CmnRet_t ret;       /* 戻り値               */

    /* 初期化 */
    size1 = ( size_t ) pAddr - ( size_t ) pEntry->entry.pAddr;

    /* 前方分割 */
    ret = SetMemMapListFront( pEntry,
                              pEntry->entry.pAddr,
                              size1,
                              pEntry->entry.type   );

    /* 分割結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* 中間分割 */
    ret = SetMemMapListFront( pEntry,
                              pAddr,
                              size,
                              type    );

    return ret;
}


/******************************************************************************/
/**
 * @brief       メモリマップリストエントリ設定(指定エントリ前方)
 * @details     指定したメモリマップリストエントリが示す利用可能メモリ領域の前
 *              方を分割してメモリマップリストエントリを設定する。
 *
 * @param[in]   *pEntry メモリマップリストエントリ
 * @param[in]   *pAddr  設定するメモリ領域先頭アドレス
 * @param[in]   size    設定するメモリ領域サイズ
 * @param[in]   type    設定するメモリ領域タイプ
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
static CmnRet_t SetMemMapListFront( MemMapListEntry_t *pEntry,
                                    void              *pAddr,
                                    size_t            size,
                                    uint32_t          type     )
{
    MLibRet_t         retMLib;  /* MLib関数戻り値               */
    MemMapListEntry_t *pEmpty;  /* 空メモリマップリストエントリ */

    /* 空メモリマップリストエントリ取得 */
    pEmpty = ( MemMapListEntry_t * ) MLibListRemoveTail( &gEmptyList );

    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* 失敗 */

        /* デバッグログ出力 */
        DEBUG_LOG( "MLibListRemoveTail() failed." );

        return CMN_FAILURE;
    }

    /* メモリマップリストエントリ設定 */
    pEmpty->entry.pAddr  = pAddr;
    pEmpty->entry.size   = size;
    pEmpty->entry.type   = type;
    pEntry->entry.pAddr  = END_ADDR( pAddr, size );
    pEntry->entry.size  -= size;

    /* メモリマップリスト挿入 */
    retMLib = MLibListInsertPrev( &gMemMapList,
                                  ( MLibListNode_t * ) pEntry,
                                  ( MLibListNode_t * ) pEmpty  );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* デバッグログ出力 */
        DEBUG_LOG( "MLibListInsertPrev() failed." );
        return CMN_FAILURE;
    }

    return CMN_SUCCESS;
}


/******************************************************************************/
