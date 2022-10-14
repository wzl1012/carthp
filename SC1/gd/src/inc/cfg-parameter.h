
#ifndef _CFG_PARAMETER_H_
#define _CFG_PARAMETER_H_
#include "stdint.h"
#include "API.h"
/*******************************************************************************
* 1.Included files
*******************************************************************************/
/*******************************************************************************
* 2.Global constant and macro definitions using #define
*******************************************************************************/
#define  EPRADR   0x0000
#define  EEPRADR  0x00
#define  EE0PGl  127
#define  SPGADR1  0x01   
#define  RDMSCLTIME 5
#define  DELAYTIMES  10
#define  MONI_INTMODE_PU
//#define  MONI_INTMODE_PP

typedef enum{
	PCF7945_E_ROM_SZ=8192,
	PCF7945_EE_ROM_SZ=1024,
}PCF7945_ROM_ENUM;
typedef enum{
	PCF7952_E_ROM_SZ=4096,
	PCF7952_EE_ROM_SZ=512,
}PCF7952_ROM_ENUM;
typedef enum{
	PCF7953_E_ROM_SZ=8192,
	PCF7953_EE_ROM_SZ=1024,
}PCF7953_ROM_ENUM;
typedef struct {
	NXP_DEVTPE_ENUM devtype;
	DEV_FTCODE  fctcode;
	uint8_t Epgsize;
	uint8_t EEpgsize;
	uint16_t EROMsize;
	uint16_t EEROMsize;
	uint8_t* EProbuff;
	uint8_t* EEProbuff;
	uint16_t  reserv;
}Rom_parameter_struct;

/*******************************************************************************
* 3.Global structures, unions and enumerations using typedef
*******************************************************************************/
extern   uint8_t devmod;
/*******************************************************************************
* 4.Global variable extern declarations
*******************************************************************************/

/*******************************************************************************
* 5.Global function prototypes
*******************************************************************************/


#endif

