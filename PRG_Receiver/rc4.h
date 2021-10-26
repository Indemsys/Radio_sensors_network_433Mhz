#ifndef _RC4_H
#define _RC4_H

typedef struct
{
    unsigned char x, y, m[256];
} t_rc4_state;

void rc4_setup(unsigned char *key,  unsigned char length );
void rc4_crypt(unsigned char *data, unsigned int length );

#endif /* rc4.h */
