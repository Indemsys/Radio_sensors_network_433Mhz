#include  "main.h"
#include  <stdlib.h>

// ==========================================================
//  Проверка соответствия символа шестнадцатеричному представлению
// ==========================================================
unsigned char is_hex_digit(unsigned char c)
{
    if ((c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F')) return (1);
    return (0);
} 

// ==========================================================
//  Преобразование символа в шестнадцатеричное представление
// ==========================================================
unsigned char ascii_to_hex(unsigned char c)
{
    if (c >= '0' && c <= '9')      return ( c - '0' ) & 0x0f;
    else if (c >= 'a' && c <= 'f') return ( c - 'a' + 10 ) & 0x0f;
    else                           return ( c - 'A' + 10 ) & 0x0f;
} 

// ==========================================================
//  Преобразование байта в ASCII 
// ==========================================================
unsigned char hex_to_ascii(unsigned char c)
{
    c = c & 0xf;
    if (c <= 9) return (c + 0x30);
    return (c + 'A' - 10);
} 

// ==========================================================
//  Преобразовать не символьные байты в точку
// ==========================================================
unsigned char write_ascii(unsigned char c)
{
    if (c >= 0x20) return (c);
    return ('.');
} 




// ==========================================================
//  Расчет контрольной суммы по стандарту CRC CCITT 
// ==========================================================
unsigned int GetCRC(unsigned int CRC,unsigned char b)
{
#define POLI 0x1021 // CRC-16/CITT
  unsigned char i;
  CRC=CRC ^ (b << 8);
  for (i=0;i<8;i++)
  {
    if ((CRC & 0x8000) != 0) 
    {
      CRC=(CRC << 1) ^ POLI; 
    }
    else
    {
      CRC=(CRC << 1);
    }   
  } 
  return CRC;
}

// ==========================================================
//  Расчет контрольной суммы блока данных
// ==========================================================
unsigned int GetBlockCRC(unsigned char* b,long len)
{
  long i;
  unsigned int CRC=0xFFFF;
  
  for (i=0;i<len;i++)
  {
     CRC=GetCRC(CRC, *(b+i)); 
  }
  return CRC;
}


// ==========================================================
//  Перевод беззнакового целого числа (unsigned long) в строку
//  buf   - указатель на буфер приемник строки
//  ln    - максимальная длина строки
//  ul    - преобразуемое число
//  base  - база исчисления: 10 - десятичная, 16 - шестнадцатеричная, 2 - двоичная и т.д.  
//  Возвращает количество символов в строке 
// ==========================================================
int Num_to_str(unsigned char *buf,int ln,unsigned long ul,unsigned char base)
{
  int    n;          // Счетчик позиции в буфере    
  ldiv_t res;        // Результат целочисленного деления
  unsigned char b;
  int i;
  
  n=0;
  for (;;)                            
  {                                   
    res=ldiv(ul,base);                 
    *(buf+n)=hex_to_ascii(res.rem);       
    n++;                              
    if (ln!=0) 
    {                       
      if (n>=ln) break;             
    }  
    else                    
    {          
      if (res.quot==0) break;         
    }  
    ul=res.quot;                      
  }              

  // Переписать буфер в обратной последовательности
  for (i=0;i<(n / 2);i++)
  {
    b=*(buf+i);
    *(buf+i)=*(buf+n-i-1);
    *(buf+n-i-1)=b; 
  }
    
  *(buf+n)=0; // Закончить строку нулем 
  return n;
}   

// ==========================================================
//  Преобразует строку в беззнаковое целое число 
//  *buf - указатель на буфер в котором находиться строка
//  base - база исчисления: 10 - десятичная, 16 - шестнадцатеричная, 2 - двоичная и т.д.  
//  Возвращает  число
// ==========================================================
unsigned long Str_to_num(unsigned char *buf, unsigned char base)
{
  unsigned long b=0;  
  while (*buf!=0)
  {
    b=b*base + ascii_to_hex(*buf);
    buf++;
  }   
  return b; 
  
} 
/* ========================================================
   Выравнивание строки вправо
   ========================================================*/
void Right_align_str(unsigned char *buf, int buf_len)
{
  int l=0;
  int i;
  while (*(buf+l)!=0) l++; // Найдем длину строки в буфере
  if (l<buf_len)
  {
    for (i=0;i<l;i++) 
    {
      *(buf+buf_len-1-i)=*(buf+l-1-i);
      *(buf+l-1-i)=' ';
    }
    *(buf+buf_len)=0; 
  } 
} 

// ==========================================================
//  Преобразование числа в строковое представление. Возвращает указатель на конец занятого буфера 
// ==========================================================
unsigned char *float_conversion(float value,   // Преобразуемая величина
                              short nr_of_digits,    // Общее количество цифр для представления числа 
                                                     // Если число меньше 1, то не считая предварительных нулей 
                              unsigned char *buf,     
                              unsigned char format_flag,    // 'E' или 'e'
                              unsigned char g_flag,         // 1 - представлять в формате с фиксированной точкой
                              unsigned char alternate_flag) // 1 - не убирать не значащие нули
{
  unsigned char *cp, *buf_pointer;
  short n, i, dec_point_pos, integral_10_log;

  buf_pointer = buf;
  integral_10_log = 0;
  // Показать знак числа
  if (value<0)
  {
    value=-value;
    *buf_pointer++ = '-';
  } 
  
  if (value >= 1)
  {
    // Вычисляем количество десятков в экспоненте
    while (value >= 1e11)        
    {
      value /= 1e10;
      integral_10_log += 10;
    }
    // Вычисляем количество единиц в экспоненте
    while (value >= 10)
    {
      value /= 10;
      integral_10_log++;
    }
  }
  else if (value)            
  {
    // Вычисляем количество десятков в знаменателе экспоненты
    while (value <= 1e-10)        
    {
      value *= 1e10;
      integral_10_log -= 10;
    }
    // Вычисляем количество единиц в знаменателе экспоненты
    while (value < 1)
    {
      value *= 10;
      integral_10_log--;
    }
  }
  
  if (g_flag)
  {
    if (integral_10_log < nr_of_digits && integral_10_log >= -4)
    {
      format_flag = 0;
      nr_of_digits = nr_of_digits - integral_10_log;
    }
    nr_of_digits--;
    if (alternate_flag)
    {
      g_flag = 0;         /* %#G - No removal of trailing zeros */
    }
    else
    {
      alternate_flag = 1;  /* %G - Removal of trailing zeros */
    }
  }
  
  if (format_flag)        /* %e or %E */
  {
    // Для представления с экспонентой
    dec_point_pos = 0;
  }
  else
  {
    if (integral_10_log < 0)       
    {
      // Для чисел меньших по абсолютному значению чем 1 в представлении без экспоненты
      *buf_pointer++ = '0';
      if ((n = nr_of_digits) || alternate_flag)
      {
        *buf_pointer++ = '.';
      }
      i = 0;
      while (--i > integral_10_log && nr_of_digits)
      {
        *buf_pointer++ = '0';
        nr_of_digits--;
      }
      if (integral_10_log < (-n - 1))
      {
        goto CLEAN_UP;     /* Nothing more to do */
      }
      dec_point_pos = 1;
    }
    else
    {
      // Для чисел больших по абсолютному значению чем 1 в представлении без экспоненты
      dec_point_pos = - integral_10_log;
    }
  }

  i = dec_point_pos;
  while (i <= nr_of_digits )
  {
    n = (short)value;
    value = value - n;          /* n=Digit value=Remainder */
    value = value * 10;         /* Prepare for next shot */
    *buf_pointer++ = n + '0';
    if ( ! i++ && (nr_of_digits || alternate_flag))
    {
      *buf_pointer++ = '.';
    }
  } // while
  
  if (value >= 5)    /* Rounding possible */
  {
    n = 1;    /* Carry */
    cp = buf_pointer - 1;
    do
    {
      if (*cp != '.')
        if ( (*cp += n) == ('9' + 1) )
        {
          *cp = '0';
          n = 1;
        }
        else
        {
          n = 0;
        }
    } while (cp-- > buf);
    if (n)
    {
      if (format_flag)        /* %e or %E */
      {
        cp = buf_pointer;
        while (cp > buf)
        {
          if (*(cp - 1) == '.')
          {
            *cp = *(cp - 2);
            cp--;
          }
          else
          {
            *cp = *(cp - 1);
          }
          cp--;
        }
        integral_10_log++;
      }
      else
      {
        cp = ++buf_pointer;
        while (cp > buf)
        {
          *cp = *(cp - 1);
          cp--;
        }
      }
      *buf = '1';
    }
  }
CLEAN_UP:
  if (g_flag)            /* %G - Remove trailing zeros */
  {
    while (*(buf_pointer - 1) == '0')
    {
      buf_pointer--;
    }
    if (*(buf_pointer - 1) == '.')
    {
      buf_pointer--;
    }
  }
  if (format_flag)        /* %e or %E */
  {
    *buf_pointer++ = format_flag;
    if (integral_10_log < 0)
    {
      *buf_pointer++ = '-';
      integral_10_log = -integral_10_log;
    }
    else
    {
      *buf_pointer++ = '+';
    }
    n = 0;
    buf_pointer +=3;
    do
    {
      n++;
      *buf_pointer++ = (integral_10_log % 10) + '0'; // Запись в буфер цифр экспоненты
      integral_10_log /= 10;
    } while ( integral_10_log || n < 2 );
    for ( i = n ; n > 0 ; n-- )
      *(buf_pointer - 4 - i + n) = *(buf_pointer - n);
    buf_pointer -= 3;
  }
  *buf_pointer=0;
  return buf_pointer;
}
