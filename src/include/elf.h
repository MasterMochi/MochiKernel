/******************************************************************************/
/* src/include/elf.h                                                          */
/*                                                                 2017/06/16 */
/* Copyright (C) 2017 Mochi                                                   */
/******************************************************************************/
#ifndef _ELF_H_
#define _ELF_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* ELF識別インデックス */
#define EI_MAG0        ( 0 )            /** ファイル識別           */
#define EI_MAG1        ( 1 )            /** ファイル識別           */
#define EI_MAG2        ( 2 )            /** ファイル識別           */
#define EI_MAG3        ( 3 )            /** ファイル識別           */
#define EI_CLASS       ( 4 )            /** ファイルクラス         */
#define EI_DATA        ( 5 )            /** データエンコーディング */
#define EI_VERSION     ( 6 )            /** ファイルバージョン     */
#define EI_OSABI       ( 7 )            /** OS/ABI識別             */
#define EI_ABIVERSION  ( 8 )            /** ABIバージョン          */
#define EI_PAD         ( 9 )            /** パディング開始         */
#define EI_NIDENT      ( 16 )           /** ELF識別フィールド長    */

/* ファイル識別 */
#define ELFMAG0        ( 0x7F )         /** ファイル識別子 */
#define ELFMAG1        ( 'E' )          /** ファイル識別子 */
#define ELFMAG2        ( 'L' )          /** ファイル識別子 */
#define ELFMAG3        ( 'F' )          /** ファイル識別子 */

/* ファイルクラス */
#define ELFCLASSNONE   ( 0 )            /** 無効クラス        */
#define ELFCLASS32     ( 1 )            /** 32bitオブジェクト */
#define ELFCLASS64     ( 2 )            /** 64bitオブジェクト */

/* データエンコーディング */
#define ELFDATANONE    ( 0 )            /** 無効データエンコーディング  */
#define ELFDATA2LSB    ( 1 )            /** 2の補数、リトルエンディアン */
#define ELFDATA2MSB    ( 2 )            /** 2の補数、ビッグエンディアン */

/* オブジェクトファイルタイプ */
#define ET_NONE        ( 0 )            /** ファイルタイプ無し       */
#define ET_REL         ( 1 )            /** 再配置可能ファイル       */
#define ET_EXEC        ( 2 )            /** 実行可能ファイル         */
#define ET_DYN         ( 3 )            /** 共有オブジェクトファイル */
#define ET_CORE        ( 4 )            /** コアファイル             */
#define ET_LOOS        ( 0xFE00 )       /** OS固有（最小値）         */
#define ET_HIOS        ( 0xFEFF )       /** OS固有（最大値）         */
#define ET_LOPROC      ( 0xFF00 )       /** プロセッサ固有（最小値） */
#define ET_HIPROC      ( 0xFFFF )       /** プロセッサ固有（最大値） */

/* マシンアーキテクチャ */
#define EM_NONE        ( 0 )            /** マシンアーキテクチャ未定義    */
#define EM_M32         ( 1 )            /** AT&T WE 32100                 */
#define EM_SPARC       ( 2 )            /** SPARC                         */
#define EM_386         ( 3 )            /** Intelアーキテクチャ           */
#define EM_68K         ( 4 )            /** Motorola 68000                */
#define EM_88K         ( 5 )            /** Motorola 88000                */
#define EM_860         ( 7 )            /** Intel 80860                   */
#define EM_MIPS        ( 8 )            /** MIPS R3000 ビッグエンディアン */

/* オブジェクトファイルバージョン */
#define EV_NONE        ( 0 )            /** 不正バージョン     */
#define EV_CURRENT     ( 1 )            /** カレントバージョン */

/* セグメントタイプ */
#define PT_NULL        ( 0 )            /** 未使用セグメント         */
#define PT_LOAD        ( 1 )            /** ロード可能セグメント     */
#define PT_DYNAMIC     ( 2 )            /** 動的リンク情報           */
#define PT_INTERP      ( 3 )            /** インタプリタ情報         */
#define PT_NOTE        ( 4 )            /** 補足情報                 */
#define PT_SHLIB       ( 5 )            /** 未定義セグメント         */
#define PT_PHDR        ( 6 )            /** プログラムヘッダテーブル */

/* フラグ */
#define PF_X           ( 0x00000001 )   /** 実行可能セグメント   */
#define PF_W           ( 0x00000002 )   /** 書込み可能セグメント */
#define PF_R           ( 0x00000004 )   /** 読込み可能セグメント */

/* 32bitデータ型 */
typedef uint32_t Elf32_Addr;            /** プログラムアドレス */
typedef uint16_t Elf32_Half;            /** 符号無2バイト整数  */
typedef uint32_t Elf32_Off;             /** ファイルオフセット */
typedef int32_t  Elf32_Sword;           /** 符号有4バイト整数  */
typedef uint32_t Elf32_Word;            /** 符号無4バイト整数  */

/** ELFヘッダ */
typedef struct {
    unsigned char e_ident[ EI_NIDENT ]; /**< ELF識別                          */
    Elf32_Half    e_type;               /**< オブジェクトファイルタイプ       */
    Elf32_Half    e_machine;            /**< マシンアーキテクチャ             */
    Elf32_Word    e_version;            /**< オブジェクトファイルバージョン   */
    Elf32_Addr    e_entry;              /**< エントリポイント（仮想アドレス） */
    Elf32_Off     e_phoff;              /**< PHTファイルオフセット            */
    Elf32_Off     e_shoff;              /**< SHTファイルオフセット            */
    Elf32_Word    e_flags;              /**< プロセッサ固有フラグ             */
    Elf32_Half    e_ehsize;             /**< ELFヘッダサイズ                  */
    Elf32_Half    e_phentsize;          /**< PHTエントリサイズ                */
    Elf32_Half    e_phnum;              /**< PHTエントリ数                    */
    Elf32_Half    e_shentsize;          /**< SHTエントリサイズ                */
    Elf32_Half    e_shnum;              /**< SHTエントリ数                    */
    Elf32_Half    e_shstrndx;           /**< SHTインデックス                  */
} Elf32_Ehdr;

/** プログラムヘッダ */
typedef struct {
    Elf32_Word p_type;                  /**< セグメントタイプ   */
    Elf32_Off  p_offset;                /**< ファイルオフセット */
    Elf32_Addr p_vaddr;                 /**< 仮想メモリアドレス */
    Elf32_Addr p_paddr;                 /**< 物理メモリアドレス */
    Elf32_Word p_filesz;                /**< ファイルサイズ     */
    Elf32_Word p_memsz;                 /**< メモリサイズ       */
    Elf32_Word p_flags;                 /**< フラグ             */
    Elf32_Word p_align;                 /**< アライメント値     */
} Elf32_Phdr;


/******************************************************************************/
#endif
