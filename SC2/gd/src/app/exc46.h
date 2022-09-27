#ifndef EXC46_H
#define EXC46_H
#include "headfile.h"

#define EXCIDCMD   0xF0
#define EXCAUDCMD  0x12340b2f667a55820006//0001,0010,0011,0100,0000,1011,0010,1111,0110,0110,0111,1010,0101,0101,1000,0010,0000,0000,0000,0110b
#define EXCIDXRP   0xC9129DA7 
#define EXCPGDXRP  0xA8914F48
#define EXCGIDC    0x148c4221
#define EXCIDEPTGC 1111110100001111110110000001111110
#define EXCIDNEPTGC 1111110101011110001101101010111101
enum{
	exc_pg0rwcmd=0x7a,  //1111010b,
	exc_pg1rwcmd=0x7e,//1111110b,
	exc_pg2rwcmd=0x72,//1110010
	exc_pg3rwcmd=0x76,//1110110
	exc_pg4rwcmd=0x6a,//1101010
	exc_pg5rwcmd=0x6e,//1101110
	exc_pg6rwcmd=0x62,//1100010
	exc_pg7rwcmd=0x66 //1100110
}exc_pgrwcmd_t;
typedef enum
{
	e_pwoff,
	e_idcfm,
	e_authed,
} exc46_stage_t;
typedef enum
{
	exc46_not,
	exc46_yes,
	exc46_cracked
}exc46_mode_t;
typedef struct
{
	exc46_stage_t exc46state;
	exc46_mode_t mode;
	uint8_t tmcf_enc;
	modulation_t tmcf_dcs;
	uint8_t a_page[8][4];
	uint8_t rgidc[4];			// MSbit:  :LSBit 
	uint8_t rid[4];		// MSbit:  :LSBit,hitag2:48bit,hitag2+:96bit
} exc46_tag_t;


#endif



