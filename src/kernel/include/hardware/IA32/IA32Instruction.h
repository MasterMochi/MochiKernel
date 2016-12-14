/******************************************************************************/
/* src/kernel/include/hardware/IA32/IA32Instruction.h                         */
/*                                                                 2016/12/13 */
/* Copyright (C) 2016 Mochi.                                                  */
/******************************************************************************/
#ifndef IA32_INSTRUCTION_H
#define IA32_INSTRUCTION_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>
#include "IA32Descriptor.h"


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
 * @brief       espレジスタ値取得
 * @details     espレジスタの値を指定したアドレスに格納する。
 * 
 * @param[out]  *pValue espレジスタ値格納先
 */
/******************************************************************************/
static inline void IA32InstructionGetEsp( uint32_t *pValue )
{
    /* mov命令実行 */
    __asm__ __volatile__ ( "mov %0, esp"
                           : "=m" ( *pValue )
                           :
                           :                  );
    
    return;
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
                           : "=a" ( *pValue )
                           :
                           :                  );
    
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
 * @details     指定したスタックポインタをespレジスタに設定し、jmp命令を用いて
 *              指定したアドレスを実行する。
 * 
 * @param[in]   *pEip 移動先アドレス
 * @param[in]   *pEsp スタックポインタ
 */
/******************************************************************************/
static inline void IA32InstructionSwitchTask( void *pEip,
                                              void *pEsp )
{
    /* タスクスイッチ */
    __asm__ __volatile__ ( "mov eax, %0\n"
                           "mov esp, %1\n"
                           "jmp eax"
                           :
                           : "m" ( pEip ), "m" ( pEsp )
                           :                            );
}


/******************************************************************************/
#endif
