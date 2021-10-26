#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include "main.h"
#include "USART.h"
#include "DS1Wire.h"
#include "Timers.h"
#include "ADC.h"
#include "RF_transmitter.h"
#include "Util.h"


void ADC_init(void)
{
  // Установка источника опорного напряжения - внутренний 2.56 В (bit 6, bit 7)
  // Результат выровнен в право (bit 5)
  // Мультиплексор переключен на канал 0
  ADMUX  = b11000000;

  // АЦП включен в однократном режиме, прерывания запрещенв, тактовая частота в 128 раз ниже частоты кварца 
  ADCSRA  = b10000111;
  
}

unsigned int ADC_get_select_ch(unsigned char ch)
{
  ADMUX  = (ADMUX & 0xF0) | ( ch & 0x0F) ;
  ADCSRA  = b11010111; // Очищаем флаг прерывания и стартуем 
  
  while ((ADCSRA & b00010000)==0); // Ждем появления флага ADIF
  
  return ADC;
}


void ADC_switch_off(void)
{
  ADMUX  = 0;
  ADCSRA  = 0;
}  


