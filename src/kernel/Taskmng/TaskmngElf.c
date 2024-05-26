/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngElf.c                                            */
/*                                                                 2023/01/04 */
/* Copyright (C) 2017-2023 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stddef.h>

/* ライブラリヘッダ */
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <elf.h>
#include <memmap.h>
#include <hardware/IA32/IA32Paging.h>
#include <kernel/config.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>
#include <Taskmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                 \
    DebugOutput( CMN_MODULE_TASKMNG_ELF, \
                 __LINE__,               \
                 __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* ELFヘッダチェック */
static CmnRet_t ElfCheckElfHeader( void   *pAddr,
                                   size_t size    );

/* プログラムヘッダチェック */
static CmnRet_t ElfCheckPrgHeader( void   *pAddr,
                                   size_t size    );


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ELFファイル読込
 * @details     ELFファイルをメモリを読み込む。
 *
 * @param[in]   *pAddr         ELFファイルアドレス
 * @param[in]   size           ELFファイルサイズ
 * @param[in]   dirId          ページディレクトリID
 * @param[out]  **ppEntryPoint エントリポイント
 * @param[out]  **ppEndPoint   エンドポイント
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t ElfLoad( void              *pAddr,
                  size_t            size,
                  MemmngPageDirId_t dirId,
                  void              **ppEntryPoint,
                  void              **ppEndPoint    )
{
    void       *pPhyAddr;   /* 物理アドレス                     */
    uint32_t   index;       /* インデックス                     */
    uint32_t   attrRw;      /* 読込/書込許可属性                */
    CmnRet_t   ret;         /* 関数戻り値                       */
    Elf32_Ehdr *pElfHdr;    /* ELFヘッダ                        */
    Elf32_Phdr *pPrgHdr;    /* プログラムヘッダテーブル         */
    Elf32_Phdr *pEntry;     /* プログラムヘッダテーブルエントリ */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.",                   __func__            );
    DEBUG_LOG( " pAddr=%010p, size=%u",         pAddr, size         );
    DEBUG_LOG( " dirId=%u, ppEntryPoint=%010p", dirId, ppEntryPoint );

    /* 初期化 */
    pElfHdr = ( Elf32_Ehdr * ) pAddr;
    pPrgHdr = ( Elf32_Phdr * ) ( ( uint32_t ) pAddr + pElfHdr->e_phoff );

    /* ELFヘッダチェック */
    ret = ElfCheckElfHeader( pAddr, size );

    /* チェック結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 不正 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* プログラムヘッダチェック */
    ret = ElfCheckPrgHeader( pAddr, size );

    /* チェック結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 不正 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

        return CMN_FAILURE;
    }

    /* プログラムヘッダテーブル1エントリ毎に繰り返し */
    for ( index = 0; index < pElfHdr->e_phnum; index++ ) {
        /* プログラムヘッダ参照変数設定 */
        pEntry = ( Elf32_Phdr * ) ( ( uint32_t ) pPrgHdr +
                                    pElfHdr->e_phentsize * index );

        /* セグメントタイプ判定 */
        if ( pEntry->p_type != PT_LOAD ) {
            /* ロード可能セグメント以外 */

            /* 次のプログラムヘッダテーブルエントリ */
            continue;
        }

        /* セグメントサイズ取得 */
        size = MLIB_UTIL_ALIGN( pEntry->p_memsz, IA32_PAGING_PAGE_SIZE );

        /* メモリ領域割当 */
        pPhyAddr = MemmngPhysAlloc( size );

        /* メモリ領域割当結果判定 */
        if ( pPhyAddr == NULL ) {
            /* 失敗 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

            return CMN_FAILURE;
        }

        /* 0初期化 */
        MemmngCtrlSet( pPhyAddr, 0, size );

        /* セグメントコピー */
        MemmngCtrlCopyVirtToPhys(
            pPhyAddr,
            ( void * ) ( ( uint32_t ) pAddr + pEntry->p_offset ),
            pEntry->p_filesz                                      );

        /* フラグ判定 */
        if ( MLIB_UTIL_HAVE_FLAG( pEntry->p_flags, PF_R | PF_W ) ) {
            /* 読書可能セグメント */

            attrRw = IA32_PAGING_RW_RW;

        } else {
            /* 読込専用セグメント */

            attrRw = IA32_PAGING_RW_R;
        }

        /* ページマッピング設定 */
        ret = MemmngPageSet( dirId,
                             ( void * ) pEntry->p_vaddr,
                             pPhyAddr,
                             size,
                             IA32_PAGING_G_NO,
                             IA32_PAGING_US_USER,
                             attrRw                      );

        /* 設定結果判定 */
        if ( ret != CMN_SUCCESS ) {
            /* 失敗 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );

            return CMN_FAILURE;
        }

        /* エンドポイント更新 */
        *ppEndPoint = ( void * ) ( pEntry->p_vaddr + size );
    }

    /* エントリポイント設定 */
    *ppEntryPoint = ( void * ) pElfHdr->e_entry;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS, *ppEntryPoint=%010p",
               __func__,
               *ppEntryPoint                                     );

    return CMN_SUCCESS;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ELFヘッダチェック
 * @details     ELFファイルのELFヘッダが許容される内容かチェックする。
 *
 * @param[in]   *pAddr ELFファイルアドレス
 * @param[in]   size   ELFファイルサイズ
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t ElfCheckElfHeader( void   *pAddr,
                                   size_t size    )
{
    Elf32_Ehdr *pHdr;   /* ELFヘッダ */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pAddr=%010p, size=%u",
               __func__,
               pAddr,
               size );

    /* 初期化 */
    pHdr = ( Elf32_Ehdr * ) pAddr;

    /* ELFヘッダサイズチェック */
    if ( size < sizeof ( Elf32_Ehdr ) ) {
        /* 不正 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end.", __func__ );

        return CMN_FAILURE;
    }

    /* ファイル識別子チェック */
    if ( ( pHdr->e_ident[ EI_MAG0 ] != ELFMAG0 ) ||
         ( pHdr->e_ident[ EI_MAG1 ] != ELFMAG1 ) ||
         ( pHdr->e_ident[ EI_MAG2 ] != ELFMAG2 ) ||
         ( pHdr->e_ident[ EI_MAG3 ] != ELFMAG3 )    ) {
        /* 不正ファイル識別子 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
        DEBUG_LOG( " e_ident[ EI_MAG0 ]=%02X %02X %02X %02X",
                   pHdr->e_ident[ EI_MAG0 ],
                   pHdr->e_ident[ EI_MAG1 ],
                   pHdr->e_ident[ EI_MAG2 ],
                   pHdr->e_ident[ EI_MAG3 ]  );

        return CMN_FAILURE;
    }

    /* ファイルクラスチェック */
    if ( pHdr->e_ident[ EI_CLASS ] != ELFCLASS32 ) {
        /* 無効値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_ident[ EI_CLASS ]=0x%02X",
                   __func__,
                   pHdr->e_ident[ EI_CLASS ] );

        return CMN_FAILURE;
    }

    /* データエンコーディングチェック */
    if ( pHdr->e_ident[ EI_DATA ] != ELFDATA2LSB ) {
        /* 無効値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_ident[ EI_DATA ]=0x%02X",
                   __func__,
                   pHdr->e_ident[ EI_DATA ] );

        return CMN_FAILURE;
    }

    /* ファイルバージョンチェック */
    if ( pHdr->e_ident[ EI_VERSION ] != EV_CURRENT ) {
        /* 無効値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_ident[ EI_VERSION ]=0x%02X",
                   __func__,
                   pHdr->e_ident[ EI_VERSION ] );

        return CMN_FAILURE;
    }

    /* オブジェクトファイルタイプチェック */
    if ( pHdr->e_type != ET_EXEC ) {
        /* 無効値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_type=0x%04X",
                   __func__,
                   pHdr->e_type );

        return CMN_FAILURE;
    }

    /* マシンアーキテクチャチェック */
    if ( pHdr->e_machine != EM_386 ) {
        /* 無効値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_machine=0x%04X",
                   __func__,
                   pHdr->e_machine );

        return CMN_FAILURE;
    }

    /* ファイルバージョンチェック */
    if ( pHdr->e_version != EV_CURRENT ) {
        /* 無効値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_version=0x%08X",
                   __func__,
                   pHdr->e_version );

        return CMN_FAILURE;
    }

    /* エントリポイントチェック */
    if ( ( pHdr->e_entry < MEMMAP_VADDR_USER       ) &&
         ( pHdr->e_entry > MEMMAP_VADDR_USER_STACK )    ) {
        /* 不正値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_entry=0x%08X",
                   __func__,
                   pHdr->e_entry );

        return CMN_FAILURE;
    }

    /* ELFヘッダサイズチェック */
    if ( pHdr->e_ehsize != sizeof ( Elf32_Ehdr ) ) {
        /* 不正値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE, e_ehsize=0x%04X",
                   __func__,
                   pHdr->e_ehsize );

        return CMN_FAILURE;
    }

    /* プログラムヘッダテーブルサイズチェック */
    if ( size < ( pHdr->e_phoff + pHdr->e_phentsize * pHdr->e_phnum ) ) {
        /* 不正値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
        DEBUG_LOG( " e_phoff=0x%08X, e_phentsize=0x%04X, e_phnum=0x%04X",
                   pHdr->e_phoff,
                   pHdr->e_phentsize,
                   pHdr->e_phnum );

        return CMN_FAILURE;
    }

    /* セクションヘッダテーブルサイズチェック */
    if ( size < ( pHdr->e_shoff + pHdr->e_shentsize * pHdr->e_shnum ) ) {
        /* 不正値 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
        DEBUG_LOG( " e_shoff=0x%08X, e_shentsize=0x%04X, e_shnum=%04X",
                   pHdr->e_shoff,
                   pHdr->e_shentsize,
                   pHdr->e_shnum );

        return CMN_FAILURE;
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );

    return CMN_SUCCESS;
}



/******************************************************************************/
/**
 * @brief       プログラムヘッダチェック
 * @details     ELFファイルのプログラムヘッダが許容される内容かチェックする。
 *
 * @param[in]   *pAddr ELFファイルアドレス
 * @param[in]   size   ELFファイルサイズ
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t ElfCheckPrgHeader( void   *pAddr,
                                   size_t size    )
{
    uint32_t   index;       /* プログラムヘッダテーブルインデックス */
    Elf32_Ehdr *pElfHdr;    /* ELFヘッダ                            */
    Elf32_Phdr *pPrgHdr;    /* プログラムヘッダテーブル             */
    Elf32_Phdr *pEntry;     /* プログラムヘッダテーブルエントリ     */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pAddr=%010p, size=%u",
               __func__,
               pAddr,
               size );

    /* 初期化 */
    pElfHdr = ( Elf32_Ehdr * ) pAddr;
    pPrgHdr = ( Elf32_Phdr * ) ( ( uint32_t ) pAddr + pElfHdr->e_phoff );

    /* 1エントリ毎に繰り返し */
    for ( index = 0; index < pElfHdr->e_phnum; index++ ) {
        /* エントリ参照変数設定 */
        pEntry = ( Elf32_Phdr * ) ( ( uint32_t ) pPrgHdr +
                                    pElfHdr->e_phentsize * index );

        /* セグメントタイプチェック */
        if ( pEntry->p_type != PT_LOAD ) {
            /* ロード可能セグメント以外 */

            /* 次のプログラムヘッダテーブルエントリ */
            continue;
        }

        /* ファイルサイズチェック */
        if ( ( pEntry->p_offset + pEntry->p_filesz ) >= size ) {
            /* 不正値 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=CMN_FAILURE, size=%d",
                       __func__,
                       pEntry->p_offset + pEntry->p_filesz );

            return CMN_FAILURE;
        }

        /* 仮想メモリチェック */
        if ( ( pEntry->p_vaddr < MEMMAP_VADDR_USER              ) ||
             ( ( pEntry->p_vaddr + pEntry->p_memsz ) >
               MEMMAP_VADDR_USER_STACK                          ) ||
             ( ( pEntry->p_vaddr % IA32_PAGING_PAGE_SIZE ) != 0 )    ) {
            /* 不正値 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=CMN_FAILURE, vaddr=%#010X, end=%#010X",
                       __func__,
                       pEntry->p_vaddr,
                       ( pEntry->p_vaddr + pEntry->p_memsz ) );

            return CMN_FAILURE;
        }

        /* フラグチェック */
        if ( !( MLIB_UTIL_HAVE_FLAG( pEntry->p_flags, PF_X ) ||
                MLIB_UTIL_HAVE_FLAG( pEntry->p_flags, PF_W ) ||
                MLIB_UTIL_HAVE_FLAG( pEntry->p_flags, PF_R )    ) ) {
            /* 不正値 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=CMN_FAILURE, p_flags=%x",
                       __func__,
                       pEntry->p_flags );

            return CMN_FAILURE;
        }
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_SUCCESS", __func__ );

    return CMN_SUCCESS;
}


/******************************************************************************/
