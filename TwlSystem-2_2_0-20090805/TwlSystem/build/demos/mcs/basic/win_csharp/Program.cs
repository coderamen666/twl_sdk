/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - mcs - basic - win
  File:     Program.cs

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
using System.IO;
using System.IO.Pipes;
using Microsoft.Win32.SafeHandles;

namespace basic
{
    class Program
    {
        const ushort MCS_CHANNEL0 = 0;
        const ushort MCS_CHANNEL1 = 1;

        static Mcs.NNS_McsOpenStream openStream;
        static Mcs.NNS_McsOpenStreamEx openStreamEx;

        static int Main(string[] args)
        {
            // Expand the module path
            string modulePath = Environment.ExpandEnvironmentVariables(
                @"%TWLSYSTEM_ROOT%\tools\mcsserver\" + Mcs.NNS_MCS_MODULE_NAME);

            // Load the module
            IntPtr hModule = Win32.LoadLibrary(modulePath);
            if (hModule == IntPtr.Zero)
            {
                Console.Error.WriteLine(Mcs.NNS_MCS_MODULE_NAME + " not found.");
                return 1;
            }

            IntPtr pfOpenStreamEx = Win32.GetProcAddress(hModule, Mcs.NNS_MCS_API_OPENSTREAMEX);
            IntPtr pfOpenStream = Win32.GetProcAddress(hModule, Mcs.NNS_MCS_API_OPENSTREAM);

            if (pfOpenStream != IntPtr.Zero && pfOpenStreamEx != IntPtr.Zero)
            {
                openStream = (Mcs.NNS_McsOpenStream)Marshal.GetDelegateForFunctionPointer(
                    pfOpenStream,
                    typeof(Mcs.NNS_McsOpenStream));
                openStreamEx = (Mcs.NNS_McsOpenStreamEx)Marshal.GetDelegateForFunctionPointer(
                    pfOpenStreamEx,
                    typeof(Mcs.NNS_McsOpenStreamEx));

                MainLoop();
            }

            Win32.FreeLibrary(hModule);

	        return 0;
        }

        static void MainLoop()
        {
            // Connection
            SafePipeHandle hStream = openStream(MCS_CHANNEL0, 0);

            if (hStream.IsInvalid)
            {
                // Change channel to MCS_CHANNEL1 and try connecting again
                Mcs.NNSMcsStreamInfo streamInfo = new Mcs.NNSMcsStreamInfo();
                streamInfo.structBytes = (uint)Marshal.SizeOf(streamInfo.GetType());
                hStream = openStreamEx(MCS_CHANNEL1, 0, out streamInfo);

                if (hStream.IsInvalid)
                {
                    PrintWin32Error();
                    return;
                }

                Console.WriteLine("device type {0}", (Mcs.NNS_MCS_DEVICE_TYPE)streamInfo.deviceType);
            }

            using (NamedPipeClientStream stream = new NamedPipeClientStream(
                PipeDirection.InOut,
                false,                  // isAsync,
                true,                   // isConnected,
                hStream))
            {
                try
                {
                    int pos = 0;
                    int val = 1;
                    while (true)
                    {
                        pos += val;

                        byte[] posBuf = BitConverter.GetBytes(pos); // Data to write
                        stream.Write(posBuf, 0, posBuf.Length);

                        System.Threading.Thread.Sleep(100);

                        uint bytesRead;
                        uint totalBytesAvail;
                        uint bytesLeftThisMessage;
                        bool bSuccess = Win32.PeekNamedPipe(
                            hStream,
                            null,
                            0,
                            out bytesRead,              // Number of bytes read (will not be used)
                            out totalBytesAvail,        // Total number of bytes that can be read
                            out bytesLeftThisMessage);  // Number of bytes remaining in the message (will not be used)
                        if (!bSuccess)
                        {
                            PrintWin32Error();
                            break;
                        }

                        if (totalBytesAvail > 0)
                        {
                            byte[] valBuf = new byte[Marshal.SizeOf(typeof(int))];
                            int readBytes = stream.Read(
                                valBuf,             // Buffer to store the data that is read
                                0,
                                valBuf.Length);

                            if (readBytes == posBuf.Length)
                            {
                                val = BitConverter.ToInt32(valBuf, 0);
                            }
                        }
                    }
                }
                catch (IOException ex)
                {
                    // Program execution comes here if we are disconnected by the MCS server.
                    Console.WriteLine(string.Format("Exception - {0}", ex.Message));
                }
            }
        }

        static void PrintWin32Error()
        {
            int errNum = Marshal.GetLastWin32Error();
            Exception exception = Marshal.GetExceptionForHR(Marshal.GetHRForLastWin32Error());
            Console.Error.WriteLine("Win32 error [{0}] {1}\n", errNum, exception.Message);
        }
    }
}
