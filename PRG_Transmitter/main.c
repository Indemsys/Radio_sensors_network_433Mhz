/*
  Программа передатчика на ATMEGA8
*/
#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "wrk_params.h"
#include "USART.h"
#include "Monitor.h"
#include "DS1Wire.h"
#include "ADC.h"
#include "Timers.h"
#include "RF_settings.h"
#include "RF_transmitter.h"
#include "Util.h"
#include "rc4.h"


__no_init D1W_device devices[MAX_DEVICES];



char rf_packet[20];
volatile tflags flags;
char fswon;

static unsigned char fbut1; // Флаг нажатия кнопки 1
static unsigned char fbut2; // Флаг нажатия кнопки 2

extern __no_init TPARAMS wp;   // Рабочие параметры в RAM

__no_init unsigned long  pack_cnt;

__no_init unsigned char wdt_div;        // Коэффициент прореживания частоты активизации устройства
__no_init unsigned char wdtpresc;
__no_init unsigned long seed;           // Начальная величина для генератора случайных чисел


void          send_temperatures(void);
unsigned char prepare_temper_pack(D1W_device* device);
void          send_ADC_results(void);
void          send_buttons(void);
void          send_packet(void);
unsigned char rand_gen_byte(unsigned long* seed);
void          heart_beat_msg(void);
/*--------------------------------------------------------------------------------------

     MAIN

  --------------------------------------------------------------------------------------*/
void main( void )
{

  DDRB  = P_LED | P_RFD | P_SWP;
  PORTB = 0xFF & (~P_RFD) & (~P_SWP) ;

  DDRC  = 0;
  PORTC = b01000000;

  DDRD  = P_DSD;
  PORTD = 0xFF & (~P_TXD) & (~P_JM1) & (~P_JM2) & (~P_JM3); // На TXD и перемычках Z-состояние

  WDTCR = b00011000;
  WDTCR = b00001111;  // Включили WDT на максимальный период

  // Сбросили флаги внешних прерываний.
  GIFR  = b11000000;
  // Разрешили внешние прерывания
  GICR  = b11000000;


  _SEI();            // До Sleep прерывания должны быть разрешены иначе не будет реакции на внешний INT

  // Определим источник сброса
  if ((MCUCSR & (1 << WDRF))!=0)
  {
    // Это сброс по WDT
    MCUCSR = 0;
    wdtpresc++;
    if (wdtpresc < wdt_div)
    {
      // Прошло недостаточно периодов WDT. Возвращаемся снова в Power Down, для этого выставим флаг  fswon.
      // По этому флагу в Power Down перейдем в главном цикле
      fswon = 0;
    }
    else
    {
      // Пришла пора активизировать устройство
      fswon    = 1;
      // Пересчитать время для следующей активизации
      wdt_div  = wp.wdt_div_const + (wp.wdt_div_var & rand_gen_byte(&seed));
      wdtpresc = 0;
    }

  }
  else
  {
    // Это сброс после подачи или сбоя в питании
    wdtpresc = 0;
    MCUCSR   = 0;   // Внешние прерывания по низкому уровню на INT0, INT1
    fswon    = 1;
    pack_cnt = 0;

    USART_Init( 16 );  // 115200 при кварце 16 Мгц

    // Инициализируем оперативные переменные из EEPROM
    if (Restore_settings_from_eeprom()== 0)
    {
      Restore_default_settings();
      USART_sendstr("CRC error!\n\r");
    }
    else
    {
      if (wp.version != PRG_VERSION)
      {
        Restore_default_settings();
        USART_sendstr("Ver. error!\n\r");
      }
      else
      {
        USART_sendstr("EEPROM Ok!\n\r");
      }
    }
    USART_sendstr("Settings restored!\n\r");

    seed     = 0;
    D1W_Init();
    if (DS1W_SearchBuses(devices, MAX_DEVICES)==SEARCH_SUCCESSFUL)
    {
      D1W_device *dev;
      dev = DS1W_FindFamily(DS1820_FAMILY_ID, devices, MAX_DEVICES);
      if (dev!=NULL)
      {
      	// Начальная величина для генератора случайных чисел
      	// вычисляеться на основе уникальной части идентификатора первого обнаруженного чипа DS1820
        seed = (((unsigned long)(*dev).id[1]<<24) ^ ((unsigned long)(*dev).id[5]<<24)) |
               (((unsigned long)(*dev).id[2]<<16) ^ ((unsigned long)(*dev).id[6]<<16)) |
               ((unsigned long)(*dev).id[3]<<8)  |
               ((unsigned long)(*dev).id[4]);
      }
    }
    wdt_div = wp.wdt_div_const + (wp.wdt_div_var & rand_gen_byte(&seed));

  }

  for( ; ; )        /* Forever */
  {
    unsigned char tmp;
    USART_Init( 16 );  // 115200 при кварце 16 Мгц

    if (fswon!=0)  // На промежеточных активизациях ничего не делать
    {
      D1W_Init();

      if ((wp.flags & HEART_BEAT_DISABLED)==0) heart_beat_msg();
      if ((wp.flags & DS1WIRE_DISABLED)   ==0) send_temperatures();
      if ((wp.flags & ADC_DISABLED)       ==0) send_ADC_results();
    }

    do
    {
    	if (DataInReceiveBuffer()!=0) terminal();
    }	
    while ((PIND & b00001100) != b00001100); // Ожидать пока не отпустят кнопки.
                                             // Сработка WDT приведет к выходу из цикла.

    // Выключить UART, чтобы он не мешал выставить нужный уровень на линии TXD
    USART_close();

    // Привести линии портов в состояние с наименьшим потреблением
    PORTB = 0xFF & (~P_RFD) & ~(P_SWP) ;
    DDRD  = P_TXD;
    PORTD = P_BT1 | P_BT2;

    // Установим таймер WDT

    tmp = b00001000 | (wp.wdt_period & 0x07); // Подготовим значения для загрузки в WDT
    __watchdog_reset();
    WDTCR = b00011000;
    WDTCR = tmp;        // Включили WDT на заданный период

    // Сбросили флаги внешних прерываний.
    GIFR  = b11000000;

    if ((wp.flags & BUTTONS_DISABLED)==0)
      GICR  = b11000000;  // Разрешили внешние прерывания
    else
      GICR  = b00000000;  // Запретили внешние прерывания

    // Выход в режим Power Down
    MCUCR = b10100000; // Внешние прерывания по низкому уровню на INT0, INT1
    __sleep();
    // Режим Power Down прерываеться и переход к посылке состояния кнопок.
    __watchdog_reset();
    send_buttons();
    fswon    = 0;
    wdtpresc = 0;
  }
}

/*--------------------------------------------------------------------------------------
   Сформироать первый байт пакета с информацией о типе пакета и номере передатчика
  --------------------------------------------------------------------------------------*/
unsigned char get_pack_first_byte(unsigned char type)
{
  unsigned char b;

  // Определить и записать номер передатчика
  PORTD |= (P_JM1 | P_JM2 | P_JM3); // Включим на перемычках Pull-Up

  __delay_cycles(16); // Задержка для установки уровня на 1 мкс

  b = (PIND >> 5) & 0x07;

  PORTD &= ((~P_JM1) & (~P_JM2) & (~P_JM3)) ; // Выключаем на перемычках Pull-Up
  // Записать тип пакета
  b |= (type << 4);
  return b;
}




/*--------------------------------------------------------------------------------------
    Выслать показания всех датчиков температуры обнаруженных в 1-Wire сети
  --------------------------------------------------------------------------------------*/
void send_temperatures(void)
{
  unsigned char i;

  for (i=0;i<MAX_DEVICES;i++)
  {
    if (devices[i].id[0] == DS1820_FAMILY_ID)
    {
      if (prepare_temper_pack(&devices[i])==FALSE)  // Подготовим пакет для отправки
      {
         DS1W_SearchBuses(devices, MAX_DEVICES);
         return;
      }
      else
      {
        send_packet();
      }
    }
  }
}


/*--------------------------------------------------------------------------------------
   Снять показания температуры с датчика и сформировать пакет для отправки

   Возвращает: TRUE если температура прочитана успешно
  --------------------------------------------------------------------------------------*/
unsigned char prepare_temper_pack(D1W_device* device)
{
  signed int temperature;
  unsigned char i;

  rf_packet[0] = get_pack_first_byte(PACK_TEMPER);

  temperature = DS1820_ReadTemperature((*device).id);

  if (temperature == -1000) return FALSE;


  // Идентификатор DS1820 состоит из 48-и бит
  for (i=1;i<7;i++) rf_packet[i] = (*device).id[i];
  rf_packet[7] = temperature >> 8;
  rf_packet[8] = temperature & 0xFF;


  return TRUE;
}



/*--------------------------------------------------------------------------------------
    Выслать результаты измерения ЦАП во всех каналах
  --------------------------------------------------------------------------------------*/
void send_ADC_results(void)
{
  unsigned char i,j,k, bitcnt;
  unsigned int res;


  ADC_init();
  ADC_get_select_ch(0); // Проведем одно пустое измерение

  rf_packet[0] = get_pack_first_byte(PACK_ADC);

  for (i=1;i<9;i++) rf_packet[i]=0;
  k      = 1;
  bitcnt = 0;
  for (i=0;i<6;i++)
  {
    if ((i==0) && ((wp.flags & TRANSMIT_REFCOD)!=0))
      res = wp.refcod;
    else
      res = ADC_get_select_ch(i);  // Получим результат преобразования в канале
    // Запакуем результат в отправляемый пакет
    for (j=0;j<10;j++)  // Пройдем все биты результата
    {
      if (bitcnt == 8)
      {
        k++;
        bitcnt=0;
      }
      else
      {
        rf_packet[k] <<=1;
      }
      if ((res & 0x0200)!=0)  rf_packet[k]++;
      res <<=1;
      bitcnt++;

    }
  }
  send_packet();
  ADC_switch_off();

}


/*--------------------------------------------------------------------------------------
    Выслать состояние кнопок
  --------------------------------------------------------------------------------------*/
void send_buttons(void)
{
  unsigned char i;
  rf_packet[0] = get_pack_first_byte(PACK_BUTTONS);
  for (i=1;i<9;i++) rf_packet[i]=0;
  if (fbut1 !=0) rf_packet[1] = 0xFF;
  if (fbut2 !=0) rf_packet[2] = 0xFF;
  fbut1 = 0;
  fbut2 = 0;
  send_packet();
  GIFR  = b11000000; // Стереть флаги прерываний
}



/*--------------------------------------------------------------------------------------
    Выслать подготовленный пакет
  --------------------------------------------------------------------------------------*/
void send_packet(void)
{
  PORTB &= ~P_LED;
  RF_send(rf_packet,9);
  while (flags.done!=1);
  PORTB |= P_LED;
}


/*--------------------------------------------------------------------------------------
    Генератор случайного байта
  --------------------------------------------------------------------------------------*/
unsigned char rand_gen_byte(unsigned long* seed)
{
  *seed = 1664525ul*(* seed) + 1013904223ul;
  return ((*seed) >> 24);
}

/*--------------------------------------------------------------------------------------
    Отправка в последовательный порт контрольного сообщения
  --------------------------------------------------------------------------------------*/
void heart_beat_msg(void)
{
   pack_cnt++;
   USART_Transmit(hex_to_ascii(pack_cnt>>28));
   USART_Transmit(hex_to_ascii(pack_cnt>>24));
   USART_Transmit(hex_to_ascii(pack_cnt>>20));
   USART_Transmit(hex_to_ascii(pack_cnt>>16));
   USART_Transmit(hex_to_ascii(pack_cnt>>12));
   USART_Transmit(hex_to_ascii(pack_cnt>>8));
   USART_Transmit(hex_to_ascii(pack_cnt>>4));
   USART_Transmit(hex_to_ascii(pack_cnt));
   USART_Transmit('\n');
   USART_Transmit('\r');
}


#pragma vector=INT0_vect
__interrupt void INT0_vect_interrupt( void )
{
  GICR  = 0; // Запретим прерывания, чтобы они вновь не возникали в случае если кнопку не отпустили
  fbut1 = 1;
}


#pragma vector=INT1_vect
__interrupt void INT1_vect_interrupt( void )
{
  GICR  = 0; // Запретим прерывания, чтобы они вновь не возникали в случае если кнопку не отпустили
  fbut2 = 1;
}
