#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include "main.h"
#include "Timers.h"
#include "USART.h"
#include "RF_receiver.h"
#include "..\PRG_Transmitter\RF_settings.h"
#include "Util.h"
#include "bin_defines.h"

extern __no_init TPARAMS wp;

#define REC_PREAMBLE 0
#define REC_PACK     1


extern volatile tflags flags;
unsigned char rec_state;
unsigned char smpls[PREAMBLE_LEN*4/8];
unsigned char inbit;
unsigned char hitcnt  = 0;
unsigned char misscnt = 0;
unsigned char ccor1;
unsigned char ccor2;
unsigned char ccor3;
unsigned char smplcnt;
unsigned char gapcnt;
char         *recbuf;
unsigned char inbyte;
unsigned char inbytecnt;

unsigned char preamb7;
unsigned char preamb6;
unsigned char preamb5;
unsigned char preamb4;
unsigned char preamb3;
unsigned char preamb2;
unsigned char preamb1;
unsigned char preamb0;



/*
Тестовый массив
unsigned char tstarr[100];
*/

__flash const char corrtbl[256]=
{
0x08, 0x07, 0x07, 0x06, 0x07, 0x06, 0x06, 0x05,
0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
0x03, 0x02, 0x02, 0x01, 0x02, 0x01, 0x01, 0x00,
};

void prepare_preamble(void);

void RF_receiver_init(char *buf)
{
  flags.done = 0;
  rec_state  = REC_PREAMBLE;
  recbuf     = buf;
  prepare_preamble();
  TIMER1_CTC_Init(wp.rf_bitrate/4); // Чтение состояний каждую 1/4 полубита
  TIMER1_OCB_int_en();
}

void prepare_preamble(void)
{
/* пример
#define PREAMB7  b11111111
#define PREAMB6  b00001111
#define PREAMB5  b11110000
#define PREAMB4  b00001111
#define PREAMB3  b11111111
#define PREAMB2  b00000000
#define PREAMB1  b00000000
#define PREAMB0  b11110000
*/
  preamb7 = 0;
  preamb6 = 0;
  preamb5 = 0;
  preamb4 = 0;
  preamb3 = 0;
  preamb2 = 0;
  preamb1 = 0;
  preamb0 = 0;

  if (wp.preamble & 0x8000) preamb7 |= b11110000;
  if (wp.preamble & 0x4000) preamb7 |= b00001111;

  if (wp.preamble & 0x2000) preamb6 |= b11110000;
  if (wp.preamble & 0x1000) preamb6 |= b00001111;

  if (wp.preamble & 0x0800) preamb5 |= b11110000;
  if (wp.preamble & 0x0400) preamb5 |= b00001111;

  if (wp.preamble & 0x0200) preamb4 |= b11110000;
  if (wp.preamble & 0x0100) preamb4 |= b00001111;

  if (wp.preamble & 0x0080) preamb3 |= b11110000;
  if (wp.preamble & 0x0040) preamb3 |= b00001111;

  if (wp.preamble & 0x0020) preamb2 |= b11110000;
  if (wp.preamble & 0x0010) preamb2 |= b00001111;

  if (wp.preamble & 0x0008) preamb1 |= b11110000;
  if (wp.preamble & 0x0004) preamb1 |= b00001111;

  if (wp.preamble & 0x0002) preamb0 |= b11110000;
  if (wp.preamble & 0x0001) preamb0 |= b00001111;
}




#pragma vector=TIMER1_COMPB_vect
__interrupt void TIMER1_COMPB_interrupt( void )
{

unsigned char ccor;

  PORTC |= P_TST;

  inbit = (PIND >> P_DIN_OFS) & 1;

  switch (rec_state)
  {
  case REC_PREAMBLE:
    // Принимаем отсчет бита преамбулы
    smpls[7] = (smpls[7] << 1) | (smpls[6] >> 7);
    smpls[6] = (smpls[6] << 1) | (smpls[5] >> 7);
    smpls[5] = (smpls[5] << 1) | (smpls[4] >> 7);
    smpls[4] = (smpls[4] << 1) | (smpls[3] >> 7);
    smpls[3] = (smpls[3] << 1) | (smpls[2] >> 7);
    smpls[2] = (smpls[2] << 1) | (smpls[1] >> 7);
    smpls[1] = (smpls[1] << 1) | (smpls[0] >> 7);
    smpls[0] = (smpls[0] << 1) | inbit;
    // Вычисляем корреляцию с эталонной преамбулой
    ccor  = corrtbl[(smpls[7] ^ preamb7)&0xFF];
    ccor += corrtbl[(smpls[6] ^ preamb6)&0xFF];
    ccor += corrtbl[(smpls[5] ^ preamb5)&0xFF];
    ccor += corrtbl[(smpls[4] ^ preamb4)&0xFF];
    ccor += corrtbl[(smpls[3] ^ preamb3)&0xFF];
    ccor += corrtbl[(smpls[2] ^ preamb2)&0xFF];
    ccor += corrtbl[(smpls[1] ^ preamb1)&0xFF];
    ccor += corrtbl[(smpls[0] ^ preamb0)&0xFF];

    if (ccor >= DECISLEV)
    {
      misscnt = 0;
      hitcnt++;
      ccor3 = ccor2;
      ccor2 = ccor1;
      ccor1 = ccor;

      if ((hitcnt > 2) && (ccor2 > ccor3))
      {
        if ((ccor2 > ccor1) || (ccor2 == ccor1))
        {
          // Нашли саксимум на 3-х осчетах. Считаем его пиком корреляции, означающем конец приамбулы
          rec_state = REC_PACK; // Переходим в режим приема битов пакета
          gapcnt    = 5;        // Выборка 1-го бита на 5-ом отсчете.
          inbytecnt = 0;
          smplcnt   = 0;
        }
      }
    }
    else
    {
      hitcnt = 0;
      if (misscnt<255) misscnt ++;
    }
    break;
  case REC_PACK:
    // Принимаем отсчет бита пакета
    smpls[0] = (smpls[0] << 1) | inbit;
    ccor = corrtbl[(smpls[0] ^ 0xF0)& 0xFE]; // Коррелируем с шаблоном единицы

    if (gapcnt==0)
    {
      gapcnt = 7;

      // Выборка значения бита
      inbyte = (inbyte << 1);
      if (ccor>4)
      {
         // Бит = 1
         inbyte |= 1;
      }
/*
      else
      {
         // Бит = 0

      }
*/
      smplcnt++;
      if (smplcnt == 8)
      {
        smplcnt = 0;
        // Принят полный байт
        recbuf[inbytecnt] = inbyte;
        inbytecnt++;
        if (inbytecnt == PACK_LEN)
        {
          // Принят весь пакет
          rec_state = REC_PREAMBLE;
          smpls[7] = 0;
          smpls[6] = 0;
          smpls[5] = 0;
          smpls[4] = 0;
          smpls[3] = 0;
          smpls[2] = 0;
          smpls[1] = 0;
          smpls[0] = 0;
          hitcnt   = 0;

          flags.done = 1;
//          TIMER1_OCB_int_dis();
        }
      }
    }
    else gapcnt--;

/* Тестовый блок. Начало---------------------------------------------------------------------------------

    tstarr[smplcnt] = ccor;
    smplcnt++;

    if (smplcnt == 100)
    {
      rec_state = REC_PREAMBLE;
      smpls[7] = 0;
      smpls[6] = 0;
      smpls[5] = 0;
      smpls[4] = 0;
      smpls[3] = 0;
      smpls[2] = 0;
      smpls[1] = 0;
      smpls[0] = 0;
      hitcnt   = 0;

      flags.done = 1;
      TIMER1_OCB_int_dis();
    }

   Тестовый блок. Конец----------------------------------------------------------------------------------
*/
    break;
  }

  PORTC &= ~P_TST;

}
