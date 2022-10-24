/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - demo - characterRecognition-1
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/prc/algo_fine.h>

#include "patterns.h"

//----- Specify how many places to output recognition result
#define RESULT_NUM              5

//----- The number of queue levels for recognition process wait
#define RECOGNITION_QUEUE_SIZE  2

//----- Maximum number of points per input
//-----  * As a standard, 60 points are entered each second
#define POINT_PER_INPUT_MAX     1024
//----- Maximum stroke count per input
#define STROKE_PER_INPUT_MAX    20
//----- Maximum number of points after resampling
#define POINT_AFTER_RESAMPLING_MAX 40

#define VRAM_BASE_ADDR          ((u16*)HW_LCDC_VRAM_C)
#define VRAM_LINE_SIZE          256
#define DRAW_COLOR              0

#define MEMORY_MAIN_MAX_HEAPS   1
#define MEMORY_MAIN_HEAP_SIZE   0x200000

static void InitOS(void);
static void InitTP(void);
static void InitDisplay(void);
static void DrawLine(int x1, int y1, int x2, int y2, u16 col);
static void PutDot(int x, int y, u16 col);
static void ClearScreen(void);
void    DrawStrokes(const PRCStrokes *strokes, int sx, int sy);
static void PrintRecognitionResult(PRCPrototypeEntry **result, fx32 *scores);
static void VBlankIntr(void);


typedef struct RecognitionObject
{
    BOOL    recognized;                // TRUE if variables such as results have valid values

    PRCStrokes strokes;
    PRCPoint points[POINT_PER_INPUT_MAX];

    PRCPrototypeEntry *results[RESULT_NUM];
    fx32    scores[RESULT_NUM];

    PRCStrokes inputPatternStrokes;
    PRCPoint inputPatternPoints[POINT_AFTER_RESAMPLING_MAX];
}
RecognitionObject;

void    InitRecognition(void);
void    DestroyRecognition(void);
BOOL    RecognizePatternAsync(RecognitionObject * packet);


void NitroMain(void)
{
    InitOS();
    InitDisplay();
    InitTP();

    {
        PRCPoint points[POINT_PER_INPUT_MAX];
        PRCStrokes strokes;
        RecognitionObject queue[RECOGNITION_QUEUE_SIZE];        // May be consuming a large amount of memory. Be aware of that.
        int     queueHead, queueTail, queueSize;
        u16     prevKey, currKey, trgKey;
        BOOL    touching = FALSE;
        TPData  prevSample = { 0, 0, 0, 0 };

        InitRecognition();
        {
            // Embed the entry number in entry.data of the sample database to facilitate subsequent reverse lookup
            int     iEntry;
            for (iEntry = 0; iEntry < PrototypeList.entrySize; iEntry++)
            {
                ((PRCPrototypeEntry *)&PrototypeList.entries[iEntry])->data = (void *)iEntry;
            }
        }

        PRC_InitStrokes(&strokes, points, POINT_PER_INPUT_MAX);

        {
            int     i;
            for (i = 0; i < RECOGNITION_QUEUE_SIZE; i++)
            {
                PRC_InitStrokes(&queue[i].strokes, queue[i].points, POINT_PER_INPUT_MAX);
                PRC_InitStrokes(&queue[i].inputPatternStrokes, queue[i].inputPatternPoints,
                                POINT_AFTER_RESAMPLING_MAX);
            }
        }
        queueHead = 0;
        queueTail = 0;
        queueSize = 0;

        ClearScreen();

        //---- Dummy READ workaround for pushing 'A' at IPL
        currKey = PAD_Read();

        while (TRUE)
        {
            BOOL    fClearScreen = FALSE;
            BOOL    fRecognizeChar = FALSE;
            TPData  sample;

            prevKey = currKey;
            currKey = PAD_Read();
            trgKey = (u16)(currKey & ~prevKey);

            (void)TP_RequestCalibratedSampling(&sample);

            if (sample.touch && (sample.validity == 0))
            {
                // Pen is touching the screen

                if (!PRC_IsFull(&strokes))
                {
                    if (touching)
                    {
                        // Writing
                        DrawLine(prevSample.x, prevSample.y, sample.x, sample.y, DRAW_COLOR);
                    }
                    else
                    {
                        // The moment pen touches the screen
                        PutDot(sample.x, sample.y, DRAW_COLOR);
                        touching = TRUE;
                    }
                    // Store stroke data
                    PRC_AppendPoint(&strokes, sample.x, sample.y);
                }
            }
            else
            {
                // Pen is off the screen

                if (touching)
                {
                    // The moment the pen comes off the screen
                    if (!PRC_IsFull(&strokes))
                    {
                        PRC_AppendPenUpMarker(&strokes);
                    }
                    touching = FALSE;
                }
            }

            // Recognition begins when A button is pressed
            if (trgKey & PAD_BUTTON_A)
            {
                fRecognizeChar = TRUE;
            }

            // Clear screen when B button is pressed
            if (trgKey & (PAD_BUTTON_START | PAD_BUTTON_B))
            {
                fClearScreen = TRUE;
            }

            // End when SELECT button is pressed
            if (trgKey & PAD_BUTTON_SELECT)
            {
                break;
            }

            if (fRecognizeChar)
            {
                if (PRC_IsEmpty(&strokes))
                {
                    OS_Printf("\n-----------------------------------------------\n");
                    OS_Printf("No input.\n");
                }
                else
                {
                    // Want to start graphics recognition
                    if (queueSize < RECOGNITION_QUEUE_SIZE)
                    {
                        // Create data for request
                        (void)PRC_CopyStrokes(&strokes, &queue[queueTail].strokes);
                        queue[queueTail].recognized = FALSE;

                        // Send request for graphics recognition
                        if (RecognizePatternAsync(&queue[queueTail]))
                        {
                            // Put on queue
                            queueTail++;
                            queueSize++;
                            if (queueTail >= RECOGNITION_QUEUE_SIZE)
                            {
                                queueTail = 0;
                            }

                            // Clear screen and input stroke data
                            ClearScreen();
                            PRC_Clear(&strokes);
                        }
                        else
                        {
                        }              // If message queue for other thread is full, do nothing
                        // This should not happen if the sizes of the two queues have been matched
                    }
                    else
                    {
                    }                  // If queue controlled by main is full, do nothing
                }
            }


            // Wait for V-Blank interrupt
            // While waiting with OS_WaitIrq, process moves to different thread.
            OS_WaitIrq(1, OS_IE_V_BLANK);
            // Return to main thread with V-Blank interrupt


            // Is there currently data waiting for recognition?
            if (queueSize > 0)
            {
                // Get and display any recognition results produced
                if (queue[queueHead].recognized == TRUE)
                {
                    // Recognition results were produced
                    PRCPrototypeEntry **results = queue[queueHead].results;
                    fx32   *scores = queue[queueHead].scores;

                    ClearScreen();
                    // Display again the stroke data currently being input
                    DrawStrokes(&strokes, 0, 0);
                    // In the upper-left portion of the screen, display the interpretation of the input
                    DrawStrokes(&queue[queueHead].inputPatternStrokes, 0, 0);

                    if (results[0] != NULL)
                    {
                        // If something could be recognized, it is pulled from the sample database and displayed on the upper-right corner of the screen
                        PRCStrokes drawingStrokes;
                        int     dictIndex = (int)results[0]->data;      // Entry number was stored in data in advance
                        PRC_GetEntryStrokes(&drawingStrokes, &PrototypeList,
                                            &PrototypeList.entries[dictIndex]);
                        DrawStrokes(&drawingStrokes, 256 - PrototypeList.normalizeSize, 0);
                    }

                    // Display recognition results in debugging output
                    PrintRecognitionResult(results, scores);

                    // Get from queue
                    queueHead++;
                    queueSize--;
                    if (queueHead >= RECOGNITION_QUEUE_SIZE)
                    {
                        queueHead = 0;
                    }
                }
            }

            if (fClearScreen)
            {
                ClearScreen();
                PRC_Clear(&strokes);
            }

            prevSample = sample;
        }
        DestroyRecognition();
    }
    OS_Printf("Terminated.\n");
    OS_Terminate();
}

void InitOS(void)
{
    void   *nstart;
    void   *heapStart;
    OSHeapHandle handle;

    OS_Init();

    nstart =
        OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(),
                     MEMORY_MAIN_MAX_HEAPS);
    OS_SetMainArenaLo(nstart);

    heapStart = OS_AllocFromMainArenaLo(MEMORY_MAIN_HEAP_SIZE, 32);
    handle =
        OS_CreateHeap(OS_ARENA_MAIN, heapStart, (void *)((u32)heapStart + MEMORY_MAIN_HEAP_SIZE));

    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, handle);

    OS_InitTick();
    OS_InitAlarm();
    OS_InitThread();
}

void InitDisplay(void)
{

    GX_Init();
    FX_Init();

    GX_DispOff();
    GXS_DispOff();

    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request
    (void)OS_EnableIrq();

    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);   // Clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);     // Clear the standard palette

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);     // Clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);       // Clear the standard palette
    MI_DmaFill32(3, (void *)HW_LCDC_VRAM_C, 0x7FFF7FFF, 256 * 192 * sizeof(u16));

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode
                       (GXBGMode)0,    // Dummy
                       (GXBG0As)0);    // Dummy

    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();
}

void InitTP(void)
{
    TPCalibrateParam calibrate;

    TP_Init();

    // Get CalibrateParameter from FlashMemory
    if (!TP_GetUserInfo(&calibrate))
    {
        OS_Panic("Can't get Calibration Parameter from NVRAM\n");
    }
    else
    {
        OS_Printf("Get Calibration Parameter from NVRAM\n");
    }

    TP_SetCalibrateParam(&calibrate);
}

void DrawLine(int x1, int y1, int x2, int y2, u16 col)
{
    u16     sx, sy, ey, wx, wy;
    u16     i;
    int     err, plus, minus;
    u16    *point = VRAM_BASE_ADDR;

    SDK_ASSERT(x1 >= 0 && y1 >= 0 && x2 >= 0 && y2 >= 0);

    // Swap (x1, y1) <-> (x2, y2) if x1 > x2
    if (x1 <= x2)
    {
        sx = (u16)x1;
        sy = (u16)y1;
        ey = (u16)y2;
        wx = (u16)(x2 - x1);           // Width
    }
    else
    {
        sx = (u16)x2;
        sy = (u16)y2;
        ey = (u16)y1;
        wx = (u16)(x1 - x2);           // Width
    }

    point += sy * VRAM_LINE_SIZE + sx;
    if (sy <= ey)
    {
        /* Line goes to upper */
        wy = (u16)(ey - sy);           // Height
        if (wx > wy)
        {
            /* If (X size > Y size) draw a Dot per Xdot */
            plus = wy * 2;
            minus = -wx * 2;
            err = -wx;
            for (i = 0; i <= wx; i++)
            {
                *point = col;          // PutDot
                ++point;
                err += plus;
                if (err >= 0)
                {
                    point += VRAM_LINE_SIZE;
                    err += minus;
                }
            }

        }
        else
        {
            /* If (X size <= Y size) draw a Dot per Ydot */
            plus = wx * 2;
            minus = -wy * 2;
            err = -wy;
            for (i = 0; i <= wy; i++)
            {
                *point = col;          // PutDot
                point += VRAM_LINE_SIZE;
                err += plus;
                if (err >= 0)
                {
                    ++point;
                    err += minus;
                }
            }
        }
    }
    else
    {
        /* line goes to lower */
        wy = (u16)(sy - ey);           // Height
        if (wx > wy)
        {
            /* if (X size > Y size) draw a Dot per Xdot */
            plus = wy * 2;
            minus = -wx * 2;
            err = -wx;
            for (i = 0; i <= wx; i++)
            {
                *point = col;          // PutDot
                ++point;
                err += plus;
                if (err >= 0)
                {
                    point -= VRAM_LINE_SIZE;
                    err += minus;
                }
            }

        }
        else
        {
            /* If (X size <= Y size) draw a Dot per Ydot */
            plus = wx * 2;
            minus = -wy * 2;
            err = -wy;
            for (i = 0; i <= wy; i++)
            {
                *point = col;          // PutDot
                point -= VRAM_LINE_SIZE;
                err += plus;
                if (err >= 0)
                {
                    ++point;
                    err += minus;
                }
            }
        }
    }
}

void PutDot(int x, int y, u16 col)
{
    *(u16 *)((u32)VRAM_BASE_ADDR + y * 256 * 2 + x * 2) = col;
}

void ClearScreen(void)
{
    MI_DmaFill32(3, (void *)HW_LCDC_VRAM_C, 0x7FFF7FFF, 256 * 192 * sizeof(u16));
}

void DrawStrokes(const PRCStrokes *strokes, int sx, int sy)
{
    int     iPoint;
    PRCPoint prev;
    BOOL    newFlag;
    const PRCPoint *point;

    newFlag = TRUE;
    point = strokes->points;
    for (iPoint = 0; iPoint < strokes->size; iPoint++, point++)
    {
        if (!PRC_IsPenUpMarker(point))
        {
            if (newFlag)
            {
                PutDot(sx + point->x, sy + point->y, 1);
            }
            else
            {
                DrawLine(sx + prev.x, sy + prev.y, sx + point->x, sy + point->y, 1);
            }
            prev = *point;
            newFlag = FALSE;
        }
        else
        {
            newFlag = TRUE;
        }
    }
}

void PrintRecognitionResult(PRCPrototypeEntry **results, fx32 *scores)
{
    int     iResult;
    (void)scores;

    OS_Printf("\n-----------------------------------------------\n");

    // Recognition results output
    if (results[0] == NULL)
    {
        OS_Printf("Result: can't recognize as char\n");
    }
    else
    {
        OS_Printf("Result:\n");
        for (iResult = 0; iResult < RESULT_NUM; iResult++)
        {
            int     code;

            if (results[iResult] == NULL)
                break;

            code = PRC_GetEntryCode(results[iResult]);
            OS_Printf("  '%s' (score: %d.%03d)\n", PatternName[code], FX_Whole(scores[iResult]),
                      (scores[iResult] & FX16_DEC_MASK) * 1000 / (1 << FX16_DEC_SIZE));
        }
    }
}

void VBlankIntr(void)
{
    // Set IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}





/****************************************************************************
  Figure recognition execution section
 ****************************************************************************/
#define RECOGNITION_THREAD_STACK_SIZE   0x4000
#define RECOGNITION_THREAD_PRIO         20

#define RECOGNITION_REQUEST_MESSAGE_QUEUE_COUNT RECOGNITION_QUEUE_SIZE

PRCPrototypeDB gProtoDB;
PRCInputPattern gInputPattern;
void   *gPrototypeListBuffer;
void   *gPatternBuffer;
void   *gRecognitionBuffer;

OSThread gRecognitionThread;
void   *gRecognitionThreadStack;

OSMessageQueue gRecognitionRequestMessageQueue;
OSMessage gRecognitionRequestMessages[RECOGNITION_REQUEST_MESSAGE_QUEUE_COUNT];

void    RecognizePattern(void *data);

void InitRecognition(void)
{
    PRC_Init();

    // Expand the sample database
    // PrototypeList is a list of the sample patterns defined in a separate file
    {
        u32     size;

        size = PRC_GetPrototypeDBBufferSize(&PrototypeList);
        OS_Printf("required buffer for InitPrototypeDB: %d bytes\n", size);
        gPrototypeListBuffer = OS_Alloc(size);
        SDK_ASSERT(gPrototypeListBuffer);
    }
    if (!PRC_InitPrototypeDB(&gProtoDB, gPrototypeListBuffer, &PrototypeList))
    {
        OS_Printf("cannot initialize the prototype DB.\n");
        OS_Terminate();
    }

    {
        u32     size;

        // Allocate memory for expansion of input stroke data
        size = PRC_GetInputPatternBufferSize(POINT_AFTER_RESAMPLING_MAX, STROKE_PER_INPUT_MAX);
        OS_Printf("required buffer for InitInputPattern: %d bytes\n", size);
        gPatternBuffer = OS_Alloc(size);
        SDK_ASSERT(gPatternBuffer);

        // Allocate memory for recognition algorithm
        size =
            PRC_GetRecognitionBufferSize(POINT_AFTER_RESAMPLING_MAX, STROKE_PER_INPUT_MAX,
                                         &gProtoDB);
        OS_Printf("required buffer for Recognition: %d bytes\n", size);
        gRecognitionBuffer = OS_Alloc(size);
        SDK_ASSERT(gRecognitionBuffer);
    }

    // Initialize the message queue used for communication with the recognition routine thread
    OS_InitMessageQueue(&gRecognitionRequestMessageQueue, gRecognitionRequestMessages,
                        RECOGNITION_REQUEST_MESSAGE_QUEUE_COUNT);

    // Allocate the stack region for the thread
    gRecognitionThreadStack = OS_Alloc(RECOGNITION_THREAD_STACK_SIZE);
    SDK_ASSERT(gRecognitionThreadStack);

    // Create separate thread for recognition routine
    OS_CreateThread(&gRecognitionThread,
                    RecognizePattern,
                    NULL,
                    (void *)((int)gRecognitionThreadStack + RECOGNITION_THREAD_STACK_SIZE),
                    RECOGNITION_THREAD_STACK_SIZE, RECOGNITION_THREAD_PRIO);

    // Start separate thread
    // However, not immediately executed, because main routine has higher priority.
    OS_WakeupThreadDirect(&gRecognitionThread);
}

void DestroyRecognition(void)
{
    if (!OS_IsThreadTerminated(&gRecognitionThread))
    {
        // Terminate recognition thread

        // Send termination command
        (void)OS_JamMessage(&gRecognitionRequestMessageQueue, (OSMessage)NULL, OS_MESSAGE_BLOCK);

        // Wait for thread to be terminated
        OS_JoinThread(&gRecognitionThread);
    }

    OS_Free(gRecognitionThreadStack);
    OS_Free(gRecognitionBuffer);
    OS_Free(gPatternBuffer);
    OS_Free(gPrototypeListBuffer);
}

BOOL RecognizePatternAsync(RecognitionObject * object)
{
    if (object == NULL)
    {
        return FALSE;
    }
    // Request recognition processing from a separate thread
    return OS_SendMessage(&gRecognitionRequestMessageQueue, (OSMessage)object, OS_MESSAGE_NOBLOCK);
}


// Functions running in a separate thread
void RecognizePattern(void *data)
{
    PRCInputPatternParam inputParam;

    (void)data;

    inputParam.normalizeSize = gProtoDB.normalizeSize;
//    inputParam.resampleMethod = PRC_RESAMPLE_METHOD_DISTANCE;
//    inputParam.resampleThreshold = 12;
//    inputParam.resampleMethod = PRC_RESAMPLE_METHOD_ANGLE;
//    inputParam.resampleThreshold = 4096;
    inputParam.resampleMethod = PRC_RESAMPLE_METHOD_RECURSIVE;
    inputParam.resampleThreshold = 2;

    while (TRUE)
    {
        OSMessage msg;
        RecognitionObject *obj;

        // Wait for recognition to be requested
        (void)OS_ReceiveMessage(&gRecognitionRequestMessageQueue, &msg, OS_MESSAGE_BLOCK);
        if (msg == (OSMessage)NULL)
        {
            // Termination command
            break;
        }
        obj = (RecognitionObject *) msg;

        if (PRC_InitInputPatternEx(&gInputPattern, gPatternBuffer, &obj->strokes,
                                   POINT_AFTER_RESAMPLING_MAX, STROKE_PER_INPUT_MAX, &inputParam))
        {
            // Compare prototypeList and inputPattern and perform recognition
            (void)PRC_GetRecognizedEntriesEx(obj->results, obj->scores, RESULT_NUM,
                                             gRecognitionBuffer, &gInputPattern, &gProtoDB,
                                             PRC_KIND_ALL, NULL);

            {
                PRCStrokes tmpStrokes;
                PRC_GetInputPatternStrokes(&tmpStrokes, &gInputPattern);
                (void)PRC_CopyStrokes(&tmpStrokes, &obj->inputPatternStrokes);
            }
        }
        else
        {
            // Failure in PRC_InitInputPattern due to incorrect input
            int     i;

            PRC_Clear(&obj->inputPatternStrokes);
            for (i = 0; i < RESULT_NUM; i++)
            {
                obj->results[i] = NULL;
                obj->scores[i] = 0;
            }
        }

        // Set flag indicating that recognition was completed
        obj->recognized = TRUE;
    }
}
