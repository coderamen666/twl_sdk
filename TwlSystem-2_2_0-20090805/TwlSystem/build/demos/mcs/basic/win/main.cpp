/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - mcs - basic - win
  File:     main.cpp

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "stdafx.h"
#include <nnsys/mcs.h>
#include <locale.h>

static const WORD MCS_CHANNEL0 = 0;
static const WORD MCS_CHANNEL1 = 1;

namespace
{

NNSMcsPFOpenStream      pfOpenStream;
NNSMcsPFOpenStreamEx    pfOpenStreamEx;

void
PrintWin32Error(DWORD errNum)
{
    _TCHAR* buffer;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errNum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        reinterpret_cast<LPTSTR>(&buffer),
        0,
        NULL);
    _ftprintf(stderr, _T("Win32 error [%d] %s\n"), errNum, buffer);
    LocalFree(buffer);
}

void
MainLoop()
{
    // Connection
    HANDLE hStream = pfOpenStream(MCS_CHANNEL0, 0);

    if (hStream == INVALID_HANDLE_VALUE)
    {
        // Change channel to MCS_CHANNEL1 and try connecting again
        NNSMcsStreamInfo streamInfo = { sizeof(streamInfo) };
        hStream = pfOpenStreamEx(MCS_CHANNEL1, 0, &streamInfo);

        if (hStream == INVALID_HANDLE_VALUE)
        {
            PrintWin32Error(GetLastError());
            return;
        }

        _tprintf(_T("device type %d\n"), streamInfo.deviceType);
    }

    LONG pos = 0;
    LONG val = 1;
    while (true)
    {
        pos += val;

        DWORD cbWritten;
        BOOL bSuccess = WriteFile(
            hStream,
            &pos,                   // Pointer to buffer where data is to be written
            sizeof(pos),            // Number of bytes of data to write
            &cbWritten,             // Number of bytes of data actually written
            NULL);
        if (! bSuccess)
        {
            PrintWin32Error(GetLastError());
            break;
        }

        Sleep(100);

        DWORD totalBytesAvail;
        bSuccess = PeekNamedPipe(
            hStream,
            NULL,
            0,
            NULL,
            &totalBytesAvail,   // Number of bytes available
            NULL);
        if (! bSuccess)
        {
            PrintWin32Error(GetLastError());
            break;
        }

        if (totalBytesAvail > 0)
        {
            LONG wkVal;
            DWORD readBytes;
            bSuccess = ReadFile(
                hStream,
                &wkVal,             // Pointer to buffer storing read data
                sizeof(wkVal),      // Number of bytes of data to read
                &readBytes,         // Number of bytes of data actually read
                NULL);
            if (! bSuccess)
            {
                PrintWin32Error(GetLastError());
                break;
            }

            if (readBytes == sizeof(wkVal))
            {
                val = wkVal;
            }
        }
    }

    // Shuts down stream.
    CloseHandle(hStream);
}

}   // namespace

int
_tmain(int argc, _TCHAR* argv[])
{
    _TCHAR modulePath[MAX_PATH];

    _tsetlocale(LC_ALL, _T(""));

    // Expand the module path
    const DWORD writtenChars = ExpandEnvironmentStrings(
        _T("%TWLSYSTEM_ROOT%\\tools\\mcsserver\\") NNS_MCS_MODULE_NAME,
        modulePath,
        MAX_PATH);

    if (writtenChars > MAX_PATH)
    {
        _ftprintf(stderr, _T("module name too long.\n"));
        return 1;
    }

    // Load the module
    const HMODULE hModule = LoadLibrary(modulePath);
    if (! hModule)
    {
        _ftprintf(stderr, NNS_MCS_MODULE_NAME _T(" not found.\n"));
        return 1;
    }

    pfOpenStream   = reinterpret_cast<NNSMcsPFOpenStream  >(GetProcAddress(hModule, NNS_MCS_API_OPENSTREAM  ));
    pfOpenStreamEx = reinterpret_cast<NNSMcsPFOpenStreamEx>(GetProcAddress(hModule, NNS_MCS_API_OPENSTREAMEX));

    if (pfOpenStream && pfOpenStreamEx)
    {
        MainLoop();
    }

    FreeLibrary(hModule);

	return 0;
}
