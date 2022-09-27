#ifndef _FLASH_H
#define _FLASH_H
#include "gd32f10x.h"
#define BIN_EFL_START_ADDR    ((uint32_t)(FLS_START_ADR+37*FLS_PGSZ))
#define CKSZ   2048
#define BIN_CRTY_MDFLSADR      BIN_EFL_START_ADDR+CKSZ-1
#define BIN_EFL_END_ADDR      (0x0800D3bfU)
#define  EFLWD_NUM  (( BIN_EFL_END_ADDR  -BIN_EFL_START_ADDR) >> 2)
#define  FLSRDBSZ  3072
#define  KEY_CRTY_FLSADR    0x2000
#define  KEY_CRTY_EDFLSADR  KEY_CRTY_FLSADR+CKSZ-1
#define  CRTYPPSZ  32
#define  FLSCKBSZ  10
#endif
