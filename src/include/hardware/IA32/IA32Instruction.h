/******************************************************************************/
/* src/kernel/include/hardware/IA32/IA32Instruction.h                         */
/*                                                                 2018/05/05 */
/* Copyright (C) 2016-2018 Mochi.                                             */
/******************************************************************************/
#ifndef IA32_INSTRUCTION_H
#define IA32_INSTRUCTION_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* 内部モジュールヘッダ */
#include "IA32Descriptor.h"
#include "IA32Paging.h"


/******************************************************************************/
/* インライン関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       espレジスタ加算
 * @details     espレジスタを指定した値で加算する。
 * 
 * @param[in]   value 加算値
 */
/******************************************************************************/
static inline void IA32InstructionAddEsp( int32_t value )
{
    /* add命令実行 */
    __asm__ __volatile__ ( "add esp, %0"
                           :
                           : "a" ( value ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       call命令実行
 * @details     call命令を用いて、指定したアドレスを実行する。
 * 
 * @param[in]   *pFunc 呼出し先アドレス
 */
/******************************************************************************/
static inline void IA32InstructionCall( void *pFunc )
{
    /* call命令実行 */
    __asm__ __volatile__ ( "call %0"
                           :
                           : "a" ( pFunc ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       cli命令実行
 * @details     cli命令を実行して、CPUへの割込みを禁止する。
 */
/******************************************************************************/
static inline void IA32InstructionCli( void )
{
    /* cli命令実行 */
    __asm__ __volatile__ ( "cli" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       ebpレジスタ値取得
 * @details     ebpレジスタの値を返す。
 * 
 * @return      ebpレジスタ値
 */
/******************************************************************************/
static inline uint32_t IA32InstructionGetEbp( void )
{
    uint32_t ebp;   /* ebpレジスタ値 */
    
    /* mov命令実行 */
    __asm__ __volatile__ ( "mov %0, ebp"
                           : "=m" ( *&ebp )
                           :
                           :                );
    
    return ebp;
}


/******************************************************************************/
/**
 * @brief       espレジスタ値取得
 * @details     espレジスタの値を返す。
 * 
 * @return      espレジスタ値
 */
/******************************************************************************/
static inline uint32_t IA32InstructionGetEsp( void )
{
    uint32_t esp;   /* espレジスタ値 */
    
    /* mov命令実行 */
    __asm__ __volatile__ ( "mov %0, esp"
                           : "=m" ( *&esp )
                           :
                           :                );
    
    return esp;
}


/******************************************************************************/
/**
 * @brief       hlt命令実行
 * @details     hlt命令を実行して、プロセッサをHALT状態にする。
 */
/******************************************************************************/
static inline void IA32InstructionHlt( void )
{
    /* hlt命令実行 */
    __asm__ __volatile__ ( "hlt" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       in（byte）命令実行
 * @details     in（byte）命令を実行して、指定したI/Oポートから8bitの値を読み込
 *              み、指定したアドレスに格納する。
 * 
 * @param[out]  *pValue 取得値格納先
 * @param[in]   portNo  I/Oポート番号
 */
/******************************************************************************/
static inline void IA32InstructionInByte( uint8_t  *pValue,
                                          uint16_t portNo   )
{
    /* in al命令実行 */
    __asm__ __volatile__ ( "in %0, %1"
                           : "=a" ( *pValue )
                           : "d"  ( portNo  ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       in（double word）命令実行
 * @details     in（double word）命令を実行して、指定したI/Oポートから32bitの値
 *              を読み込み、指定したアドレスに格納する。
 * 
 * @param[out]  *pValue 取得値格納先
 * @param[in]   portNo  I/Oポート番号
 */
/******************************************************************************/
static inline void IA32InstructionInDWord( uint32_t *pValue,
                                           uint16_t portNo   )
{
    /* in eax命令実行 */
    __asm__ __volatile__ ( "in %0, %1"
                           : "=a" ( *pValue )
                           : "d"  ( portNo  ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       in（word）命令実行
 * @details     in（word）命令を実行して、指定したI/Oポートから16bitの値を読み
 *              込み、指定したアドレスに格納する。
 * 
 * @param[out]  *pValue 取得値格納先
 * @param[in]   portNo  I/Oポート番号
 */
/******************************************************************************/
static inline void IA32InstructionInWord( uint16_t *pValue,
                                          uint16_t portNo   )
{
    /* in ax命令実行 */
    __asm__ __volatile__ ( "in %0, %1"
                           : "=a" ( *pValue )
                           : "d"  ( portNo  ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       iretd命令実行
 * @details     iretd命令を実行する。
 */
/******************************************************************************/
static inline void IA32InstructionIretd( void )
{
    /* iretd命令実行 */
    __asm__ __volatile__ ( "iretd" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       lgdt命令実行
 * @details     lgdt命令を実行して、GDTをCPUに設定する。
 * 
 * @param[in]   *pBase GDTベースアドレス
 * @param[in]   limit  リミット（GDTサイズ - 1Byte）
 */
/******************************************************************************/
static inline void IA32InstructionLgdt( IA32Descriptor_t *pBase,
                                        uint16_t         limit   )
{
    IA32DescriptorPseudo_t gdtr;
    
    /* 疑似ディスクリプタ設定 */
    gdtr.pBase = pBase;
    gdtr.limit = limit;
    
    /* lgdt命令実行 */
    __asm__ __volatile__ ( "lgdt %0"
                           :
                           : "m"( gdtr ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       lidt命令実行
 * @details     lidt命令を実行して、IDTをCPUに設定する。
 * 
 * @param[in]   *pBase IDTベースアドレス
 * @param[in]   limit  リミット（IDTサイズ - 1Byte）
 */
/******************************************************************************/
static inline void IA32InstructionLidt( IA32Descriptor_t *pBase,
                                        uint16_t         limit   )
{
    IA32DescriptorPseudo_t idtr;
    
    /* 疑似ディスクリプタ設定 */
    idtr.pBase = pBase;
    idtr.limit = limit;
    
    /* lidt命令実行 */
    __asm__ __volatile__ ( "lidt %0"
                           :
                           : "m"( idtr ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       ltr命令実行
 * @details     ltr命令を実行して、タスクレジスタにTSSセレクタを設定する。
 * 
 * @param[in]   selector TSSセレクタ値
 */
/******************************************************************************/
static inline void IA32InstructionLtr( uint16_t selector )
{
    /* ltr命令実行 */
    __asm__ __volatile__ ( "ltr %0"
                           :
                           : "a" ( selector ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       out（byte）命令実行
 * @details     out（byte）命令を実行して、指定した8bitの値を指定したI/Oポート
 *              に書き込む。
 * 
 * @param[in]   portNo I/Oポート番号
 * @param[in]   value  I/Oポート設定値
 */
/******************************************************************************/
static inline void IA32InstructionOutByte( uint16_t portNo,
                                           uint8_t  value   )
{
    /* out al命令実行 */
    __asm__ __volatile__ ( "out %0, %1"
                           : 
                           : "d" ( portNo ),
                             "a" ( value  )  );
    
    return;
}


/******************************************************************************/
/**
 * @brief       out（double word）命令実行
 * @details     out（double word）命令を実行して、指定した32bitの値を指定した
 *              I/Oポートに書き込む。
 * 
 * @param[in]   portNo I/Oポート番号
 * @param[in]   value  I/Oポート設定値
 */
/******************************************************************************/
static inline void IA32InstructionOutDWord( uint16_t portNo,
                                            uint32_t value   )
{
    /* out eax命令実行 */
    __asm__ __volatile__ ( "out %0, %1"
                           : 
                           : "d" ( portNo ),
                             "a" ( value  )  );
    
    return;
}


/******************************************************************************/
/**
 * @brief       out（word）命令実行
 * @details     out（word）命令を実行して、指定した16bitの値を指定したI/Oポート
 *              に書き込む。
 * 
 * @param[in]   portNo I/Oポート番号
 * @param[in]   value  I/Oポート設定値
 */
/******************************************************************************/
static inline void IA32InstructionOutWord( uint16_t portNo,
                                           uint16_t value   )
{
    /* out ax命令実行 */
    __asm__ __volatile__ ( "out %0, %1"
                           : 
                           : "d" ( portNo ),
                             "a" ( value  )  );
    
    return;
}


/******************************************************************************/
/**
 * @brief       pop命令実行
 * @details     pop命令を実行して、スタックから指定したアドレスにポップする。
 * 
 * @param[out]  *pValue 取得値格納先アドレス
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
/* スタックポインタが破壊される事に注意 */
static inline void IA32InstructionPop( uint32_t *pValue )
{
    /* pop命令実行 */
    __asm__ __volatile__ ( "pop %0"
                           : "=a" ( *pValue ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       popad命令実行
 * @details     popad命令を実行して、スタックからedi, esi, ebp, esp, ebx, edx,
 *              ecx, eaxレジスタにポップする。
 * 
 * @attention   - スタックポインタが破壊される。
 *              - スタックからespレジスタの値はポップされるが、espレジスタに値
 *                は格納されない。
 */
/******************************************************************************/
static inline void IA32InstructionPopad( void )
{
    /* popad命令実行 */
    __asm__ __volatile__ ( "popad" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       pop（ds）命令実行
 * @details     pop命令を実行して、スタックからdsレジスタにポップする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPopDs( void )
{
    /* pop ds命令実行 */
    __asm__ __volatile__ ( "pop ds" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       pop（es）命令実行
 * @details     pop命令を実行して、スタックからesレジスタにポップする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPopEs( void )
{
    /* pop es命令実行 */
    __asm__ __volatile__ ( "pop es" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       popfd命令実行
 * @details     popfd命令を実行して、スタックからeflagsレジスタにポップする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPopfd( void )
{
    /* popfd命令実行 */
    __asm__ __volatile__ ( "popfd" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       pop（fs）命令実行
 * @details     pop命令を実行して、スタックからfsレジスタにポップする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPopFs( void )
{
    /* pop fs命令実行 */
    __asm__ __volatile__ ( "pop fs" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       pop（gs）命令実行
 * @details     pop命令を実行して、スタックからgsレジスタにポップする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPopGs( void )
{
    /* pop gs命令実行 */
    __asm__ __volatile__ ( "pop gs" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       push命令実行
 * @details     push命令を実行して、指定した32bit値をスタックにプッシュする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPush( uint32_t value )
{
    /* push命令実行 */
    __asm__ __volatile__ ( "push %0"
                           :
                           : "a" ( value ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       pushad命令実行
 * @details     pushad命令を実行して、eax, ecx, edx, ebx, esp, ebp, esi, ediレ
 *              ジスタの値をスタックにプッシュする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPushad( void )
{
    /* pushad命令実行 */
    __asm__ __volatile__ ( "pushad" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       push（ds）命令実行
 * @details     push命令を実行して、dsレジスタの値をスタックにプッシュする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPushDs( void )
{
    /* push ds命令実行 */
    __asm__ __volatile__ ( "push ds" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       push（es）命令実行
 * @details     push命令を実行して、esレジスタの値をスタックにプッシュする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPushEs( void )
{
    /* push es命令実行 */
    __asm__ __volatile__ ( "push es" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       pushfd命令実行
 * @details     pushfd命令を実行して、eflagsレジスタの値をスタックにプッシュす
 *              る。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPushfd( void )
{
    /* pushfd命令実行 */
    __asm__ __volatile__ ( "pushfd" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       push（fs）命令実行
 * @details     push命令を実行して、fsレジスタの値をスタックにプッシュする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPushFs( void )
{
    /* push fs命令実行 */
    __asm__ __volatile__ ( "push fs" );
    
    return;
}



/******************************************************************************/
/**
 * @brief       push（gs）命令実行
 * @details     push命令を実行して、gsレジスタの値をスタックにプッシュする。
 * 
 * @attention   スタックポインタが破壊される。
 */
/******************************************************************************/
static inline void IA32InstructionPushGs( void )
{
    /* push gs命令実行 */
    __asm__ __volatile__ ( "push gs" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       rep insw命令実行
 * @details     rep insw命令を実行して、指定I/Oポートから16bitの値を読み込み、
 *              指定アドレスに格納する処理をアドレスを進めながら指定回数分繰り
 *              返す。
 * 
 * @param[out]  *pAddr 格納先アドレス
 * @param[in]   port   I/Oポート番号
 * @param[in]   count  繰り返し回数
 */
/******************************************************************************/
static inline void IA32InstructionRepInsw( void     *pAddr,
                                           uint16_t port,
                                           size_t   count   )
{
    /* insw命令実行 */
    __asm__ __volatile__ ( "mov edi, %0;"   /* edi 格納先アドレス設定 */
                           "cld;"           /* 増加設定               */
                           "rep insw"       /* insw命令実行           */
                           :                /* 出力無し               */
                           : "m" ( pAddr ), /* edi 格納先アドレス     */
                             "d" ( port  ), /* dx  I/Oポート番号設定  */
                             "c" ( count )  /* ecx 繰り返し回数設定   */
                           : "edi",         /* ediレジスタ破壊指定    */
                             //"cx",         /* ecxレジスタ破壊指定    */
                             "cc" );        /* EFLAGSレジスタ破壊指定 */
    
    return;
}


/******************************************************************************/
/**
 * @brief       cr0レジスタ設定
 * @details     cr0レジスタにシステム制御フラグを設定する。
 * 
 * @param[in]   cr0  システム制御フラグ
 *                  - IA32_CR0_PG ページング
 *                  - IA32_CR0_CD キャッシュディスエーブル
 *                  - IA32_CR0_NW ノットライトスルー
 *                  - IA32_CR0_AM アライメントマスク
 *                  - IA32_CR0_WP 書込み保護
 *                  - IA32_CR0_NE 数値演算エラー
 *                  - IA32_CR0_ET 拡張タイプ
 *                  - IA32_CR0_TS タスクスイッチ
 *                  - IA32_CR0_EM エミュレーション
 *                  - IA32_CR0_MP モニタコプロセッサ
 *                  - IA32_CR0_PE 保護イネーブル
 * @param[in]   mask 設定マスク
 *                  - IA32_CR0_PG ページング
 *                  - IA32_CR0_CD キャッシュディスエーブル
 *                  - IA32_CR0_NW ノットライトスルー
 *                  - IA32_CR0_AM アライメントマスク
 *                  - IA32_CR0_WP 書込み保護
 *                  - IA32_CR0_NE 数値演算エラー
 *                  - IA32_CR0_ET 拡張タイプ
 *                  - IA32_CR0_TS タスクスイッチ
 *                  - IA32_CR0_EM エミュレーション
 *                  - IA32_CR0_MP モニタコプロセッサ
 *                  - IA32_CR0_PE 保護イネーブル
 */
/******************************************************************************/
static inline void IA32InstructionSetCr0( uint32_t cr0,
                                          uint32_t mask )
{
    /* 初期化 */
    cr0  = cr0 & mask;
    mask = ~mask;
    
    /* cr0レジスタ取得 */
    __asm__ __volatile__ ( "mov eax, cr0"
                           :
                           :
                           : "eax"        );
    
    /* システム制御フラグ設定 */
    __asm__ __volatile__ ( "and eax, %0;"
                           "or  eax, %1"
                           :
                           : "r" ( mask ), "r" ( cr0 )
                           : "eax"                     );
    
    /* cr0レジスタ設定 */
    __asm__ __volatile__ ( "mov cr0, eax"
                           :
                           :
                           : "eax"        );
    
    return;
}


/******************************************************************************/
/**
 * @brief       cr3レジスタ設定
 * @details     cr3レジスタにページディレクトリのベースアドレスを設定する。
 * 
 * @param[in]   pdbr ページディレクトリベースレジスタ
 */
/******************************************************************************/
static inline void IA32InstructionSetCr3( IA32PagingPDBR_t pdbr )
{
    /* cr3レジスタ設定 */
    __asm__ __volatile__ ( "mov cr3, %0"
                           :
                           : "a" ( pdbr ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       espレジスタ設定
 * @details     指定した値をespレジスタに設定する。
 * 
 * @param[in]   value 設定値
 */
/******************************************************************************/
static inline void IA32InstructionSetEsp( uint32_t value )
{
    /* mov命令実行 */
    __asm__ __volatile__ ( "mov esp, %0"
                           :
                           : "m" ( value )
                           :               );
    
    return;
}


/******************************************************************************/
/**
 * @brief       sti命令実行
 * @details     sti命令を実行して、CPUへの割込みを許可する。
 */
/******************************************************************************/
static inline void IA32InstructionSti( void )
{
    /* sti命令実行 */
    __asm__ __volatile__ ( "sti" );
    
    return;
}


/******************************************************************************/
/**
 * @brief       espレジスタ減算
 * @details     espレジスタを指定した値で減算する。
 * 
 * @param[in]   value 減算値
 */
/******************************************************************************/
static inline void IA32InstructionSubEsp( int32_t value )
{
    /* add命令実行 */
    __asm__ __volatile__ ( "sub esp, %0"
                           :
                           : "a" ( value ) );
    
    return;
}


/******************************************************************************/
/**
 * @brief       タスクスイッチ
 * @details     指定したスタックポインタをespレジスタに、ベースポインタをebpレ
 *              ジスタに、ページディレクトリベースをpdbrレジスタに設定し、jmp命
 *              令を用いて指定したアドレスを実行する。
 * 
 * @param[in]   pdbr   ページディレクトリベースレジスタ
 * @param[in]   *pEip  移動先アドレス
 * @param[in]   *pEsp  スタックポインタ
 * @param[in]   *pEbp  ベースポインタ
 */
/******************************************************************************/
static inline void IA32InstructionSwitchTask( IA32PagingPDBR_t pdbr,
                                              void             *pEip,
                                              void             *pEsp,
                                              void             *pEbp  )
{
    /* タスクスイッチ */
    __asm__ __volatile__ ( "mov eax, %0\n"
                           "mov ebx, %1\n"
                           "mov ebp, %2\n"
                           "mov esp, %3\n"
                           "mov cr3, eax\n"
                           "jmp ebx"
                           :
                           : "m" ( pdbr ),
                             "m" ( pEip ),
                             "m" ( pEbp ),
                             "m" ( pEsp )
                           : "eax","ebx","ebp","esp"         );
}


/******************************************************************************/
/**
 * @brief       wbinvd命令実行
 * @details     キャッシュをメモリにライトバックし、キャッシュを無効化する。
 */
/******************************************************************************/
static inline void IA32InstructionWbinvd( void )
{
    /* wbinvd命令実行 */
    __asm__ __volatile__ ( "wbinvd" );
    
    return;
}


/******************************************************************************/
#endif
