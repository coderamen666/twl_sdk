/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - showversion
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-03#$
  $Rev: 9495 $
  $Author: sasaki_yu $
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

extern const unsigned long SDK_DATE_OF_LATEST_FILE;

#define  LE(a)   ((((a)<<24)&0xff000000)|(((a)<<8)&0x00ff0000)|\
                  (((a)>>8)&0x0000ff00)|(((a)>>24)&0x000000ff))

#define  MAGIC_CODE     0x2106c0de
#define  BUILD_MAGIC_CODE   0x3381c0de

#define ERROR_NO_VERSION_INFO 1
#define ERROR_BUILDTYPE_MISMATCH 2

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef union
{
    u8      byte[4];
    u32     word;
}
tFormat;

typedef struct
{
    u8      reserve[32];               // 
    u32     rom_offset;                // Transfer source ROM offset
    u32     entry_address;             // Execution start address
    u32     ram_address;               // Transfer destination RAM address
    u32     size;                      // Size of the transmission
}
tRomHeader;

#define ROMOFFSET_MODULE9       0x00000020      // ARM9 resident module information region

typedef struct
{
    u8      major;
    u8      minor;
    u16     relstep;
    u8      relclass;
    u8      relnumber_major;
    u8      relnumber_minor;
    char   *relname;
}
tVersion;


#define RELCLASS_TEST          0
#define RELCLASS_PR            1
#define RELCLASS_RC            2

#define RELSTEP_PR6            106
#define RELSTEP_FC             200
#define RELSTEP_RELEASE        300

int findMagicNumber(FILE *fp, u32 magic_code, u32 *buffer, u32 buffer_len);


//
// Search magic number and store preceding data in an array.
// Example: magic_code = 0x12345678, buffer_len = 4
//   When the data is lined up as data2, data1, 0x12345678, 0x87654321, the following are the results.
// buffer[0] = 0x87654321
// buffer[1] = 0x12345678
// buffer[2] = data1
// buffer[3] = data2
// 
// Return value: If the magic number is found, 1 is returned; if it is not found, 0 is returned.
int findMagicNumber(FILE *fp, u32 magic_code, u32 *buffer, u32 buffer_len)
{
    u32 i = 0;
    while (1 == fread(buffer, sizeof(u32), 1, fp))
    {
        if(buffer[1] == magic_code && buffer[0] == LE(magic_code))
        {
            return 1;
        }
        
        for(i = 1; i < buffer_len; ++i)
        {
            buffer[buffer_len - i] = buffer[buffer_len - i - 1];
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    u32     buffer[4];
    FILE   *fp;
    tRomHeader romHeader;
    u32     magicCodeBE, magicCodeLE;
    u32     buildMagicCodeBE, buildMagicCodeLE;
    u16     one = 0x0001;
    tFormat f;
    tVersion v;
    u16     relnumber;
    u32     error = 0;
    magicCodeBE = *(u8 *)&one ? LE(MAGIC_CODE) : MAGIC_CODE;
    magicCodeLE = LE(magicCodeBE);
    buildMagicCodeBE = *(u8 *)&one ? LE(BUILD_MAGIC_CODE) : BUILD_MAGIC_CODE;
    buildMagicCodeLE = LE(buildMagicCodeBE);

    if (argc != 2)
    {
        fprintf(stderr,
                "NITRO-SDK Development Tool - showversion - Show version information\n"
                "Build %lu\n"
                "\n"
                "Usage: showversion ROMFILE\n"
                "\n"
                "  Show NITRO-SDK version information in ROMFILE\n" "\n", SDK_DATE_OF_LATEST_FILE);
        return 5;
    }

    if (NULL == (fp = fopen(argv[1], "rb")))
    {
        fprintf(stderr, "Error: Cannot open '%s'\n", argv[1]);
        return 1;
    }

    if (1 != fread(&romHeader, sizeof(romHeader), 1, fp))
    {
        fclose(fp);
        fprintf(stderr, "Error: Cannot open '%s'\n", argv[1]);
        return 1;
    }

    //
    // Determine whether it is a ROM file
    //          Here this determines whether execution start address and transfer destination RAM address are within the range 0x02000000 to 0x027fffff.
    //           
    //
    if ((romHeader.ram_address & 0xff800000) != 0x02000000 ||
        (romHeader.entry_address & 0xff800000) != 0x02000000)
    {
        //
        // If not a ROM file, destroy ROM header information (zero clears it) and return file pointer to top.
        //  
        //
        (void)memset(&romHeader, 0, sizeof(tRomHeader));
        rewind(fp);
    }

    v.major = 0;
    buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;

    //
    // Determine that magicCode is lined up in this order in ROM and display versionInfo:
    //      buffer[2] versionInfo
    //      buffer[1] magicCodeBE
    //      buffer[0] magicCodeLE
    //  
    //
    while (findMagicNumber(fp, magicCodeBE, buffer, 3))
    {
        f.word = buffer[2];
        v.major = f.byte[3];       // 31..24bit
        v.minor = f.byte[2];       // 23..16bit
        v.relstep = ((u16)f.byte[0]) | (((u16)f.byte[1]) << 8);     // 15...0bit
        v.relclass = v.relstep / 10000;
        relnumber = v.relstep % 10000;
        v.relnumber_major = relnumber / 100;
        v.relnumber_minor = relnumber % 100;

        switch (v.relstep / 100)   // Determine the last three digits
        {
            //
            // Special number determination
            //
        case RELSTEP_PR6:
            if (v.relnumber_minor == 50)
            {
                v.relname = "IPL";
                v.relnumber_major = 0;
                v.relnumber_minor = 0;
            }
            else
            {
                v.relname = "PR";
                if (v.relnumber_minor > 50)
                {
                    v.relnumber_minor -= 50;
                }
            }
            break;

        case RELSTEP_FC:
            v.relname = "FC";
            break;

        case RELSTEP_RELEASE:
            v.relname = "RELEASE";
            break;

        default:

            //
            // Determine normal numbers (postfix notation)
            //
            switch (v.relclass)
            {
            case RELCLASS_TEST:
                v.relname = "test";
                break;

            case RELCLASS_PR:
                v.relname = "PR";
                break;

            case RELCLASS_RC:
                v.relname = "RC";
                break;

            default:
                v.relname = NULL;
                break;
            }
            break;
        }

        printf("NITRO-SDK VERSION: %d.%02d", v.major, v.minor);

        //
        // Displaying release class name (test/PR/RC and the like)
        //
        if (v.relname)
        {
            printf(" %s", v.relname);

            if (v.relnumber_major)
            {
                printf("%d", v.relnumber_major);
            }
            if (v.relnumber_minor)
            {
                printf(" plus");

                if (v.relnumber_minor > 1)
                {
                    printf("%d", v.relnumber_minor);
                }
            }
        }
        else
        {
            printf("XXX [unknown RelID=%d]", v.relstep);
        }

        //
        // Determine whether within a permanent area
        //
        if ((u32)ftell(fp) - romHeader.rom_offset < romHeader.size)
        {
            printf(" [STATIC MODULE]");
        }

        printf("\n");

        //
        // Message for when the value had no setting
        //
        if (v.major == 0)
        {
            fprintf(stderr, "** Error: No version information in '%s'\n", argv[1]);
            fprintf(stderr, "(older than 2.0FC?)\n");
            error |= ERROR_NO_VERSION_INFO;
        }
    }
    
    //
    // Determine build type
    //      Determine ARM9 and ARM7
    //      Determine FINALROM, RELEASE, DEBUG
    // 
    // If FINALROM, RELEASE, DEBUG are mixed, display an error
    //
    {
        u32 dBuildType = 0xffffffff, buildType = 0, target = 0;
        int mismatch = 0;
        rewind(fp);
        
        while(findMagicNumber(fp, buildMagicCodeBE, buffer, 4))
        {
            u32 version = buffer[2] & 0x000000ff;
            char* buildName[] = {"FINALROM", "RELEASE", "DEBUG"};
            if(version == 1)
            {
                target = (buffer[3] >> 8) & 0x000000ff;
                buildType = buffer[3] & 0x000000ff;
                printf("BUILD_TYPE:ARM%d %s\n", (int)target, 
                    ((buildType != 0xff) ? buildName[buildType] : "Invalid"));

                // Detect whether a different build type is mixed in.
                if(dBuildType != 0xffffffff && dBuildType != buildType)
                {
                    mismatch = 1;
                }
                dBuildType = buildType;
            }
        }
        if(mismatch)
        {
            fprintf(stderr, "** Error: Build type mismatch\n");
            error |= ERROR_BUILDTYPE_MISMATCH;
        }
    }

    fclose(fp);

    return error;
}
