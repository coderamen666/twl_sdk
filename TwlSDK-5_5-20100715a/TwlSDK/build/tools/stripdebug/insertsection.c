/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     insertsection.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include "types.h"
#include "elf.h"
#include "elf_loader.h"
#include "arch.h"
#include "loader_subset.h"
#include <string.h>
#include <stdlib.h>



extern u32 ELi_ALIGN( u32 addr, u32 align_size);



u16 ELi_LoadObject2( ELHandle* ElfHandle, void* obj_offset, void* buf)
{
    u16         i;
#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
    u16         j;
#endif
    ELShdrEx*   FwdShdrEx;
    ELShdrEx*   CurrentShdrEx;
    ELShdrEx    DmyShdrEx;
    u32         newelf_shoff = 0; //Write pointer to stripped ELF image
    u32         buf_shdr;
    u32         section_num = 0;
    u32         stripped_section_num = 0;
    u32         tmp_buf;
    u32         *shdr_table;      //Table for new/old number correspondence for section headers
    u32         special_sect_name;


    /* Check initialization of ELHandle */
    if( ElfHandle->process != EL_INITIALIZED) {
        return EL_FAILED;
    }
    /* Buffer NULL check */
    if( buf == NULL) {
        return EL_FAILED;
    }
    /* Get ELF header */
    ElfHandle->ELi_ReadStub( &(ElfHandle->CurrentEhdr), ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head), (u32)(obj_offset), sizeof( Elf32_Ehdr));

    /* Section handle build */
    ElfHandle->elf_offset = obj_offset;
    ElfHandle->buf_current = (void*)((u32)buf + sizeof( Elf32_Ehdr));
    ElfHandle->shentsize = ElfHandle->CurrentEhdr.e_shentsize;

    /*Section header table build*/
    shdr_table = (u32*)malloc( 4 * ElfHandle->CurrentEhdr.e_shnum);
  
    /*---------- Create ELShdrEx and shdr_table Lists  ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
        CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
        memset( CurrentShdrEx, 0, sizeof(ELShdrEx));     //Clear zero
        
        /*Set flag to distinguish whether this is debugging information*/
        if( ELi_ShdrIsDebug( ElfHandle, i) == TRUE) {    /*When it is debugging information*/
            CurrentShdrEx->debug_flag = 1;
            shdr_table[i] = 0xFFFFFFFF;
        }else{                                           /*When not debugging information*/
            CurrentShdrEx->debug_flag = 0;
            shdr_table[i] = stripped_section_num;        /*Create section new/old table*/
            stripped_section_num++;
        }
        /*Copy section header*/
        ELi_GetShdr( ElfHandle, i, &(CurrentShdrEx->Shdr));
//        printf( "shdr_table[0x%x] = 0x%x\n", i, section_num);
        section_num++;
        /*Get section string*/
#if 0 // Do not dynamically ensure section string buffer
        CurrentShdrEx->str = (char*)malloc( 128);    //Get 128 character buffer
        ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                       CurrentShdrEx->Shdr.sh_name,
                       CurrentShdrEx->str, 128);
#else
        {
            u32 length;
            
            length = ELi_GetStrLen( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx, CurrentShdrEx->Shdr.sh_name ) + 1;
            
            CurrentShdrEx->str = (char*)malloc(length);
            if(CurrentShdrEx->str == 0)
            {
                return EL_FAILED;
            }
            ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
               CurrentShdrEx->Shdr.sh_name,
               CurrentShdrEx->str, length);
        }
#endif
        /*Index maintenance of section name of special section*/
        if( i == ElfHandle->CurrentEhdr.e_shstrndx){
            special_sect_name = CurrentShdrEx->Shdr.sh_size;
        }
    }
#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
    section_num++;                    //Add special section
    ElfHandle->CurrentEhdr.e_shnum++; //Add special section
    CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
    CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
    memset( CurrentShdrEx, 0, sizeof(ELShdrEx));    //Clear zero
    CurrentShdrEx->Shdr.sh_name = special_sect_name;
    CurrentShdrEx->Shdr.sh_type = SPECIAL_SECTION_TYPE;
    CurrentShdrEx->Shdr.sh_flags = 0;
    CurrentShdrEx->Shdr.sh_addr = 0;
    CurrentShdrEx->Shdr.sh_offset = 0;
    CurrentShdrEx->Shdr.sh_size = (stripped_section_num)*4; //(ElfHandle->CurrentEhdr.e_shnum)*4; (When old section number is in the index)
    CurrentShdrEx->Shdr.sh_link = 0;
    CurrentShdrEx->Shdr.sh_info = 0;
    CurrentShdrEx->Shdr.sh_addralign = 4;
    CurrentShdrEx->Shdr.sh_entsize = 4;
#if 0 // Do not dynamically allocate section string buffer 
    CurrentShdrEx->str = (char*)malloc( 128);
#else
    CurrentShdrEx->str = (char*)malloc( strlen(SPECIAL_SECTION_NAME) + 1 );
    if(CurrentShdrEx->str == 0)
    {
        return EL_FAILED;
    }
#endif
    strcpy( CurrentShdrEx->str, SPECIAL_SECTION_NAME);
#endif
    CurrentShdrEx->next = NULL;
    ElfHandle->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------------------*/
//printf( "stripped section_num : %d\n", stripped_section_num);
   
    /*---------- Check and copy all sections  ----------*/
//    printf( "\nLoad to RAM:\n");
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
            if( CurrentShdrEx->Shdr.sh_type == SPECIAL_SECTION_TYPE) {
                //Series of processes equivalent to ELi_CopySectionToBuffer
                u32 load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
                u32 sh_size    = CurrentShdrEx->Shdr.sh_size;
                u32* reverse_shdr_table = (u32*)malloc( 4 * stripped_section_num);
                /*Index converts to new section number table*/
                for( j=0; j<(ElfHandle->CurrentEhdr.e_shnum - 1); j++) {
                    if( shdr_table[j] != 0xFFFFFFFF) {
                        reverse_shdr_table[shdr_table[j]] = j;
                    }
                }
                memcpy( (u32*)load_start, reverse_shdr_table, sh_size); //Copy new/old section correspondence table
                free( reverse_shdr_table);
                CurrentShdrEx->loaded_adr = load_start;
                ElfHandle->buf_current = (void*)(load_start + sh_size);
            }else{
#else
            {
#endif
                if( CurrentShdrEx->Shdr.sh_type != SHT_NULL) { //NULL section (section 0) has no content
                    //Copy to memory
                    CurrentShdrEx->loaded_adr = (u32)
                                    ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                }
#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
                //For section string sections
                if( (CurrentShdrEx->Shdr.sh_type == SHT_STRTAB)&&
                    (i == ElfHandle->CurrentEhdr.e_shstrndx)) {
                    strcpy( ElfHandle->buf_current, SPECIAL_SECTION_NAME);
                    //Update section data capacity
                    CurrentShdrEx->Shdr.sh_size += (strlen( SPECIAL_SECTION_NAME) + 1);
                    //Update buffer address
                    ElfHandle->buf_current += (strlen( SPECIAL_SECTION_NAME) + 1);
                }
#endif
            }
    }
    /*-------------------------------------------------------*/

    
    /*Only the section content up to this point was copied to stripped ELF*/

  
    /*---------- Copy section header to stripped elf ----------*/
    buf_shdr = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    ElfHandle->buf_current = (void*)buf_shdr;
//    printf( "buf_shdr = 0x%x\n", buf_shdr);
//    printf( "buf = 0x%x\n", (u32)buf);
    
    newelf_shoff = buf_shdr - ((u32)(buf));
//    printf( "newelf_shoff = 0x%x\n", newelf_shoff);

    section_num = 0;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        tmp_buf = buf_shdr + ( section_num * sizeof( Elf32_Shdr));
        
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
        /*Update offset*/
        if( CurrentShdrEx->loaded_adr != 0) {
            CurrentShdrEx->Shdr.sh_offset = (CurrentShdrEx->loaded_adr - (u32)buf);
        }
        /*Copy section header to stripped ELF image*/
        memcpy( (u8*)tmp_buf, &(CurrentShdrEx->Shdr), 
                sizeof( Elf32_Shdr));
        section_num++;                        /*Update section count*/
    }
//printf( "section_num : %d\n", section_num);

    // After copy ends
    ElfHandle->process = EL_COPIED;
    /*------------------------------------------------------------*/

    ElfHandle->newelf_size = (buf_shdr - (u32)buf) + (section_num*sizeof( Elf32_Shdr));
//    printf( "newelf_size = 0x%x\n", ElfHandle->newelf_size);

    /*---------- Update ELF Header ----------*/
    ElfHandle->CurrentEhdr.e_shnum = section_num; /*Apply the increased section count to the ELF header*/
    //Update the section header offset
    ElfHandle->CurrentEhdr.e_shoff = newelf_shoff;
    memcpy( (u8*)buf, &(ElfHandle->CurrentEhdr), sizeof( Elf32_Ehdr)); /*Copy ELF header to the stripped ELF image*/
    /*-----------------------------------*/

    /*---------- Free the ELShdrEx List  ----------*/
    CurrentShdrEx = ElfHandle->ShdrEx;
    if( CurrentShdrEx) {
        do{
            FwdShdrEx = CurrentShdrEx;
            CurrentShdrEx = CurrentShdrEx->next;
            free( FwdShdrEx->str);
            free( FwdShdrEx);
        }while( CurrentShdrEx != NULL);
        ElfHandle->ShdrEx = NULL;
    }
    /*-----------------------------------------*/

    /*Flush cache before DLL is called on RAM*/
//    DC_FlushAll();
//    DC_WaitWriteBufferEmpty();
    
    return (ElfHandle->process);
}
