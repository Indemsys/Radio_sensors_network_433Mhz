#include <iom8.h>
#include <ina90.h>
#include "main.h"
#include "Timers.h"
#include "bin_defines.h"


/*
  Программирование таймера 1 в режиме CTC на заданный период

  Период рассчитываеться в секундах как:  period/Fosc



*/
void TIMER1_CTC_Init(unsigned int period)
{

  unsigned char tmp;
  // Предварительно выключим

  TIMER1_CTC_Stop();

  OCR1A = period;

  // Программируем канал B в режим очистки таймера по совпадению (CTC) c TOP величиной в OCR1A
  // Канал A отключен
  TCCR1A = b00000000; //

  // Источник тактов - частота кварца 16 Мег
  // Режим CTC
  TCCR1B = b00001001;

  tmp    = TIMSK;
  tmp    = tmp & ~(1 << TICIE1) & ~(1 << OCIE1A) & ~(1 << OCIE1B) & ~(1 << TOIE1);
  TIMSK  = tmp;

  tmp    = TIFR;
  tmp    = ~tmp | (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1); // Флаги сбрасываються записью единицы
  TIFR   = tmp;


}

/*
  Разрешение прерывания от компаратора B таймера 1

*/
void TIMER1_OCB_int_en(void)
{
  unsigned char tmp;

  tmp    = TIFR;
  tmp    = ~tmp | (1 << OCF1B); // Флаги сбрасываються записью единицы
  TIFR   = tmp;

  tmp    = TIMSK;
  tmp    = tmp  | (1 << OCIE1B);
  TIMSK  = tmp;

}

/*
  Запрещение прерывания от компаратора B таймера 1

*/
void TIMER1_OCB_int_dis(void)
{
  unsigned char tmp;

  tmp    = TIFR;
  tmp    = ~tmp | (1 << OCF1B); // Флаги сбрасываються записью единицы
  TIFR   = tmp;

  tmp    = TIMSK;
  tmp    = tmp  & ~(1 << OCIE1B);
  TIMSK  = tmp;

}


/*
  Принудительно установить 0 на выходе OCA и подготовить установку заданного бита

*/
void TIMER1_OCB_force_state(unsigned char bits)
{

  if (bits & 1)
    TIMER1_OCB_outmod_set1();
  else
    TIMER1_OCB_outmod_set0();

}

void TIMER1_OCB_outmod_set1(void)
{
  unsigned char tmp;
  tmp = TCCR1A;
  tmp = (tmp & 0xCF) | b00110000;
  TCCR1A = tmp;
}

void TIMER1_OCB_outmod_set0(void)
{
  unsigned char tmp;
  tmp = TCCR1A;
  tmp = (tmp & 0xCF) | b00100000;
  TCCR1A = tmp;
}


void TIMER1_CTC_Stop(void)
{
  TCCR1B = 0;
  TCCR1A = 0;

}

void TIMER0_init(void)
{
  // Инициализируем прерывания от таймера 0, для обслуживания всяких временных интервалов	
  // Частоту следования прерываний выбираем (16 000 000/64)/256 = 976,5625 Гц -> 0.001024 мс
  TCCR0  = b00000011; // Предделитель = 64
  TIFR  |= b00000001; //
  TIMSK |= b00000001; // Разрешаем прерывания
}
