/*
 *  An implementation of the ARC4 algorithm
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "rc4.h"

t_rc4_state rc4state;

void rc4_setup(unsigned char *key,  unsigned char length )
{
    unsigned int  i;
    unsigned char j, k, a;
    unsigned char *m;

    rc4state.x = 0;
    rc4state.y = 0;
    m = rc4state.m;

    for( i = 0; i < 256; i++ )
    {
        m[i] = i;
    }

    j = k = 0;

    for( i = 0; i < 256; i++ )
    {
        a = m[i];
        j = (unsigned char) ( j + a + key[k] );
        m[i] = m[j]; 
        m[j] = a;
        k++;
        if(k >= length ) k = 0;
    }
}

void rc4_crypt(unsigned char *data, unsigned int length )
{
    unsigned int  i; 
    unsigned char x, y, a, b;
    unsigned char  *m;

    x = rc4state.x;
    y = rc4state.y;
    m = rc4state.m;

    for( i = 0; i < length; i++ )
    {
        x = (unsigned char) ( x + 1 ); a = m[x];
        y = (unsigned char) ( y + a );
        m[x] = b = m[y];
        m[y] = a;
        data[i] ^= m[(unsigned char) ( a + b )];
    }

    rc4state.x = x;
    rc4state.y = y;
}
