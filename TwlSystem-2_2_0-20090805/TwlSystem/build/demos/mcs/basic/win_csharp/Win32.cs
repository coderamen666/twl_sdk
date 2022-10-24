/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - mcs - basic - win
  File:     Win32.cs

  Copyright 2004-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 701 $
 *---------------------------------------------------------------------------*/

using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace basic
{
    class Win32
    {
        // HMODULE WINAPI LoadLibrary(LPCWSTR lpLibFileName);
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr LoadLibrary(string libFileName);

        // bool WINAPI FreeLibrary(HMODULE hLibModule);
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool FreeLibrary(IntPtr hLibModule);

        // FARPROC WINAPI GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        // BOOL WINAPI PeekNamedPipe(HANDLE hNamedPipe, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesRead, LPDWORD lpTotalBytesAvail, LPDWORD lpBytesLeftThisMessage);
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool PeekNamedPipe(SafePipeHandle hNamedPipe, [Out] byte[] buffer, uint bufferSize, out uint bytesRead, out uint totalBytesAvail, out uint bytesLeftThisMessage);
    }
}
