#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include "main.h"
#include "Timers.h"
#include "RF_transmitter.h"
#include "RF_settings.h"
#include "Util.h"
#include "rc4.h"

extern TPARAMS wp;   // Рабочие параметры в RAM

volatile unsigned long   preamble;   // Текущий остаток преамбулы для передачи
unsigned int   bit_count;  // Счетчик посылаемых бит
unsigned char  sended_cnt; // Количество посланных байта
volatile char *ptr_pack;   // Указатель на буфер содержащий посылаемый пакет
unsigned char  pack_len;   // Длина посылаемого пакета

extern volatile tflags flags;
extern unsigned int rf_bitrate;


/*----------------------------------------------------------------------
Отправка пакета передатчику

Аргументы:
*buf - указатель на буффер с данным
cnt - количество данных в буфере

------------------------------------------------------------------------*/
void RF_send(char *buf,unsigned char cnt)
{
  unsigned int crc;
  crc=GetBlockCRC((unsigned char*)buf,cnt);
  buf[cnt]  =(crc>>8);
  buf[cnt+1]=(crc & 0xFF);

  rc4_setup((unsigned char*)&wp.keycode, 4);
  rc4_crypt((unsigned char*)buf, 11 );

  // Назначение глобальных переменных используемых обработчиком прерывания
  preamble       = 0xFFFF0000 | wp.preamble;
  bit_count      = 0;
  sended_cnt     = 0;
  ptr_pack       = buf;
  pack_len       = cnt + 2; // С учетом добавленной контролльной суммы

  flags.centre   =0;
  flags.lastb    =0;
  flags.done     =0;
  flags.preamble =1;

  // Взвести бит на выходе к передатчику
  // Разрешить прерывания таймера 1 по каналу A каждые полбита
  TIMER1_CTC_Init(wp.rf_bitrate);
  //TIMER1_OCB_force_state((unsigned char)PREAMBLE>>15);
  TIMER1_OCB_force_state(1);
  TIMER1_OCB_int_en();

}

#pragma vector=TIMER1_COMPB_vect
__interrupt void TIMER1_COMPB_interrupt( void )
{

  if (flags.preamble)
  {

    preamble<<=1;
    bit_count++;
    if (bit_count==(PREAMBLE_LEN + 16))
    {
      flags.preamble=0;
      bit_count=0;
      // Запрограммируем так, чтобы по прерыванию был сгенерирован правильный перепад для начала первого бита данных
      if ((ptr_pack[sended_cnt] & 0x80)==0)
        TIMER1_OCB_outmod_set0();       // Компаратор выставит 0 на выходе после срабатывания
      else
        TIMER1_OCB_outmod_set1();       // Компаратор выставит 1 на выходе после срабатывания
    }
    else
    {
      if (preamble & 0x80000000ul)
        TIMER1_OCB_outmod_set1();        // Компаратор выставит 1 на выходе после срабатывания
      else
        TIMER1_OCB_outmod_set0();        // Компаратор выставит 0 на выходе после срабатывания
    }

  }
  else
  {
    if (flags.centre)
    {
      // Здесь середина передаваемого бита
      flags.centre=0;

      bit_count++; // Увиличить счетчик переданных битов
      if (bit_count==8)
      {
        bit_count=0;
        sended_cnt++;
        if (sended_cnt==pack_len) flags.lastb=1;
      }
      if (flags.lastb) TIMER1_OCB_outmod_set0();
      else
      {
        // Определить перепад в начале следующего бита (если 1, то 0->1;  если 0, то 1->0)
        if ((ptr_pack[sended_cnt] & (0x80 >> bit_count))==0)
          TIMER1_OCB_outmod_set0();
        else
          TIMER1_OCB_outmod_set1();
      }
    }
    else
    {
      // Здесь начало/конец передачи бита

      flags.centre=1;  // Следующее прерывание будет в центре бита

      if  (flags.lastb)
      {
        // Здесь закончился последний бит последнего байта
        TIMER1_OCB_int_dis();
        TIMER1_CTC_Stop();
        flags.done=1;
      }
      else
      {
        // Определить перепад в центре бита (если 1, то 1->0;  если 0, то 0->1)
        if ((ptr_pack[sended_cnt] & (0x80 >> bit_count))==0)
          TIMER1_OCB_outmod_set1();
        else
          TIMER1_OCB_outmod_set0();
      }
    }
  }
}

