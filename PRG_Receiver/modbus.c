#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include "main.h"
#include "USART.h"
#include "Monitor.h"
#include "Wrk_params.h"
#include "Timers.h"
#include "..\PRG_Transmitter\RF_settings.h"
#include "RF_receiver.h"
#include "Util.h"
#include "bin_defines.h"
#include "lcd.h"
#include "rc4.h"
#include "modbus.h"

extern D1W_device    nodes_temperatures[MAX_NODES][MAX_DEVICES];
extern unsigned int  nodes_an_inputs[MAX_NODES][MAX_AN_IN]; // Массив состояний аналоговых входов
extern char strbuf[INBUF_LEN];
extern __no_init TPARAMS wp;

#define STATE_IDLE   0
#define STATE_DATA   1
#define STATE_END    2

static   unsigned char  state=STATE_IDLE;
volatile unsigned int   modb_timeout;
static   unsigned int   scnt; // Промежуточный счетчик поступающих байт
static   unsigned char  addr; // Адрес полученный из пакета
static   unsigned char  fnum; // Номер функции полученный из пакета



void ascii_mode_task(char ch);
void Read_hld_reg(void);
void Preset_single_register(void);
void Preset_multiple_registers(void);
void Read_EEPROM_reg(void);
void Preset_EEPROM_reg(void);
void Read_reg_type(void);
void Send_read_response(unsigned char *pval,unsigned char nm);
void Send_exception_code(unsigned char exceptc);
unsigned char get_lrc(unsigned char *buf,unsigned int len);
void reset_timeout(void);

__flash TMODB_tcmd modbus_cmd[]=
{
  {
    3,   // чтение регистра хранения
    0,
    (void (*)(void)) Read_hld_reg
  },
  {
    6,   // запись регистра хранения
    0,
    (void (*)(void)) Preset_single_register
  },
  {
    16,   // запись n-регистров хранения
    1,
    (void (*)(void)) Preset_multiple_registers
  },
  {
    65,   // чтение регистра из EEPROM
    0,
    (void (*)(void)) Read_EEPROM_reg
  },
  {
    66,   // запись регистра в EEPROM
    0,
    (void (*)(void)) Preset_EEPROM_reg
  },
  {
    67,   // Чтение типа и длинны регистра хранения
    0,
    (void (*)(void)) Read_reg_type
  },

};

#define SIZE_CMD_ARR (sizeof(modbus_cmd)/sizeof(modbus_cmd[0]))
// MODBUS exception codes
#define ILLEGAL_FUNCTION      1
#define ILLEGAL_DATA_ADDRESS  2
#define ILLEGAL_DATA_VALUE    3
#define SLAVE_DEVICES_FAILURE 4



void modbus_task(char ch)
{
  ascii_mode_task(ch);
}


/*--------------------------------------------------------------------------------------
   Обработчик MODBUS протокола в режиме ASCII

     +-----+------+------+------+-----+-----+
     ¦старт¦адрес ¦ ф-ия ¦данные¦ LRC ¦конец¦
     +-----+------+------+------+-----+-----+
     ¦1 сим¦2 сим ¦2 сим ¦n сим ¦2 сим¦2 сим¦
     ¦   : ¦      ¦      ¦      ¦     ¦CR LF¦
     +-----+------+------+------+-----+-----+

Пример: :010301000003B8

        :010301000001BA 

        :010303000006B3 - прочитать 6-ть каналов АЦП с датчика 0 

  --------------------------------------------------------------------------------------*/
void ascii_mode_task(char ch)
{
  unsigned int  i;
  unsigned char lrc;
  
  
  if (modb_timeout == 0) state = STATE_IDLE;

  switch (state)
  {
  case STATE_IDLE:
    if (ch == ':')
    {
      reset_timeout();
      state = STATE_DATA;
      scnt = 0;
    }
    break;
  case STATE_DATA:
    reset_timeout();
    if (ch == 0x0D)
    {
      state = STATE_END;
      break;
    }
    strbuf[scnt] = ch;
    scnt++;
    if (scnt > INBUF_LEN)
    {
      state = STATE_IDLE;
      break;
    }

    break;
  case STATE_END:
    reset_timeout();

    if (ch == 0x0A)
    {
      strbuf[scnt] = 0;
      lrc = Str_to_byte((unsigned char *)&strbuf[scnt-2]);
      // Проверим контрольную сумму
      if (lrc!=get_lrc((unsigned char *)strbuf,scnt-2))
      {
        state = STATE_IDLE;
        break;
      }
      else
      {
        addr =  Str_to_byte((unsigned char *)&strbuf[0]); //Str_to_num((unsigned char *)strbuf, 16);
        // Проверим адрес
        if ((addr!=0) && (addr!=wp.devaddr))
        {
          state = STATE_IDLE;
          break;
        }
        else
        {
          // Проверим функцию
          fnum = Str_to_byte((unsigned char *)&strbuf[2]);
          for (i=0;i<SIZE_CMD_ARR;i++)
          {
            if  (modbus_cmd[i].id == fnum)
            {
              // Функция найдена выполняем
              modbus_cmd[i].func();
              state = STATE_IDLE;
              return;
            }
          }
          Send_exception_code(ILLEGAL_FUNCTION);

        }
      }
      state = STATE_IDLE;
      break;
    }
    state = STATE_IDLE;


    break;


  }


}


unsigned char get_lrc(unsigned char *buf,unsigned int len)
{
  unsigned char lrc = 0 ;
  while (len--) lrc += *buf++ ;
  return (unsigned char)(-lrc);
}


void reset_timeout(void)
{
   modb_timeout = MODB_TIMEOUT;
}






/*--------------------------------------------------------------------------------------
  Чтение внутренних переменных из устройства

  Интерпретация поля данных
     +----------------+-------------------------+
     ¦стартовый адрес ¦ количество читаемых     |
     |                |  2-х байтных регистров  |
     +----------------+-------------------------+
     ¦   4 сим        ¦       4 сим             |
     ¦                ¦                         |
     +----------------+-------------------------+


     Интерпретация стартового адреса:

     Старший байт указывает тип данных: 1 - идентификаторы температурных датчиков
                                        2 - значения температуры с температурных датчиков
                                        3 - значения АЦП с удадленных датчиков

     Младший байт указывает индексы данных в двумерных массивах:
       Старший полубайт указывает номер удаленного узла (датчика)
       Младший полубайт указывает номер температурного датчика в узле или номер аналогового входа
  --------------------------------------------------------------------------------------*/
void Read_hld_reg(void)
{
  unsigned int  nm;
  unsigned char type, dev, chan;
  unsigned char *pval;

  nm   = Str_to_byte((unsigned char *)&strbuf[10])*2; // Вычислим количество байт для передачи


  type =  Str_to_byte((unsigned char *)&strbuf[4]);
  dev  = (Str_to_byte((unsigned char *)&strbuf[6]) >> 4) & 0x000F;
  chan =  Str_to_byte((unsigned char *)&strbuf[6]) & 0x000F;
  switch (type)
  {
  case 1:
    if (dev  >= MAX_NODES)   return;
    if (chan >= MAX_DEVICES) return;
    pval = nodes_temperatures[dev][chan].id;
    break;
  case 2:
    if (dev  >= MAX_NODES)   return;
    if (chan >= MAX_DEVICES) return;
    pval = (unsigned char*)&nodes_temperatures[dev][chan].temperature;
    break;
  case 3:
    if (dev  >= MAX_NODES)   return;
    if (chan >= MAX_AN_IN)   return;
    pval = (unsigned char*)&nodes_an_inputs[dev][chan];
    break;
  }

  Send_read_response(pval,nm);

}



void Preset_single_register(void)
{

}
void Preset_multiple_registers(void)
{

}
void Read_EEPROM_reg(void)
{


}
void Preset_EEPROM_reg(void)
{


}
void Read_reg_type(void)
{

}


/*--------------------------------------------------------------------------------------
  Передача пакета на запрос чтения
  --------------------------------------------------------------------------------------*/
void Send_read_response(unsigned char *pval,unsigned char nm)
{
  unsigned char lrc;
  unsigned char i;
  unsigned char b;
  USART_Transmit(':');
  Num_to_str((unsigned char *)strbuf,   2,wp.devaddr,16);
  Num_to_str((unsigned char *)&strbuf[2],2,fnum,16);
  Num_to_str((unsigned char *)&strbuf[4],2,nm,16);
  for (i=0;i<nm;i++)
  {
    b = *(pval++);
    Num_to_str((unsigned char *)&strbuf[6+i*2],2,b,16);
  }
  lrc = get_lrc((unsigned char *)strbuf,nm+6);
  Num_to_str((unsigned char *)&strbuf[6+nm*2],2,lrc,16);
  strbuf[8+nm*2]  = 0x0D;
  strbuf[9+nm*2]  = 0x0A;
  strbuf[10+nm*2] = 0;
  USART_sendstr(strbuf);
}

/*--------------------------------------------------------------------------------------
  Функци выдачи кода исключения
  --------------------------------------------------------------------------------------*/
void Send_exception_code(unsigned char exceptc)
{
  unsigned char lrc;
  USART_Transmit(':');
  fnum |=0x80;

  Num_to_str((unsigned char *)strbuf,   2,wp.devaddr,16);
  Num_to_str((unsigned char *)&strbuf[2],2,fnum,16);
  Num_to_str((unsigned char *)&strbuf[4],2,exceptc,16);
  lrc = get_lrc((unsigned char *)strbuf,6);
  Num_to_str((unsigned char *)&strbuf[6],2,lrc,16);
  strbuf[8]  = 0x0D;
  strbuf[9]  = 0x0A;
  strbuf[10] = 0;
  USART_sendstr(strbuf);
}
