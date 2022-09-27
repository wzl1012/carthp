#ifndef FWUPDATE_H
#define FWUPDATE_H 
#include "headfile.h"
#define  MD   0xA9A9CC33
#define  KMD  0x9A9A6633
#define  BINCD 0x6A5A88BB
#define  FLS_PGSZ        2048
#define  FLS_SEC_PGSZ      55
#define  FLS_SEC_PGSZ2      1
#define  FLS_START_ADR   0x08000000
#define  FLS_SEC0_ADR      FLS_START_ADR+2*FLS_PGSZ
#define  FLS_SEC1_ADR      FLS_START_ADR+16*FLS_PGSZ//FLS_START_ADR+44*FLS_PGSZ
#define  FLS_SEC2_ADR      FLS_START_ADR+72*FLS_PGSZ//FLS_START_ADR+86*FLS_PGSZ
#define  FWCURVFLSADR      FLS_START_ADR+4*FLS_PGSZ+8  
#define  FWCURSZFLSADR     FLS_START_ADR+4*FLS_PGSZ+12
#define  FWLSTVFLSADR      FLS_START_ADR+4*FLS_PGSZ+16
#define  FWLSSZFLSADR      FLS_START_ADR+4*FLS_PGSZ+20
#define  FWCURRNFLSADR     FLS_START_ADR+4*FLS_PGSZ+24
#define  FWCRTYFLSADR      FLS_START_ADR+4*FLS_PGSZ+28
#define   C1    0x00C10000
#define   C2    0x000000C2
#define  FWUPDPKGDSZ       25          
  
	                          
void fw_apprunset(void);
uint16_t fm_verchk(void);
bool fm_updte(uint32_t periph_addr,uint32_t memadr,uint8_t sz);
//bool fmc_program_check(uint32_t flswadr,uint32_t rdadr,uint8_t wsz);
bool fm_backlv(void);
bool fmu_verupdchk(uint16_t v);
bool fw_cpy(uint32_t dladr,uint32_t rnadr,uint32_t bytesz);
uint32_t BYTASSBL(uint8_t*x,uint8_t n);
uint8_t fm_get_pklen(NXP_DEVTPE_ENUM devtype,uint8_t pklen,uint16_t sz);
bool fm_set_swfwadr(uint32_t swflsz,uint32_t updfwv);
bool fw_dlfls(uint32_t periph_addr,uint32_t memadr,uint8_t wdsz);

extern void fmc_erase_pages(uint32_t flsadr,uint8_t pgsz);
extern  bool fmc_prg(uint32_t staradr, uint32_t *pdata,uint32_t bytesz);
extern  bool fmc_program_check(uint32_t flswadr,uint32_t rdadr,uint8_t wsz);
#endif



