#include "headfile.h"

#define AT5577_T_SGAP				(15 * 8)	//start gap
#define AT5577_T_WGAP				(10 * 8)	//write gap

/* downlink data coding scheme with fixed-bit-length */
#define AT5577_T_FBL_NORMAL_D0		(24 * 8)
#define AT5577_T_FBL_NORMAL_D1		(56 * 8)
#define AT5577_T_FBL_FAST_D0		(12 * 8)
#define AT5577_T_FBL_FAST_D1		(28 * 8)

/* downlink data coding scheme with long leading reference */
#define AT5577_T_LLR_NORMAL_DREF	(160 * 8)
#define AT5577_T_LLR_NORMAL_D0		(AT5577_T_LLR_NORMAL_DREF - (136*8))
#define AT5577_T_LLR_NORMAL_D1		(AT5577_T_LLR_NORMAL_DREF - (104*8))
#define AT5577_T_LLR_FAST_DREF		(144 * 8)
#define AT5577_T_LLR_FAST_D0		(AT5577_T_LLR_FAST_DREF - (132*8))
#define AT5577_T_LLR_FAST_D1		(AT5577_T_LLR_FAST_DREF - (116*8))

/* downlink data coding scheme with leading-zero reference */
#define AT5577_T_LZR_NORMAL_DREF	(42 * 8)
#define AT5577_T_LZR_NORMAL_D0		(AT5577_T_LZR_NORMAL_DREF)
#define AT5577_T_LZR_NORMAL_D1		(AT5577_T_LZR_NORMAL_DREF + (16*8))
#define AT5577_T_LZR_FAST_DREF		(38 * 8)
#define AT5577_T_LZR_FAST_D0		(AT5577_T_LZR_FAST_DREF)
#define AT5577_T_LZR_FAST_D1		(AT5577_T_LZR_FAST_DREF + (8*8))

/* downlink data coding scheme with 1-of-4 coding */
#define AT5577_T_OOF_NORMAL_DREF	(42 * 8)
#define AT5577_T_OOF_NORMAL_D00		(AT5577_T_OOF_NORMAL_DREF)
#define AT5577_T_OOF_NORMAL_D01		(AT5577_T_OOF_NORMAL_DREF + (16*8))
#define AT5577_T_OOF_NORMAL_D10		(AT5577_T_OOF_NORMAL_DREF + (32*8))
#define AT5577_T_OOF_NORMAL_D11		(AT5577_T_OOF_NORMAL_DREF + (48*8))
#define AT5577_T_OOF_FAST_DREF		(38 * 8)
#define AT5577_T_OOF_FAST_D00		(AT5577_T_OOF_FAST_DREF)
#define AT5577_T_OOF_FAST_D01		(AT5577_T_OOF_FAST_DREF + (8*8))
#define AT5577_T_OOF_FAST_D10		(AT5577_T_OOF_FAST_DREF + (16*8))
#define AT5577_T_OOF_FAST_D11		(AT5577_T_OOF_FAST_DREF + (24*8))

/********************************************************
* function: reorganize data of reader to tag
* 
* p_data:	pointer for write data sequence(MSBit first)
* bits_add: bits of add data
* p_add:	pointer for add data(MSBit first)
********************************************************/
static void add_wrdata(at5577_wr_data_t *p_data, uint8_t bits_add, const uint8_t *p_add)
{
	uint8_t i_byte = p_data->bits >> 3;		//index of store byte
	uint8_t i_bit = p_data->bits & 0x07;	//index of store bit
	
	copy_msbits(&(p_data->a_wr[i_byte]), i_bit, p_add, 0, bits_add);
	p_data->bits += bits_add;
}

void at5577_power_control(uint8_t flag)
{
	if (flag) {
		pcf7991_active_driver();
		pcf7991_set_page(0, PCF7991_PAGE0_AT5577);
		pcf7991_read_tag();
		delay_1us(30*1000);
	}
	else {
		pcf7991_inactive_driver();
		delay_1us(30*1000);
	}
}

/* declare as variable, can change for test special times */
static uint16_t t_fbl_normal_d0 = AT5577_T_FBL_NORMAL_D0;
static uint16_t t_fbl_normal_d1 = AT5577_T_FBL_NORMAL_D1;

/********************************************************
* function: set fixed-bit-length times
* 
* d0: bit 0 period(actual (d0 * 8)us)
* d1: bit 1 period(actual (d1 * 8)us)
********************************************************/
static void at5577_set_t_fbl(uint8_t d0, uint8_t d1)
{
	t_fbl_normal_d0 = (uint16_t)d0 << 3;
	t_fbl_normal_d1 = (uint16_t)d1 << 3;
}

/********************************************************
* A start gap will be accepted at any time after the mode register has been loaded (>=3ms)

* protocol: downlink coding type, four types
* fast:		enable fast downlink
* bits_wr:	downlink bits
* p_wr:		pointer for donwlink data(MSBit first)
* recharge_ms: driver on times after downlink complete
********************************************************/
void at5577_downlink(at5577_protocol_t protocol, uint8_t fast, 
				uint8_t bits_wr, const uint8_t *p_wr, uint8_t recharge_ms)
{
		uint8_t i, byte=0;
	pcf7991_driver_off();
	delay_1us(AT5577_T_SGAP);
	/* fixed-bit-length protocol */
	if (protocol == e_fbl) {
		for (i = 0; i < bits_wr; i++) {
			if ((i % 8) == 0)
				byte = *p_wr++;
		
			pcf7991_driver_on();
			if (byte & 0x80) {
				if (fast)
					delay_1us(AT5577_T_FBL_FAST_D1);
				else
					delay_1us(t_fbl_normal_d1);
			}
			else {
				if (fast)
					delay_1us(AT5577_T_FBL_FAST_D0);
				else
					delay_1us(t_fbl_normal_d0);
			}
			
			pcf7991_driver_off();
			delay_1us(AT5577_T_WGAP);
			
			byte <<= 1;
		}
	}
	
	/* long-leading-reference protocol */
	else if (protocol == e_llr) {
		pcf7991_driver_on();
		if (fast)
			delay_1us(AT5577_T_LLR_FAST_DREF);
		else
			delay_1us(AT5577_T_LLR_NORMAL_DREF);
		pcf7991_driver_off();
		delay_1us(AT5577_T_WGAP);

		for (i = 0; i < bits_wr; i++) {
			if ((i % 8) == 0)
				byte = *p_wr++;

			pcf7991_driver_on();
			if (byte & 0x80) {
				if (fast)
					delay_1us(AT5577_T_LLR_FAST_D1);
				else
					delay_1us(AT5577_T_LLR_NORMAL_D1);
			}
			else {
				if (fast)
					delay_1us(AT5577_T_LLR_FAST_D0);
				else
					delay_1us(AT5577_T_LLR_NORMAL_D0);
			}
			
			pcf7991_driver_off();
			delay_1us(AT5577_T_WGAP);
			
			byte <<= 1;
		}
	}
	
	/* leading-zero-reference protocol */
	else if (protocol == e_lzr) {
		pcf7991_driver_on();
		if (fast)
			delay_1us(AT5577_T_LZR_FAST_DREF);
		else
			delay_1us(AT5577_T_LZR_NORMAL_DREF);
		pcf7991_driver_off();
		delay_1us(AT5577_T_WGAP);
		
		for (i = 0; i < bits_wr; i++) {
			if ((i % 8) == 0)
				byte = *p_wr++;
		
			pcf7991_driver_on();
			if (byte & 0x80) {
				if (fast)
					delay_1us(AT5577_T_LZR_FAST_D1);
				else
					delay_1us(AT5577_T_LZR_NORMAL_D1);
			}
			else {
				if (fast)
					delay_1us(AT5577_T_LZR_FAST_D0);
				else
					delay_1us(AT5577_T_LZR_NORMAL_D0);
			}
			
			pcf7991_driver_off();
			delay_1us(AT5577_T_WGAP);
			
			byte <<= 1;
		}
	}
	
	/* 1-of-4-coding protocol */
	else if (protocol == e_oof) {
		pcf7991_driver_on();
		if (fast)
			delay_1us(AT5577_T_OOF_FAST_DREF);
		else
			delay_1us(AT5577_T_OOF_NORMAL_DREF);
		pcf7991_driver_off();
		delay_1us(AT5577_T_WGAP);
		
		for (i = 0; i < bits_wr; i += 2) {
			if ((i % 8) == 0)
				byte = *p_wr++;
		
			pcf7991_driver_on();
			switch (byte & 0xC0) {
				case 0x00:
					if (fast)
						delay_1us(AT5577_T_OOF_FAST_D00);
					else
						delay_1us(AT5577_T_OOF_NORMAL_D00);
					break;
				case 0x40:
					if (fast)
						delay_1us(AT5577_T_OOF_FAST_D01);
					else
						delay_1us(AT5577_T_OOF_NORMAL_D01);
					break;
				case 0x80:
					if (fast)
						delay_1us(AT5577_T_OOF_FAST_D10);
					else
						delay_1us(AT5577_T_OOF_NORMAL_D10);
					break;
				case 0xC0:
					if (fast)
						delay_1us(AT5577_T_OOF_FAST_D11);
					else
						delay_1us(AT5577_T_OOF_NORMAL_D11);
					break;
				default :
					break;
			}
			
			pcf7991_driver_off();
			delay_1us(AT5577_T_WGAP);
			
			byte <<= 2;
		}
	}
	
	pcf7991_driver_on();
	delay_1us(recharge_ms*1000);
}

uint8_t at5577_reader_to_tag(at5577_protocol_t protocol, uint8_t fast, const at5577_wr_data_t *p_wr)
{
	at5577_power_control(TRUE);
	pcf7991_write_tag_n(0);
	at5577_downlink(protocol, fast, p_wr->bits, p_wr->a_wr, 0);
	pcf7991_set_page(0x00, PCF7991_PAGE0_AT5577);
	pcf7991_read_tag();
	delay_1us(300*1000);
	at5577_power_control(FALSE);

	return FALSE;
}

static uint8_t cmd_standard_wr(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* lock bit */
	temp = (p_data[1] > 0) ? 0x80 : 0x00;
	add_wrdata(&a_wr, 1, &temp);
	
	/* write data */
	add_wrdata(&a_wr, 32, &p_data[2]);
	
	/* address */
	temp = p_data[6] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_protect_wr(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	if ((protocol == e_lzr) || (protocol == e_oof))
		add_wrdata(&a_wr, 2, &temp);

	/* password */
	add_wrdata(&a_wr, 32, &p_data[1]);
	
	/* lock bit */
	temp = (p_data[5] > 0) ? 0x80 : 0x00;
	add_wrdata(&a_wr, 1, &temp);
	
	/* write data */
	add_wrdata(&a_wr, 32, &p_data[6]);
	
	/* address */
	temp = p_data[10] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_aor(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	if ((protocol == e_lzr) || (protocol == e_oof))
		add_wrdata(&a_wr, 2, &temp);

	/* password */
	add_wrdata(&a_wr, 32, &p_data[1]);

	at5577_reader_to_tag(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_pwd_access(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	if ((protocol == e_lzr) || (protocol == e_oof))
		add_wrdata(&a_wr, 2, &temp);

	/* password */
	add_wrdata(&a_wr, 32, &p_data[1]);
	
	/* padding zero */
	add_wrdata(&a_wr, 1, &temp);
	
	/* address */
	temp = p_data[5] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_direct_access(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	add_wrdata(&a_wr, 1, &temp);
	
	/* address */
	temp = p_data[1] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_regular_read(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	at5577_reader_to_tag(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_reset(at5577_protocol_t protocol)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = 0x00;
	add_wrdata(&a_wr, 2, &temp);
	
	at5577_reader_to_tag(protocol, FALSE, &a_wr);
	return FALSE;
}

uint8_t at5577_reader_to_tag_1(at5577_protocol_t protocol, uint8_t fast, const at5577_wr_data_t *p_wr)
{
//	led_flicker();
	
	pcf7991_write_tag_n(0);
	at5577_downlink(protocol, fast, p_wr->bits, p_wr->a_wr, 0);
	pcf7991_set_page(0x00, PCF7991_PAGE0_AT5577);
	pcf7991_read_tag();
	delay_1us(300*1000);
	
//	led_flicker();

	return FALSE;
}


static uint8_t cmd_standard_wr_1(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* lock bit */
	temp = (p_data[1] > 0) ? 0x80 : 0x00;
	add_wrdata(&a_wr, 1, &temp);
	
	/* write data */
	add_wrdata(&a_wr, 32, &p_data[2]);
	
	/* address */
	temp = p_data[6] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag_1(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_protect_wr_1(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	if ((protocol == e_lzr) || (protocol == e_oof))
		add_wrdata(&a_wr, 2, &temp);

	/* password */
	add_wrdata(&a_wr, 32, &p_data[1]);
	
	/* lock bit */
	temp = (p_data[5] > 0) ? 0x80 : 0x00;
	add_wrdata(&a_wr, 1, &temp);
	
	/* write data */
	add_wrdata(&a_wr, 32, &p_data[6]);
	
	/* address */
	temp = p_data[10] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag_1(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_aor_1(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	if ((protocol == e_lzr) || (protocol == e_oof))
		add_wrdata(&a_wr, 2, &temp);

	/* password */
	add_wrdata(&a_wr, 32, &p_data[1]);

	at5577_reader_to_tag_1(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_pwd_access_1(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	if ((protocol == e_lzr) || (protocol == e_oof))
		add_wrdata(&a_wr, 2, &temp);

	/* password */
	add_wrdata(&a_wr, 32, &p_data[1]);
	
	/* padding zero */
	add_wrdata(&a_wr, 1, &temp);
	
	/* address */
	temp = p_data[5] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag_1(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_direct_access_1(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	/* padding zero */
	temp = 0x00;
	add_wrdata(&a_wr, 1, &temp);
	
	/* address */
	temp = p_data[1] << 5;
	add_wrdata(&a_wr, 3, &temp);
	
	at5577_reader_to_tag_1(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_regular_read_1(at5577_protocol_t protocol, const uint8_t *p_data)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = (p_data[0] > 0) ? 0xC0 : 0x80;
	add_wrdata(&a_wr, 2, &temp);
	
	at5577_reader_to_tag_1(protocol, FALSE, &a_wr);
	return FALSE;
}

static uint8_t cmd_reset_1(at5577_protocol_t protocol)
{
	uint8_t temp;
	at5577_wr_data_t a_wr;
	
	a_wr.bits = 0;
	/* opcode */
	temp = 0x00;
	add_wrdata(&a_wr, 2, &temp);
	
	at5577_reader_to_tag_1(protocol, FALSE, &a_wr);
	return FALSE;
}

uint8_t at5577_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send;
	
	len_send = 1;
	p_answer[0] = 0x00;

	switch (p_ask[0]) {
		case 0x01:
			cmd_standard_wr((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x02:
			cmd_protect_wr((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x03:
			cmd_aor((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x04:
			cmd_pwd_access((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x05:
			cmd_direct_access((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x06:
			cmd_regular_read((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x07:
			cmd_reset((at5577_protocol_t)p_ask[1]);
			break;
		
		case 0x11:
			cmd_standard_wr_1((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x12:
			cmd_protect_wr_1((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x13:
			cmd_aor_1((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x14:
			cmd_pwd_access_1((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x15:
			cmd_direct_access_1((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x16:
			cmd_regular_read_1((at5577_protocol_t)p_ask[1], &p_ask[2]);
			break;
		case 0x17:
			cmd_reset_1((at5577_protocol_t)p_ask[1]);
			break;
			
		case 0x21:
			at5577_power_control(p_ask[1]);
			p_answer[0] = p_ask[0];
			break;
		
		case 0x22:
			at5577_power_control(TRUE);
			delay_1us(p_ask[1]*1000);
			at5577_power_control(FALSE);
			p_answer[0] = p_ask[0];
			break;
		
		case 0x31:
			at5577_set_t_fbl(p_ask[1], p_ask[2]);
			p_answer[0] = p_ask[0];
			break;
		
		default:
			break;
	}
	
	return len_send;
}

e5561_tag_t e5561_tag1;

void e5561_off_to_read(void)
{
	pcf7991_active_driver();
	delay_1us(10);
	pcf7991_set_page(0, PCF7991_PAGE0_E5561);
	delay_1us(10);
	pcf7991_read_tag();
	delay_1us(10);

	delay_1us(15*1000);
}

void e5561_to_off(void)
{
	pcf7991_inactive_driver();
	delay_1us(5*1000);
}

void e5561_wr_lsbits(uint8_t start_gap, const uint8_t *p_wr, uint8_t bits, uint8_t eot)
{
	uint8_t i;
	if (start_gap) {
		pcf7991_driver_off();
		delay_1us(E5561_T_SGAP);
	}

	for (i = 0; i < bits; i++) {
		uint8_t byte=0;
		if ((i % 8) == 0)
			byte = *p_wr++;

		pcf7991_driver_on();
		if (byte & 0x01)
			delay_1us(E5561_T_BIT1);
		else
			delay_1us(E5561_T_BIT0);
		
		pcf7991_driver_off();
		delay_1us(E5561_T_WGAP - 2);
		
		byte >>= 1;
	}

	if (eot) {
		pcf7991_driver_on();
		delay_1us(E5561_T_EOT);
	}
}

uint8_t e5561_find_termi1(uint8_t *p_periods)
{
		uint8_t result;
	uint16_t tcnt_pre=0;
	const uint16_t eighth = ((uint16_t)(*p_periods) >> 2);
	enum {
		s_miss_high,
		s_low1_15, s_high2_05, s_low3_05,
		s_exit
	} state;
	switch (*p_periods) {
		case 32:
		case 64:
			break;
		default:
			return FALSE;
	}
	

	
	timer1_stop();
	timer1_clear_ov();
	TCNT1 = /*65536 - */((uint16_t)(*p_periods) << 9);		// 128 bits, 2times
	timer1_start_64();

	result = FALSE;
	state = s_miss_high;
	while ((state != s_exit) && (timer1_ov_flag == 0)) {
		switch (state) {
			case s_miss_high:
				if (pcf7991_dout_level == 0) {
					tcnt_pre = TCNT1;
					state = s_low1_15;
				}
				break;
			case s_low1_15:
				if (pcf7991_dout_level) {
					if (((TCNT1-tcnt_pre)>=(eighth*10)) && ((TCNT1-tcnt_pre)<=(eighth*14))) {
						tcnt_pre = TCNT1;
						state = s_high2_05;
					}
					else
						state = s_miss_high;
				}
				break;
			case s_high2_05:
				if (pcf7991_dout_level == 0) {
					if (((TCNT1-tcnt_pre)>=(eighth*2)) && ((TCNT1-tcnt_pre)<=(eighth*6)))
						state = s_low3_05;
					else
						state = s_low1_15;
					tcnt_pre = TCNT1;
				}
				break;
			case s_low3_05:
				if (pcf7991_dout_level) {
					if ((TCNT1-tcnt_pre) <= (eighth<<3)) {
						result = TRUE;
						state = s_exit;
					}
					else
						state = s_miss_high;
				}
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	return result;
}

uint8_t e5561_find_termi2(uint8_t *p_periods)
{
	// todo
	return FALSE;
}

uint8_t e5561_decide_termi(e5561_tag_t *p_tag)
{
	uint8_t result, a_period[2];

	a_period[0] = 64;
	a_period[1] = 32;
	result = TRUE;
	e5561_off_to_read();
	if (e5561_find_termi1(&a_period[0])) {
		p_tag->termi = 1;
		p_tag->period = a_period[0];
	}
	else if (e5561_find_termi1(&a_period[1])) {
		p_tag->termi = 1;
		p_tag->period = a_period[1];
	}
	else if (e5561_find_termi2(&(p_tag->period))) {
		p_tag->termi = 2;
	}
	else {
		p_tag->termi = 3;
		result = FALSE;
	}
	e5561_to_off();

	return result;
}

uint8_t e5561_find_id(e5561_tag_t *p_tag)
{
	uint8_t (*pf_termi)(uint8_t *);
	uint8_t (*pf_decode)(uint8_t , uint8_t *, uint8_t , uint8_t);

	if (p_tag->termi == 1)
		pf_termi = e5561_find_termi1;
	else if (p_tag->termi == 2)
		pf_termi = e5561_find_termi2;
	else
		return FALSE;

	if (p_tag->modu == e_modu_mc)
		pf_decode = pcf7991_decode_mc;
	else if (p_tag->modu == e_modu_bp)
		pf_decode = pcf7991_decode_bp;
	else
		return FALSE;

	/* starting with the least significant bit */
	if (pf_termi(&(p_tag->period)))
		if (pf_decode(p_tag->period, p_tag->a_block[1], p_tag->id_bits, e_dir_lsbit))
			return TRUE;

	return FALSE;
}

uint8_t e5561_decide_id(e5561_tag_t *p_tag)
{
	uint8_t result = FALSE;

	e5561_off_to_read();
	/* 128bits, bp */
	p_tag->id_bits = 128;
	p_tag->modu = e_modu_bp;
	if (e5561_find_id(p_tag))
		result = TRUE;
	
	/* 128bits, mc */
	if (!result) {
		p_tag->modu = e_modu_mc;
		if (e5561_find_id(p_tag))
			result = TRUE;
	}
	
	/* 64bits, mc */
	if (!result) {
		p_tag->id_bits = 64;
		if (e5561_find_id(p_tag))
			result = TRUE;
	}

	/* 64bits, bp, mazda */
	if (!result) {
		p_tag->modu = e_modu_bp;
		if (e5561_find_id(p_tag))
			result = TRUE;
	}
	e5561_to_off();
	
	return result;
}

uint8_t e5561_program(modulation_t modu, uint8_t period, uint8_t addr, const uint8_t *p_wr)
{
	uint8_t result, cmd, a_rd[5];
	uint8_t (*pf_decode)(uint8_t, uint8_t *, uint8_t, uint8_t);

	if (modu == e_modu_mc)
		pf_decode = pcf7991_decode_mc;
	else if (modu == e_modu_bp)
		pf_decode = pcf7991_decode_bp;
	else
		return FALSE;

	result = FALSE;
	cmd = E5561_OPCODE_RW;
	e5561_off_to_read();
	pcf7991_write_tag_n(0);
	e5561_wr_lsbits(TRUE, &cmd, 2, FALSE);
	e5561_wr_lsbits(FALSE, p_wr, 32, FALSE);
	e5561_wr_lsbits(FALSE, &addr, 4, TRUE);
	pcf7991_set_page(0, PCF7991_PAGE0_E5561);
	pcf7991_read_tag();
	delay_1us(E5561_T_PROGRAM);

	/*an Fh preburst followed by terminator 1. After that, the data just 
	programmed is read out of the EEPROM and sent in loop with terminator 1. */
	result = FALSE;
	if (!e5561_find_termi1(&period))
		;
	else if (!pf_decode(period, a_rd, 32, e_dir_lsbit))
		;
	else if (!equal_bytes(p_wr, a_rd, 4))
		;
	else
		result = TRUE;

	e5561_to_off();
	return result;
}

uint8_t e5561_program_no_power(modulation_t modu, uint8_t period, uint8_t addr, const uint8_t *p_wr)
{
	uint8_t result, cmd, a_rd[5];
	uint8_t (*pf_decode)(uint8_t, uint8_t *, uint8_t, uint8_t);

	if (modu == e_modu_mc)
		pf_decode = pcf7991_decode_mc;
	else if (modu == e_modu_bp)
		pf_decode = pcf7991_decode_bp;
	else
		return FALSE;

	result = FALSE;
	cmd = E5561_OPCODE_RW;
	pcf7991_write_tag_n(0);
	e5561_wr_lsbits(TRUE, &cmd, 2, FALSE);
	e5561_wr_lsbits(FALSE, p_wr, 32, FALSE);
	e5561_wr_lsbits(FALSE, &addr, 4, TRUE);
	pcf7991_set_page(0, PCF7991_PAGE0_E5561);
	pcf7991_read_tag();
	delay_1us(E5561_T_PROGRAM);

	/*an Fh preburst followed by terminator 1. After that, the data just 
	programmed is read out of the EEPROM and sent in loop with terminator 1. */
	result = FALSE;
	if (!e5561_find_termi1(&period))
		;
	else if (!pf_decode(period, a_rd, 32, e_dir_lsbit))
		;
	else if (!equal_bytes(p_wr, a_rd, 4))
		;
	else
		result = TRUE;

	return result;
}

uint8_t e5561_access(modulation_t modu, uint8_t period, uint8_t addr, uint8_t *p_rd)
{
	uint8_t result, cmd;
	uint8_t (*pf_decode)(uint8_t, uint8_t *, uint8_t, uint8_t);

	if (modu == e_modu_mc)
		pf_decode = pcf7991_decode_mc;
	else if (modu == e_modu_bp)
		pf_decode = pcf7991_decode_bp;
	else
		return FALSE;

	cmd = E5561_OPCODE_RW;
	e5561_off_to_read();
	pcf7991_write_tag_n(0);
	e5561_wr_lsbits(TRUE, &cmd, 2, FALSE);
	e5561_wr_lsbits(FALSE, &addr, 4, TRUE);
	pcf7991_set_page(0, PCF7991_PAGE0_E5561);
	pcf7991_read_tag();
	delay_1us(E5561_T_ACCESS);

	/*an FFh preburst is sent followed by terminator 1. After that, 
	the addressed block and terminator 1 are sent in loop.*/
	result = FALSE;
	if (!e5561_find_termi1(&period))
		;
	else if (!pf_decode(period, p_rd, 32, e_dir_lsbit))
		;
	else
		result = TRUE;

	e5561_to_off();
	return result;
}

uint8_t e5561_access_no_power(modulation_t modu, uint8_t period, uint8_t addr, uint8_t *p_rd)
{
	uint8_t result, cmd;
	uint8_t (*pf_decode)(uint8_t, uint8_t *, uint8_t, uint8_t);

	if (modu == e_modu_mc)
		pf_decode = pcf7991_decode_mc;
	else if (modu == e_modu_bp)
		pf_decode = pcf7991_decode_bp;
	else
		return FALSE;

	cmd = E5561_OPCODE_RW;
	pcf7991_write_tag_n(0);
	e5561_wr_lsbits(TRUE, &cmd, 2, FALSE);
	e5561_wr_lsbits(FALSE, &addr, 4, TRUE);
	pcf7991_set_page(0, PCF7991_PAGE0_E5561);
	pcf7991_read_tag();
	delay_1us(E5561_T_ACCESS);

	/*an FFh preburst is sent followed by terminator 1. After that, 
	the addressed block and terminator 1 are sent in loop.*/
	result = FALSE;
	if (!e5561_find_termi1(&period))
		;
	else if (!pf_decode(period, p_rd, 32, e_dir_lsbit))
		;
	else
		result = TRUE;

	return result;
}

/*Once the crypto key is locked, it can not be overwritten or read out anymore
with direct-access mode.
the e5561 transmits the ID code to identify itself. Thereby, the base station 
can identify the transponder and knows which crypto key to use. */
uint8_t e5561_auth(modulation_t modu, uint8_t period, const uint8_t *p_wr, uint8_t *p_rd)
{
	uint8_t result, cmd;
	uint8_t (*pf_decode)(uint8_t, uint8_t *, uint8_t, uint8_t);

	if (modu == e_modu_mc)
		pf_decode = pcf7991_decode_mc;
	else if (modu == e_modu_bp)
		pf_decode = pcf7991_decode_bp;
	else
		return FALSE;

	cmd = E5561_OPCODE_CRYPTO;
	e5561_off_to_read();
	pcf7991_write_tag_n(0);
	e5561_wr_lsbits(TRUE, &cmd, 2, FALSE);
	e5561_wr_lsbits(FALSE, p_wr, 64, TRUE);
	pcf7991_set_page(0, PCF7991_PAGE0_E5561);
	pcf7991_read_tag();
	delay_1us(E5561_T_AUTH);

	result = FALSE;
	if (!e5561_find_termi1(&period))
		;
	else if (!pf_decode(period, p_rd, 32, e_dir_lsbit))
		;
	else
		result = TRUE;

	delay_1us(80);
	e5561_to_off();
	return result;
}


/* this function is no good, todo modify*/
uint8_t e5561_copy_e5561(const e5561_tag_t *p_tag)
{
	uint8_t result;
	const uint8_t a_psw[4] = {0x58, 0xF4, 0x0B, 0xC8};	// see tango copy wave
	uint8_t a_config[4] = {0x17, 0x00, 0x00, 0x00};

	e5561_off_to_read();
	
	delay_1us(30);
	e5561_program_no_power(p_tag->modu, p_tag->period, 9, &a_psw[0]);
	delay_1us(30);
	e5561_program_no_power(p_tag->modu, p_tag->period, 0, &a_config[0]);
	delay_1us(30);
	e5561_program_no_power(p_tag->modu, p_tag->period, 1, p_tag->a_block[1]);
	delay_1us(30);
	e5561_program_no_power(p_tag->modu, p_tag->period, 4, p_tag->a_block[2]);
	delay_1us(30);
	result = FALSE;
	if (!e5561_access_no_power(p_tag->modu, p_tag->period, 4, &a_config[0]))
		;
	else if (!equal_bytes(&a_config[0], p_tag->a_block[2], 4))
		;
	else
		result = TRUE;
	
	e5561_to_off();

	return result;
}

uint8_t e5561_find_tag(e5561_tag_t *p_tag)
{
	uint8_t result;

	result = FALSE;
	p_tag->mode = e_e5561_not;
	if (!e5561_decide_termi(p_tag))
		;
	else if (!e5561_decide_id(p_tag))
		;
	else {
		p_tag->mode = e_e5561_yes;
		result = TRUE;
	}

	return result;
}

uint8_t e5561_keypad_interaction(e5561_tag_t *p_tag)
{
		uint8_t key;
		enum {
		s_show_id1, s_show_id2, s_show_copy_tag,
		s_copying, s_copy_ok, s_copy_ng,
		s_exit
	} state;
	if (p_tag->mode == e_e5561_not)
		return FALSE;


	state = s_show_id1;
	while (state != s_exit) {
		switch (state) {
			case s_show_id1:
				lcd_clear_screen();
				lcd_print_str(0, 0, str_8c_tag, 6);
				delay_1us(T_DISPLAY);

				lcd_print_hex(30, 40,200,24,24, p_tag->a_block[1], 8);
				if (p_tag->id_bits == 128)
					lcd_print_str(0, 1, str_read_r, 6);
				else
					lcd_print_str(0, 1, str_exit_r, 6);
				lcd_print_str(8, 1, str_copy_c, 6);

				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_WRITE)
					state = s_show_copy_tag;
				else if (p_tag->id_bits == 128)
					state = s_show_id2;
				else
					state = s_exit;
				break;
				
			case s_show_id2:
				lcd_print_hex(30, 40,200,24,24, p_tag->a_block[3], 8);
				lcd_print_str(0, 1, str_exit_r, 6);
				
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_WRITE)
					state = s_show_copy_tag;
				else
					state = s_exit;
				break;

			case s_show_copy_tag:
				lcd_print_insert();
				lcd_print_str(8, 0, str_new_8ctag, 8);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_copying;
				break;
			case s_copying:
				lcd_print_copying();
				if (e5561_copy_e5561(p_tag))
					state = s_copy_ok;
				else
					state = s_copy_ng;
				break;
			case s_copy_ok:
			case s_copy_ng:
				lcd_clear_screen();
				if (state == s_copy_ok)
					lcd_print_str(0, 0, str_copy_ok, 8);
				else
					lcd_print_str(0, 0, str_copy_ng, 8);
				lcd_print_str(0, 1, str_exit_r, 6);
				lcd_print_str(8, 1, str_copy_c, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_show_copy_tag;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return TRUE;
}

uint8_t e5561_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send;
	
	len_send = 1;
	p_answer[0] = 0x00;

	switch (p_ask[0]) {
		case 0x01:
			if (e5561_access((modulation_t)p_ask[1], p_ask[2], p_ask[3], &p_answer[1])) {
				len_send += 4;
				p_answer[0] = p_ask[0];
			}
			break;

		case 0x02:
			if (e5561_program((modulation_t)p_ask[1], p_ask[2], p_ask[3], &p_ask[4])) {
				p_answer[0] = p_ask[0];
			}
			break;
			
		case 0x03:
			if (e5561_auth((modulation_t)p_ask[1], p_ask[2], &p_ask[3], &p_answer[1])) {
				len_send += 4;
				p_answer[0] = p_ask[0];
			}
			break;
		
		case 0x11:
			if (e5561_access_no_power((modulation_t)p_ask[1], p_ask[2], p_ask[3], &p_answer[1])) {
				len_send += 4;
				p_answer[0] = p_ask[0];
			}
			break;
		
		case 0x12:
			if (e5561_program_no_power((modulation_t)p_ask[1], p_ask[2], p_ask[3], &p_ask[4])) {
				p_answer[0] = p_ask[0];
			}
			break;
			
		case 0x21:
			if (p_ask[1] > 0)
				e5561_off_to_read();
			else
				e5561_to_off();
			break;
			
		default:
			break;
	}
	
	return len_send;
}


#if 0
********** 8C-mazda1 **********
17 01 00 00

F0 00 B6 A1
AA AA AA AA
FF FF FF FF
F0 00 B6 A1

-- -- -- --
-- -- -- --
-- -- -- --
-- -- -- --

07 00 00 00
********** 8C-mazda2 **********
17 01 00 00

14 1F B0 94
AA AA AA AA
FF FF FF FF
14 1F B0 94

-- -- -- --
-- -- -- --
-- -- -- --
-- -- -- --

07 00 00 00
********** 8C-mazda3 **********
17 01 00 00

C4 03 9A A2
AA AA AA AA
FF FF FF FF
C4 03 9A A2

-- -- -- --
-- -- -- --
-- -- -- --
-- -- -- --

07 00 00 00
********** 8C-mazda4 **********
17 01 00 00

4E 07 92 9A
AA AA AA AA
FF FF FF FF
4E 07 92 9A

-- -- -- --
-- -- -- --
-- -- -- --
-- -- -- --

07 00 00 00
********** 8C-mazda5 **********
17 01 00 00

32 18 B6 9A
AA AA AA AA
FF FF FF FF
32 18 B6 9A

-- -- -- --
-- -- -- --
-- -- -- --
-- -- -- --

07 00 00 00
********** 8C-mazda6 **********
17 01 00 00

94 17 B6 9A
AA AA AA AA
FF FF FF FF
94 17 B6 9A

-- -- -- --
-- -- -- --
-- -- -- --
-- -- -- --

07 00 00 00
********** 8C-mazda7 **********
17 01 00 00

42 14 B6 9A
AA AA AA AA
FF FF FF FF
42 14 B6 9A

-- -- -- --
-- -- -- --
-- -- -- --
-- -- -- --

07 00 00 00
#endif



