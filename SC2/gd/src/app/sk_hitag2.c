#include "headfile.h"

static unsigned int scn;
static unsigned char sgt[4];
uint8_t sk_uid[4];
static uint8_t rf_tx_data[10];

const unsigned char fa4box[16] = {0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x1,0x0,0x0};
const unsigned char fb4box[16] = {0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x0,0x0,0x1,0x1,0x0};
const unsigned char fc5box[32] = {0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x0,
                                  0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0};

const unsigned char sdbox1[144] = { 
0xaa, 0xbb, 0x04, 0xcb, 0xa7, 0xab, 0x98, 0xb1, 0xfd, 0x07, 0x10, 0x55,  
0xaa, 0xbb, 0x04, 0xfc, 0x06, 0x25, 0xd2, 0xeb, 0xdf, 0x77, 0x4a, 0x55, 
0xaa, 0xbb, 0x04, 0xac, 0x04, 0x84, 0xe7, 0x53, 0x77, 0xc3, 0x28, 0x55, 
0xaa, 0xbb, 0x04, 0x68, 0x93, 0xc8, 0x97, 0xc6, 0x89, 0x31, 0xde, 0x55,  
0xaa, 0xbb, 0x04, 0xc5, 0x42, 0xf9, 0x66, 0x99, 0xc1, 0x85, 0xc1, 0x55,  
0xaa, 0xbb, 0x04, 0xcb, 0xa7, 0xab, 0x98, 0xb1, 0xfd, 0x07, 0x10, 0x55, 
0xaa, 0xbb, 0x04, 0x22, 0x55, 0x85, 0x2f, 0x1d, 0x19, 0xcd, 0x10, 0x55,
0xaa, 0xbb, 0x04, 0xfc, 0x06, 0x25, 0xd2, 0xeb, 0xdf, 0x77, 0x4a, 0x55,  
0xaa, 0xbb, 0x04, 0xac, 0x04, 0x84, 0xe7, 0x53, 0x77, 0xc3, 0x28, 0x55,  
0xaa, 0xbb, 0x04, 0x68, 0x93, 0xc8, 0x97, 0xc6, 0x89, 0x31, 0xde, 0x55,  
0xaa, 0xbb, 0x04, 0xc5, 0x42, 0xf9, 0x66, 0x99, 0xc1, 0x85, 0xc1, 0x55,   
0xaa, 0xbb, 0x04, 0xc5, 0x42, 0xf9, 0x66, 0x99, 0xc1, 0x85, 0xc1, 0x55};

const unsigned char sdbox2[144] = { 
0xaa, 0xbb, 0x04, 0x22, 0x55, 0x85, 0x2f, 0x1d, 0x19, 0xcd, 0x10, 0x55,
0xaa, 0xbb, 0x04, 0xcb, 0xa7, 0xab, 0x98, 0xb1, 0xfd, 0x07, 0x10, 0x55,
0xaa, 0xbb, 0x04, 0xfc, 0x06, 0x25, 0xd2, 0xeb, 0xdf, 0x77, 0x4a, 0x55,
0xaa, 0xbb, 0x04, 0xac, 0x04, 0x84, 0xe7, 0x53, 0x77, 0xc3, 0x28, 0x55, 
0xaa, 0xbb, 0x04, 0x68, 0x93, 0xc8, 0x97, 0xc6, 0x89, 0x31, 0xde, 0x55,  
0xaa, 0xbb, 0x04, 0xc5, 0x42, 0xf9, 0x66, 0x99, 0xc1, 0x85, 0xc1, 0x55,  
0xaa, 0xbb, 0x04, 0xcb, 0xa7, 0xab, 0x98, 0xb1, 0xfd, 0x07, 0x10, 0x55, 
0xaa, 0xbb, 0x04, 0xfc, 0x06, 0x25, 0xd2, 0xeb, 0xdf, 0x77, 0x4a, 0x55,  
0xaa, 0xbb, 0x04, 0xac, 0x04, 0x84, 0xe7, 0x53, 0x77, 0xc3, 0x28, 0x55,  
0xaa, 0xbb, 0x04, 0x68, 0x93, 0xc8, 0x97, 0xc6, 0x89, 0x31, 0xde, 0x55,  
0xaa, 0xbb, 0x04, 0xc5, 0x42, 0xf9, 0x66, 0x99, 0xc1, 0x85, 0xc1, 0x55,   
0xaa, 0xbb, 0x04, 0xc5, 0x42, 0xf9, 0x66, 0x99, 0xc1, 0x85, 0xc1, 0x55};

unsigned char cknumber(unsigned char *sxdata,unsigned char *rsdata)
{
  unsigned char i,j,cnt=0;
  unsigned char noright=0;
  for(i=0;i<12;i++)
  {
	for(j=0;j<7;j++)
	{
	  if(*(rsdata+cnt*7 + j) != *(sxdata+j))
	  {
		noright = 0x01;
		break;
	  } 
	}
        
	if(noright == 0x01)
	{
	  noright = 0x00;
	  cnt++;
	}
	else
	  break;
  }
  
  if(cnt==12)
    cnt=0;
  
  return cnt;
}

void Rdone(unsigned char *data, unsigned char *rsdata, unsigned char cnt)
{
	unsigned char i;
	
	for(i=0;i<7;i++)
		*(rsdata + cnt*7 + i ) = *(data+i);
	
	for(i=0;i<11;i++)
	{
		*(data+i) = *(data+i) >> 1;
		*(data+i) = ((*(data+i+1)&0x01)<<7) | *(data+i); 
	}
	
	*(data+11) = *(data+11) >> 1;
}

unsigned char Getfb (unsigned char *xstate)
{
	unsigned char fa0,fb0,fb1,fb2,fa1;
	unsigned char temp;
	
	temp = 0;
	temp = ((*xstate & 0x02)>>1) | ((*xstate & 0x04)>>1) | ((*xstate & 0x10)>>2) | ((*xstate & 0x20)>>2);
	fa0 = fa4box[temp & 0x0f];
	
	temp = 0;
	temp = ((*xstate & 0x80)>>7) | ((*(xstate+1) & 0x08)>>2) | ((*(xstate+1) & 0x20)>>3) | ((*(xstate+1) & 0x40)>>3);
	fb0 = fb4box[temp & 0x0f];
	
	temp = 0;
	temp = (*(xstate+2) & 0x01) | ((*(xstate+2) & 0x10)>>3) | ((*(xstate+2) & 0x40)>>4) | ((*(xstate+3) & 0x02)<<2);
	fb1 = fb4box[temp & 0x0f];
	
	temp = 0;
	temp = ((*(xstate+3)&0x08)>>3) | ((*(xstate+3)&0x10)>>3) | ((*(xstate+3)&0x40)>>4) | ((*(xstate+4)&0x01)<<3);
	fb2 = fb4box[temp & 0x0f];
	
	temp = 0;
	temp = ((*(xstate+4)&0x02)>>1) | ((*(xstate+5)&0x04)>>1) | ((*(xstate+5)&0x08)>>1) | ((*(xstate+5)&0x20)>>2);
	fa1 = fa4box[temp & 0x0f];
	
	temp = 0;
	temp = fa0 | (fb0<<1) | (fb1<<2) | (fb2<<3) | (fa1<<4);
	
	return fc5box[temp & 0x1f];
}

void Findkey (unsigned char *xstate, unsigned char *sk_uid, unsigned char *iv, unsigned char *key)
{
	unsigned char i,j;
	
	*key = *xstate;
	*(key+1) = *(xstate+1);
	
	for(i=0;i<4;i++){
		sk_uid[i] = reverse_byte(sk_uid[i]);
	}
	for(i=0;i<4;i++){
		iv[i] = reverse_byte(iv[i]);
	}
	for (j=0; j<32; j++) {
		for(i=5;i>=3;i--){
			*(key+i) = (*(key+i) << 1) | ((*(key+(i-1))&0x80)>>7);
		}
		*(key+2) = (*(key+2) << 1) | ( Getfb(&xstate[0]) ^ ((*(xstate+5)&0x80)>>7) ^ ((*(iv+3)&0x80)>>7));

		for(i=5;i>=1;i--){
			*(xstate+i) = (*(xstate+i) << 1) | ((*(xstate+(i-1))&0x80)>>7); 
		}
		*xstate = (*xstate << 1) | ((*(sk_uid+3) & 0x80)>>7);
		for(i=3;i>=1;i--){
			*(sk_uid+i) = (*(sk_uid+i) << 1) | ((*(sk_uid+(i-1))&0x80)>>7); 
			*(iv+i) = (*(iv+i) << 1) | ((*(iv+(i-1))&0x80)>>7); 
		} 
		*sk_uid = *sk_uid << 1;
		*iv = *iv << 1;
	}
}

void Turnback (unsigned char *fdata, unsigned char hmtimes)
{
	unsigned char out20,out21,out22,out23;
	unsigned char i,j;
	for(j=0;j<hmtimes;j++)
	{
		out20 = ((*fdata & 0x02)>>1) ^ ((*fdata & 0x04)>>2) ^ ((*fdata & 0x20)>>5) ^ ((*fdata & 0x40)>>6) ^ ((*fdata & 0x80)>>7); 
		out21 = ((*(fdata+1) & 0x80)>>7) ^ ((*(fdata+2) & 0x20)>>5) ^ ((*(fdata+2) & 0x40) >> 6) ^ ((*(fdata+3) & 0x02) >> 1);
		out22 = ((*(fdata+3) & 0x20)>>5) ^ (*(fdata+5) & 0x01) ^ ((*(fdata+5) & 0x02)>>1) ^ ((*(fdata+5) & 0x04)>>2);
		out23 = ((*(fdata+5) & 0x20)>>5) ^ ((*(fdata+5) & 0x40)>>6) ^ ((*(fdata+5) & 0x80)>>7);
		
		for(i=5;i>=1;i--){
			*(fdata+i) = (*(fdata+i) << 1) | ((*(fdata+(i-1))&0x80)>>7); 
		}
		*fdata = (*fdata << 1) | (out20 ^ out21 ^ out22 ^ out23);
	}
}

void SendOneck(unsigned char *fndata,unsigned char *rsdata)
{
	unsigned char i,j,flag;
	unsigned char cnt = 0;
//	uint8_t a_temp[13];

	//a_temp[0] = 0xAA;
	//a_temp[1] = 0xBB;
	//a_temp[2] = 0x46;
	//a_temp[11] = 0x55;
	for(i=0; i<6; i++) {
		flag = 0x46;
		for(j=0; j<7; j++) {
			//a_temp[3+j] = fndata[j];
			flag ^= fndata[j];
		}
		//a_temp[10] = flag;
		//lfe3_send_data(a_temp, 12);
		Rdone(&fndata[0],&rsdata[0],cnt);
		cnt++;
	}
} 

void send_nbit_cmd(uint16_t cmd)
{
	uint8_t a_wr[2];

	a_wr[0] = (uint8_t)(cmd>>2);
	a_wr[1] = (uint8_t)(cmd<<6);
	delay_us(PCF7936_T_WAIT_BS);
	pcf7991_write_tag_n(0);
	pcf7936_wr_msbits(a_wr, 10, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag(); 							// switching the device into transparent mode (READ_TAG mode)
	delay_us(PCF7936_T_WAIT_TR);
}

uint8_t basicflow(uint8_t code, uint8_t *cpdata)
{
		uint8_t cmd, result;
	uint8_t a_read[5];
	uint8_t (*p_fun)(uint8_t , uint8_t * , uint8_t , uint8_t );
	switch (code) {
		case e_modu_mc:
		case e_modu_bp:
			break;
		default:
			return FALSE;
	}
	
	if (code == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else
		p_fun = pcf7991_decode_bp;

	cmd = 0xC0;
	result = FALSE;
	pcf7991_write_tag_n(0);
	pcf7936_wr_msbits(&cmd, 5, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();		// switching the device into transparent mode (READ_TAG mode)
	delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		result = p_fun(32, a_read, 37, e_dir_msbit);
	}
	if (result) {
		result = FALSE;
		delay_us(PCF7936_T_WAIT_BS);
		pcf7991_write_tag_n(0);
		pcf7936_wr_msbits(cpdata, 64, TRUE);
		pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
		pcf7991_read_tag();
		delay_us(PCF7936_T_WAIT_TR);
		if (pcf7991_decode_miss_low()) {
			result = p_fun(32, a_read, 37, e_dir_msbit);
		}
	}

	if (result)
		return 0x00;
	else
		return 0xEE;
} 

unsigned char Fixtwobit (unsigned char code, unsigned char *ckdata)
{
	unsigned int i, scnt=0;
	unsigned char rclength=0, state=0, wpcmd=0;
	unsigned char sgtmp[4];
	uint8_t (*p_fun)(uint8_t , uint8_t * , uint8_t , uint8_t );
	if (code == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else
		p_fun = pcf7991_decode_bp;
	
	for(scn=0; scn<=1023; scn++) {
		pcf7936_to_off();
		pcf7936_off_to_wait();
		if(basicflow(code, &ckdata[0]) == 0x00) {
			send_nbit_cmd(scn); 
			if (pcf7991_decode_miss_low())
				if (p_fun(32, sgtmp, 5, e_dir_msbit)) {
					if (p_fun(32, sgtmp, 32, e_dir_msbit))
						rclength = 0x00; /*read or readinv command*/
					else
						rclength = 0x01; /*halt of write command*/
					
					state = 0x00;
					break;	/*It need only just one response, save so much time*/
				}
		}
		else {
			state = 0xEE;
			break;
		}
	}
	pcf7936_to_off();
	
	if(state == 0x00) {
 		/*make the read or readinv command to halt or write command */
		if(rclength == 0x00)
			scn = scn ^ 0x0108;// 01_0000_1000
			
		for(i=0; i<8; i++) {
			scnt = scn ^ i;
			scnt ^= (i<<5);
			pcf7936_to_off();
			pcf7936_off_to_wait();
			if(basicflow(code, &ckdata[0]) == 0x00) {
				send_nbit_cmd(scnt);
				if (pcf7991_decode_miss_low()) {
					if (p_fun(32, sgtmp, 15, e_dir_msbit)) {
						state = 0xEE;
					}
					else {
						wpcmd = 0x01;
						break;
					}
				}
				else {
					wpcmd = 0x01;
					break;
				}
			}
			else {
				state = 0xEE;
				break;
			}
		}
		pcf7936_to_off();
	}

	if (wpcmd == 0x01)
		scn = scnt ^ 0x0210;// 10_0001_0000 halt cmd 
	else 
		scn = scnt;// halt cmd

	return state;
}

unsigned char Fixthreebit(unsigned char code, unsigned char *ckdata)
{
	unsigned int  i;
	unsigned int  scnt;
	unsigned char wptmp = 0x00;
	unsigned char sgtmp[4];
	unsigned char state = 0x00;
	uint8_t (*p_fun)(uint8_t , uint8_t * , uint8_t , uint8_t );
	if (code == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else
		p_fun = pcf7991_decode_bp;
	
	scn = scn ^ 0x0318;// 11000_11000  make the command to read command
	for(i=0;i<4;i++)
	{
		scnt = scn ^ i;
		scnt ^= (i<<5);
		pcf7936_to_off();
		pcf7936_off_to_wait();
		if(basicflow(code, &ckdata[0]) == 0x00)
		{
			send_nbit_cmd(scnt);
			if (pcf7991_decode_miss_low()) {
				if (p_fun(32, sgtmp, 15, e_dir_msbit)) {
					state = 0xEE;
				}
				else {
					wptmp = 0x01;
					break;
				}
			}
			else {
				wptmp = 0x01;
				break;
			}
		}
		else
		{
			state = 0xEE;
			break;
		}
	}
	pcf7936_to_off();
	
	if(wptmp == 0x01)
		scn = scnt;
	else
		scn = scnt ^ 0x0084;// 00100_00100
	
	if(state == 0x00)
	{
		pcf7936_off_to_wait();
		basicflow(code, &ckdata[0]);
		send_nbit_cmd(scn);
		if (!pcf7991_decode_miss_low())
			scn = scn ^ 0x0021;
		else if (!p_fun(32, sgtmp, 5, e_dir_msbit))
			scn = scn ^ 0x0021;
		pcf7936_to_off();
	}

	return state;
}

void Fixckdata(unsigned char *fndata)
{
	unsigned int temp;
	
	sgt[0] = sgt[0] ^ sk_uid[0] ^ 0xc1;//11000 00111	11000 00111
	sgt[1] = sgt[1] ^ sk_uid[1] ^ 0xf0;
	sgt[2] = sgt[2] ^ sk_uid[2] ^ 0x7c;
	sgt[3] = sgt[3] ^ sk_uid[3] ^ 0x1c;
	temp = scn;
	temp<<=6;  
	rf_tx_data[0]=(unsigned char)(temp>>8);
	rf_tx_data[1]=(unsigned char)(temp);
	rf_tx_data[1]= (rf_tx_data[1] & 0xc0) | (sgt[0]>>2);
	rf_tx_data[2]= (sgt[0]<<6) | (sgt[1]>>2);
	rf_tx_data[3]= (sgt[1]<<6) | (sgt[2]>>2);
	rf_tx_data[4]= (sgt[2]<<6) | (sgt[3]>>2);
	
	*(fndata) = rf_tx_data[0] ^ 0xC1;		  ///11000_00111 110000 0111
	*(fndata+1) = rf_tx_data[1] ^ 0xF0;
	*(fndata+2) = rf_tx_data[2] ^ 0x7C;
	*(fndata+3) = rf_tx_data[3] ^ 0x1F;
	*(fndata+4) = rf_tx_data[4] ^ 0x07;
}

unsigned char Fixmore (unsigned char code, unsigned char *fndata, unsigned char *ckdata)
{
	unsigned char i, result;
	uint8_t (*p_fun)(uint8_t , uint8_t * , uint8_t , uint8_t );
	if (code == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else
		p_fun = pcf7991_decode_bp;
	
	pcf7936_to_off();
	pcf7936_off_to_wait();
	basicflow(code, &ckdata[0]);
	rf_tx_data[0]  = *(fndata)^ 0xC1;		  ///11000_00111 110000 0111
	rf_tx_data[1]  = *(fndata+1)^ 0xF0;
	rf_tx_data[2]  = *(fndata+2)^ 0x7C;
	rf_tx_data[3]  = *(fndata+3)^ 0x1F;
	rf_tx_data[4]  = *(fndata+4)^ 0x07;
	rf_tx_data[5]  = *(fndata+5)^ 0xC1;
	rf_tx_data[6]  = *(fndata+6)^ 0xF0;
	rf_tx_data[7]  = *(fndata+7)^ 0x7C;
	rf_tx_data[8]  = *(fndata+8)^ 0x1F; 
	
	delay_us(PCF7936_T_WAIT_BS);
	pcf7991_write_tag_n(0);
	pcf7936_wr_msbits(rf_tx_data, 40, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();								// switching the device into transparent mode (READ_TAG mode)
	delay_us(PCF7936_T_WAIT_TR);
	pcf7991_decode_miss_low();
	if (p_fun(32, &sgt[0], 5, e_dir_msbit)) {
		p_fun(32, &sgt[0], 32, e_dir_msbit);
		sgt[0] = sgt[0] ^ sk_uid[0];
		sgt[1] = sgt[1] ^ sk_uid[1];
		sgt[2] = sgt[2] ^ sk_uid[2];
		sgt[3] = sgt[3] ^ sk_uid[3];
		if((*(fndata+5) == sgt[0]) &  (*(fndata+6) == sgt[1]) & (*(fndata+7) == sgt[2]) & (*(fndata+8) == sgt[3]))
		{
			for(i=0;i<12;i++)
				fndata[i] = reverse_byte(fndata[i]);
			result = 0x00;
		}
		else
			result = 0xEE;
	}
	else
		result = 0xEE;

	pcf7936_to_off();
	return result;
}

unsigned char Fixfivebit(unsigned char code, unsigned char *fndata, unsigned char *ckdata)
{
	unsigned char state = 0x00;
	uint8_t (*p_fun)(uint8_t , uint8_t * , uint8_t , uint8_t );
	if (code == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else
		p_fun = pcf7991_decode_bp;
	
	pcf7936_to_off();
	pcf7936_off_to_wait();
	basicflow(code, &ckdata[0]);
	send_nbit_cmd(scn);
	pcf7991_decode_miss_low();
	p_fun(32, &sgt[0], 5, e_dir_msbit);
	p_fun(32, &sgt[0], 32, e_dir_msbit); /*we first scn guest the scn is read pag0 command, page0 is sk_uid, that is we have known*/
	pcf7936_to_off();
	pcf7936_off_to_wait();
	basicflow(code, &ckdata[0]);
	Fixckdata(&fndata[0]);

	delay_us(PCF7936_T_WAIT_BS);
	pcf7991_write_tag_n(0);
	pcf7936_wr_msbits(rf_tx_data, 40, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();								// switching the device into transparent mode (READ_TAG mode)
	delay_us(PCF7936_T_WAIT_TR);
	pcf7991_decode_miss_low();
	if (p_fun(32, &sgt[0], 5, e_dir_msbit)) {
		p_fun(32, &sgt[0], 32, e_dir_msbit);
		*(fndata+5) = sgt[0] ^ sk_uid[0];
		*(fndata+6) = sgt[1] ^ sk_uid[1];
		*(fndata+7) = sgt[2] ^ sk_uid[2];
		*(fndata+8) = sgt[3] ^ sk_uid[3];
	}
	else 
	{
		scn = scn ^ 0x0063;// 00011_00011
		pcf7936_to_off();
		pcf7936_off_to_wait();
		basicflow(code, &ckdata[0]);
		send_nbit_cmd(scn); 
		pcf7991_decode_miss_low();
		p_fun(32, &sgt[0], 5, e_dir_msbit);
		p_fun(32, &sgt[0], 32, e_dir_msbit);
		
		pcf7936_to_off();
		pcf7936_off_to_wait();
		basicflow(code, &ckdata[0]);
		Fixckdata(&fndata[0]);

		delay_us(PCF7936_T_WAIT_BS);
		pcf7991_write_tag_n(0);
		pcf7936_wr_msbits(rf_tx_data, 40, TRUE);
		pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
		pcf7991_read_tag(); 							// switching the device into transparent mode (READ_TAG mode)
		delay_us(PCF7936_T_WAIT_TR);
		pcf7991_decode_miss_low();
		if (p_fun(32, &sgt[0], 5, e_dir_msbit)) {
			p_fun(32, &sgt[0], 32, e_dir_msbit);
			*(fndata+5) = sgt[0] ^ sk_uid[0];
			*(fndata+6) = sgt[1] ^ sk_uid[1];
			*(fndata+7) = sgt[2] ^ sk_uid[2];
			*(fndata+8) = sgt[3] ^ sk_uid[3];
		}
		else 
			state = 0xEE;
	}
	pcf7936_to_off();
	
	return state;
}

unsigned char Fixok(unsigned char code,unsigned int scnt, unsigned char *ckdata, unsigned char *fndata)
{
	unsigned int temp;
	uint8_t result;
	uint8_t (*p_fun)(uint8_t , uint8_t * , uint8_t , uint8_t );
	if (code == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else
		p_fun = pcf7991_decode_bp;
	
	pcf7936_to_off();
	pcf7936_off_to_wait();
	basicflow(code,&ckdata[0]);
	sgt[0] = sgt[0] ^ sk_uid[0] ^ 0xc1;//11000 00111	11000 00111
	sgt[1] = sgt[1] ^ sk_uid[1] ^ 0xf0;
	sgt[2] = sgt[2] ^ sk_uid[2] ^ 0x7c;
	sgt[3] = sgt[3] ^ sk_uid[3] ^ 0x1c;
	temp = scnt;
	temp<<=6;  
	rf_tx_data[0]=(unsigned char) ((temp&0xff00)>>8);
	rf_tx_data[1]=(unsigned char) (temp&0x00ff);
	rf_tx_data[1]= (rf_tx_data[1] & 0xc0) | (sgt[0]>>2);
	rf_tx_data[2]= (sgt[0]<<6) | (sgt[1]>>2);
	rf_tx_data[3]= (sgt[1]<<6) | (sgt[2]>>2);
	rf_tx_data[4]= (sgt[2]<<6) | (sgt[3]>>2);
	
	*(fndata) = rf_tx_data[0] ^ 0xC1;		  ///11000_00111 110000 0111
	*(fndata+1) = rf_tx_data[1] ^ 0xF0;
	*(fndata+2) = rf_tx_data[2] ^ 0x7C;
	*(fndata+3) = rf_tx_data[3] ^ 0x1F;
	*(fndata+4) = rf_tx_data[4] ^ 0x07;
	delay_us(PCF7936_T_WAIT_BS);
	pcf7991_write_tag_n(0);
	pcf7936_wr_msbits(rf_tx_data, 40, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();								// switching the device into transparent mode (READ_TAG mode)
	delay_us(PCF7936_T_WAIT_TR);
	pcf7991_decode_miss_low();
	if (p_fun(32, &sgt[0], 5, e_dir_msbit)) {
		p_fun(32, &sgt[0], 32, e_dir_msbit);
		*(fndata+5) = sgt[0] ^ sk_uid[0];
		*(fndata+6) = sgt[1] ^ sk_uid[1];
		*(fndata+7) = sgt[2] ^ sk_uid[2];
		*(fndata+8) = sgt[3] ^ sk_uid[3];
		result = 0x00;
	}
	else
		result = 0xee;

	pcf7936_to_off();
	return result;
}

unsigned char Fixallbit(unsigned char code, unsigned char *fndata, unsigned char *ckdata)
{
	unsigned int  i;
	unsigned int  scnt;
	unsigned char state = 0xEE;
	uint8_t (*p_fun)(uint8_t , uint8_t * , uint8_t , uint8_t );
	if (code == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else
		p_fun = pcf7991_decode_bp;
	
	scn = scn ^ 0x0318;// 11000_11000 make the command to read command
	for(i=0;i<8;i++) {
		scnt = scn ^ i;
		scnt = scnt ^ (i<<5);
		pcf7936_to_off();
		pcf7936_off_to_wait();
		if(basicflow(code, &ckdata[0]) == 0x00) {
			send_nbit_cmd(scnt);
			if (pcf7991_decode_miss_low())
				if (p_fun(32, sgt, 5, e_dir_msbit)) {
					p_fun(32, sgt, 32, e_dir_msbit);
					if (Fixok(code, scnt, &ckdata[0], &fndata[0]) == 0x00) {
						state = 0x00;
						break;
					}
				}
		}
		else {
			state = 0xEE;
			break;
		}
	}
	
	pcf7936_to_off();
	return state;
}

unsigned char Findinit (unsigned char code, unsigned char *crkdata , unsigned char *fndata)
{
    Fixtwobit(code, &crkdata[0]);
    Fixthreebit(code, &crkdata[0]);
    if(Fixfivebit(code, &fndata[0], &crkdata[0]) == 0x00)
    {
        if(Fixmore (code, &fndata[0], &crkdata[0])==0x00)
            return 0x00;
        else
            return 0xEE;
    }
    else
        return 0xEE;
}

unsigned char Findinit2 (unsigned char mode_hitag2_code, unsigned char *crkdata , unsigned char *fndata)
{
	Fixtwobit(mode_hitag2_code,&crkdata[0]);
	if(Fixallbit(mode_hitag2_code,&fndata[0],&crkdata[0]) == 0x00) {
		if(Fixmore (mode_hitag2_code,&fndata[0],&crkdata[0]) == 0x00)
			return 0x00;
		else 
			return 0xee;
	}
	else
		return 0xee;
}

uint8_t sk_check_ori(uint8_t code, const uint8_t *ckdata, uint8_t reverse_flag)
{
	uint8_t result, i;
	uint8_t a_skey[8];

	if (reverse_flag) {
		for(i = 0; i < 8; i++)
			a_skey[i] = reverse_byte(ckdata[i]);
	}
	else {
		for(i = 0; i < 8; i++)
			a_skey[i] = ckdata[i];
	}
	
	pcf7936_to_off();
	pcf7936_off_to_wait();
	if (basicflow(code, a_skey) == 0x00)
		result = TRUE;
	else
		result = FALSE;
	pcf7936_to_off();

	return result;
}

#define CRACK_PCF7936_MAX_TIME			(190)
uint8_t Fccrack46(uint8_t code, uint8_t *crkdata, uint8_t *key)
{ 
	uint8_t fndata[10], sxdata[LFE3_RECE_46_LEN], xstate[6], sgdata[7], rsdata[84];
	uint8_t result=1, i, j;

	for(i = 0; i < 8; i++)
		crkdata[i] = reverse_byte(crkdata[i]);

	if(Findinit(code, crkdata,&fndata[0]) != 0x00)                       
		if(Findinit2(code, crkdata,&fndata[0]) != 0x00)
			return FALSE;

	//lfe3_start_clock();
	SendOneck(&fndata[0],&rsdata[0]);

	lcd_print_cracking();
	#if DISPLAY_CRACK_TIME
	while (t2_tick_1s < CRACK_PCF7936_MAX_TIME) {
		//if (lfe3_sta_status) {
			//delay_us(500*1000);							// remove shake
		//	if (lfe3_sta_status)
				break;
		//}
	}
	if (t2_tick_1s < CRACK_PCF7936_MAX_TIME) {
		delay_us(2500*1000);
		INTX_DISABLE();
		//result = lfe3_rece_data(sxdata, LFE3_RECE_46_LEN);
		INTX_ENABLE();
	}
	else
		result = FALSE;
	timer2_ov_interrupt_dis();
	timer2_stop();
	#else
	 INTX_DISABLE();
	//result = lfe3_rece_data(sxdata, LFE3_RECE_46_LEN);
	INTX_ENABLE();
	#endif
	//lfe3_stop_clock();

	if (result) {
		for(j = 0; j < 7; j++)
			sgdata[j] = sxdata[j + 6];
		
		for(j = 0; j < 6; j++)
			xstate[j] = sxdata[j + 13];

		Turnback(&xstate[0], 120 + cknumber(&sgdata[0],&rsdata[0]));
		Findkey(&xstate[0], &sk_uid[0], &crkdata[0], &key[0]);

		for(i = 0; i < 6; i++)
			key[i] = reverse_byte(key[i]);
	}
	
	return result;
} 




