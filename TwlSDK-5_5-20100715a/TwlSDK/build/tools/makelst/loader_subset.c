/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     loader_subset.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "loader_subset.h"


extern ELUnresolvedEntry*    ELUnrEntStart;

extern u16        dbg_print_flag;
extern u32        unresolved_table_block_flag;


//Use the following statements to determine whether the processor is ARM7.
//#ifdef  SDK_ARM7
//#endif    /*SDK_ARM7*/


/*------------------------------------------------------
  Align the addresses
 -----------------------------------------------------*/
#define ALIGN( addr, align_size ) (((addr)  & ~((align_size) - 1)) + (align_size))

static u32 ELi_ALIGN( u32 addr, u32 align_size);
u32 ELi_ALIGN( u32 addr, u32 align_size)
{
    u32 aligned_addr;
    
    if( (addr % align_size) == 0) {
        aligned_addr = addr;
    }else{
        aligned_addr = (((addr) & ~((align_size) - 1)) + (align_size));
    }
    
    return aligned_addr;
}


/*------------------------------------------------------
  Copy the section to the buffer
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr    sh_size;

    /*Align*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    /*Set size*/
    sh_size = Shdr->sh_size;

    /*copy*/
    ElfHandle->ELi_ReadStub( (void*)load_start,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset)+(u32)(Shdr->sh_offset),
                             sh_size);

    /*Move the buffer pointer*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*Return the loaded header address*/
    return (void*)load_start;
}


/*------------------------------------------------------
  Allocate a section in the buffer (do not copy) and embed a 0 for that region.
  
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32            load_start;
    Elf32_Addr    sh_size;

    /*Align*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    /*Set size*/
    sh_size = Shdr->sh_size;

    /*Move the buffer pointer*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*Embed with 0 (because .bss section is expected)*/
    memset( (void*)load_start, 0, sh_size);
    
    /*Return the allocated header address  */
    return (void*)load_start;
}


/*------------------------------------------------------
  Get section header for the specified index to the buffer
 -----------------------------------------------------*/
void ELi_GetShdr( ELHandle* ElfHandle, u32 index, Elf32_Shdr* Shdr)
{
    u32 offset;
    
    offset = (ElfHandle->CurrentEhdr.e_shoff) + ((u32)(ElfHandle->shentsize) * index);
    
    ElfHandle->ELi_ReadStub( Shdr,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + offset,
                             sizeof( Elf32_Shdr));
}

/*------------------------------------------------------
  Get section entry for the specified index to the buffer
 -----------------------------------------------------*/
void ELi_GetSent( ELHandle* ElfHandle, u32 index, void* entry_buf, u32 offset, u32 size)
{
    Elf32_Shdr    Shdr;
    //u32            entry_adr;

    ELi_GetShdr( ElfHandle, index, &Shdr);
    
    ElfHandle->ELi_ReadStub( entry_buf,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + (u32)(Shdr.sh_offset) + offset,
                             size);
}


/*------------------------------------------------------
  Get specified index entry gpt the specified section header to the buffer.
    (Only section with a fixed entry size, such as Rel, Rela, Sym, and the like.)
    Shdr: Specified section header
    index: Entry number (from 0)
 -----------------------------------------------------*/
void ELi_GetEntry( ELHandle* ElfHandle, Elf32_Shdr* Shdr, u32 index, void* entry_buf)
{
    u32 offset;

    offset = (u32)(Shdr->sh_offset) + ((Shdr->sh_entsize) * index);
    
    ElfHandle->ELi_ReadStub( entry_buf,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + offset,
                             Shdr->sh_entsize);
}

/*------------------------------------------------------
  Get specified index character string of the STR section header

    Shdr: Specified section header
    index: Entry index (from 0)
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELHandle* ElfHandle, u32 strsh_index, u32 ent_index, char* str, u32 len)
{
    /*Header address of the string entry*/
    ELi_GetSent( ElfHandle, strsh_index, str, ent_index, len);
}

/*------------------------------------------------------
  Redefine symbol.

    <Rel section header>
    RelShdr->sh_link: Symbol section number
    RelShdr->sh_info: Target section number (ex.: Representing .text of rel.text)
    
    <Rel entry>
    Rel->r_offset: Offset address in target section.
    ELF32_R_SYM( Rel->r_info): Symbol entry number.
    ELF32_R_TYPE( Rel->r_info): Relocate type.

    <Sym section header>
    SymShdr->sh_link: Character string table section number of symbol
    
    <Sym entry>
    Sym->st_name : Character string entry number of symbol
    Sym->st_value : Offset address in section to which the symbol belongs
    Sym->st_size : Size in section to which the symbol belongs
    Sym->st_shndx: Section number to which the symbol belongs
    ELF32_ST_BIND (Sym->st_info) : Bind (Local or Global)
    ELF32_ST_TYPE( Sym->st_info) : Type (Target associated with the symbol)
 -----------------------------------------------------*/
void ELi_RelocateSym( ELHandle* ElfHandle, u32 relsh_index)
{
    u32                    i;
    u32                    num_of_sym;        //Overall symbol count
    u32                    num_of_rel;        //Number of symbols that should be redefined
    Elf32_Shdr             RelOrRelaShdr;    //REL or RELA section header
    Elf32_Rela            CurrentRela;    //REL or RELA entry copy destination
    Elf32_Shdr*         SymShdr;
    ELSymEx*            CurrentSymEx;
    ELSymEx*            FwdSymEx;
    ELSymEx                DmySymEx;
    ELShdrEx*            TargetShdrEx;
    ELShdrEx*            CurrentShdrEx;
    u32                    relocation_adr;
    char                sym_str[128];
    u32                    copy_size;
    ELAdrEntry*            CurrentAdrEntry;
    u32                    sym_loaded_adr;
    ELAdrEntry*            ExportAdrEntry;
    u32                    thumb_func_flag;
    ELUnresolvedEntry    UnresolvedInfo;
    ELUnresolvedEntry*    UnrEnt;
    u32                    unresolved_num = 0;

    /*Get REL or RELA section header*/
    ELi_GetShdr( ElfHandle, relsh_index, &RelOrRelaShdr);

    /*REL section and SYM section have 1:1 correspondence.*/
    ELi_GetShdr( ElfHandle, RelOrRelaShdr.sh_link, &(ElfHandle->SymShdr));
    SymShdr = &(ElfHandle->SymShdr);
    if( dbg_print_flag == 1) {
        printf( "SymShdr->link:%02x, SymShdr->info:%02x\n",
                (int)(SymShdr->sh_link), (int)(SymShdr->sh_info));
    }

    /*Target section EX header*/
    TargetShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, RelOrRelaShdr.sh_info);
    
    num_of_rel = (RelOrRelaShdr.sh_size) / (RelOrRelaShdr.sh_entsize);    //Number of symbols that should be redefined
    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //Overall symbol count

    /*---------- Create ELSymEx List  ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx->next = (void*)(malloc( sizeof(ELSymEx)));
        CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
        
        /*Copy symbol entry*/
        ELi_GetEntry( ElfHandle, SymShdr, i, &(CurrentSymEx->Sym));
        
        /*Set debug information flag*/
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
        if( CurrentShdrEx) {
            CurrentSymEx->debug_flag = CurrentShdrEx->debug_flag;
        }else{
            CurrentSymEx->debug_flag = 0;
        }

//        printf( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
    }
    CurrentSymEx->next = NULL;
    ElfHandle->SymEx = DmySymEx.next;
    /*-------------------------------------------*/

    /*----- Set the ELSymEx Thumb flag (Only the function symbol is required) -----*/
    CurrentSymEx = ElfHandle->SymEx;
    for( i=0; i<num_of_sym; i++) {
        if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
            CurrentSymEx->thumb_flag = (u16)(ELi_CodeIsThumb( ElfHandle, CurrentSymEx->Sym.st_shndx,
                                                           CurrentSymEx->Sym.st_value));
        }else{
            CurrentSymEx->thumb_flag = 0;
        }
        CurrentSymEx = CurrentSymEx->next;
    }
    /*---------------------------------------------------------------*/

    /*--- Redefine symbols that must be redefined ---*/
    for( i=0; i<num_of_rel; i++) {
        
        /*- Get Rel or Rela Entry -*/
        ELi_GetEntry( ElfHandle, &RelOrRelaShdr, i, &CurrentRela);
        
        if( RelOrRelaShdr.sh_type == SHT_REL) {
            CurrentRela.r_addend = 0;
        }
        /*-----------------------------*/

        /*Get symbol Ex entry*/
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx,
                                             ELF32_R_SYM( CurrentRela.r_info));

        if( CurrentSymEx->debug_flag == 1) {            /*When it is debugging information*/
        }else{                                            /*When not debugging information*/
            /**/
            ELi_UnresolvedInfoInit( &UnresolvedInfo);
            /*Rewritten address (Called P in the specifications)*/
            relocation_adr = (TargetShdrEx->loaded_adr) + (CurrentRela.r_offset);
            UnresolvedInfo.r_type = ELF32_R_TYPE( CurrentRela.r_info);
            UnresolvedInfo.A_ = (CurrentRela.r_addend);
            UnresolvedInfo.P_ = (relocation_adr);
            UnresolvedInfo.sh_type = (RelOrRelaShdr.sh_type);
            
            /*Identify the symbol address*/
            if( CurrentSymEx->Sym.st_shndx == SHN_UNDEF) {
                /*Search from address table*/
                ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
                CurrentAdrEntry = EL_GetAdrEntry( sym_str);
                if( CurrentAdrEntry) {
                    sym_loaded_adr = (u32)(CurrentAdrEntry->adr);
                    /*THUMB function flag (called T in the specifications) to differentiate THUMB or ARM*/
                    thumb_func_flag = CurrentAdrEntry->thumb_flag;
                    if( dbg_print_flag == 1) {
                        printf( "\n symbol found %s : %8x\n", sym_str, (int)(sym_loaded_adr));
                    }
                }else{
                    /*Error when not found (Cannot resolve S_ and T_)*/
                    copy_size = (u32)strlen( sym_str) + 1;
                    UnresolvedInfo.sym_str = (char*)(malloc( copy_size));
                    //MI_CpuCopy8( sym_str, UnresolvedInfo.sym_str, copy_size);
                    memcpy( UnresolvedInfo.sym_str, sym_str, copy_size);

                    /*Added to global unresolved table*/
                    copy_size = sizeof( ELUnresolvedEntry);
                    UnrEnt = (ELUnresolvedEntry*)(malloc( copy_size));
                    //MI_CpuCopy8( &UnresolvedInfo, UnrEnt, copy_size);
                    memcpy( UnrEnt, &UnresolvedInfo, copy_size);
                    
                    if( unresolved_table_block_flag == 0) {    //If adding to the table is not prohibited
                        ELi_AddUnresolvedEntry( UnrEnt);
                    }

                    unresolved_num++;    /*Count number of unresolved symbols*/
                    if( dbg_print_flag == 1) {
                         printf( "WARNING! cannot find symbol : %s\n", sym_str);
                    }
                }
            }else{
                /*Get Ex header of section where symbol belongs*/
                CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
                sym_loaded_adr = CurrentShdrEx->loaded_adr;
                sym_loaded_adr += CurrentSymEx->Sym.st_value;    //sym_loaded_adr is called S in the specifications
                /*THUMB function flag (called T in the specifications) to differentiate THUMB or ARM*/
                thumb_func_flag = CurrentSymEx->thumb_flag;
            }

            if( !UnresolvedInfo.sym_str) {        /*Symbol cannot be resolved if sym_str is not set*/
                /*Set to same variable name as in the specifications*/
                UnresolvedInfo.S_ = (sym_loaded_adr);
                UnresolvedInfo.T_ = (thumb_func_flag);

                /*--------------- Resolve symbol (Execute Relocation) ---------------*/
//                CurrentSymEx->relocation_val = ELi_DoRelocate( &UnresolvedInfo);
                /*------------------------------------------------------------*/
            }
        }
    }
    /*-----------------------------------*/
    /*--- Made GLOBAL symbol in library public to address table ---*/
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx, i);
        /*When global and the related section exist in the library*/
        if( ((ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_WEAK) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_MW_SPECIFIC))&&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = (ELAdrEntry*)(malloc( sizeof(ELAdrEntry)));    /*Memory allocation*/
            
            ExportAdrEntry->next = NULL;
            
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)strlen( sym_str) + 1;
            ExportAdrEntry->name = (char*)(malloc( copy_size));
            //MI_CpuCopy8( sym_str, ExportAdrEntry->name, copy_size);
            memcpy( ExportAdrEntry->name, sym_str, copy_size);

            CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
            //There are cases where Sym.st_value is adjusted to enable determination of ARM/Thumb with odd/even numbers, so delete the adjustment to attain the net value.
            ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;
            
            if( EL_GetAdrEntry( ExportAdrEntry->name) == NULL) {    //If not in
                if( dbg_print_flag == 1) {
                    printf( "Add Entry : %s(0x%x), func=%d, thumb=%d\n",
                                ExportAdrEntry->name,
                                (int)(ExportAdrEntry->adr),
                                ExportAdrEntry->func_flag,
                                ExportAdrEntry->thumb_flag);
                }
                EL_AddAdrEntry( ExportAdrEntry);    //registration
            }
        }
    }
    /*----------------------------------------------------------------*/

    /*---------- Free the ELSymEx List  ----------*/
    CurrentSymEx = ElfHandle->SymEx;
    if( CurrentSymEx) {
        while( CurrentSymEx->next != NULL) {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            free( FwdSymEx);
        }
        ElfHandle->SymEx = NULL;
    }
    /*-----------------------------------------*/

    /* After completing relocation */
    if( unresolved_num == 0) {
        ElfHandle->process = EL_RELOCATED;
    }
}

/*------------------------------------------------------
    makelst-specific function
    Register global items to address table from the symbol section
    
 -----------------------------------------------------*/
void ELi_DiscriminateGlobalSym( ELHandle* ElfHandle, u32 symsh_index)
{
    u32                    i;
    u32                    num_of_sym;
    Elf32_Shdr            CurrentSymShdr;
    Elf32_Shdr*         SymShdr;        //SYM section header
    ELSymEx*            CurrentSymEx;
    ELSymEx*            FwdSymEx;
    ELSymEx                DmySymEx;
    ELShdrEx*            CurrentShdrEx;
    ELAdrEntry*            ExportAdrEntry;
    char                sym_str[128];
    u32                    copy_size;
    
    /*Get SYM section header*/
    ELi_GetShdr( ElfHandle, symsh_index, &CurrentSymShdr);
    SymShdr = &CurrentSymShdr;

    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //Overall symbol count

    /*---------- Create ELSymEx List  ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx->next = (void*)(malloc( sizeof(ELSymEx)));
        CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
        
        /*Copy symbol entry*/
        ELi_GetEntry( ElfHandle, SymShdr, i, &(CurrentSymEx->Sym));
        
        /*Set debug information flag*/
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
        if( CurrentShdrEx) {
            CurrentSymEx->debug_flag = CurrentShdrEx->debug_flag;
        }else{
            CurrentSymEx->debug_flag = 0;
        }

//        printf( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
    }
    CurrentSymEx->next = NULL;
    ElfHandle->SymEx = DmySymEx.next;
    /*-------------------------------------------*/

    /*----- Set the ELSymEx Thumb flag (Only the function symbol is required) -----*/
    CurrentSymEx = ElfHandle->SymEx;
    for( i=0; i<num_of_sym; i++) {
        if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
            CurrentSymEx->thumb_flag = (u16)(ELi_CodeIsThumb( ElfHandle, CurrentSymEx->Sym.st_shndx,
                                                           CurrentSymEx->Sym.st_value));
        }else{
            CurrentSymEx->thumb_flag = 0;
        }
        CurrentSymEx = CurrentSymEx->next;
    }
    /*---------------------------------------------------------------*/
    /*--- Made GLOBAL symbol in library public to address table ---*/
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx, i);
        /*When global and the related section exist in the library*/
        if( ((ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_WEAK) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_MW_SPECIFIC))&&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = (ELAdrEntry*)(malloc( sizeof(ELAdrEntry)));    /*Memory allocation*/
            
            ExportAdrEntry->next = NULL;
            
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)strlen( sym_str) + 1;
            ExportAdrEntry->name = (char*)(malloc( copy_size));
            //MI_CpuCopy8( sym_str, ExportAdrEntry->name, copy_size);
            memcpy( ExportAdrEntry->name, sym_str, copy_size);

            if( (CurrentSymEx->Sym.st_shndx) < SHN_LORESERVE) { //When there is a related section
                if( (CurrentSymEx->Sym.st_shndx == SHN_ABS)) {
                    //There are cases where Sym.st_value is adjusted to enable determination of ARM/Thumb with odd/even numbers, so delete the adjustment to attain the net value.
                    ExportAdrEntry->adr = (void*)((CurrentSymEx->Sym.st_value)&0xFFFFFFFE);
                }else{
                    CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
                    //There are cases where Sym.st_value is adjusted to enable determination of ARM/Thumb with odd/even numbers, so delete the adjustment to attain the net value.
                    ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
                }
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;

            if( EL_GetAdrEntry( ExportAdrEntry->name) == NULL) {    //If not in
                if( dbg_print_flag == 1) {
                    printf( "Add Entry : %s(0x%x), func=%d, thumb=%d\n",
                                ExportAdrEntry->name,
                                (int)(ExportAdrEntry->adr),
                                ExportAdrEntry->func_flag,
                                ExportAdrEntry->thumb_flag);
                }            
                EL_AddAdrEntry( ExportAdrEntry);    //registration
            }
            }
        }
    }
    /*----------------------------------------------------------------*/

    /*---------- Free the ELSymEx List  ----------*/
    CurrentSymEx = ElfHandle->SymEx;
    if( CurrentSymEx) {
        while( CurrentSymEx->next != NULL) {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            free( FwdSymEx);
        }
        ElfHandle->SymEx = NULL;
    }
    /*-----------------------------------------*/
}


/*------------------------------------------------------
  Resolve symbol based on unresolved information

    It is necessary to know all of the r_type, S_, A_, P_, and T_
 -----------------------------------------------------*/
#define _S_    (UnresolvedInfo->S_)
#define _A_    (UnresolvedInfo->A_)
#define _P_    (UnresolvedInfo->P_)
#define _T_    (UnresolvedInfo->T_)
u32    ELi_DoRelocate( ELUnresolvedEntry* UnresolvedInfo)
{
    s32 signed_val;
    u32 relocation_val = 0;
    u32 relocation_adr;

    relocation_adr = _P_;

    switch( (UnresolvedInfo->r_type)) {
      case R_ARM_PC24:
      case R_ARM_PLT32:
      case R_ARM_CALL:
      case R_ARM_JUMP24:
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (((*(vu32*)relocation_adr)|0xFF800000) << 2);    //Generally, this should be -8
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        if( _T_) {        /*Jump from ARM to Thumb with BLX instruction (If less than v5, there must be a way to change a BX instruction into a BL instruction in a veneer)*/
            relocation_val = (0xFA000000) | ((signed_val>>2) & 0x00FFFFFF) | (((signed_val>>1) & 0x1)<<24);
        }else{            /*Jump from ARM to ARM using the BL instruction*/
            signed_val >>= 2;
            relocation_val = (*(vu32*)relocation_adr & 0xFF000000) | (signed_val & 0x00FFFFFF);
        }
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_ABS32:
        relocation_val = (( _S_ + _A_) | _T_);
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_REL32:
        relocation_val = (( _S_ + _A_) | _T_) - _P_;
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_LDR_PC_G0:
        signed_val = ( (s32)(_S_) + _A_) - (s32)(_P_);
        signed_val >>= 2;
        relocation_val = (*(vu32*)relocation_adr & 0xFF000000) | (signed_val & 0x00FFFFFF);
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_ABS16:
      case R_ARM_ABS12:
      case R_ARM_THM_ABS5:
      case R_ARM_ABS8:
        relocation_val = _S_ + _A_;
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_THM_PC22:/*Different Name: R_ARM_THM_CALL*/
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (((*(vu16*)relocation_adr & 0x07FF)<<11) + ((*((vu16*)(relocation_adr)+1)) & 0x07FF));
            _A_ = (_A_ | 0xFFC00000) << 1;    //Generally, this should be -4 (Because the PC is the current instruction address +4)
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        signed_val >>= 1;
        if( _T_) {    /*Jump from Thumb to Thumb using the BL instruction*/
            relocation_val = ((*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF)) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xF800) | (signed_val & 0x07FF)) << 16);
        }else{        /*Jump from Thumb to ARM with BLX instruction (If less than v5, there must be a way to change a BX instruction into a BL instruction in a veneer)*/
            if( (signed_val & 0x1)) {    //Come here when _P_ is not 4-byte aligned
                signed_val += 1;
            }
            relocation_val = ((*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF)) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xE800) | (signed_val & 0x07FF)) << 16);
        }
        *(vu16*)relocation_adr = (vu16)relocation_val;
        *((vu16*)relocation_adr+1) = (vu16)((u32)(relocation_val) >> 16);
        break;
      case R_ARM_THM_JUMP24:
        break;
      default:
        if( dbg_print_flag == 1) {
            printf( "ERROR! : unsupported relocation type!\n");
        }
        break;
    }
    
    return relocation_val;
}
#undef _S_
#undef _A_
#undef _P_
#undef _T_

/*------------------------------------------------------
  Read ELSymEx of the index specified from the list.
 -----------------------------------------------------*/
ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index)
{
    u32         i;
    ELSymEx*    SymEx;

    SymEx = SymExStart;
    for( i=0; i<index; i++) {
        SymEx = (ELSymEx*)(SymEx->next);
        if( SymEx == NULL) {
            break;
        }
    }
    return SymEx;
}

/*------------------------------------------------------
  Read ELShdrx of the index specified from the list.
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index)
{
    u32         i;
    ELShdrEx*    ShdrEx;

    ShdrEx = ShdrExStart;
    for( i=0; i<index; i++) {
        ShdrEx = (ELShdrEx*)(ShdrEx->next);
        if( ShdrEx == NULL) {
            break;
        }
    }
    return ShdrEx;
}



/*------------------------------------------------------
  Determine whether the section for the specified index is debug information

<Definition of debugging information>
- Sections whose section name begins with .debug.

- Section, such as .rel.debug - section, where the sh_info indicates the debug information section number
  
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELHandle* ElfHandle, u32 index)
{
    Elf32_Shdr    TmpShdr;
    char        shstr[6];

    /*-- Get six characters of character string for section name --*/
    ELi_GetShdr( ElfHandle, index, &TmpShdr);
    ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                   TmpShdr.sh_name, shstr, 6);
    /*-------------------------------------*/
    
    if( strncmp( shstr, ".debug", 6) == 0) {    /*When debugging section*/
        return TRUE;
    }else{                        /*When relocated section relates to the debugging section*/
        if( (TmpShdr.sh_type == SHT_REL) || (TmpShdr.sh_type == SHT_RELA)) {
            if( ELi_ShdrIsDebug( ElfHandle, TmpShdr.sh_info) == TRUE) {
                return TRUE;
            }
        }
    }

    return FALSE;
}



/*------------------------------------------------------
  Check the ElfHandle SymEx table, and determine whether the code in the specified offset of the specified index is ARM or THUMB
  
  (Preset ElfHandle->SymShdr and ElfHandle->SymEx.)
    
    sh_index: Section index to be checked
    offset: Offset in section to check
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELHandle* ElfHandle, u16 sh_index, u32 offset)
{
    u32            i;
    u32            thumb_flag;
    Elf32_Shdr*    SymShdr;
    char        str_adr[3];
    ELSymEx*    CurrentSymEx;

    /*Get symbol section header and SymEx list*/
    SymShdr = &(ElfHandle->SymShdr);
    CurrentSymEx = ElfHandle->SymEx;

    i = 0;
    thumb_flag = 0;
    while( CurrentSymEx != NULL) {
        
        if( CurrentSymEx->Sym.st_shndx == sh_index) {
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, str_adr, 3);
            if( strncmp( str_adr, "$a\0", strlen("$a\0")) == 0) {
                thumb_flag = 0;
            }else if( strncmp( str_adr, "$t\0", strlen("$t\0")) == 0) {
                thumb_flag = 1;
            }
            if( CurrentSymEx->Sym.st_value > offset) {
                break;
            }
        }
        
        CurrentSymEx = CurrentSymEx->next;
        i++;
    }

    return thumb_flag;
}


/*---------------------------------------------------------
 Initialize unresolved information entries
 --------------------------------------------------------*/
void ELi_UnresolvedInfoInit( ELUnresolvedEntry* UnresolvedInfo)
{
    UnresolvedInfo->sym_str = NULL;
    UnresolvedInfo->r_type = 0;
    UnresolvedInfo->S_ = 0;
    UnresolvedInfo->A_ = 0;
    UnresolvedInfo->P_ = 0;
    UnresolvedInfo->T_ = 0;
    UnresolvedInfo->remove_flag = 0;
}

/*------------------------------------------------------
  Delete entry from the unresolved information table
 -----------------------------------------------------*/
BOOL ELi_RemoveUnresolvedEntry( ELUnresolvedEntry* UnrEnt)
{
    ELUnresolvedEntry    DmyUnrEnt;
    ELUnresolvedEntry*    CurrentUnrEnt;

    if( UnrEnt == NULL) {
        return FALSE;
    }
    
    DmyUnrEnt.next = ELUnrEntStart;
    CurrentUnrEnt = &DmyUnrEnt;

    while( CurrentUnrEnt->next != UnrEnt) {
        if( CurrentUnrEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentUnrEnt = (ELUnresolvedEntry*)(CurrentUnrEnt->next);
        }
    }

    CurrentUnrEnt->next = UnrEnt->next;
    free( UnrEnt->sym_str);
    free( UnrEnt);
    ELUnrEntStart = DmyUnrEnt.next;
    
    return TRUE;
}

/*---------------------------------------------------------
 Add entry to the unresolved information table
 --------------------------------------------------------*/
void ELi_AddUnresolvedEntry( ELUnresolvedEntry* UnrEnt)
{
    ELUnresolvedEntry    DmyUnrEnt;
    ELUnresolvedEntry*    CurrentUnrEnt;

    if( !ELUnrEntStart) {
        ELUnrEntStart = UnrEnt;
    }else{
        DmyUnrEnt.next = ELUnrEntStart;
        CurrentUnrEnt = &DmyUnrEnt;

        while( CurrentUnrEnt->next != NULL) {
            CurrentUnrEnt = CurrentUnrEnt->next;
        }
        CurrentUnrEnt->next = (void*)UnrEnt;
    }
    UnrEnt->next = NULL;
}

/*------------------------------------------------------
  Search for entry corresponding to specified string in the unresolved information table
 -----------------------------------------------------*/
ELUnresolvedEntry* ELi_GetUnresolvedEntry( char* ent_name)
{
    ELUnresolvedEntry* CurrentUnrEnt;

    CurrentUnrEnt = ELUnrEntStart;
    if( CurrentUnrEnt == NULL) {
        return NULL;
    }
    while( 1) {
        if( (strcmp( CurrentUnrEnt->sym_str, ent_name) == 0)&&
            (CurrentUnrEnt->remove_flag == 0)) {
            break;
        }
        CurrentUnrEnt = (ELUnresolvedEntry*)CurrentUnrEnt->next;
        if( CurrentUnrEnt == NULL) {
            break;
        }
    }
    return CurrentUnrEnt;
}
