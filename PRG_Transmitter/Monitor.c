#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "main.h"
#include "USART.h"
#include "Monitor.h"
#include "wrk_params.h"
#include "DS1Wire.h"
#include "ADC.h"
#include "Timers.h"
#include "RF_settings.h"
#include "RF_transmitter.h"
#include "Util.h"

extern __no_init D1W_device devices[MAX_DEVICES];

#define  MONIT_OFF_TIMEOUT 65535
#define  LED_BLINK_TIMEOUT 100

#define INBUF_LEN 30
char strbuf[INBUF_LEN];

volatile unsigned int monit_off_tim; // Число тиков до выключения монитора
unsigned char         led_blink_tim;

unsigned char exec_cmd(void);

/*--------------------------------------------------------------------------------------
  Процедура работы через последовательный интерфейс
  --------------------------------------------------------------------------------------*/
void terminal(void)
{
  char ch;
  unsigned char k = 0;

  PORTD |= P_TXD;
  // Инициализируем прерывания от таймера 0, для обслуживания всяких временных интервалов	
  // Частоту следования прерываний выбираем (16 000 000/64)/256 = 976,5625 Гц -> 0.001024 мс
  TCCR0  = b00000011; // Предделитель = 64
  TIFR  |= b00000001; //
  TIMSK |= b00000001; // Разрешаем прерывания

  monit_off_tim = 0; // Монтор выключиться спустя приблизительно минуту после последнего принятого символа
  led_blink_tim = 0;

  do
  {
    if (DataInReceiveBuffer())
    {
      unsigned char res;
      monit_off_tim = 0;
      ch = USART_Receive();
      if (ch=='\r')
      {
        crlf();
        USART_Transmit(' ');
        res = exec_cmd();
        crlf();
        if (res==0)
        {
          wait_until_tx_complete();
          break;
        }
        USART_Transmit('>');
        k = 0;
      }
      else
      {
        // Отправить эхо
        USART_Transmit(ch);
        strbuf[k] = ch;
        k++;
        strbuf[k] = 0;
        if (k == (INBUF_LEN-1))
        {
          USART_sendstr("\n\rIn buf. overflow!\n\r>");
          k = 0;
        }
      }
    }

  }
  while (monit_off_tim < MONIT_OFF_TIMEOUT);


  // Выход из монитора
  PORTB |= P_LED;     // Гасим светодиод
  TIMSK &= b11111110; // Запрещаем прерывания от таймера 0
  TCCR0  = 0;
}
	

/*--------------------------------------------------------------------------------------
   Выполнение поступившей команды
  --------------------------------------------------------------------------------------*/
unsigned char exec_cmd(void)
{
  unsigned char i,j,k;
  char* cname;
  char ch;

  i=0;

  while (isalnum(strbuf[i])!=0) i++;

  k = i;
  ch = strbuf[k];
  strbuf[k] = 0;  // Обозначим конец строки имени параметра
  k++;

  // Искать параметр с заданным именем
  for (i=0;i < get_params_num(); i++)
  {
    cname = get_params_name(i);
    if (strcmp(strbuf, cname)==0)
    {
      // Найден параметр
      if (ch=='=')
      {
        j = k;
        // Обнаружена команда записи, прочитать аргумент
        while (isalnum(strbuf[j])!=0) j++;
        Str_to_param((unsigned char*)&strbuf[k],i);
        Param_to_str((unsigned char*)strbuf,i);
        USART_Transmit('=');
        USART_sendstr(strbuf);
        return 1;
      }

    }
  }
  // Если не нашли такой параметр, то проверить не запрос ли это всех параметров
  if (ch=='?')
  {
    for (i=0;i < get_params_num(); i++)
    {
      crlf();
      cname = get_params_name(i);
      USART_sendstr(cname);
      USART_Transmit('=');
      Param_to_str((unsigned char*)strbuf,i);
      USART_sendstr(strbuf);
    }
    return 1;
  }

  if (ch=='+')  // Сохраним все параметры в EEPROM
  {
    Save_Params_To_EEPROM();
    USART_sendstr("All parameters saved!");
    return 1;
  }

  if (ch=='-')  // Восстановим параметры из EEPROM
  {
    Restore_settings_from_eeprom();
    USART_sendstr("Restored from EEPROM!");
    return 1;
  }

  if (ch=='*')  // Восстановим параметры по умолчанию
  {
    Restore_default_settings();
    USART_sendstr("Restored defaults!");
    return 1;
  }

  if (ch=='.')  // Выйти из режима терминала
  {
    return 0;
  }

  if (ch=='#')  // Показать номер устройства
  {
    ch = (char)get_pack_first_byte(0);
    USART_Transmit(hex_to_ascii(ch>>4));
    USART_Transmit(hex_to_ascii(ch));
    return 1;
  }

  if (ch=='^')  // Показать результаты измерения АЦП
  {
    unsigned int res;
    ADC_init();
    ADC_get_select_ch(0); // Проведем одно пустое измерение
    crlf();
    for (i=0;i<6;i++)
    {
      res = ADC_get_select_ch(i);  // Получим результат преобразования в канале
      USART_Transmit(hex_to_ascii(res>>12));
      USART_Transmit(hex_to_ascii(res>>8));
      USART_Transmit(hex_to_ascii(res>>4));
      USART_Transmit(hex_to_ascii(res));
      crlf();
    }
    ADC_switch_off();
    return 1;
  }

/*
  if (ch=='!')
  {
    unsigned int temperature;
    for (i=0;i<MAX_DEVICES;i++)
    {
      if (devices[i].id[0] == DS1820_FAMILY_ID)
      {
        temperature = DS1820_ReadTemperature(devices[i].id);
        sprintf((char*)strbuf,"%f",(float)temperature/2);
        strcat(strbuf,"\n\r");
        USART_sendstr(strbuf);
      }

    }
    return 1;
  }
*/



  return 1;
}






void crlf(void)
{
  USART_Transmit('\n');
  USART_Transmit('\r');
}

	
	
#pragma vector=TIMER0_OVF_vect
__interrupt void TIMER0_OVF_vect_interrupt( void )
{
 __watchdog_reset(); // Не забываем сбрасывать WDT, поскольку в мониторе можем находиться очень долго

 if (monit_off_tim < 65535) monit_off_tim++;

 if (led_blink_tim == LED_BLINK_TIMEOUT)
 {
 	 led_blink_tim = 0;
 	 PORTB ^= P_LED;
 }
 else
 {
 	 led_blink_tim++;
 }
}
