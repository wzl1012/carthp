#ifndef   _LCD_H_
#define   _LCD_H_
#include "gd32f10x.h"
#define LCD_ROW_LENGTH				(21)
#define LCD_COLUMN_LENGTH			(2)
#ifdef STR_DEFINE
#define STR_EXT
#else
#define STR_EXT		extern
#endif

void lcd_print_hex(uint8_t x, uint8_t y,uint16_t width,uint16_t height,uint8_t size, const uint8_t *p_hex, uint8_t bytes);
void lcd_print_bcd(uint8_t x, uint8_t y, uint8_t byte);
void lcd_print_ready_press(void);
void lcd_print_ng_press(void);
void lcd_print_reading(void);
void lcd_print_writing(void);
void lcd_print_cracking(void);
void lcd_print_crack_end(void);
//void lcd_print_insert(void);
void lcd_print_copying(void);
void lcd_print_blank(uint8_t y);
extern void hex_to_char(const uint8_t *p_hex, uint8_t *p_ch, uint8_t bytes);
STR_EXT  uint8_t str_ready_rd[LCD_ROW_LENGTH]
#if STR_ENGLISH
 = "Read Ready   "
#elif STR_CHINESE
 = "????        "
#endif
;
STR_EXT const uint8_t str_press_rd[LCD_ROW_LENGTH]
#if STR_ENGLISH
 = "Press checkkey again"
#elif STR_CHINESE
 = "?????      "
#endif
;
STR_EXT const uint8_t str_r_is_press_read[16]
#if STR_ENGLISH
 = "/R is Press READ"
#elif STR_CHINESE
 = "/R???READ?  "
#endif
;
STR_EXT const uint8_t str_c_is_press_copy[16]
#if STR_ENGLISH
 = "/C is Press COPY"
#elif STR_CHINESE
 = "/C???COPY?  "
#endif
;
STR_EXT const uint8_t str_read_r[6]
#if STR_ENGLISH
 = "Read/R"
#elif STR_CHINESE
 = "??/R"
#endif
;
STR_EXT  uint8_t str_exit_r[6]
#if STR_ENGLISH
 = "Exit/R"
#elif STR_CHINESE
 = "??/R"
#endif
;
STR_EXT const uint8_t str_exit_c[6]
#if STR_ENGLISH
 = "Exit/C"
#elif STR_CHINESE
 = "??/C"
#endif
;
STR_EXT  uint8_t str_are_crack[10]
#if STR_ENGLISH
 = "Are Crack?"
#elif STR_CHINESE
 = "????? "
#endif
;
STR_EXT uint8_t str_crack_c[7]
#if STR_ENGLISH
 = "Crack/C"
#elif STR_CHINESE
 = "??/C "
#endif
;
STR_EXT  uint8_t str_copy_c[6]
#if STR_ENGLISH
 = "Copy/C"
#elif STR_CHINESE
 = "??/C"
#endif
;
STR_EXT  uint8_t str_next_r[6]
#if STR_ENGLISH
 = "Next/R"
#elif STR_CHINESE
 = "??/R"
#endif
;
STR_EXT  uint8_t str_copy_sniff_data[14]
#if STR_ENGLISH
 = "CopySniffData?"
#elif STR_CHINESE
 = "??????? "
#endif
;
STR_EXT  uint8_t str_dot8[LCD_ROW_LENGTH]
#if STR_ENGLISH
 = "........    "
#elif STR_CHINESE
 = "........    "
#endif
;
STR_EXT  uint8_t str_reading[LCD_ROW_LENGTH]
#if STR_ENGLISH
 = "Reading....."
#elif STR_CHINESE
 = "???          "
#endif
;
STR_EXT  uint8_t str_writing[LCD_ROW_LENGTH]
#if STR_ENGLISH
 = "Writing....."
#elif STR_CHINESE
 = "???          "
#endif
;
STR_EXT  uint8_t str_read_ng[LCD_ROW_LENGTH]
#if STR_ENGLISH
 //= "Read Not        "
 = "Not check it"
#elif STR_CHINESE
 = "????        "
#endif
;
STR_EXT  uint8_t str_cracking[8]
#if STR_ENGLISH
 = "Cracking"
#elif STR_CHINESE
 = "???  "
#endif
;
STR_EXT  uint8_t str_crack_ok[8]
#if STR_ENGLISH
 = "Crack OK"
#elif STR_CHINESE
 = "????"
#endif
;
STR_EXT  uint8_t str_crack_no[32]
#if STR_ENGLISH
 = "Crack no. Recopy ID and Sniff   "
#elif STR_CHINESE
 = "????. ?????ID?????"
#endif
;
STR_EXT  uint8_t str_time[9]
#if STR_ENGLISH
 = "Time: 000"
#elif STR_CHINESE
 = "??: 000"
#endif
;
STR_EXT  uint8_t str_insert[6]
#if STR_ENGLISH
 = "Insert"
#elif STR_CHINESE
 = "???"
#endif
;
STR_EXT uint8_t str_copying[7]
#if STR_ENGLISH
 = "Copying"
#elif STR_CHINESE
 = "??? "
#endif
;
STR_EXT  uint8_t str_copy_ok[8]
#if STR_ENGLISH
 = "Copy OK "
#elif STR_CHINESE
 = "????"
#endif
;
STR_EXT  uint8_t str_copy_ng[8]
#if STR_ENGLISH
 = "Copy Not"
#elif STR_CHINESE
 = "????"
#endif
;

/////////////////////////////////////////////////// string_pcf7936
STR_EXT  uint8_t str_46_tag[LCD_ROW_LENGTH]
#if STR_ENGLISH
 = "46Tag              "
#elif STR_CHINESE
 = "46??"
#endif
;
STR_EXT  uint8_t str_new_46[6]
#if STR_ENGLISH
 = "New 46"
#elif STR_CHINESE
 = "??46"
#endif
;
STR_EXT  uint8_t str_blank[5]
#if STR_ENGLISH
 = "Blank"
#elif STR_CHINESE
 = "?? "
#endif
;
STR_EXT  uint8_t str_locked[6]
#if STR_ENGLISH
 = "Unlock"
#elif STR_CHINESE
 = "??? "
#endif
;
STR_EXT  uint8_t str_sniff_car_data[14]
#if STR_ENGLISH
 = "Sniff Car Data"
#elif STR_CHINESE
 = "??????  "
#endif
;

/////////////////////////////////////////////////// string_ts06
STR_EXT  uint8_t str_insert_ori_key[14]
#if STR_ENGLISH
 = "Insert Ori Key"
#elif STR_CHINESE
 = "??????  "
#endif
;

/////////////////////////////////////////////////// string_tms37145
STR_EXT  uint8_t str_tag[3]
#if STR_ENGLISH
 = "Tag"
#elif STR_CHINESE
 = "??"
#endif
;
STR_EXT  uint8_t str_4d_tag[6]
#if STR_ENGLISH
 = "4DTag "
#elif STR_CHINESE
 = "4D??"
#endif
;
STR_EXT  uint8_t str_63_tag[6]
#if STR_ENGLISH
 = "4D(63)"
#elif STR_CHINESE
 = "4D(63)"
#endif
;
STR_EXT  uint8_t str_64_tag[6]
#if STR_ENGLISH
 = "4D(64)"
#elif STR_CHINESE
 = "4D(64)"
#endif
;
STR_EXT const uint8_t str_80_tag[6]
#if STR_ENGLISH
 = "4D(++)"
#elif STR_CHINESE
 = "4D(++)"
#endif
;
STR_EXT  uint8_t str_40[6]
#if STR_ENGLISH
 = "4D(40)"
#elif STR_CHINESE
 = "40"
#endif
;
STR_EXT  uint8_t str_80[6]
#if STR_ENGLISH
 = "4D(80)"
#elif STR_CHINESE
 = "80"
#endif
;
STR_EXT const uint8_t str_new_4d[6]
#if STR_ENGLISH
 = "New 4D"
#elif STR_CHINESE
 = "??4D"
#endif
;
////////////////////////////////////////////////// string_tms37206
STR_EXT const uint8_t str_h_tag[6]
#if STR_ENGLISH
 = "H Tag "
#elif STR_CHINESE
 = "H ??"
#endif
;

/////////////////////////////////////////////////// string_em4170
STR_EXT  uint8_t str_48_tag[6]
#if STR_ENGLISH
 = "48 Tag"
#elif STR_CHINESE
 = "48??"
#endif
;

/////////////////////////////////////////////////// string_e5561
STR_EXT const uint8_t str_8c_tag[6]
#if STR_ENGLISH
 = "8C Tag"
#elif STR_CHINESE
 = "8C??"
#endif
;
STR_EXT const uint8_t str_new_8ctag[8]
#if STR_ENGLISH
 = "New8CTag"
#elif STR_CHINESE
 = "?8C??"
#endif
;

/////////////////////////////////////////////////// string_em4100
STR_EXT  uint8_t str_13_tag[6]
#if STR_ENGLISH
 = "13 Tag"
#elif STR_CHINESE
 = "13??"
#endif
;
STR_EXT const uint8_t str_new_13tag[8]
#if STR_ENGLISH
 = "New13Tag"
#elif STR_CHINESE
 = "?13??"
#endif
;
STR_EXT const uint8_t str_mc32[6]
#if STR_ENGLISH
 = "(MC32)"
#elif STR_CHINESE
 = "(MC32)"
#endif
;
STR_EXT const uint8_t str_mc64[6]
#if STR_ENGLISH
 = "(MC64)"
#elif STR_CHINESE
 = "(MC64)"
#endif
;
STR_EXT const uint8_t str_bp32[6]
#if STR_ENGLISH
 = "(BP32)"
#elif STR_CHINESE
 = "(BP32)"
#endif
;
STR_EXT const uint8_t str_bp64[6]
#if STR_ENGLISH
 = "(BP64)"
#elif STR_CHINESE
 = "(BP64)"
#endif
;

/////////////////////////////////////////////////// string_nova
STR_EXT const uint8_t str_t5_tag[6]
#if STR_ENGLISH
 = "T5 Tag"
#elif STR_CHINESE
 = "T5??"
#endif
;
STR_EXT const uint8_t str_13_mode[8]
#if STR_ENGLISH
 = "(13Mode)"
#elif STR_CHINESE
 = "  13??"
#endif
;
STR_EXT const uint8_t str_type[5]
#if STR_ENGLISH
 = "Type:"
#elif STR_CHINESE
 = "??:"
#endif
;
STR_EXT const uint8_t str_mc[2]
#if STR_ENGLISH
 = "MC"
#elif STR_CHINESE
 = "MC"
#endif
;
STR_EXT const uint8_t str_bp[2]
#if STR_ENGLISH
 = "BP"
#elif STR_CHINESE
 = "BP"
#endif
;
STR_EXT const uint8_t str_liw[3]
#if STR_ENGLISH
 = "LIW"
#elif STR_CHINESE
 = "LIW"
#endif
;
STR_EXT const uint8_t str_pmc[3]
#if STR_ENGLISH
 = "PMC"
#elif STR_CHINESE
 = "PMC"
#endif
;
STR_EXT const uint8_t str_nul[3]
#if STR_ENGLISH
 = "NUL"
#elif STR_CHINESE
 = "NUL"
#endif
;

/////////////////////////////////////////////////// string_r9wk
STR_EXT  uint8_t str_4c_tag[6]
#if STR_ENGLISH
 = "4C TAG"
#elif STR_CHINESE
 = "4C??"
#endif
;
STR_EXT const uint8_t str_crc_ok[6]
#if STR_ENGLISH
 = "CRC OK"
#elif STR_CHINESE
 = "CRC OK"
#endif
;
STR_EXT const uint8_t str_crc_no[6]
#if STR_ENGLISH
 = "CRC NO"
#elif STR_CHINESE
 = "CRC NO"
#endif
;

/////////////////////////////////////////////////// string_pcf7935
STR_EXT const uint8_t str_44_tag[6]
#if STR_ENGLISH
 = "44 TAG"
#elif STR_CHINESE
 = "44??"
#endif
;

/////////////////////////////////////////////////// string_animal
STR_EXT const uint8_t str_hdx[3]
#if STR_ENGLISH
 = "HDX"
#elif STR_CHINESE
 = "HDX"
#endif
;
STR_EXT const uint8_t str_fdxa[4]
#if STR_ENGLISH
 = "FDXA"
#elif STR_CHINESE
 = "FDXA"
#endif
;
STR_EXT const uint8_t str_fdxb[4]
#if STR_ENGLISH
 = "FDXB"
#elif STR_CHINESE
 = "FDXB"
#endif
;
STR_EXT  uint8_t str_em4102[6]
#if STR_ENGLISH
 = "EM4102"
#elif STR_CHINESE
 = "EM4102"
#endif
;

#endif
