#include "headfile.h"


static  uint32_t g_mciramloadaddr;
enum _vector_table_entries {
    fInitialSP = 0, //!< Initial stack pointer.
    fInitialPC = 1, //!< Reset vector.
};
/*******************************************************************************
*   Name: get_user_application_entry 获取用户应用程序入口
*  Brief:    获取用户应用程序入口
*  Input:  *appEntry 应用程序入口指针,*appStack 应用程序的堆栈指针
* Output:  appEntry pointer & appStack pointer
* Return:
*******************************************************************************/
static void get_user_application_entry(uint32_t *appEntry, uint32_t *appStack)
{

    volatile uint32_t *APP_VECTOR_TABLE;
    APP_VECTOR_TABLE = (uint32_t *)g_mciramloadaddr;
    *appEntry = APP_VECTOR_TABLE[fInitialPC];
    *appStack = APP_VECTOR_TABLE[fInitialSP];
}
/*******************************************************************************
*   Name: jump_to_application 跳转到应用程序入口
*  Brief:  跳转到应用程序入口并执行启动应用程序
*  Input:  applicationAddress ,stackPointer
* Output:
* Return:
*******************************************************************************/
static void jump_to_application(uint32_t applicationAddress, uint32_t stackPointer)
{
    // need to ensure the values we are using are not stored on the previous stack
    //uint32_t s_stackPointer = 0;
    //s_stackPointer = stackPointer;
    static void (*MCAPPBoot)(void) = 0;
    MCAPPBoot = (void (*)(void))applicationAddress;
   // debug_printf("start MC program...\r\n");
    // Set stack pointers to the application stack pointer.
    __set_MSP(stackPointer);
    __set_PSP(stackPointer);
    // Jump to the application.
    MCAPPBoot();
}
void fw_apprunset(void)
{
	uint32_t  Entryadr=0,appStack=0;
	g_mciramloadaddr=*(uint32_t*)(FWCURRNFLSADR);
	get_user_application_entry(&Entryadr, &appStack);
	jump_to_application(Entryadr, appStack);
}
uint16_t fm_verchk(void)
{
	uint32_t dtmp=0;
	dtmp=*(uint32_t*)(FWCURVFLSADR);
 return (dtmp);	
}
bool fmu_verupdchk(uint16_t v)
{
	if(v>*(uint32_t*)(FWCURVFLSADR))
		return TRUE;
	else
		return FALSE;
}
/*
void fmc_erase_pages(uint32_t flsadr,uint8_t pgsz)
{
    uint8_t erase_counter;

    fmc_unlock();

    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    for(erase_counter = 0; erase_counter < pgsz; erase_counter++){
        fmc_page_erase(flsadr + (FLS_PGSZ * erase_counter));
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    }

    fmc_lock();
}*/
/*
static void fmc_program(void)
{

    fmc_unlock();

    address = FMC_WRITE_START_ADDR;

    while(address < FMC_WRITE_END_ADDR){
        fmc_word_program(address, data0);
        address += 4;
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
    }

 
    fmc_lock();
}
 bool fmc_program_check(uint32_t flswadr,uint32_t rdadr,uint8_t wsz)
{
    uint32_t i;
    uint32_t* ptrd=NULL;
	  uint32_t* rtrd=NULL; 
    ptrd = (uint32_t *)flswadr;
    rtrd =(uint32_t *)rdadr;
    for(i = 0; i <wsz; i++){
        if((*ptrd) != (*rtrd)){
					  return FALSE;
           // break;
        }else{
            ptrd++;
					  rtrd++;
        }
    }
		return TRUE;
}*/
bool fm_updte(uint32_t periph_addr,uint32_t memadr,uint8_t sz)
{ 
	 uint32_t timeout=100000;	
   //uint32_t tmcnt=0;	
	 dma_start(DMA0,DMA_CH1,periph_addr ,memadr,sz);
	 while(dma_flag_get(DMA0, DMA_CH1,DMA_FLAG_FTF)==RESET){
		 if(--timeout==0){
			  return FALSE;
		 }
	 }
   //tmcnt=DMA_CHCNT(DMA0, DMA_CH1);	 
	 dma_flag_clear(DMA0, DMA_CH1,DMA_FLAG_FTF);  		 
   return TRUE;	 
}
bool fw_dlfls(uint32_t periph_addr,uint32_t memadr,uint8_t wdsz)
{
	    fmc_unlock();
    uint32_t address = periph_addr;
	  uint32_t addrs=memadr;
    uint32_t tmpdata=0;
	  uint8_t tmsz=wdsz;
		//fmc_erase_pages(address);
    while(tmsz--){
			    tmpdata=*(uint32_t*)addrs;
        fmc_word_program(address, tmpdata);
        address += 4;
			  addrs+=4;
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
    }
		if(fmc_program_check(periph_addr,memadr,wdsz)!=TRUE)
		{
			return FALSE;
		}
    fmc_lock();
   return TRUE;		
}
uint32_t BYTASSBL(uint8_t*x,uint8_t n) 
{
	uint32_t y=0;
	 if(n>2){
		 y=x[0];
		 y=(y<<8)|x[1];
		 y=(y<<8)|x[2];
		 if(n==4)
		 y=(y<<8)|x[3];	 
		 return y;
	 }else{
		 y=x[0];
		 y=(y<<8)|x[1];
		 return y;
	 }
}
/*
bool fw_cpy(uint32_t bytesz)
{
	    fmc_unlock();
	uint32_t adrtar=0,addrs=0,adrend=0;
	  if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC1_ADR){*/
    // adrtar =FLS_SEC2_DLADR;
	  // addrs=dladr;
		 //adrend=
/*	}else{
		
	}
    uint32_t tmpdata=0;
	  uint32_t tmpwsz=0;
	   if(bytesz%4){
			   return FALSE;
		 }else{
			 tmpwsz=bytesz/4;
		 }
		  fmc_erase_pages();
    while(address < (FLS_SEC1_RNADR+tmpwsz)){
			    tmpdata=*(uint32_t*)addrs;
        fmc_word_program(address, tmpdata);
        address += 4;
			  addrs+=4;
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
    }
		if(fmc_program_check(FLS_SEC1_RNADR,FLS_SEC2_DLADR,tmpwsz)!=TRUE)
		{
			return FALSE;
		}
    fmc_lock();
   return TRUE;		
}*/
uint8_t fm_get_pklen(NXP_DEVTPE_ENUM devtype,uint8_t pklen,uint16_t sz)
{
	 uint8_t calpksz=0;
	 if(sz==0&&devtype!=OTHERDEV){
		  calpksz=pklen-FWUPDPKGDSZ;
	 }else{
		  calpksz=pklen-9;
	 }	
   return calpksz;	 
}
/*
bool fmc_prg(uint32_t staradr, uint32_t *pdata,uint32_t bytesz)
{
	  fmc_unlock();
	  uint32_t adrtar=staradr,adrend=0;
    uint32_t* dtmp=NULL;	
	   if(bytesz%4){
			   return FALSE;
		 }
		   adrend=adrtar+bytesz;
		   dtmp= pdata;
    while(adrtar < adrend){
        if(fmc_word_program(adrtar, *dtmp)!=FMC_READY)return FALSE;
        adrtar += 4;
			  dtmp+=1;
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
    }
		if(fmc_program_check(staradr,(uint32_t)pdata,bytesz/4)!=TRUE)
		{
			return FALSE;
		}
    fmc_lock();
   return TRUE;		
}*/
bool fm_set_swfwadr(uint32_t swflsz,uint32_t updfwv)
{

	   uint32_t tmdata[14]={0};
		 for(uint8_t i=0;i<14;i++)
		 tmdata[i]=*(uint32_t*)(FWCURVFLSADR+i*4);
		 tmdata[0]=updfwv;
		 tmdata[1]=swflsz;
		 tmdata[2]=*(uint32_t*)(FWCURVFLSADR);
		 tmdata[3]=*(uint32_t*)(FWCURSZFLSADR);
		 if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC1_ADR){
		 tmdata[4]=FLS_SEC2_ADR;
			tmdata[5]=(tmdata[5]&0xFFFFFF00)|C2; 
		 }
		 else{
			tmdata[4]=FLS_SEC1_ADR;
			tmdata[5]=(tmdata[5]&0xFF00FFFF)|C1; 
		 }			 
		 fmc_erase_pages((FLS_START_ADR+4*FLS_PGSZ),FLS_SEC_PGSZ2);
	   if(fmc_prg(FWCURVFLSADR,tmdata,56)!=TRUE){
			   return FALSE;
		 }else{
			   return TRUE;
		 }    	 
}
bool fm_backlv(void)
{
	//if(*(uint32_t*)(FWLSTVFLSADR)<*(uint32_t*)(FWCURVFLSADR)){
		uint32_t tmdata[14]={0};
		 for(uint8_t i=0;i<14;i++)
		 tmdata[i]=*(uint32_t*)(FWCURVFLSADR+i*4);
		 tmdata[0]=*(uint32_t*)(FWLSTVFLSADR);
		 tmdata[1]=*(uint32_t*)(FWLSSZFLSADR);
		 tmdata[2]=*(uint32_t*)(FWCURVFLSADR);
		 tmdata[3]=*(uint32_t*)(FWCURSZFLSADR);
		 if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC1_ADR)
		 tmdata[4]=FLS_SEC2_ADR;
		 else
			tmdata[4]=FLS_SEC1_ADR;
      tmdata[5]=*(uint32_t*)(FWCRTYFLSADR);		 
				 fmc_erase_pages((FLS_START_ADR+4*FLS_PGSZ),FLS_SEC_PGSZ2);
	   if(fmc_prg(FWCURVFLSADR,tmdata,56)!=TRUE){
			   return FALSE;
		 }else{
			   return TRUE;
		 } 		 
	//}else{
		// return FALSE; 
	//}
}



