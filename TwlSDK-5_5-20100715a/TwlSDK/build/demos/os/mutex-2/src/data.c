/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - mutex-2
  File:     data.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5205 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

/*----------------------  Palette  Data   ---------------------------*/
#define RGB555(r,g,b) (b<<10|g<<5|r)

const u16 samplePlttData[16][16]  = {
    {RGB555(31,31,31), RGB555(0, 0, 0 ), RGB555(0, 0, 0 ), RGB555(0, 0, 0 ),}, // Black
    {RGB555(31,31,31), RGB555(31,0, 0 ), RGB555(31,0, 0 ), RGB555(31,0, 0 ),}, // Red
    {RGB555(31,31,31), RGB555(0, 31,0 ), RGB555(0, 31,0 ), RGB555(0, 31,0 ),}, // Green
    {RGB555(31,31,31), RGB555(0, 0, 31), RGB555(0, 0, 31), RGB555(0, 0, 31),}, // Blue
    {RGB555(31,31,31), RGB555(31,31,0 ), RGB555(31,31,0 ), RGB555(31,31,0 ),}, // Yellow
    {RGB555(31,31,31), RGB555(0, 31,31), RGB555(0, 31,31), RGB555(0, 31,31),}, // Cyan
    {RGB555(31,31,31), RGB555(31,0, 31), RGB555(31,0, 31), RGB555(31,0, 31),}, // Purple
};


/*---------------------- Character Data  -------------------------*/

const u32 sampleCharData[8*0xe0] = {
    0x00000000,0x00000000,0x00000000,0x00000000,    //0000
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x11111111,0x11111111,0x11111111,0x11111111,    //0001
    0x11111111,0x11111111,0x11111111,0x11111111,
    0x22222222,0x22222222,0x22222222,0x22222222,    //0002
    0x22222222,0x22222222,0x22222222,0x22222222,
    0x33333333,0x33333333,0x33333333,0x33333333,    //0003
    0x33333333,0x33333333,0x33333333,0x33333333,
    0x00000000,0x03333330,0x03333330,0x03333330,    //0004
    0x03333330,0x03333330,0x03333330,0x00000000,
    0x33333333,0x33333333,0x33333333,0x33333333,    //0005
    0x22222222,0x22222222,0x22222222,0x22222222,
    0x00000000,0x00003330,0x00033330,0x00333330,    //0006
    0x03333330,0x00333330,0x00033330,0x00003330,
    0x00000000,0x00000000,0x00003300,0x00033300,    //0007
    0x00333300,0x00033300,0x00003300,0x00000000,
    0x03333333,0x03333333,0x03333333,0x03333333,    //0008
    0x03333333,0x03333333,0x03333333,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0009
    0x00000000,0x00000000,0x00000000,0x33333333,
    0x00000000,0x00000000,0x00000000,0x00000000,    //000A
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //000B
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //000C
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //000D
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //000E
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //000F
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0010
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0011
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0012
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0013
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0014
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0015
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0016
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0017
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0018
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0019
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //001A
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //001B
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //001C
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //001D
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //001E
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //001F
    0x00000000,0x00000000,0x00000000,0x00000000,

    0x00000000,0x00000000,0x00000000,0x00000000,    //0020   
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00011000,0x00011000,0x00011000,0x00011000,    //0021  !
    0x00000000,0x00011000,0x00011000,0x00000000,
    0x00011011,0x00011011,0x00010010,0x00000000,    //0022  "
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00010100,0x00111110,0x00010100,    //0023  #
    0x00010100,0x00111110,0x00010100,0x00000000,
    0x00001000,0x00111110,0x00001011,0x00111110,    //0024  $
    0x01101000,0x00111110,0x00001000,0x00000000,
    0x01100111,0x00110101,0x00011111,0x00001100,    //0025  %
    0x01110110,0x01010011,0x01110001,0x00000000,
    0x00011100,0x00110110,0x00011100,0x00001110,    //0026  &
    0x00011011,0x01110011,0x00111110,0x00000000,
    0x00000011,0x00000011,0x00000010,0x00000000,    //0027  '
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00011100,0x00000110,0x00000011,0x00000011,    //0028  (
    0x00000011,0x00000110,0x00011100,0x00000000,
    0x00011100,0x00110000,0x01100000,0x01100000,    //0029  )
    0x01100000,0x00110000,0x00011100,0x00000000,
    0x00000000,0x00000000,0x00001000,0x00101010,    //002A  *
    0x00011100,0x00101010,0x00001000,0x00000000,
    0x00000000,0x00000000,0x00001000,0x00001000,    //002B  +
    0x00111110,0x00001000,0x00001000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //002C  ,
    0x00000000,0x00000011,0x00000011,0x00000010,
    0x00000000,0x00000000,0x00000000,0x00000000,    //002D  -
    0x00111110,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //002E  .
    0x00000000,0x00000011,0x00000011,0x00000000,
    0x01100000,0x00110000,0x00011000,0x00001100,    //002F  /
    0x00000110,0x00000011,0x00000001,0x00000000,
    0x00011100,0x00110010,0x01100011,0x01100011,    //0030  0
    0x01100011,0x00100110,0x00011100,0x00000000,
    0x00011000,0x00011100,0x00011110,0x00011000,    //0031  1
    0x00011000,0x00011000,0x00011000,0x00000000,
    0x00111110,0x01100011,0x01100011,0x00111000,    //0032  2
    0x00001110,0x00000011,0x01111111,0x00000000,
    0x01111110,0x00110000,0x00011000,0x00111100,    //0033  3
    0x01100000,0x01100011,0x00111110,0x00000000,
    0x00110000,0x00111000,0x00111100,0x00110110,    //0034  4
    0x00110011,0x01111111,0x00110000,0x00000000,
    0x00111111,0x00000011,0x00000011,0x00111111,    //0035  5
    0x01100000,0x01100011,0x00111110,0x00000000,
    0x00111100,0x00000110,0x00000011,0x00111111,    //0036  6
    0x01100011,0x01100011,0x00111110,0x00000000,
    0x01111111,0x01110000,0x00111000,0x00011100,    //0037  7
    0x00001100,0x00001100,0x00001100,0x00000000,
    0x00111110,0x01100011,0x01100011,0x00111110,    //0038  8
    0x01100011,0x01100011,0x00111110,0x00000000,
    0x00111110,0x01100011,0x01100011,0x01111110,    //0039  9
    0x01100000,0x00110000,0x00011110,0x00000000,
    0x00000000,0x00011000,0x00011000,0x00000000,    //003A  :
    0x00000000,0x00011000,0x00011000,0x00000000,
    0x00000000,0x00011000,0x00011000,0x00000000,    //003B  ;
    0x00000000,0x00011000,0x00011000,0x00010000,
    0x01100000,0x00111000,0x00001110,0x00000011,    //003C  <
    0x00001110,0x00111000,0x01100000,0x00000000,
    0x00000000,0x00000000,0x00111110,0x00000000,    //003D  =
    0x00000000,0x00111110,0x00000000,0x00000000,
    0x00000011,0x00001110,0x00111000,0x01100000,    //003E  >
    0x00111000,0x00001110,0x00000011,0x00000000,
    0x01111110,0x11000011,0x11000011,0x01111000,    //003F  ?
    0x00011000,0x00000000,0x00011000,0x00000000,
    0x00111110,0x01100011,0x01011001,0x01010101,    //0040  @
    0x01011101,0x01110011,0x00011110,0x00000000,
    0x00111110,0x01100011,0x01100011,0x01111111,    //0041  A
    0x01100011,0x01100011,0x01100011,0x00000000,
    0x00111111,0x01100011,0x01100011,0x00111111,    //0042  B
    0x01100011,0x01100011,0x00111111,0x00000000,
    0x00111110,0x01100011,0x01100011,0x00000011,    //0043  C
    0x01100011,0x01100011,0x00111110,0x00000000,
    0x00111111,0x01100011,0x01100011,0x01100011,    //0044  D
    0x01100011,0x01100011,0x00111111,0x00000000,
    0x01111111,0x00000011,0x00000011,0x00111111,    //0045  E
    0x00000011,0x00000011,0x01111111,0x00000000,
    0x01111111,0x00000011,0x00000011,0x00111111,    //0046  F
    0x00000011,0x00000011,0x00000011,0x00000000,
    0x00111110,0x01100011,0x00000011,0x01111011,    //0047  G
    0x01100011,0x01100011,0x00111110,0x00000000,
    0x01100011,0x01100011,0x01100011,0x01111111,    //0048  H
    0x01100011,0x01100011,0x01100011,0x00000000,
    0x00011000,0x00011000,0x00011000,0x00011000,    //0049  I
    0x00011000,0x00011000,0x00011000,0x00000000,
    0x01100000,0x01100000,0x01100000,0x01100000,    //004A  J
    0x01100000,0x01100011,0x00111110,0x00000000,
    0x01100011,0x01110011,0x00111011,0x00011111,    //004B  K
    0x00111011,0x01110011,0x01100011,0x00000000,
    0x00000011,0x00000011,0x00000011,0x00000011,    //004C  L
    0x00000011,0x00000011,0x01111111,0x00000000,
    0x01100011,0x01100011,0x01110111,0x01110111,    //004D  M
    0x01111111,0x01101011,0x01101011,0x00000000,
    0x01100011,0x01100111,0x01101111,0x01111111,    //004E  N
    0x01111011,0x01110011,0x01100011,0x00000000,
    0x00111110,0x01100011,0x01100011,0x01100011,    //004F  O
    0x01100011,0x01100011,0x00111110,0x00000000,
    0x00111111,0x01100011,0x01100011,0x00111111,    //0050  P
    0x00000011,0x00000011,0x00000011,0x00000000,
    0x00111110,0x01100011,0x01100011,0x01100011,    //0051  Q
    0x01100011,0x00111110,0x01110000,0x00000000,
    0x00111111,0x01100011,0x01100011,0x00111111,    //0052  R
    0x01100011,0x01100011,0x01100011,0x00000000,
    0x00111110,0x01100011,0x00000011,0x00111110,    //0053  S
    0x01100000,0x01100011,0x00111110,0x00000000,
    0x11111111,0x00011000,0x00011000,0x00011000,    //0054  T
    0x00011000,0x00011000,0x00011000,0x00000000,
    0x01100011,0x01100011,0x01100011,0x01100011,    //0055  U
    0x01100011,0x01100011,0x00111110,0x00000000,
    0x01100011,0x01100011,0x00110110,0x00110110,    //0056  V
    0x00011100,0x00011100,0x00001000,0x00000000,
    0x11011011,0x11011011,0x11011011,0x11011011,    //0057  W
    0x11011011,0x01111110,0x01100110,0x00000000,
    0x01000001,0x01100011,0x00110110,0x00011100,    //0058  X
    0x00011100,0x00110110,0x01100011,0x00000000,
    0x11000011,0x11000011,0x11100111,0x01111110,    //0059  Y
    0x00011000,0x00011000,0x00011000,0x00000000,
    0x01111111,0x00110000,0x00011000,0x00001100,    //005A  Z
    0x00000110,0x00000011,0x01111111,0x00000000,
    0x00001111,0x00000011,0x00000011,0x00000011,    //005B  [
    0x00000011,0x00000011,0x00001111,0x00000000,
    0x01100110,0x01100110,0x11111111,0x00011000,    //005C  \.
    0x11111111,0x00011000,0x00011000,0x00000000,
    0x01111000,0x01100000,0x01100000,0x01100000,    //005D  ]
    0x01100000,0x01100000,0x01111000,0x00000000,
    0x00011100,0x00110110,0x00100010,0x00000000,    //005E  ^
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //005F  _
    0x00000000,0x00000000,0x01111111,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0060
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x01111000,0x01100100,    //0061  a
    0x01100110,0x01110110,0x01101110,0x00000000,
    0x00000110,0x00000110,0x00111110,0x01100110,    //0062  b
    0x01100110,0x01100110,0x00111110,0x00000000,
    0x00000000,0x00000000,0x00111100,0x01100110,    //0063  c
    0x00000110,0x01100110,0x00111100,0x00000000,
    0x01100000,0x01100000,0x01111100,0x01100110,    //0064  d
    0x01100110,0x01100110,0x01111100,0x00000000,
    0x00000000,0x00000000,0x00111100,0x01100110,    //0065  e
    0x01111110,0x00000110,0x00111100,0x00000000,
    0x01110000,0x00011000,0x01111110,0x00011000,    //0066  f
    0x00011000,0x00011000,0x00011000,0x00000000,
    0x00000000,0x00000000,0x00111100,0x01100110,    //0067  g
    0x01100110,0x01111100,0x01100000,0x00111110,
    0x00000110,0x00000110,0x00000110,0x00111110,    //0068h
    0x01100110,0x01100110,0x01100110,0x00000000,
    0x00011000,0x00011000,0x00000000,0x00011000,    //0069  i
    0x00011000,0x00011000,0x00011000,0x00000000,
    0x00110000,0x00110000,0x00000000,0x00110000,    //006A  j
    0x00110000,0x00110000,0x00110011,0x00011110,
    0x00000011,0x01100011,0x00110011,0x00011111,    //006B  k
    0x00011111,0x00110011,0x01100011,0x00000000,
    0x00011000,0x00011000,0x00011000,0x00011000,    //006C  l
    0x00011000,0x00011000,0x00011000,0x00000000,
    0x00000000,0x00000000,0x01111111,0x11011011,    //006D  m
    0x11011011,0x11011011,0x11011011,0x00000000,
    0x00000000,0x00000000,0x00111110,0x01100110,    //006E  n
    0x01100110,0x01100110,0x01100110,0x00000000,
    0x00000000,0x00000000,0x00111100,0x01100110,    //006F  o
    0x01100110,0x01100110,0x00111100,0x00000000,
    0x00000000,0x00000000,0x00111110,0x01100110,    //0070  p
    0x01100110,0x00111110,0x00000110,0x00000110,
    0x00000000,0x00000000,0x01111100,0x01100110,    //0071  q
    0x01100110,0x01111100,0x01100000,0x01100000,
    0x00000000,0x00000000,0x00101100,0x00011100,    //0072  r
    0x00001100,0x00001100,0x00001100,0x00000000,
    0x00000000,0x00000000,0x00111100,0x00000110,    //0073  s
    0x00111100,0x01100000,0x00111100,0x00000000,
    0x00000000,0x00011000,0x00111100,0x00011000,    //0074  t
    0x00011000,0x00011000,0x00110000,0x00000000,
    0x00000000,0x00000000,0x01100110,0x01100110,    //0075  u
    0x01100110,0x01100110,0x01111100,0x00000000,
    0x00000000,0x00000000,0x01100110,0x01100110,    //0076  v
    0x01100110,0x00111100,0x00011000,0x00000000,
    0x00000000,0x00000000,0x11011011,0x11011011,    //0077  w
    0x11011011,0x11011011,0x01111110,0x00000000,
    0x00000000,0x00000000,0x01100110,0x00111100,    //0078  x
    0x00011000,0x00111100,0x01100110,0x00000000,
    0x00000000,0x00000000,0x01100110,0x01100110,    //0079  y
    0x01100110,0x01111100,0x01100000,0x00111110,
    0x00000000,0x00000000,0x01111110,0x00110000,    //007A  z
    0x00011000,0x00001100,0x01111110,0x00000000,
    0x00011100,0x00000110,0x00000110,0x00000111,    //007B  {
    0x00000110,0x00000110,0x00011100,0x00000000,
    0x00001000,0x00001000,0x00001000,0x00001000,    //007C  |
    0x00001000,0x00001000,0x00001000,0x00000000,
    0x00011100,0x00110000,0x00110000,0x01110000,    //007D  }
    0x00110000,0x00110000,0x00011100,0x00000000,
    0x01101100,0x00111110,0x00011011,0x00000000,    //007E  ~
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //007F
    0x00000000,0x00000000,0x00000000,0x00000000,

    0x00000000,0x00000000,0x00000000,0x00000000,    //0080
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0081
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0082
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0083
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0084
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0085
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0086
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0087
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0088
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0089
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //008A
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //008B
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //008C
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //008D
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //008E
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //008F
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0090
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0091
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0092
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0093
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0094
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0095
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0096
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0097
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0098
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //0099
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //009A
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //009B
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //009C
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //009D
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //009E
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //009F
    0x00000000,0x00000000,0x00000000,0x00000000,

    0x00000000,0x00000000,0x00000000,0x00000000,    //00A0
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //00A1
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //00A2
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //00A3
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,    //00A4
    0x00000000,0x00000000,0x00000000,0x00000000,

    0x00000000,0x00000000,0x00000000,0x00001100,    //00A5  �E
    0x00001100,0x00000000,0x00000000,0x00000000,
    0x01111110,0x01000000,0x01111110,0x01000000,    //00A6  ��
    0x01100000,0x00110000,0x00011100,0x00000000,
    0x00000000,0x00000000,0x00111110,0x00101000,    //00A7  �@
    0x00011000,0x00001000,0x00000100,0x00000000,
    0x00000000,0x00000000,0x00100000,0x00011000,    //00A8  �B
    0x00001110,0x00001000,0x00001000,0x00000000,
    0x00000000,0x00000000,0x00001000,0x00111110,    //00A9  �D
    0x00100010,0x00110000,0x00011100,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00011100,    //00AA  �F
    0x00001000,0x00001000,0x00111110,0x00000000,
    0x00000000,0x00000000,0x00010000,0x00111110,    //00AB  �H
    0x00011000,0x00010100,0x00010010,0x00000000,
    0x00000000,0x00000000,0x00000100,0x00111110,    //00AC  ��
    0x00100100,0x00010100,0x00000100,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00011100,    //00AD  ��
    0x00010000,0x00010000,0x01111110,0x00000000,
    0x00000000,0x00000000,0x00111100,0x00100000,    //00AE  ��
    0x00111100,0x00100000,0x00111100,0x00000000,
    0x00000000,0x00000000,0x00001010,0x00101010,    //00AF  �b
    0x00100000,0x00110000,0x00011100,0x00000000,
    0x00000000,0x00000000,0x00000000,0x01111110,    //00B0  �[
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x01111111,0x01000000,0x01100100,0x00111100,    //00B1  �A
    0x00000100,0x00000110,0x00000011,0x00000000,
    0x01100000,0x00110000,0x00011100,0x00010111,    //00B2  �B
    0x00010000,0x00010000,0x00010000,0x00000000,
    0x00001000,0x01111111,0x01000001,0x01000001,    //00B3  �E
    0x01100000,0x00110000,0x00011110,0x00000000,
    0x00000000,0x00111110,0x00001000,0x00001000,    //00B4  �G
    0x00001000,0x00001000,0x01111111,0x00000000,
    0x00100000,0x01111111,0x00101000,0x00101100,    //00B5  �I
    0x00100110,0x00100011,0x00110000,0x00000000,
    0x00000100,0x01111111,0x01000100,0x01000100,    //00B6  �J
    0x01000100,0x01100110,0x00110011,0x00000000,
    0x00000100,0x01111111,0x00001000,0x00001000,    //00B7  �L
    0x01111111,0x00010000,0x00010000,0x00000000,
    0x01111110,0x01000010,0x01000011,0x01000000,    //00B8  �N
    0x01100000,0x00110000,0x00011110,0x00000000,
    0x00000010,0x01111110,0x00010010,0x00010001,    //00B9  �P
    0x00010000,0x00011000,0x00001110,0x00000000,
    0x01111111,0x01000000,0x01000000,0x01000000,    //00BA  �R
    0x01000000,0x01000000,0x01111111,0x00000000,
    0x00100010,0x01111111,0x00100010,0x00100010,    //00BB  �T
    0x00100000,0x00110000,0x00011110,0x00000000,
    0x00000011,0x01000000,0x01000011,0x01000000,    //00BC  �V
    0x01000000,0x01100000,0x00111111,0x00000000,
    0x00111111,0x00100000,0x00100000,0x00010000,    //00BD  �X
    0x00001000,0x00010100,0x01100011,0x00000000,
    0x00000010,0x01111111,0x01000010,0x01100010,    //00BE  �Z
    0x00100010,0x00000110,0x01111100,0x00000000,
    0x01000001,0x01000011,0x01000010,0x01100000,    //00BF  �\.
    0x00110000,0x00011000,0x00001100,0x00000000,
    0x01111110,0x01000010,0x01000011,0x01111000,    //00C0  �^
    0x01000000,0x01100000,0x00111110,0x00000000,
    0x00110000,0x00011110,0x00010000,0x01111111,    //00C1  �`
    0x00010000,0x00011000,0x00001110,0x00000000,
    0x01000101,0x01000101,0x01000101,0x01000000,    //00C2  �c
    0x01100000,0x00110000,0x00011110,0x00000000,
    0x00111110,0x00000000,0x01111111,0x00001000,    //00C3  �e
    0x00001000,0x00001100,0x00000110,0x00000000,
    0x00000010,0x00000010,0x00000010,0x00011110,    //00C4  �g
    0x00100010,0x00000010,0x00000010,0x00000000,
    0x00001000,0x00001000,0x01111111,0x00001000,    //00C5  �i
    0x00001000,0x00001100,0x00000111,0x00000000,
    0x00111110,0x00000000,0x00000000,0x00000000,    //00C6  �j
    0x00000000,0x00000000,0x01111111,0x00000000,
    0x00111111,0x00100000,0x00110000,0x00011010,    //00C7  �k
    0x00001100,0x00010110,0x00100011,0x00000000,
    0x00001000,0x01111111,0x01100000,0x00110000,    //00C8  �l
    0x00011100,0x01101010,0x01001001,0x00000000,
    0x00100000,0x00100000,0x00100000,0x00100000,    //00C9  �m
    0x00110000,0x00011000,0x00001110,0x00000000,
    0x00011000,0x00110000,0x01100001,0x01000001,    //00CA  �n
    0x01000001,0x01000001,0x01000001,0x00000000,
    0x00000001,0x00000001,0x00111111,0x00000001,    //00CB  �q
    0x00000001,0x00000011,0x00111110,0x00000000,
    0x01111111,0x01000000,0x01000000,0x01000000,    //00CC  �t
    0x01100000,0x00110000,0x00011110,0x00000000,
    0x00000000,0x00000000,0x00001110,0x00011001,    //00CD  �w
    0x00110001,0x01100000,0x01000000,0x00000000,
    0x00001000,0x01111111,0x00001000,0x00001000,    //00CE  �z
    0x01001001,0x01001001,0x01001001,0x00000000,
    0x01111111,0x01000000,0x01000000,0x01100011,    //00CF  �}
    0x00111110,0x00001100,0x00011000,0x00000000,
    0x00011111,0x01110000,0x00000110,0x00011100,    //00D0  �~
    0x00110000,0x00000111,0x01111100,0x00000000,
    0x00001100,0x00000110,0x00000010,0x01000011,    //00D1  ��
    0x01000001,0x01000001,0x01111111,0x00000000,
    0x01000000,0x01100010,0x00110100,0x00011000,    //00D2  ��
    0x00001100,0x00010110,0x00100011,0x00000000,
    0x01111111,0x00000100,0x01111111,0x00000100,    //00D3  ��
    0x00000100,0x00001100,0x01111000,0x00000000,
    0x00000100,0x01111111,0x01000100,0x01100100,    //00D4  ��
    0x00110100,0x00000100,0x00000100,0x00000000,
    0x00011110,0x00010000,0x00010000,0x00010000,    //00D5  ��
    0x00010000,0x00010000,0x01111111,0x00000000,
    0x01111110,0x01000000,0x01000000,0x01111110,    //00D6  ��
    0x01000000,0x01000000,0x01111110,0x00000000,
    0x01111111,0x00000000,0x01111111,0x01000000,    //00D7  ��
    0x01000000,0x01100000,0x00111110,0x00000000,
    0x01000010,0x01000010,0x01000010,0x01000010,    //00D8  ��
    0x01000000,0x01100000,0x00111100,0x00000000,
    0x00001010,0x00001010,0x00001010,0x00001010,    //00D9  ��
    0x01001010,0x01101010,0x00111011,0x00000000,
    0x00000001,0x00000001,0x01000001,0x01100001,    //00DA  ��
    0x00110001,0x00011001,0x00001111,0x00000000,
    0x01111111,0x01000001,0x01000001,0x01000001,    //00DB  ��
    0x01000001,0x01000001,0x01111111,0x00000000,
    0x01111111,0x01000001,0x01000001,0x01000000,    //00DC  ��
    0x01100000,0x00110000,0x00011110,0x00000000,
    0x00000111,0x01000000,0x01000000,0x01000000,    //00DD  ��
    0x01100000,0x00110000,0x00011111,0x00000000,
    0x00001001,0x00010010,0x00000000,0x00000000,    //00DE  �J
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00001110,0x00001010,0x00001110,0x00000000,    //00DF  �K
    0x00000000,0x00000000,0x00000000,0x00000000,
};