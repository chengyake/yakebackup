require"pins"
module(...,package.seeall)

local bit = require "bit"

--GPIO
DAT = {pin=pio.P0_1,dir=pio.OUTPUT1,valid=1}--pin27
DCS = {pin=pio.P0_3,dir=pio.OUTPUT1,valid=1}--pin26
--GPO
CLK = {pin=pio.P1_6,dir=pio.OUTPUT1,valid=1}--pin23
CSE = {pin=pio.P1_8,dir=pio.OUTPUT1,valid=1}--pin22
RST = {pin=pio.P1_9,dir=pio.OUTPUT1,valid=1}--pin33
--register
pins.reg(DAT, DCS, CLK, CSE, RST)


--[[local awesome = {
--  宽度x高度=84x48
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0x70, 0x30, 0x18, 0x1C,
  0x0C, 0x0C, 0x06, 0x06, 0x07, 0x07, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07,
  0x07, 0x07, 0x0E, 0x06, 0x1C, 0x1C, 0x38, 0x70, 0x70, 0xE0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF0, 0x3C, 0xCE, 0x67, 0x33, 0x18, 0x08,
  0x08, 0xC8, 0xF8, 0xF0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
  0x70, 0x38, 0x18, 0x18, 0x08, 0x08, 0x08, 0xF8, 0xF0, 0xF0, 0xE0, 0xC0, 0x00, 0x00, 0x01, 0x07,
  0x0F, 0x3C, 0xF8, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0x0F, 0x00, 0x0C, 0x7F,
  0x60, 0x60, 0x60, 0x60, 0x60, 0x61, 0x61, 0x61, 0x61, 0x61, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x7F, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x61, 0x61, 0x61, 0x61, 0x63,
  0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF,
  0xF0, 0x00, 0x00, 0x00, 0x08, 0x08, 0xFC, 0x8C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
  0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
  0x0C, 0x0C, 0x0C, 0xF8, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x3C, 0x70, 0xE0, 0x80, 0x00, 0x07, 0x0C, 0x38, 0x60, 0xC0,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF0, 0xF0, 0xF8, 0xF8, 0xF8, 0xF8, 0xF0,
  0xF0, 0xE0, 0xC0, 0x80, 0xC0, 0x30, 0x18, 0x0F, 0x00, 0x00, 0x80, 0xC0, 0x70, 0x3C, 0x1F, 0x07,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x06,
  0x0E, 0x1C, 0x18, 0x38, 0x31, 0x73, 0x62, 0x66, 0x64, 0xC7, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF,
  0xC7, 0xC7, 0xC7, 0x67, 0x63, 0x63, 0x71, 0x30, 0x38, 0x18, 0x1C, 0x0C, 0x06, 0x03, 0x03, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};]]--


local girl = {
--  宽度x高度=45x48
0x00,0x00,0x00,0x00,0x00,0x0C,0x0E,0x0C,0x04,0x0C,0x04,0x0E,0x1A,0x0E,0x00,0x00,
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0x60,0x30,0x18,0x08,0x0C,
0x0C,0x04,0x04,0x04,0x06,0x06,0x06,0x06,0x04,0x0C,0x0C,0x18,0x18,0x30,0xF8,0xCC,
0x06,0x1E,0x1B,0x17,0x2F,0x77,0xDE,0xFC,0xF0,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0xFF,0x0F,0x01,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFC,0x3A,
0x78,0xFC,0xF4,0xBE,0x1F,0x0F,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x03,0x1E,0x70,0xAE,0xFF,0xFA,0xD4,0x34,0x3C,0x08,0x08,0x08,0x08,0x0E,0x1C,
0x18,0x1F,0x5E,0x78,0xF0,0x50,0xFC,0x00,0xE0,0xC0,0x78,0x1F,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x03,0x07,0x1F,0x1A,0x24,0x04,0x0C,0x4C,0xFA,0xFF,0x78,0x3C,0xFC,0xF4,0xCC,
0x0F,0x0F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x1F,0x1E,0x1C,0x1C,0x17,0x03,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
}


local ASCII_code = {
 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} -- 20
,{0x00, 0x00, 0x00, 0x5f, 0x00, 0x00} -- 21 !
,{0x00, 0x00, 0x07, 0x00, 0x07, 0x00} -- 22 "
,{0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14} -- 23 #
,{0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12} -- 24 $
,{0x00, 0x23, 0x13, 0x08, 0x64, 0x62} -- 25 %
,{0x00, 0x36, 0x49, 0x55, 0x22, 0x50} -- 26 &
,{0x00, 0x00, 0x05, 0x03, 0x00, 0x00} -- 27 '
,{0x00, 0x00, 0x1c, 0x22, 0x41, 0x00} -- 28 (
,{0x00, 0x00, 0x41, 0x22, 0x1c, 0x00} -- 29 )
,{0x00, 0x14, 0x08, 0x3e, 0x08, 0x14} -- 2a *
,{0x00, 0x08, 0x08, 0x3e, 0x08, 0x08} -- 2b +
,{0x00, 0x00, 0x50, 0x30, 0x00, 0x00} -- 2c ,
,{0x00, 0x08, 0x08, 0x08, 0x08, 0x08} -- 2d -
,{0x00, 0x00, 0x60, 0x60, 0x00, 0x00} -- 2e .
,{0x00, 0x20, 0x10, 0x08, 0x04, 0x02} -- 2f /
,{0x00, 0x3e, 0x51, 0x49, 0x45, 0x3e} -- 30 0
,{0x00, 0x00, 0x42, 0x7f, 0x40, 0x00} -- 31 1
,{0x00, 0x42, 0x61, 0x51, 0x49, 0x46} -- 32 2
,{0x00, 0x21, 0x41, 0x45, 0x4b, 0x31} -- 33 3
,{0x00, 0x18, 0x14, 0x12, 0x7f, 0x10} -- 34 4
,{0x00, 0x27, 0x45, 0x45, 0x45, 0x39} -- 35 5
,{0x00, 0x3c, 0x4a, 0x49, 0x49, 0x30} -- 36 6
,{0x00, 0x01, 0x71, 0x09, 0x05, 0x03} -- 37 7
,{0x00, 0x36, 0x49, 0x49, 0x49, 0x36} -- 38 8
,{0x00, 0x06, 0x49, 0x49, 0x29, 0x1e} -- 39 9
,{0x00, 0x00, 0x36, 0x36, 0x00, 0x00} -- 3a :
,{0x00, 0x00, 0x56, 0x36, 0x00, 0x00} -- 3b ;
,{0x00, 0x08, 0x14, 0x22, 0x41, 0x00} -- 3c <
,{0x00, 0x14, 0x14, 0x14, 0x14, 0x14} -- 3d =
,{0x00, 0x00, 0x41, 0x22, 0x14, 0x08} -- 3e >
,{0x00, 0x02, 0x01, 0x51, 0x09, 0x06} -- 3f ?
,{0x00, 0x32, 0x49, 0x79, 0x41, 0x3e} -- 40 @
,{0x00, 0x7e, 0x11, 0x11, 0x11, 0x7e} -- 41 A
,{0x00, 0x7f, 0x49, 0x49, 0x49, 0x36} -- 42 B
,{0x00, 0x3e, 0x41, 0x41, 0x41, 0x22} -- 43 C
,{0x00, 0x7f, 0x41, 0x41, 0x22, 0x1c} -- 44 D
,{0x00, 0x7f, 0x49, 0x49, 0x49, 0x41} -- 45 E
,{0x00, 0x7f, 0x09, 0x09, 0x09, 0x01} -- 46 F
,{0x00, 0x3e, 0x41, 0x49, 0x49, 0x7a} -- 47 G
,{0x00, 0x7f, 0x08, 0x08, 0x08, 0x7f} -- 48 H
,{0x00, 0x00, 0x41, 0x7f, 0x41, 0x00} -- 49 I
,{0x00, 0x20, 0x40, 0x41, 0x3f, 0x01} -- 4a J
,{0x00, 0x7f, 0x08, 0x14, 0x22, 0x41} -- 4b K
,{0x00, 0x7f, 0x40, 0x40, 0x40, 0x40} -- 4c L
,{0x00, 0x7f, 0x02, 0x0c, 0x02, 0x7f} -- 4d M
,{0x00, 0x7f, 0x04, 0x08, 0x10, 0x7f} -- 4e N
,{0x00, 0x3e, 0x41, 0x41, 0x41, 0x3e} -- 4f O
,{0x00, 0x7f, 0x09, 0x09, 0x09, 0x06} -- 50 P
,{0x00, 0x3e, 0x41, 0x51, 0x21, 0x5e} -- 51 Q
,{0x00, 0x7f, 0x09, 0x19, 0x29, 0x46} -- 52 R
,{0x00, 0x46, 0x49, 0x49, 0x49, 0x31} -- 53 S
,{0x00, 0x01, 0x01, 0x7f, 0x01, 0x01} -- 54 T
,{0x00, 0x3f, 0x40, 0x40, 0x40, 0x3f} -- 55 U
,{0x00, 0x1f, 0x20, 0x40, 0x20, 0x1f} -- 56 V
,{0x00, 0x3f, 0x40, 0x38, 0x40, 0x3f} -- 57 W
,{0x00, 0x63, 0x14, 0x08, 0x14, 0x63} -- 58 X
,{0x00, 0x07, 0x08, 0x70, 0x08, 0x07} -- 59 Y
,{0x00, 0x61, 0x51, 0x49, 0x45, 0x43} -- 5a Z
,{0x00, 0x00, 0x7f, 0x41, 0x41, 0x00} -- 5b [
,{0x00, 0x02, 0x04, 0x08, 0x10, 0x20} -- 5c \ //
,{0x00, 0x00, 0x41, 0x41, 0x7f, 0x00} -- 5d ]
,{0x00, 0x04, 0x02, 0x01, 0x02, 0x04} -- 5e ^
,{0x00, 0x40, 0x40, 0x40, 0x40, 0x40} -- 5f _
,{0x00, 0x00, 0x01, 0x02, 0x04, 0x00} -- 60 `
,{0x00, 0x20, 0x54, 0x54, 0x54, 0x78} -- 61 a
,{0x00, 0x7f, 0x48, 0x44, 0x44, 0x38} -- 62 b
,{0x00, 0x38, 0x44, 0x44, 0x44, 0x20} -- 63 c
,{0x00, 0x38, 0x44, 0x44, 0x48, 0x7f} -- 64 d
,{0x00, 0x38, 0x54, 0x54, 0x54, 0x18} -- 65 e
,{0x00, 0x08, 0x7e, 0x09, 0x01, 0x02} -- 66 f
,{0x00, 0x0c, 0x52, 0x52, 0x52, 0x3e} -- 67 g
,{0x00, 0x7f, 0x08, 0x04, 0x04, 0x78} -- 68 h
,{0x00, 0x00, 0x44, 0x7d, 0x40, 0x00} -- 69 i
,{0x00, 0x20, 0x40, 0x44, 0x3d, 0x00} -- 6a j
,{0x00, 0x7f, 0x10, 0x28, 0x44, 0x00} -- 6b k
,{0x00, 0x00, 0x41, 0x7f, 0x40, 0x00} -- 6c l
,{0x00, 0x7c, 0x04, 0x18, 0x04, 0x78} -- 6d m
,{0x00, 0x7c, 0x08, 0x04, 0x04, 0x78} -- 6e n
,{0x00, 0x38, 0x44, 0x44, 0x44, 0x38} -- 6f o
,{0x00, 0x7c, 0x14, 0x14, 0x14, 0x08} -- 70 p
,{0x00, 0x08, 0x14, 0x14, 0x18, 0x7c} -- 71 q
,{0x00, 0x7c, 0x08, 0x04, 0x04, 0x08} -- 72 r
,{0x00, 0x48, 0x54, 0x54, 0x54, 0x20} -- 73 s
,{0x00, 0x04, 0x3f, 0x44, 0x40, 0x20} -- 74 t
,{0x00, 0x3c, 0x40, 0x40, 0x20, 0x7c} -- 75 u
,{0x00, 0x1c, 0x20, 0x40, 0x20, 0x1c} -- 76 v
,{0x00, 0x3c, 0x40, 0x30, 0x40, 0x3c} -- 77 w
,{0x00, 0x44, 0x28, 0x10, 0x28, 0x44} -- 78 x
,{0x00, 0x0c, 0x50, 0x50, 0x50, 0x3c} -- 79 y
,{0x00, 0x44, 0x64, 0x54, 0x4c, 0x44} -- 7a z
,{0x00, 0x00, 0x08, 0x36, 0x41, 0x00} -- 7b {
,{0x00, 0x00, 0x00, 0x7f, 0x00, 0x00} -- 7c |
,{0x00, 0x00, 0x41, 0x36, 0x08, 0x00} -- 7d }
,{0x00, 0x10, 0x08, 0x08, 0x10, 0x08} -- 7e ~
,{0x00, 0x78, 0x46, 0x41, 0x46, 0x78} -- 7f (delete)
}

local sign = {
 {0x00, 0x08, 0x08, 0x2a, 0x1c, 0x08} -- 00 ->
,{0x00, 0x08, 0x1c, 0x2a, 0x08, 0x08} -- 01 <-
,{0x00, 0x04, 0x02, 0x7f, 0x02, 0x04} -- 02 ↑
,{0x00, 0x10, 0x20, 0x7f, 0x20, 0x10} -- 03 ↓
,{0x00, 0x15, 0x16, 0x7c, 0x16, 0x15} -- 04 ￥
,{0x00, 0x03, 0x3b, 0x44, 0x44, 0x44} -- 05 ℃
,{0x00, 0x03, 0x03, 0x7c, 0x14, 0x14} -- 06 ℉
,{0x00, 0x44, 0x28, 0x10, 0x28, 0x44} -- 07 ×
,{0x00, 0x08, 0x08, 0x2a, 0x08, 0x08} -- 08 ÷
,{0x00, 0x38, 0x44, 0x48, 0x30, 0x2c} -- 09 α
,{0x00, 0xf8, 0x54, 0x54, 0x54, 0x28} -- 0a β
,{0x00, 0x28, 0x54, 0x54, 0x44, 0x20} -- 0b ε
,{0x00, 0x3e, 0x49, 0x09, 0x09, 0x06} -- 0c ρ
,{0x00, 0x38, 0x44, 0x4c, 0x54, 0x24} -- 0d σ
,{0x00, 0x40, 0x3f, 0x08, 0x08, 0x1f} -- 0e μ
,{0x00, 0x1f, 0x08, 0x08, 0x3f, 0x40} -- 0f η
,{0x00, 0x3c, 0x4a, 0x4a, 0x4a, 0x3c} -- 10 θ
,{0x00, 0x58, 0x64, 0x04, 0x64, 0x58} -- 11 Ω
,{0x00, 0x44, 0x3c, 0x04, 0x7c, 0x44} -- 12 π
,{0x00, 0x30, 0x28, 0x10, 0x28, 0x18} -- 13 ∞
,{0x00, 0x30, 0x28, 0x24, 0x28, 0x30} -- 14 △
,{0x00, 0x08, 0x1c, 0x08, 0x08, 0x0e} -- 15 (Enter)
}



local function write_cmd(cmd)

    pins.set(false, CSE)
    pins.set(false, DCS)

    for i=0, 7 do
        tmp =  bit.band(bit.lshift(cmd, i), 0x00000080)
        if tmp > 0 then   
            pins.set(true, DAT)
        else
            pins.set(false, DAT)
        end
        pins.set(false, CLK)
        --pins.set(false, CLK)--delay
        pins.set(true, CLK)
    end

    pins.set(true, CSE)
end

local function write_data(data)

    pins.set(false, CSE)
    pins.set(true, DCS)

    for i=0, 7 do
        tmp =  bit.band(bit.lshift(data, i), 0x00000080)
        if tmp > 0 then   
            pins.set(true, DAT)
        else
            pins.set(false, DAT)
        end
        pins.set(false, CLK)
        --pins.set(false, CLK)--delay
        pins.set(true, CLK)
    end

    pins.set(true, CSE)
end

local function write_byte(data, flag) -- 写一字节 0:指令 1：数据


    pins.set(false, CSE)
    pins.set(flag==1, DCS)

    for i=0, 7  do
        tmp =  bit.band(bit.lshift(data, i), 0x00000080)
        if tmp > 0 then   
            pins.set(true, DAT)
        else
            pins.set(false, DAT)
        end

        pins.set(false, CLK)
        --pins.set(false, CLK)--delay
        pins.set(true, CLK)
    end
    pins.set(true, DCS)
    pins.set(true, CSE)
    pins.set(true, DAT)

end



local function init_5110()

    pins.set(false, RST)
    --pins.set(false, RST)--delay_1us();
    pins.set(true, RST)

    pins.set(false, CSE)
    pins.set(false, DCS)
    --pins.set(false, DCS)--delay_1us();


    write_cmd(0x21); -- 使用扩展命令设置LCD模式,PD=0,V=0,H=1
    write_cmd(0xC0); -- 设置偏置电压
    write_cmd(0x06); -- 温度校正
    write_cmd(0x13); -- 1:48

    write_cmd(0x20); -- 使用基本命令 
    write_cmd(0x0C); -- 设定显示模式，正常显示

    pins.set(true, CSE)


end

local function set_5110_xy(x, y)
    write_cmd(bit.bor(0x40, bit.band(y, 0x07)))
    write_cmd(bit.bor(0x80, bit.band(x, 0x7F)))
end



local function clear_5110()

    set_5110_xy(0, 0)
    for i=1, 6*84 do
        write_data(0x00);
    end
end



local function write_frame_all()

    for i=1, 6 do
        for j=1, 84 do
            write_byte(0xFF,1);
        end
    end
end

local function write_frame_A()

    set_5110_xy(0, 0)
    write_byte(0x00,1);
    write_byte(0x7C,1);
    write_byte(0x12,1);
    write_byte(0x11,1);
    write_byte(0x12,1);
    write_byte(0x7C,1);

    set_5110_xy(6*6, 2)
    for i=1, 6 do
        write_byte(0xFF,1);
    end

    set_5110_xy(13*6, 5)
    for i=1, 6 do
        write_byte(0xFF,1);
    end

end
--[[
local function write_frame_swesome()

    for k, v in pairs(awesome) do
        write_byte(v,1);
    end
end
]]--
local function write_frame_girl()

    for k, v in pairs(girl) do
        if k%45 == 0 then
            set_5110_xy(20, k/45)
        end
        write_byte(v,1);
    end
end

init_5110()
clear_5110()
write_frame_girl()
print("----------chengyake test 5110 lcd model----------c-")



