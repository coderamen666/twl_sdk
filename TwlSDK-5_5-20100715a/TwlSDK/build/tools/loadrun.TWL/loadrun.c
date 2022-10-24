/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - loadrun.TWL
  File:     loadrun.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-07-15#$
  $Rev: 7370 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

#include "isd_api.h"

#define  SDK_BOOL_ALREADY_DEFINED_
#include <twl_win32.h>
#include <nitro/os/common/system.h>

extern const unsigned long SDK_DATE_OF_LATEST_FILE;

//---- Application name
#define EXEC_NAME          "loadrun.TWL"
#define DEBUGGER_NAME      "IS-TWL-DEBUGGER"


//---- Version string
#define VERSION_STRING     " 1.7  Copyright 2007-2008 Nintendo. All right reserved."
// 1.7  Enabled HYBRID to run in NITRO mode (added -N)
// 1.6  Enabled display of final error
// 1.5  Made reference to debugging prohibit flag
// 1.4  Updated for matching only of end with serial specification
// 1.3  Official support for ISTD_DownloadGo function
// 1.2  Fixed print callback bug
// 1.1  Supported IS-TWL-DEBUGGER Ver.0.41
// 1.0 Published

//---- exit number
#define EXIT_NUM_NO_ERROR               207     // Successful exit (however, this will not result next time)
#define EXIT_NUM_USER_SIGNAL            206     // Forced exit by user (ctrl-C)
#define EXIT_NUM_EXEC_TIME_OUT          205     // Forced exit due to execution timeout
#define EXIT_NUM_TIME_OUT               204     // Forced exit due to display timeout
#define EXIT_NUM_SHOW_DEVICES           203     // Exit on device list display
#define EXIT_NUM_SHOW_USAGE             202     // Exit on help display
#define EXIT_NUM_SHOW_VERSION           201     // Exit on version display
#define EXIT_NUM_STRING_ABORT           200     // Forced exit by character string

#define EXIT_NUM_NO_DEVICE              -1      // There are no available devices
#define EXIT_NUM_UNKNOWN_OPTION         -2      // An unknown option was specified
#define EXIT_NUM_ILLEGAL_OPTION         -3      // Illegal use of option
#define EXIT_NUM_NO_INPUT_FILE          -4      // The specified file does not exist or cannot be opened
#define EXIT_NUM_NOT_CONNECT            -5      // Failed to connect to device
#define EXIT_NUM_CANNOT_USE_CARTRIDGE   -6      // Failed to lock cartridge
#define EXIT_NUM_CANNOT_USE_CARD        -7      // Failed to lock card
#define EXIT_NUM_PRINTF_ERROR           -8      // Error while handling printf data
#define EXIT_NUM_LOADING_ERROR          -9      // Error during loading
#define EXIT_NUM_EXEC_PROHIBITION       -10     // Execution is not allowed
#define EXIT_NUM_NO_HYBRID_NITRO        -11     // Cannot execute HYBRID with NITRO

//---- Maximum console count
#define PRINT_CONSOLE_MAX   4

//---- ROM header information
#define RH_FLAG_OFFSET					0x1c	// Various information flags in the ROM header
#define RH_FLAG_DEBUGGER_PROHIBITION	(1<<3)	// Cannot run debugger with this bit raised

//---- For device specification
char   *gDeviceName[] = {
    "TWLEMU", "TWLDEB", NULL
};
int     gDeviceTypeArray[] = {
    ISTD_DEVICE_IS_TWL_EMULATOR,
    ISTD_DEVICE_IS_TWL_DEBUGGER,
};

//---- Operating mode
BOOL    gQuietMode = FALSE;            // Quiet mode
BOOL    gVerboseMode = FALSE;          // verbose mode
BOOL    gDebugMode = FALSE;            // Debug mode

BOOL    gStdinMode = FALSE;            // stdin mode

BOOL    gIsForceNitro = FALSE;         // Set hybrid to NITRO mode?

BOOL    gIsTypeSpecified = FALSE;      // Is there a device type specification?
int     gSpecifiedType;                // Device type if one is specified

BOOL    gIsSerialSpecified = FALSE;    // Is there a serial number specification?
int     gSpecifiedSerial;              // Serial number if one was specified

BOOL    gIsCartridgeLocked = FALSE;    // Should the cartridge slot be locked?
BOOL    gIsCardLocked = FALSE;         // Should the card slot be locked?

int     gTimeOutTime = 0;              // Timeout interval in sec (0 indicates no timeout)
BOOL    gTimeOutOccured = FALSE;       // Has a timeout occurred?

int     gExecTimeOutTime = 0;          // Execution timeout interval in sec (0 indicates no timeout)
int     gExecTimeOutOccured = FALSE;   // Has an execution timeout occurred?

char   *gAbortString = NULL;           // Abort string
BOOL    gStringAborted = FALSE;        // Has execution terminated due to the abort string?

BOOL    gExitAborted = FALSE;          // Exit from OS_Exit()
int     gExitStatusNum = EXIT_NUM_STRING_ABORT; // Return value when terminated by OS_Exit()
int     gExitStrLength;                // Length of exit string

BOOL    gShowArchitecture = FALSE;      // Show the architecture?
BOOL    gShowConsoleNum = FALSE;        // Show the console number?


//---- For TWL library
HINSTANCE gDllInstance;
ISDDEVICEHANDLE gDeviceHandle;
ISDDEVICEID gDeviceId;

//Connected to the device? (for slot switch)
BOOL    gDeviceConnected = FALSE;


//---- Device list
#define DEVICE_MAX_NUM      256
#define DEVICE_SERIAL_NONE  0x7fffffff // means no specified
ISTDDevice gDeviceList[DEVICE_MAX_NUM];
int     gCurrentDevice = -1;
int     gConnectedDeviceNum = 0;

int     gDeviceTypeSpecified = ISTD_DEVICE_NONE;
int     gDeviceSerialSpecified = DEVICE_SERIAL_NONE;    // means no specified
#define DEVICE_SERIAL_STRING_MAX_SIZE 256
char	gDeviceSerialString[DEVICE_SERIAL_STRING_MAX_SIZE];
int     gDeviceSerialStringLen = 0;

//---- Input file
#define FILE_NAME_MAX_SIZE  1024
FILE   *gInputFile;
char    gInputFileNameString[FILE_NAME_MAX_SIZE];
BOOL    gIsInputFileOpened = FALSE;

//---- Time
time_t  gStartTime = 0;                // Start time
BOOL    gIsLineHead[PRINT_CONSOLE_MAX] = {TRUE, TRUE, TRUE, TRUE}; // Start of line?
BOOL    gShowLapTime = FALSE;

//---- Signal
BOOL    gStoppedByUser = FALSE;        // Indicates whether execution was stopped by the user.

//---- Displayed?
volatile BOOL    gIsOutputString = FALSE;


#define printfIfNotQuiet(str)    do{if(!gQuietMode){fputs(str,stdout);}}while(0)

void    displayErrorAndExit(int exitNum, char *message);
BOOL    outputString(TWLArch arch, int console, char *buf, int bufSize);

static void printCallback( LPVOID user, ISDCPUARCH arch, DWORD console, const void* recvBuf, DWORD size );
void checkFileFlag(void);

/*---------------------------------------------------------------------------*
  Name:         myExit

  Description:  similar to exit()

  Arguments:    exitNum: exit() number

  Returns:      None.
 *---------------------------------------------------------------------------*/
void myExit(int exitNum)
{
    //---- Turns off the slot for the cartridge and card
    if (gDeviceConnected)
    {
//        (void)ISNTD_CartridgeSlotPower(gDeviceHandle, FALSE);
//        (void)ISNTD_CardSlotPower(gDeviceHandle, FALSE);
    }

    //---- Deallocate DLL memory
//    ISNTD_FreeDll();
    ISTD_FreeDll();

    if (!gQuietMode)
    {
        if (exitNum == EXIT_NUM_USER_SIGNAL)
        {
            printf("\n*** %s: stopped by user.\n", EXEC_NAME);
        }
        else if (exitNum == EXIT_NUM_TIME_OUT)
        {
            printf("\n*** %s: stopped by print timeout.\n", EXEC_NAME);
        }
        else if (exitNum == EXIT_NUM_EXEC_TIME_OUT)
        {
            printf("\n*** %s: stopped by exec timeout.\n", EXEC_NAME);
        }
        else if (gStringAborted)
        {
            printf("\n*** %s: stopped by aborting string.\n", EXEC_NAME);
        }
        else if (gExitAborted)
        {
            exitNum = gExitStatusNum;
        }
    }

    exit(exitNum);
}

/*---------------------------------------------------------------------------*
  Name:         listDevice

  Description:  Displays a list of devices and terminates

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void listDevice(void)
{
    int     n;

    //---- Device read
    gConnectedDeviceNum = ISTD_GetDeviceList(&gDeviceList[0], DEVICE_MAX_NUM);
    if (gConnectedDeviceNum < 0)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "Cannot access devices.");
    }

    printf("---- Connected devices:\n");

    for (n = 0; n < gConnectedDeviceNum; n++)
    {
        switch (gDeviceList[n].type)
        {
			case ISTD_DEVICE_IS_TWL_EMULATOR:
				printf("%3d: [IS-TWL-DEBUGGER] serial:%8d\n", n, gDeviceList[n].serial);
				break;
			case ISTD_DEVICE_IS_TWL_DEBUGGER:
				printf("%3d: [%s] serial:%8d\n", n, DEBUGGER_NAME, gDeviceList[n].serial);
				break;
			case ISTD_DEVICE_UNKNOWN:
				printf("%3d: unknown device %x:%x\n", n, (int)gDeviceList[n].id.eType, (int)gDeviceList[n].id.nSerial );
				break;
			default:
				printf("Illegal device\n");
				break;
        }
    }

    //---- Search result
    printf("Found %d device%s.\n", gConnectedDeviceNum, (gConnectedDeviceNum<=1)?"":"s" );

    myExit(EXIT_NUM_SHOW_DEVICES);
}

/*---------------------------------------------------------------------------*
  Name:         searchDevice

  Description:  search device

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void searchDevice(void)
{
	int		tmpMatch = -1;
	int		tmpMatchCount = 0;

    //---- If there is no device
    if (gConnectedDeviceNum <= 0)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "found no device.");
    }

	//---- First one if no device has been specified
    if (gDeviceTypeSpecified == ISTD_DEVICE_NONE && gDeviceSerialSpecified == DEVICE_SERIAL_NONE)
	{
        gCurrentDevice = 0;
	}
	//---- There is a device or serial specification
	else
	{
        int     n;
		char    tmpStr[DEVICE_SERIAL_STRING_MAX_SIZE];
		char    tmpStrLen;

        gCurrentDevice = -1;
        for (n = 0; n < gConnectedDeviceNum; n++)
        {
            //---- Matching with specified device
            if (gDeviceTypeSpecified != ISTD_DEVICE_NONE && gDeviceTypeSpecified != gDeviceList[n].type)
            {
                continue;
            }

            //---- Matching with specified serial
            if (gDeviceSerialSpecified != DEVICE_SERIAL_NONE )
			{
				//---- Equal
				if ( gDeviceSerialSpecified == gDeviceList[n].serial )
				{
					gCurrentDevice = n;
					break;
				}

				//---- Partial matching because not equal
				sprintf( tmpStr, "%d", gDeviceList[n].serial );
				tmpStrLen = strlen(tmpStr);

				if ( ( tmpStrLen > gDeviceSerialStringLen ) && 
					 ( ! strcmp( &tmpStr[ tmpStrLen - gDeviceSerialStringLen ], gDeviceSerialString ) ) )
				{
					tmpMatch = n;
					tmpMatchCount ++;
				}
            }
        }
    }

	//---- If there is one partial match
	if ( gCurrentDevice < 0 )
	{
		if ( tmpMatchCount == 1 )
		{
			gCurrentDevice = tmpMatch;
		}
		else if ( tmpMatchCount >= 2 )
		{
			displayErrorAndExit(EXIT_NUM_NO_DEVICE, "two or more devices matched to sperified serial number." );
		}
	}

    //---- The specified device does not exist or is incorrect
    if (gCurrentDevice < 0
        || gDeviceList[gCurrentDevice].type == ISTD_DEVICE_NONE
        || gDeviceList[gCurrentDevice].type == ISTD_DEVICE_UNKNOWN)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "illegal device.");
    }
}

/*---------------------------------------------------------------------------*
  Name:         displayUsage

  Description:  Displays the usage

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayUsage(void)
{
    fprintf(stderr,
            "TWL-SDK Development Tool - %s - Execute TWL ROM image\n"
            "Build %lu\n\n"
            "Usage: %s [OPTION] <SrlFile>\n"
            "\tdownload TWL srl file to %s and execute.\n\n"
            "Options:\n"
            "  --version                   : Show %s version.\n"
            "  --debuggerVersion           : Show debugger version.\n"
            "  -h, --help                  : Show this help.\n"
            "  -q, --quiet                 : Quiet mode.\n"
            "  -v, --verbose               : Verbose mode.\n"
            "  -L, --list                  : List connecting device.\n"
            "  -N, --nitro                 : Run hybrid rom as NITRO mode\n"
            "  -l, --lap                   : Show lap time at each line.\n"
//            "  -d, --type=DEVICE          : Specify device type.\n"
//            "                                DEVICE=TWLEMU|TWLDEB.\n"
            "  -A, --architecture          : Show architecture at each line.\n"
            "  -n, --console               : Show console number at each line.\n"
            "  -s, --serial=SERIAL         : Specify serial number.\n"
            "  -t, --timeout=SECOND        : Specify quit time after last print.\n"
            "  -T, --exec-timeout=SECOND   : Specify quit time after execute program.\n"
            "  -a, --abort-string=STRING   : Specify aborting string.\n"
            "  -c, --card-slot=SWITCH      : Card      slot SWITCH=ON|OFF, default OFF.\n"
            "  -C, --cartridge-slot=SWITCH : Cartridge slot SWITCH=ON|OFF, default OFF.\n",
//            "  --stdin, --standard-input   : Read data from stdin instead of <SrlFile>.\n\n",
			EXEC_NAME,
            SDK_DATE_OF_LATEST_FILE,
			EXEC_NAME,
			DEBUGGER_NAME,
			EXEC_NAME );
}

/*---------------------------------------------------------------------------*
  Name:         displayVersion

  Description:  Displays the version

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayVersion(void)
{
    printf("*** %s: %s\n", EXEC_NAME, VERSION_STRING);
}

/*---------------------------------------------------------------------------*
  Name:         displayDebuggerVersion

  Description:  Displays debugger version.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayDebuggerVersion(void)
{
	BOOL r;
	DWORD major;
	DWORD minor;
	DWORD build;
	DWORD revision;
	r = ISTD_GetVersion( &major, &minor, &build, &revision);

	if (r)
	{
		printf("*** %s: %s version is %d.%d.%04d.%04d\n",
			   EXEC_NAME, DEBUGGER_NAME, (int)major, (int)minor, (int)build, (int)revision);
	}
	else
	{
		printf("*** %s: failed to get %s version.\n", EXEC_NAME, DEBUGGER_NAME );
	}
	myExit(EXIT_NUM_NO_ERROR);
}

/*---------------------------------------------------------------------------*
  Name:         displayErrorAndExit

  Description:  Displays errors.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayErrorAndExit(int exitNum, char *message)
{
	if (gDebugMode)
	{
		printf("last error=%lx\n", ISTD_GetLastError());
	}

    printf("*** %s: Error: %s\n", EXEC_NAME, message);

    //---- Deallocate DLL memory
    ISTD_FreeDll();

    exit(exitNum);
}

/*---------------------------------------------------------------------------*
  Name:         parseOption

  Description:  parses the option line

  Arguments:    argc : argument count
                argv: argument vector

  Returns:      result. less than 0 if error.
 *---------------------------------------------------------------------------*/
void parseOption(int argc, char *argv[])
{
    int     n;
    int     c;
    BOOL    helpFlag = FALSE;

    struct option optionInfo[] = {
        {"help", no_argument, NULL, 'h'},
        {"quiet", no_argument, NULL, 'q'},
        {"verbose", no_argument, NULL, 'v'},
        {"list", no_argument, NULL, 'L'},
        {"nitro", no_argument, NULL, 'N'},
        {"lap", no_argument, NULL, 'l'},
        {"debug", no_argument, NULL, 'D'},      //Hidden options
        {"version", no_argument, NULL, '1'},
        {"debuggerVersion", no_argument, NULL, '2'},
//        {"stdin", no_argument, NULL, 'I'},
//        {"standard-input", no_argument, NULL, 'I'},
        {"type", required_argument, 0, 'd'},    //Hidden options
        {"serial", required_argument, 0, 's'},
        {"timeout", required_argument, 0, 't'},
        {"exec-timeout", required_argument, 0, 'T'},
        {"abort-string", required_argument, 0, 'a'},
        {"card-slot", required_argument, 0, 'c'},
        {"cartridge-slot", required_argument, 0, 'C'},
        {"architecture", no_argument, NULL, 'A'},
        {"console", no_argument, NULL, 'n'},
        {NULL, 0, 0, 0}
    };
    int     optionIndex;

    char   *optionStr = NULL;

    //---- suppress error string of getopt_long()
    opterr = 0;

    while (1)
    {
        c = getopt_long(argc, argv, "+hqvLNlDd:s:t:T:a:c:C:An", &optionInfo[0], &optionIndex);

        //printf("optind=%d optopt=%d  %x(%c) \n", optind, optopt, c,c );

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
        case 'I':                     //---- Standard input
            gStdinMode = TRUE;
            break;
        case 'h':                     //---- Help display
            helpFlag = TRUE;
            break;
        case 'q':                     //---- Quiet mode
            gQuietMode = TRUE;
            break;
        case 'v':                     //---- Verbose mode
            gVerboseMode = TRUE;
            break;
        case 'D':                     //---- Debug mode
            gDebugMode = TRUE;
            break;
        case '1':                     //---- Program version display
            displayVersion();
            myExit(EXIT_NUM_SHOW_VERSION);
            break;
        case '2':                     //---- Debugger version display
			displayDebuggerVersion();
            myExit(EXIT_NUM_SHOW_VERSION);
			break;
        case 'L':                     //---- Device list
            listDevice();
            break;
        case 'N':                     //---- Set hybrid to NITRO mode
            gIsForceNitro = TRUE;
            break;
        case 'l':                     //---- Lap time
            gShowLapTime = TRUE;
            break;
		case 'A':
			gShowArchitecture = TRUE; // Display architecture
			break;
		case 'n':
			gShowConsoleNum = TRUE;   // Display console number
			break;
        case 'd':                     //---- Device
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            {
                int     n;
                for (n = 0; gDeviceName[n]; n++)
                {
                    if (!strcmp(optionStr, gDeviceName[n]))
                    {
                        gDeviceTypeSpecified = gDeviceTypeArray[n];
                        break;
                    }
                }

                if (gDeviceTypeSpecified == ISTD_DEVICE_NONE)
                {
                    displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION, "illegal device type.");
                }
            }
            break;
        case 's':                     //---- Serial specification
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            gDeviceSerialSpecified = atoi(optionStr);
			strncpy(gDeviceSerialString, optionStr, DEVICE_SERIAL_STRING_MAX_SIZE);
			gDeviceSerialString[DEVICE_SERIAL_STRING_MAX_SIZE-1] = '\0';
			gDeviceSerialStringLen = strlen( gDeviceSerialString );
            break;
        case 'c':                     //---- Card slot lock
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            if (!strcmp(optionStr, "ON") || !strcmp(optionStr, "on"))
            {
                gIsCardLocked = TRUE;
            }
            else if (!strcmp(optionStr, "OFF") || !strcmp(optionStr, "off"))
            {
                gIsCardLocked = FALSE;
            }
            else
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION, "illegal value for card slot option.");
            }
            break;
        case 'C':                     //---- Cartridge slot lock
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            if (!strcmp(optionStr, "ON") || !strcmp(optionStr, "on"))
            {
                gIsCartridgeLocked = TRUE;
            }
            else if (!strcmp(optionStr, "OFF") || !strcmp(optionStr, "off"))
            {
                gIsCartridgeLocked = FALSE;
            }
            else
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                    "illegal value for cartridge slot option.");
            }
            break;
        case 't':                     //---- Timeout interval beginning from final display
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            gTimeOutTime = atoi(optionStr);
            if (gTimeOutTime <= 0)
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                    "illegal value for abort timeout option.");
            }
            break;
        case 'T':                     //---- Execution timeout interval
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            gExecTimeOutTime = atoi(optionStr);
            if (gExecTimeOutTime <= 0)
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                    "illegal value for abort exec timeout option.");
            }
            break;
        case 'a':                     //---- Abort string
            gAbortString = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            {
                int     length = strlen(gAbortString);
                if (length <= 0 || length > 256)
                {
                    displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                        "illegal value for abort string option.");
                }
            }
            break;
        default:
            displayErrorAndExit(EXIT_NUM_UNKNOWN_OPTION, "unknown option.");
        }
    }

    //---- Help display
    {
        BOOL    isDisplayHelp = FALSE;

        if (helpFlag)
        {
            isDisplayHelp = TRUE;
        }
        else if (argc <= optind && !gStdinMode)
        {
            isDisplayHelp = TRUE;
        }
        else if (argc > optind && gStdinMode)
        {
            isDisplayHelp = TRUE;
        }

        if (isDisplayHelp)
        {
            displayUsage();
            exit(EXIT_NUM_SHOW_USAGE);
        }
    }

    //---- Input file name
    if (!gStdinMode)
    {
        strncpy(gInputFileNameString, argv[optind], FILE_NAME_MAX_SIZE);
    }

    if (gVerboseMode)
    {
        if (!gStdinMode)
        {
            printf("Input file is [%s]\n", gInputFileNameString);
        }
        else
        {
            printf("Input file is stdin\n");
        }
    }

    //---- Information display
    if (gVerboseMode)
    {
        printf("Print time out : %d sec.\n", gTimeOutTime);
        printf("Execute time out : %d sec.\n", gExecTimeOutTime);
        printf("Card lock : %s.\n", (gIsCardLocked) ? "ON" : "OFF");
        printf("Cartridge lock : %s.\n", (gIsCartridgeLocked) ? "ON" : "OFF");

        if (gAbortString)
        {
            printf("Abort string : [%s]\n", gAbortString);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         loadFile

  Description:  loads file

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void loadFile(void)
{
    unsigned int address = 0;

    //---- File open
    if (gStdinMode)
    {
        gInputFile = stdin;
        _setmode(_fileno(gInputFile), O_BINARY);
    }
    else
    {
        if ((gInputFile = fopen(gInputFileNameString, "rb")) == NULL)
        {
            displayErrorAndExit(EXIT_NUM_NO_INPUT_FILE, "cannot open input file.");
        }
    }
    gIsInputFileOpened = TRUE;

	//---- Check file
	checkFileFlag();

    //---- Connected to device
    if ((gDeviceHandle = ISTD_OpenEx(gDeviceList[gCurrentDevice].id)) == NULL)
    {
        displayErrorAndExit(EXIT_NUM_NOT_CONNECT, "cannot connect device.");
    }
    gDeviceConnected = TRUE;

    //---- Issue reset
    ISTD_Reset(gDeviceHandle, TRUE);
    Sleep(1000);

	//----- Specify print callback
	ISTD_PrintfSetCB( gDeviceHandle, printCallback, (void*)0 );

	//---- Set mode for hybrid
	if ( ISTD_SetHybridMode )
	{
		ISTD_SetHybridMode( gDeviceHandle, gIsForceNitro? ISDHYBRID_NTR: ISDHYBRID_TWL );
	}
	else
	{
		if ( gIsForceNitro )
		{
			displayErrorAndExit(EXIT_NUM_LOADING_ERROR, "cannot run as NITRO mode. The required version of IS-TWL-DEBUGGER is v0.64 or later.");
		}
	}

	//---- Download and startup ROM data
	if ( ! ISTD_DownloadGo( gDeviceHandle, gInputFileNameString ) )
	{
		displayErrorAndExit(EXIT_NUM_LOADING_ERROR, "troubled while loading input file.");
	}

#if 0
    //---- Transfer every 16KB
    while (1)
    {
        char    buf[16384];
        size_t  size = fread(buf, 1, sizeof(buf), gInputFile);
        static int progressCount = 0;

        if (!size)
        {
            break;
        }

        //---- Transfer
        if (!ISTD_WriteROM(gDeviceHandle, buf, address, size))
        {
            displayErrorAndExit(EXIT_NUM_LOADING_ERROR, "troubled while loading input file.");
        }

        address += size;

        if (gVerboseMode)
        {
            if (!(progressCount++ % 32))
            {
                printf("*");
            }
        }
    }
#endif

    //---- File close
    if (gStdinMode)
    {
        _setmode(_fileno(gInputFile), O_TEXT);
    }
    else
    {
        fclose(gInputFile);
    }
    gIsInputFileOpened = FALSE;

    if (gVerboseMode)
    {
        printf("\nInput file size: %d (0x%x) byte\n", address, address);
    }
}

/*---------------------------------------------------------------------------*
  Name:         checkFileFlag

  Description:  Checks ROM header execution flag.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void checkFileFlag(void)
{
	char buf[0x100];

	fseek( gInputFile, 0, SEEK_SET );
	fread( buf, 1, sizeof(buf), gInputFile );

	//---- Whether debugger can be run
	if ( buf[RH_FLAG_OFFSET] & RH_FLAG_DEBUGGER_PROHIBITION )
	{
		displayErrorAndExit(EXIT_NUM_EXEC_PROHIBITION, "cannot exec this file. (debugger forbidden)");
	}

	//---- rewind
	rewind( gInputFile );
}

/*---------------------------------------------------------------------------*
  Name:         setSlopPower

  Description:  Slot power process

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void setSlotPower(void)
{
    //---- Should the cartridge slot be locked?
    if (gIsCartridgeLocked)
    {
//        if (!ISNTD_CartridgeSlotPower(gDeviceHandle, TRUE))
        if (0)
        {
            displayErrorAndExit(EXIT_NUM_CANNOT_USE_CARTRIDGE, "cannot use cartridge slot.");
        }
    }

    //---- Should the card slot be locked?
    if (gIsCardLocked)
    {
//        if (!ISNTD_CardSlotPower(gDeviceHandle, TRUE))
        if (0)
        {
            displayErrorAndExit(EXIT_NUM_CANNOT_USE_CARD, "cannot use card slot.");
        }
    }

#if 0
    //---- Cancel reset
    Sleep(1000);
    ISTD_Reset(gDeviceHandle, FALSE);
#endif
}


/*---------------------------------------------------------------------------*
  Name:         procPrint

  Description:  printf process

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
TWLArch archNum[] = {
    TWLArchARM9,
    TWLArchARM7
};

#define PRINT_ONETIME_SIZE  512
#define PRINT_CONSOLE_MAX   4

//---- Buffer for joint display
static char gConbineBuf[PRINT_CONSOLE_MAX][PRINT_ONETIME_SIZE * 2 + 2] = {"\0", "\0", "\0", "\0" };

static char *gConbineBufPtr[PRINT_CONSOLE_MAX] = {&gConbineBuf[0][0], 
												  &gConbineBuf[1][0], 
												  &gConbineBuf[2][0], 
												  &gConbineBuf[3][0] };

//---- Buffer for joining and comparing character strings
static char gLineBuf[PRINT_ONETIME_SIZE + 1];


void procPrintf(void)
{
    int     blankTime = 0;

    //---- Size of exit string
    gExitStrLength = strlen(OS_EXIT_STRING_1);

    while (1)
    {
        //---- When stopped by the user
        if (gStoppedByUser)
        {
            myExit(EXIT_NUM_USER_SIGNAL);
        }

        //---- Exit?
        if (gStringAborted || gExitAborted)
        {
            break;
        }

        //---- If text is not being displayed
        if (!gIsOutputString)
        {
            Sleep(100);
            blankTime += 100;

            //---- Timeout decision
            if (gTimeOutTime && blankTime > gTimeOutTime * 1000)
            {
                gTimeOutOccured = TRUE;
                break;
            }
        }
        //---- If text has been displayed
        else
        {
			gIsOutputString = FALSE;
            blankTime = 0;
        }

        //---- Timeout check
        if (gExecTimeOutTime > 0)
        {
            time_t  currentTime;
            (void)time(&currentTime);

            if (currentTime - gStartTime >= gExecTimeOutTime)
            {
                gExecTimeOutOccured = TRUE;
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         printCallback

  Description:  Print callback

  Arguments:    user    :
                arch    :
                console :
                recvBuf :
                size:

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void printCallback( LPVOID user, ISDCPUARCH arch, DWORD console, const void* recvBuf, DWORD size )
{
//printf("%d arch:%d console:%d buf:%x size:%d\n", (int)user, (int)arch, (int)console, (int)recvBuf, (int)size );
	gIsOutputString = TRUE;
	outputString(arch, console, (char*)recvBuf, size);
}

/*---------------------------------------------------------------------------*
  Name:         showLapTime

  Description:  displays lap time at line head

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void showLapTime(void)
{
    int     lap;
    time_t  currentTime;

    (void)time(&currentTime);
    lap = currentTime - gStartTime;

    printf("{%d:%02d}", lap / 60, lap % 60);
}

void showAddition(TWLArch arch, int console)
{
	static char* archStr[] = {"A9", "A7"};
	int archIndex = -1;

	if ( gIsLineHead[console] )
	{
		if ( gShowArchitecture )
		{
			//---- Investigate architecture
			if ( arch == TWLArchARM9 )
			{
				archIndex=0;
			}
			else if ( arch == TWLArchARM7 )
			{
				archIndex=1;
			}
		}
		if ( !gShowConsoleNum )
		{
			console = -1;
		}

		//---- Display architecture and console number
		if ( archIndex<0 && console<0 )
		{
			// do nothing
		}
		else if ( archIndex<0 )
		{
			printf("<%d>", console );
		}
		else if ( console<0 )
		{
			printf("<%s>", archStr[archIndex] );
		}
		else
		{
			printf("<%s:%d>", archStr[archIndex], console );
		}

		//---- Time display
		if (!gQuietMode && gShowLapTime)
		{
			showLapTime();
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         outputString

  Description:  outputs string to stdout

  Arguments:    arch    : architecture(TWLArchARM9, TWLArchARM7, None)
                console : console num (0-3, -1)
                buf: buffer
                bufSize: data size in buffer

  Returns:      FALSE if to do quit.
 *---------------------------------------------------------------------------*/
BOOL outputString(TWLArch arch, int console, char *buf, int bufSize)
{
    char   *bufEnd = buf + bufSize;
    char   *p = buf;

    int     abortStrLength = gAbortString ? strlen(gAbortString) : 0;

	static int prevConsole = -1;

	if ( prevConsole >= 0 && prevConsole != console )
	{
        gIsLineHead[prevConsole] = TRUE;
		printf("\n");
	}
	prevConsole = console;

    while (p < bufEnd)
    {
        char   *crPtr = strchr(p, '\n');

        //---- \n missing
        if (!crPtr)
        {
            //---- Save for comparison
            strcat(gConbineBufPtr[console], p);
            gConbineBufPtr[console] += strlen(p);

			//---- Additional information
			showAddition( arch, console );
            gIsLineHead[console] = FALSE;

            //---- Display
            printfIfNotQuiet(p);

            //---- Destroy if buffer overflows (the responsibility is on the output side to insert \n)
            if (gConbineBufPtr[console] - &gConbineBuf[console][0] > PRINT_ONETIME_SIZE)
            {
                gConbineBufPtr[console] = &gConbineBuf[console][0];
                *gConbineBufPtr[console] = '\0';
            }

            break;
        }

        //---- copy up to \n
        {
            int     n = crPtr - p + 1;

            //---- Combine for comparison
            strncpy(gConbineBufPtr[console], p, n);
            gConbineBufPtr[console][n] = '\0';

            //---- For display
            strncpy(&gLineBuf[0], p, n);
            gLineBuf[n] = '\0';
        }

		//---- Additional information
		showAddition( arch, console );
        gIsLineHead[console] = TRUE;

        //---- Line display
        printfIfNotQuiet(gLineBuf);

        //---- Compare with abort string
        if (gAbortString && !strncmp(gConbineBuf[console], gAbortString, abortStrLength))
        {
            gStringAborted = TRUE;
            return FALSE;
        }

        //---- Exit using OS_Exit
        if (!strncmp(gConbineBuf[console], OS_EXIT_STRING_1, gExitStrLength))
        {
            gExitAborted = TRUE;
            gExitStatusNum = atoi(gConbineBuf[console] + gExitStrLength);
            return FALSE;
        }

        gConbineBufPtr[console] = &gConbineBuf[console][0];
        *gConbineBufPtr[console] = '\0';

        p = crPtr + 1;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         signalHandler

  Description:  signal handler

  Arguments:    sig
                argv: argument vector

  Returns:      ---
 *---------------------------------------------------------------------------*/
void signalHandler(int sig)
{
    gStoppedByUser = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         Main

  Description:  main proc

  Arguments:    argc : argument count
                argv: argument vector

  Returns:      ---
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    //---- Initialize DLL
    ISTD_InitDll();

    //---- Disable buffering of standard output
    setvbuf(stdout, NULL, _IONBF, 0);

    //---- Option parsing
    parseOption(argc, argv);

    //---- Device read
    gConnectedDeviceNum = ISTD_GetDeviceList(&gDeviceList[0], DEVICE_MAX_NUM);
    if (gConnectedDeviceNum < 0)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "Cannot access devices.");
    }

    //---- Search for device
    searchDevice();

    //---- Read
    loadFile();

    //---- Signal setting
    (void)signal(SIGINT, signalHandler);

    //---- Slot
    setSlotPower();

    //---- Get start time
    (void)time(&gStartTime);

    //---- printf process
    procPrintf();

    //---- End
    if (gExitAborted)                  //---- Exit from OS_Exit()
    {
        myExit(gExitStatusNum);
    }
    else if (gStringAborted)           //---- Abort string
    {
        myExit(EXIT_NUM_STRING_ABORT);
    }
    else if (gTimeOutOccured)          //---- Timeout
    {
        myExit(EXIT_NUM_TIME_OUT);
    }
    else if (gExecTimeOutOccured)
    {
        myExit(EXIT_NUM_EXEC_TIME_OUT);
    }
    else                               //---- Normal end
    {
        myExit(EXIT_NUM_NO_ERROR);
    }
    //---- never reached here

    //---- dummy to avoid warning
    return 0;
}
