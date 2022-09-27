
 const unsigned char tb1[128]= {        
                                0x08 ,0x00 ,0x08 ,0x00 ,0x00 ,0x00 ,0x08 ,0x08 ,0x00 ,0x00 ,0x08 ,0x08 ,0x00 ,0x08 ,0x00 ,0x08, 
                                0x08 ,0x00 ,0x08 ,0x00 ,0x08 ,0x08 ,0x00 ,0x00 ,0x00 ,0x08 ,0x00 ,0x08 ,0x08 ,0x08 ,0x00 ,0x00,
                                0x00 ,0x04 ,0x04 ,0x04 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x04 ,0x00 ,0x04 ,0x04 ,0x04 ,0x00,
                                0x04 ,0x00 ,0x04 ,0x00 ,0x04 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x04 ,0x04 ,0x00 ,0x04 ,0x00 ,0x04,
                                0x02 ,0x00 ,0x00 ,0x00 ,0x02 ,0x00 ,0x02 ,0x02 ,0x02 ,0x02 ,0x00 ,0x02 ,0x00 ,0x00 ,0x00 ,0x02,
                                0x02 ,0x00 ,0x02 ,0x02 ,0x02 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x02 ,0x02 ,0x02 ,0x00 ,0x02,
                                0x00 ,0x01 ,0x00 ,0x01 ,0x01 ,0x00 ,0x01 ,0x00 ,0x01 ,0x01 ,0x00 ,0x00 ,0x01 ,0x01 ,0x00 ,0x00,
                                0x00 ,0x00 ,0x01 ,0x01 ,0x00 ,0x00 ,0x01 ,0x01 ,0x00 ,0x01 ,0x00 ,0x01 ,0x01 ,0x00 ,0x01 ,0x00
                               };
 const unsigned char tb2[64]= {
                                 0x08 ,0x08 ,0x08 ,0x00 ,0x00 ,0x00 ,0x08 ,0x00 ,0x00 ,0x08 ,0x00 ,0x00 ,0x00 ,0x08 ,0x08 ,0x08,
                                 0x02 ,0x02 ,0x02 ,0x00 ,0x00 ,0x00 ,0x02 ,0x00 ,0x00 ,0x02 ,0x00 ,0x00 ,0x00 ,0x02 ,0x02 ,0x02,
                                 0x00 ,0x01 ,0x00 ,0x00 ,0x01 ,0x01 ,0x01 ,0x00 ,0x00 ,0x01 ,0x01 ,0x01 ,0x00 ,0x00 ,0x01 ,0x00,
                                 0x00 ,0x01 ,0x03 ,0x03 ,0x02 ,0x01 ,0x02 ,0x00 ,0x00 ,0x02 ,0x01 ,0x02 ,0x03 ,0x03 ,0x01 ,0x00
                              };

//=====================================================================================
//rdm---(LSB-MSB,order of pc)
//sk----(LSB-MSB,order of pc)
//cycle--calculate times
//result---signature(MSB-LSB）
//
// Note: function change input parameter rdm and sk
//=====================================================================================
unsigned long dst40_cycle(unsigned char rdm[5],unsigned char sk[5],unsigned char cycle)
{
            unsigned long result;
            unsigned int temp_r9, temp_r10, temp_r11;
            unsigned int temp1,temp2;
            unsigned int rounds;
            unsigned int cycles;
            unsigned char b0,b0_temp;
            unsigned char temp_r4, temp_r5, temp_r6, temp_r7, temp_r8;
            unsigned char test_temp;
            
            result=0;
            rounds=cycle;
            cycles=2;

            for (; rounds > 0;rounds-- )
            {
                //加密
                temp_r4 = sk[0];
                temp_r5 = sk[1];
                temp_r6 = sk[2];
                temp_r7 = rdm[0];
                temp_r8 = rdm[1];
                temp_r9 = 0;

                test_temp=(temp_r6 & 0x80)>>3;
                test_temp|=(temp_r8 & 0x80) >> 4;
                test_temp|=(temp_r5 & 0x80) >> 5;
                test_temp|=(temp_r7 & 0x80) >> 6;
                test_temp|=(temp_r4 & 0x80) >> 7;
                temp_r9=(temp_r9<<5)|(unsigned int)test_temp;
                
                temp_r4<<=1;
                temp_r5<<=1;
                temp_r6<<=1;
                temp_r7<<=1;
                temp_r8<<=1;
                temp_r10=tb1[temp_r9];

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r6 & 0x80) >>3) | (((unsigned int)temp_r8 & 0x80) >> 4) | (((unsigned int)temp_r5 & 0x80) >> 5) | (((unsigned int)temp_r7 & 0x80) >> 6) | (((unsigned int)temp_r4 & 0x80)>>7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r10 |= tb1[64+temp_r9];
                temp_r10 &= 0x00FF;

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r6 & 0x80) >> 3) | (((unsigned int)temp_r8 & 0x80) >> 4) | (((unsigned int)temp_r5 & 0x80) >> 5) | (((unsigned int)temp_r7 & 0x80) >> 6) | (((unsigned int)temp_r4 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r11 = tb1[temp_r9];

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r6 & 0x80) >> 3) | (((unsigned int)temp_r8 & 0x80) >> 4) | (((unsigned int)temp_r5 & 0x80) >> 5) | (((unsigned int)temp_r7 & 0x80) >> 6) | (((unsigned int)temp_r4 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r11 |= tb1[64 + temp_r9];
                temp_r11 &= 0x00FF;


                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r6 & 0x80) >> 3) | (((unsigned int)temp_r8 & 0x80) >> 4) | (((unsigned int)temp_r5 & 0x80) >> 5) | (((unsigned int)temp_r7 & 0x80) >> 6) | (((unsigned int)temp_r4 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9= tb1[temp_r9];
                temp1 = temp_r9;
                temp2 = temp_r9;
                temp_r9 =(unsigned int)(((temp1 & 0x00FF) << 8) |((temp2 & 0xFF00) >> 8));
                temp_r10 |= temp_r9;

                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r6 & 0x80) >> 3) | (((unsigned int)temp_r8 & 0x80) >> 4) | (((unsigned int)temp_r5 & 0x80) >> 5) | (((unsigned int)temp_r7 & 0x80) >> 6) | (((unsigned int)temp_r4 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb1[64 + temp_r9];
                temp1 = temp_r9;
                temp2 = temp_r9;
                temp_r9 =(unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r10 |= temp_r9;

                temp_r9 = (unsigned int)(((temp_r9 << 4) | ((((unsigned int)temp_r6 & 0x80) >> 4) | (((unsigned int)temp_r8 & 0x80) >> 5) | (((unsigned int)temp_r5 & 0x80) >> 6) | (((unsigned int)temp_r4 & 0x80)>>7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb2[temp_r9];
                temp1 = temp_r9;
                temp2 = temp_r9;
                temp_r9 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r11 |= temp_r9;

                temp_r9 = (unsigned int)(((temp_r9 << 4) | ((((unsigned int)temp_r6 & 0x80) >> 4) | (((unsigned int)temp_r8 & 0x80) >> 5) | (((unsigned int)temp_r5 & 0x80) >> 6) | (((unsigned int)temp_r4 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb2[16+temp_r9];
                temp1 = temp_r9;
                temp2 = temp_r9;
                temp_r9 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r11 |= temp_r9;

                temp_r4=sk[3];
                temp_r5=sk[4];
                temp_r6=rdm[2];
                temp_r7=rdm[3];
                temp_r8=rdm[4];

                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80)>>7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;               
                temp_r9 = tb1[32+temp_r9];
                temp_r10 |= temp_r9;

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb1[96 + temp_r9];
                temp_r10 |= temp_r9;

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb1[32 + temp_r9];
                temp_r11 |= temp_r9;

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb1[96 + temp_r9];
                temp_r11 |= temp_r9;

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb1[32 + temp_r9];
                temp1 = temp_r9;
                temp2 = temp_r9;
                temp_r9 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r10 |= temp_r9;

                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb1[96 + temp_r9];
                temp1 = temp_r10;
                temp2 = temp_r10;
                temp_r10 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r10 |= temp_r9;

                temp_r9 = 0;
                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9 = tb1[32 + temp_r9] ;
                temp1 = temp_r9;
                temp2 = temp_r9;
                temp_r9 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r11 |= temp_r9;

                temp_r9 = (unsigned int)(((temp_r9 << 5) | ((((unsigned int)temp_r8 & 0x80) >> 3) | (((unsigned int)temp_r5 & 0x80) >> 4) | (((unsigned int)temp_r7 & 0x80) >> 5) | (((unsigned int)temp_r4 & 0x80) >> 6) | (((unsigned int)temp_r6 & 0x80) >> 7)))&0x00FF);
                temp_r4 <<= 1;
                temp_r5 <<= 1;
                temp_r6 <<= 1;
                temp_r7 <<= 1;
                temp_r8 <<= 1;
                temp_r9=tb1[96+temp_r9];
                temp1 = temp_r11;
                temp2 = temp_r11;
                temp_r11 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r11 |= temp_r9;

                temp_r9 =(unsigned int)( temp_r11 & 0x00FF);
                temp_r4 = (unsigned char)tb2[32 + temp_r9];
                temp_r4<<= 1;
                temp_r9 =(unsigned int)(temp_r10 & 0x00FF);
                temp_r4 |= tb2[32+temp_r9];
                temp_r4 <<= 1;
                temp1 = temp_r10;
                temp2 = temp_r10;
                temp_r10 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp1 = temp_r11;
                temp2 = temp_r11;
                temp_r11 = (unsigned int)(((temp1 & 0x00FF) << 8) | ((temp2 & 0xFF00) >> 8));
                temp_r9 = (unsigned int)(temp_r11 & 0x00FF);
                temp_r4 |= (unsigned char)tb2[32 + temp_r9];
                temp_r9 = (unsigned int)(temp_r10 & 0x00FF);
                temp_r4 <<= 1;
                temp_r4 |= (unsigned char)tb2[32 + temp_r9];
                temp_r5=(unsigned char)tb2[48+temp_r4];
                temp_r5^=rdm[0];
                temp_r5 &= 0x00FF;
                b0=(unsigned char)(temp_r5&0x01);
                temp_r5=(unsigned char)((temp_r5>>1)|(temp_r5<<15));
                b0_temp =(unsigned char)( rdm[4] & 0x01);
                rdm[4]=(unsigned char)((rdm[4]>>1)|(b0<<7));
                b0=b0_temp;
                b0_temp=(unsigned char)(rdm[2]&0x01);
                temp1=(unsigned int)((rdm[3]<<8)+rdm[2]);
                temp1 = (unsigned int)((temp1 >> 1) | (b0 << 15));
                rdm[3]=(unsigned char)((temp1&0xFF00)>>8);
                rdm[2] = (unsigned char)(temp1 & 0x00FF);
                b0 = b0_temp;
                b0_temp = (unsigned char)(rdm[0] & 0x01);
                temp1 = (unsigned int)((rdm[1] << 8) + rdm[0]);
                temp1 = (unsigned int)((temp1 >> 1) | (b0 << 15));
                rdm[1]=(unsigned char)((temp1&0xFF00)>>8);
                rdm[0] = (unsigned char)(temp1 & 0x00FF);
                b0 = b0_temp;
                b0_temp=(unsigned char)(temp_r5&0x01);
                temp_r5 = (unsigned char)((temp_r5 >> 1) | (b0 << 15));
                b0 = b0_temp;
                b0_temp = (unsigned char)(rdm[4] & 0x01);
                rdm[4] = (unsigned char)((rdm[4] >> 1) | (b0 << 7));
                b0 = b0_temp;
                b0_temp = (unsigned char)(rdm[2] & 0x01);
                temp1 = (unsigned int)((rdm[3] << 8) + rdm[2]);
                temp1 = (unsigned int)((temp1 >> 1) | (b0 << 15));
                rdm[3] = (unsigned char)((temp1 & 0xFF00) >> 8);
                rdm[2] = (unsigned char)(temp1 & 0x00FF);
                b0 = b0_temp;
                temp1 = (unsigned int)((rdm[1] << 8) + rdm[0]);
                temp1 = (unsigned int)((temp1 >> 1) | (b0 << 15));
                rdm[1] = (unsigned char)((temp1 & 0xFF00) >> 8);
                rdm[0] = (unsigned char)(temp1 & 0x00FF);


                //随机数移位               
                cycles--;
                if (cycles == 0)
                {
                    cycles = 3;
                    //更新key
                    temp_r4 = sk[0];
                    temp_r5 = sk[2];
                    temp_r6 = 0x0001;
                    temp_r6 &= temp_r4;
                    if((temp_r4&0x0004)==0x0004)
                    {
                        temp_r6 ^= 0x0001;
                    }

                    if ((temp_r5 & 0x08) == 0x08)
                    {
                        temp_r6 ^= 0x0001;
                    }

                    if ((temp_r5 & 0x20) == 0x20)
                    {
                        temp_r6 ^= 0x01;
                    }

                    b0 = (unsigned char)(temp_r6 & 0x01);
                    temp_r6 = (unsigned char)((temp_r6 >> 1) | (temp_r6 << 15));
                    b0_temp = (unsigned char)(sk[4] & 0x01);
                    sk[4] = (unsigned char)((sk[4] >> 1) | (b0 << 7));
                    b0 = b0_temp;
                    b0_temp = (unsigned char)(sk[2] & 0x01);
                    temp1 = (unsigned int)((sk[3] << 8) + sk[2]);
                    temp1 = (unsigned int)((temp1 >> 1) | (b0 << 15));
                    sk[3]=(unsigned char)((temp1&0xFF00)>>8);
                    sk[2] = (unsigned char)(temp1 & 0x00FF);
                    b0 = b0_temp;
                    temp1 = (unsigned int)((sk[1] << 8) + sk[0]);
                    temp1 = (unsigned int)((temp1 >> 1) | (b0 << 15));
                    sk[1] = (unsigned char)((temp1 & 0xFF00) >> 8);
                    sk[0] = (unsigned char)(temp1 & 0x00FF);
                }
            }

            result = ((unsigned long)rdm[2] << 16) + ((unsigned long)rdm[1] << 8) + (unsigned long)rdm[0];
            return result;
}


