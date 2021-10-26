/*
  Программа центрального приемника на ATMEGA8
*/
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



__flash char TEST[] = "Test";

extern __no_init TPARAMS wp;   // Рабочие параметры в RAM
extern char strbuf[INBUF_LEN];
extern unsigned char ccor1;
extern unsigned char ccor2;
extern unsigned char ccor3;

extern volatile unsigned int  modb_timeout;


char rf_packet[20];

volatile tflags flags;

int temperature;


D1W_device    nodes_temperatures[MAX_NODES][MAX_DEVICES];
unsigned int  nodes_an_inputs[MAX_NODES][MAX_AN_IN]; // Массив состояний аналоговых входов

unsigned char nodes_btns[MAX_NODES];

unsigned int  led_timeout;


unsigned char check_packet_crc(void);
unsigned char save_temperature(unsigned char node);
unsigned char save_an_inputs(unsigned char node);
void          read_buttons(unsigned char node);
unsigned char get_node_num(void);


void          temperature_to_com(unsigned char node);
void          analog_in_to_com(unsigned char node);
void          buttons_in_to_com(unsigned char node);

void          corr_val_to_lcd(void);
/*--------------------------------------------------------------------------------------

     MAIN

  --------------------------------------------------------------------------------------*/
void main( void )
{
/*
  P_OUT1    (0x04)   Управляемый выход 1
  P_OUT2    (0x04)   Управляемый выход 2
  P_BT1     (0x04)   Кнопка 1
  P_BT2     (0x08)   Кнопка 2
  P_BT3     (0x10)   Кнопка 3
  P_BT4     (0x20)   Кнопка 4
*/
  DDRB  = P_OUT1 | P_OUT2;
  PORTB = b11111111 & (~P_OUT1) & (~P_OUT2);

/*
  P_RS      (0x01)   Сигнал выбора регистра LCD
  P_RW      (0x02)   Сигнал чтения-записи   LCD
  P_E       (0x04)   Сигнал разрешения      LCD
  P_TST     (0x20)   Вспомогательный сигнал 
*/
  DDRC  = P_RS | P_RW | P_E | P_TST;
  PORTC = b11111111 & (~P_RW) & (~P_E);

/*
  P_RXD     (0x01)
  P_TXD     (0x02)
  P_LED     (0x04)  Управление светодиодом  
  P_DIN     (0x08)  Вход сигнала с приемника
  P_D0      (0x10)  Сигнал данных LCD
  P_D1      (0x20)  Сигнал данных LCD
  P_D2      (0x40)  Сигнал данных LCD
  P_D3      (0x80)  Сигнал данных LCD
*/
  DDRD  = P_TXD | P_LED | P_D0 |  P_D1 |  P_D2 |  P_D3;
  PORTD = 0xFF & (~P_DIN) & (~P_D0) & (~P_D1) & (~P_D2) & (~P_D3) ;

  TIMER0_init();
  USART_Init( 16 );  // 115200 при кварце 16 Мгц
  _SEI();           /* Enable interrupts => enable UART interrupts */

  // Инициализируем оперативные переменные из EEPROM
  if (Restore_settings_from_eeprom()== 0)
  {
    Restore_default_settings();
    USART_Transmit('c');
  }
  else
  {
    if (wp.version != PRG_VERSION)
    {
      Restore_default_settings();
      USART_Transmit('v');
    }
    else
    {
      USART_Transmit('.');
    }
  }



  __delay_cycles(1600000);

  lcdInit();
  lcdPrintData_P(TEST);


  RF_receiver_init(rf_packet);


  for( ; ; )        /* Forever */
  {
    if (flags.done)
    {
      unsigned char node;

      flags.done = 0;

      corr_val_to_lcd();
      lcdGotoXY(7,0);
      
      PORTD &= ~P_LED;
      led_timeout = 200;

      rc4_setup((unsigned char*)&wp.keycode, 4 );
      rc4_crypt((unsigned char*)rf_packet, 11 );

      node = get_node_num();
      if (node < MAX_NODES)
      {
        // Проверим контрольную сумму
        if (check_packet_crc())
        {
          switch (rf_packet[0] >> 4)
          {
          case PACK_TEMPER:
            save_temperature(node);
//            temperature_to_com(node);
            break;
          case PACK_BUTTONS:
            read_buttons(node);
//            buttons_in_to_com(node);
            break;
          case PACK_ADC:
            save_an_inputs(node);
//            analog_in_to_com(node);
            break;
          }
          lcdDataWrite('+'); // Контрольная сумма корректная 
        }
        else
        {
          lcdDataWrite('-'); // Ошибка контрольной суммы 
          
        }  
      }

    }

    if (DataInReceiveBuffer())
    {
      char ch;
      ch = USART_Receive();

      if (ch == 0x1B)
      {
        crlf();
        USART_Transmit('>');
        terminal();
      }
      
      modbus_task(ch);
      
    }

  }
}


/*--------------------------------------------------------------------------------------
   Проверка контрольной суммы полученного пакета
  --------------------------------------------------------------------------------------*/
unsigned char check_packet_crc(void)
{
  unsigned int crc;
  crc = ((unsigned int)rf_packet[9] << 8) | rf_packet[10];

  if (crc == GetBlockCRC((unsigned char*)rf_packet,9))
    return 1;
  else
    return 0;
}

/*--------------------------------------------------------------------------------------
  Запись в память принятого пакета с температурой
  --------------------------------------------------------------------------------------*/
unsigned char save_temperature(unsigned char node)
{
  unsigned char i,k;
  unsigned char res;

  // Ищем зарегистрированный идентификатор
  for (i=0;i<MAX_DEVICES;i++)
  {
     if (nodes_temperatures[node][i].id[0]!=0)
     {
        res = 0;
        for (k=0;k<6;k++)
        {
          if (nodes_temperatures[node][i].id[k]!= rf_packet[k+1])
          {
            res = 1;
            break;
          }
        }
        if (res == 0)
        {
          // Идентификатор уже зарегистрирован, запишем температуру для данного идентификатора
          nodes_temperatures[node][i].temperature =
            ((unsigned int)rf_packet[7] << 8) | rf_packet[8];
          return 1;
        }
     }
  }
  // Ищем пустой идентификатор
  for (i=0;i<MAX_DEVICES;i++)
  {
    if (nodes_temperatures[node][i].id[0]==0)
    {
      // Нашли. Запишем идентификатор и температуру
      for (k=0;k<6;k++) nodes_temperatures[node][i].id[k]= rf_packet[k+1];
      nodes_temperatures[node][i].temperature =
          ((unsigned int)rf_packet[7] << 8) | rf_packet[8];
      return 1;
    }
  }
  return 1;
}


/*--------------------------------------------------------------------------------------

  --------------------------------------------------------------------------------------*/
unsigned char save_an_inputs(unsigned char node)
{
  unsigned char i,j,k, bitcnt;

  bitcnt = 0;
  k      = 0;

  // Очистим предварительно
  for (i=0;i<MAX_AN_IN;i++) nodes_an_inputs[node][i]=0;

  // Распакуем результат
  for (i=1;i<9;i++)     // Проходим все байты пакета с результатами АЦП
  {
    for (j=0;j<8;j++)   // Проходим все биты в байте
    {
      if (bitcnt == 10) // В результате 10 бит
      {
        k++;  // Перейдем к следующему результату
        if (k==MAX_AN_IN) return 1;
        bitcnt=0;
      }
      else
      {
        nodes_an_inputs[node][k] <<=1;
      }
      if ((rf_packet[i] & 0x80)!=0)  nodes_an_inputs[node][k]++;
      rf_packet[i] <<=1;

      bitcnt++;

    }
  }
  return 1;

}


/*--------------------------------------------------------------------------------------
   Выставить состояние внешних выходов в соответствии с нажатыми кнопками
  --------------------------------------------------------------------------------------*/
void read_buttons(unsigned char node)
{
  if (rf_packet[1]!=0) PORTB ^= P_OUT1;
  if (rf_packet[2]!=0) PORTB ^= P_OUT2;
}

/*--------------------------------------------------------------------------------------
   Получить номер узла приславшего пакет
  --------------------------------------------------------------------------------------*/
unsigned char get_node_num(void)
{
  return rf_packet[0] & 0x07;
}


/*--------------------------------------------------------------------------------------
   Выдать информацию о замерах температуры для заданного узла в последовательный порт
  --------------------------------------------------------------------------------------*/
void temperature_to_com(unsigned char node)
{
  unsigned char i,j;
  float tmp;

  crlf();
  USART_sendstr("Node ");
  USART_Transmit(hex_to_ascii(node>>4));
  USART_Transmit(hex_to_ascii(node));
  crlf();
  for (i=0;i<MAX_DEVICES;i++)
  {
     for (j=0;j<6;j++)
     {
       USART_Transmit(hex_to_ascii(nodes_temperatures[node][i].id[j]>>4));
       USART_Transmit(hex_to_ascii(nodes_temperatures[node][i].id[j]));
     }
     USART_Transmit('=');
     tmp = ((float)nodes_temperatures[node][i].temperature)/2;
     float_conversion(tmp,3,(unsigned char*)strbuf,'E',1,1);
     USART_sendstr(strbuf);
     crlf();
  }
}

/*--------------------------------------------------------------------------------------
   Выдать информацию о аналоговых входах для заданного узла в последовательный порт
  --------------------------------------------------------------------------------------*/
void analog_in_to_com(unsigned char node)
{
  unsigned char i;

  crlf();
  USART_sendstr("Node ");
  USART_Transmit(hex_to_ascii(node>>4));
  USART_Transmit(hex_to_ascii(node));
  crlf();
  for (i=0;i<MAX_AN_IN;i++)
  {
     USART_Transmit(hex_to_ascii(i>>4));
     USART_Transmit(hex_to_ascii(i));
     USART_Transmit('=');
     USART_Transmit(hex_to_ascii(nodes_an_inputs[node][i]>>12));
     USART_Transmit(hex_to_ascii(nodes_an_inputs[node][i]>>8));
     USART_Transmit(hex_to_ascii(nodes_an_inputs[node][i]>>4));
     USART_Transmit(hex_to_ascii(nodes_an_inputs[node][i]));
     crlf();
  }


}

/*--------------------------------------------------------------------------------------
   Выдать информацию о пакете с информацией о нажатых кнопках
  --------------------------------------------------------------------------------------*/
void buttons_in_to_com(unsigned char node)
{
  unsigned char i;

  crlf();
  USART_sendstr("Node ");
  USART_Transmit(hex_to_ascii(node>>4));
  USART_Transmit(hex_to_ascii(node));
  crlf();
  for (i=1;i<9;i++)
  {
     USART_Transmit(hex_to_ascii(rf_packet[i]>>4));
     USART_Transmit(hex_to_ascii(rf_packet[i]));
     USART_Transmit(' ');
  }
  crlf();
}


void crlf(void)
{
  USART_Transmit('\n');
  USART_Transmit('\r');
}


void corr_val_to_lcd(void)
{
  lcdGotoXY(0,1);
  lcdDataWrite(hex_to_ascii(ccor1>>4));
  lcdDataWrite(hex_to_ascii(ccor1));
  lcdDataWrite(hex_to_ascii(ccor2>>4));
  lcdDataWrite(hex_to_ascii(ccor2));
  lcdDataWrite(hex_to_ascii(ccor3>>4));
  lcdDataWrite(hex_to_ascii(ccor3));

}


/*--------------------------------------------------------------------------------------
   Прерывания таймера 0 используються для формирования временных интервалов
   Период прерываний = 0.001024 мс
  --------------------------------------------------------------------------------------*/
#pragma vector=TIMER0_OVF_vect
__interrupt void TIMER0_OVF_vect_interrupt( void )
{
 __watchdog_reset(); // Не забываем сбрасывать WDT

  if (led_timeout)
  {
    led_timeout--;
  }
  else
    PORTD |= P_LED;


  // Отработка таймаута modbus протокола
  if (modb_timeout)
  {
    modb_timeout--;
  }


}
