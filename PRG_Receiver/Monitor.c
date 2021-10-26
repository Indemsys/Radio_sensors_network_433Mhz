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
#include "Timers.h"
#include "..\PRG_Transmitter\RF_settings.h"
#include "RF_receiver.h"
#include "Util.h"

extern __no_init D1W_device devices[MAX_DEVICES];


char strbuf[INBUF_LEN];


unsigned char exec_cmd(void);

/*--------------------------------------------------------------------------------------
  Процедура работы через последовательный интерфейс
  --------------------------------------------------------------------------------------*/
void terminal(void)
{
  char ch;
  unsigned char k = 0;


  do
  {
    if (DataInReceiveBuffer())
    {
      unsigned char res;
      ch = USART_Receive();
      if (ch=='\r')
      {
        crlf();
        USART_Transmit(' ');
        res = exec_cmd();
        crlf();
        if (res==0)
        {
          break;
        }
        USART_Transmit('>');
        k = 0;
      }
      else if (ch==0x1B)
      {
        crlf();
        break;
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
  while (1);
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
      USART_Transmit('\n');
      USART_Transmit('\r');
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

  if (ch=='.')  // Восстановим параметры по умолчанию
  {
    return 0;
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








	
	
