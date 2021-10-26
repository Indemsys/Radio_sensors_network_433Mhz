unsigned char is_hex_digit(unsigned char c);
unsigned char ascii_to_hex(unsigned char c);
unsigned char hex_to_ascii(unsigned char c);
unsigned char write_ascii(unsigned char c);
unsigned int GetCRC(unsigned int CRC,unsigned char b);
unsigned int GetBlockCRC(unsigned char* b,long len);
extern unsigned char *float_conversion(      float value,
                                             short nr_of_digits,
                                     unsigned char *buf,
                                     unsigned char format_flag,
                                     unsigned char g_flag,
                                     unsigned char alternate_flag);
int Num_to_str(unsigned char *buf,int ln,unsigned long ul,unsigned char base);
unsigned long Str_to_num(unsigned char *buf, unsigned char base);
void Right_align_str(unsigned char *buf, int buf_len);

