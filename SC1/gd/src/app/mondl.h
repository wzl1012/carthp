#ifndef MONDL_H
#define MONDL_H
#include "gd32f10x.h"

#define BIN_EEFLSBR_START_ADDR    ((uint32_t)(FLS_START_ADR+121*FLS_PGSZ))//((uint32_t)(FLS_START_ADR+80*FLS_PGSZ))//((uint32_t)(FLS_START_ADR+39*FLS_PGSZ))//0x3c800
//#define BIN_EEFLSBR_END_ADDR      ((uint32_t)0x0800FBFFU)
#define KFW_PGSZ     5
typedef enum{
	GC_DEV=0xD1,
	NXP_DEV=0xD2
}DEV_FTCODE;
typedef enum{
	NUNDEV=0,
	PCF7945=0xA1,
	PCF7952=0xA2,
	PCF7953=0xA3,
	PCF7941=0xA4,
	OTHERDEV=0xB0
}NXP_DEVTPE_ENUM;
extern bool kfw_pkgszchk(NXP_DEVTPE_ENUM devtype,uint16_t fmpkgsz);
extern bool Rom_parameter_cofg(DEV_FTCODE fcode,NXP_DEVTPE_ENUM devtype,uint32_t ERdlflsadr,uint32_t EERdlflsadr);
extern bool fm_burd(DEV_FTCODE fcode,NXP_DEVTPE_ENUM devtype,/*uint32_t binsz,*/uint32_t ERdlflsadr,uint32_t EERdlflsadr);
extern uint8_t key_id(DEV_FTCODE devcd,NXP_DEVTPE_ENUM devtype,uint8_t* rddtype);
#endif

