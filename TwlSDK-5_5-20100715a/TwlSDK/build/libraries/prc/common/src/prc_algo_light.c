/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - 
  File:     prc_algo_light.c

  Copyright 2003-2008 Nintendo. All rights reserved.

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
#include <nitro/prc/algo_light.h>

/*===========================================================================*
  Prototype Declarations
 *===========================================================================*/

/*===========================================================================*
  Variable Definitions
 *===========================================================================*/

/*===========================================================================*
  Functions
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         PRC_GetRecognitionBufferSizeEx_Light

  Description:  Calculates the work area size required for recognition algorithm.
                

  Arguments:    maxPointCount: Upper limit of number of input points (includes pen up marker)
                maxStrokeCount: Upper limit of stroke count
                protoDB: Sample DB
                param: Parameters for recognition processing

  Returns:      The memory capacity required for the recognition algorithm.
 *---------------------------------------------------------------------------*/
u32
PRC_GetRecognitionBufferSizeEx_Light(int maxPointCount,
                                     int maxStrokeCount,
                                     const PRCPrototypeDB_Light *protoDB,
                                     const PRCRecognizeParam_Light *param)
{
    (void)maxPointCount;
    (void)maxStrokeCount;
    (void)protoDB;
    (void)param;

    return 1;                          // Because OS_Alloc(0) causes an error
}

/*---------------------------------------------------------------------------*
  Name:         PRC_GetRecognizedEntriesEx_Light

  Description:  Compares and recognizes a specific kind entry and input pattern to return the top numRanking ranking of the result.
                

  Arguments:    resultEntries: Pointer to array that holds pointers to recognition results.
                                If less than the requested number could be recognized, fill in remainder with NULL.
                                
                resultScores: Pointer to recognition result score array
                numRanking: Number of returns to result*
                buffer: Pointer to memory region used by recognition algorithm
                                (Region size >= return value of PRC_GetRecognitionBufferSize)
                input: Input pattern
                protoDB: Sample DB
                kindMask: Takes the logical AND of the 'kind' values for all the sample DB entries. Considered valid if it is non-zero.
                                
                param: Parameters for recognition processing

  Returns:      Number of patterns in the compared sample DBs.
 *---------------------------------------------------------------------------*/
int
PRC_GetRecognizedEntriesEx_Light(PRCPrototypeEntry **resultEntries,
                                 fx32 *resultScores,
                                 int numRanking,
                                 void *buffer,
                                 const PRCInputPattern_Light *input,
                                 const PRCPrototypeDB_Light *protoDB,
                                 u32 kindMask, const PRCRecognizeParam_Light *param)
{
    int     i;
    const PRCiPatternData_Common *inputData;
    int     numCompared;

    (void)buffer;
    (void)param;

    SDK_ASSERT(resultEntries);
    SDK_ASSERT(resultScores);
    SDK_ASSERT(input);
    SDK_ASSERT(protoDB);
    SDK_ASSERT(numRanking > 0);

    for (i = 0; i < numRanking; i++)
    {
        resultEntries[i] = NULL;
        resultScores[i] = FX32_ONE * 32768;
    }

    inputData = &input->data;
    numCompared = 0;

    {
        const PRCiPrototypeEntry_Common *proto;
        int     iPattern;
        fx32    border;

        border = FX32_ONE;             // Cutoff level: If this value is set to FX32_ONE/2, then those with a score below 0.5 will not be evaluated at all.
        border *= 32768;               // Coefficient of the internal score

        proto = protoDB->patterns;

        for (iPattern = 0; iPattern < protoDB->patternCount; iPattern++, proto++)
        {
            const PRCiPatternData_Common *protoData;
            int     iStroke;
            fx32    wholeScore;
            fx32    patternBorder;

            if (!proto->entry->enabled || !(proto->entry->kind & kindMask))
                continue;

            protoData = &proto->data;

            if (inputData->strokeCount != protoData->strokeCount)
                continue;

            wholeScore = 0;

            // Set as cutoff the value which returned the amount of correction
            patternBorder = border;
            if (proto->entry->correction != 0)
            {
                // For parts where [patternBorder *= 1 / (FX32_ONE - correction)] really must be done, cut corners with larger values by suitably approximating with straight lines
                //  
                if (proto->entry->correction < -FX32_ONE)
                {
                    patternBorder >>= 1;        // Approximated by 1/2 or less
                }
                else if (proto->entry->correction < 0)
                {
                    // -1 < correction < 0  is approximated by correction/2+1
                    patternBorder = FX_Mul(patternBorder, FX32_ONE + proto->entry->correction / 2);
                }
                else if (proto->entry->correction < FX32_ONE / 2)
                {
                    // 0 < correction < 1/2 is approximated by correction*2+1
                    patternBorder = FX_Mul(patternBorder, FX32_ONE + proto->entry->correction * 2);
                }
                else
                {
                    patternBorder = FX32_ONE * 32768;
                }                      // Give up on the cutoff
            }

            for (iStroke = 0; iStroke < inputData->strokeCount; iStroke++)
            {
                int     iProto, iInput;
                int     protoStrokeIndex, inputStrokeIndex;
                int     protoSize, inputSize;
                const u16 *protoAngle;
                const u16 *inputAngle;
                const fx16 *protoRatio;
                const fx16 *inputRatio;
                fx16    protoNextRatio, inputNextRatio;
                fx32    score;
                fx32    localBorder;
                fx16    strokeRatio;
                int     loopEnd;

                score = 0;

                protoStrokeIndex = protoData->strokes[iStroke];
                inputStrokeIndex = inputData->strokes[iStroke];
                protoSize = protoData->strokeSizes[iStroke];
                inputSize = inputData->strokeSizes[iStroke];
                protoAngle = &protoData->lineSegmentAngleArray[protoStrokeIndex];
                inputAngle = &inputData->lineSegmentAngleArray[inputStrokeIndex];
                protoRatio = &protoData->lineSegmentRatioToStrokeArray[protoStrokeIndex];
                inputRatio = &inputData->lineSegmentRatioToStrokeArray[inputStrokeIndex];

                strokeRatio = protoData->strokeRatios[iStroke]; // If here do inputData->, then the relative weight of the strokes of the input image takes on great importance

                // Set the cutoff level
                if (strokeRatio == FX32_ONE)
                {
                    localBorder = patternBorder;
                }
                else if (strokeRatio >= FX32_ONE / 2)
                {
                    localBorder = patternBorder * 2;    // No problem with taking excess
                }
                else if (strokeRatio >= FX32_ONE / 3)
                {
                    localBorder = patternBorder * 3;    // No problem with taking excess
                }
                else
                {
                    localBorder = FX32_ONE * 32768;     // Doesn't consider the cutoff
                }

                SDK_ASSERT(protoSize >= 2);
                SDK_ASSERT(inputSize >= 2);

                iProto = iInput = 1;
                protoNextRatio = protoRatio[iProto];
                inputNextRatio = inputRatio[iInput];
                loopEnd = protoSize + inputSize - 3;
                for (i = 0; i < loopEnd; i++)
                {
                    int     diff;
                    SDK_ASSERT(iProto < protoSize);
                    SDK_ASSERT(iInput < inputSize);
                    diff = (s16)(protoAngle[iProto] - inputAngle[iInput]);
                    if (diff < 0)
                    {
                        diff = -diff;
                    }
                    if (protoNextRatio <= inputNextRatio)
                    {
                        score += protoNextRatio * diff;
                        iProto++;
                        inputNextRatio -= protoNextRatio;
                        protoNextRatio = protoRatio[iProto];
                    }
                    else
                    {
                        score += inputNextRatio * diff;
                        iInput++;
                        protoNextRatio -= inputNextRatio;
                        inputNextRatio = inputRatio[iInput];
                    }
                    // Cutoff check
                    if (score > localBorder)
                    {
                        // The likelihood of getting held up on the lowest-ranked candidate is gone now
                        wholeScore = FX32_ONE * 32768;
                        goto quit_compare;
                    }
                }

                wholeScore += FX_Mul(score, strokeRatio);
            }

            if (proto->entry->correction != 0)
            {
                wholeScore = FX_Mul(wholeScore, FX32_ONE - proto->entry->correction);
            }

//                wholeScore = FX_Mul(wholeScore, FX32_ONE - proto->entry->correction)
//                            + proto->entry->correction;

            // Note that low scores are better at this stage
          quit_compare:
            numCompared++;
            if (resultScores[numRanking - 1] > wholeScore)
            {
                resultScores[numRanking - 1] = wholeScore;
                resultEntries[numRanking - 1] = (PRCPrototypeEntry *)proto->entry;
                for (i = numRanking - 2; i >= 0; i--)
                {
                    if (resultScores[i] > resultScores[i + 1])
                    {
                        fx32    tmpScore;
                        PRCPrototypeEntry *tmpEntry;
                        tmpScore = resultScores[i];
                        resultScores[i] = resultScores[i + 1];
                        resultScores[i + 1] = tmpScore;
                        tmpEntry = resultEntries[i];
                        resultEntries[i] = resultEntries[i + 1];
                        resultEntries[i + 1] = tmpEntry;
                    }
                }
                // Set the cutoff score
                border = resultScores[numRanking - 1];
            }
        }
    }
    // Normalize the score
    {
        int     iRank;
        fx32    score;
        for (iRank = 0; iRank < numRanking; iRank++)
        {
            if (resultEntries[iRank] == NULL)
                break;

            score = resultScores[iRank];
            score = FX32_ONE - (score / 32768);
            if (score < 0)
                score = 0;
            if (score >= FX32_ONE)
                score = FX32_ONE;

            resultScores[iRank] = score;
        }
    }

    return numCompared;
}

/*===========================================================================*
  Static Functions
 *===========================================================================*/


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
