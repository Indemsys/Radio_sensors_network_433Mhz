#include <iom8.h>
#include <ina90.h>
#include <string.h>
#include <pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "Util.h"
#include "RF_settings.h"
#include "wrk_params.h"

/*
  Объявления рабочих параметров
*/

__no_init __eeprom TPARAMS ee_wp;       // Рабочие параметры в EEPROM
__no_init __eeprom unsigned int ee_crc; // Контрольная сумма в EEPROM

__no_init TPARAMS wp;                   // Рабочие параметры в RAM



__flash DWAR_TYPE dwvar[]=
{
  {
    "WDTPER",                // Строковое описание
    &ee_wp.wdt_period,       // Указатель на значение переменной в EEPROM
    &wp.wdt_period,          // Указатель на значение переменной в RAM
    tunsigned_char,          // Идентификатор типа переменной
    7,                       // Значение по умолчанию
    0,                       // Минимальное возможное значение
    7,                       // Максимальное возможное значение
    0,                       // Количество знаков после запятой
    0,                       // Аттрибуты переменной
    0,                       // Указатель на функцию выполняемую после редактирования
    sizeof(unsigned char)    // Длинна переменной
  },
  {
    "WDTDVC",
    &ee_wp.wdt_div_const,
    &wp.wdt_div_const,
    tunsigned_char,
    2,
    0,
    255,
    0,
    0,
    0,
    sizeof(unsigned char)
  },
  {
    "WDTDVV",
    &ee_wp.wdt_div_var,
    &wp.wdt_div_var,
    tunsigned_char,
    0,
    0,
    255,
    0,
    0,
    0,
    sizeof(unsigned char)
  },
  {
    "BTRATE",
    &ee_wp.rf_bitrate,
    &wp.rf_bitrate,
    tunsigned_int,
    16000,
    4000,
    65535,
    0,
    0,
    0,
    sizeof(unsigned int)
  },
  {
    "SFLAGS",
    &ee_wp.flags,
    &wp.flags,
    tunsigned_int,
    0,
    0,
    65535,
    0,
    0,
    0,
    sizeof(unsigned int)
  },
  {
    "PREAMB",
    &ee_wp.preamble,
    &wp.preamble,
    tunsigned_int,
    PREAMBLE,
    0,
    65535,
    0,
    0,
    0,
    sizeof(unsigned int)
  },
  {
    "REFCOD",
    &ee_wp.refcod,
    &wp.refcod,
    tunsigned_int,
    999,
    0,
    65535,
    0,
    0,
    0,
    sizeof(unsigned int)
  },
  {
    "KEYCOD",
    &ee_wp.keycode,
    &wp.keycode,
    tunsigned_long,
    999,
    0,
    0xFFFFFFFF,
    0,
    0,
    0,
    sizeof(unsigned long)
  },



};

int get_params_num(void)
{
  return  (sizeof(dwvar)/sizeof(dwvar[0]));
}

char* get_params_name(unsigned char indx)
{
  return  dwvar[indx].name;
}


/* ==========================================================
   Преобразовать параметр в строку
   ========================================================== */
void Param_to_str(unsigned char *buf,int indx)
{
  switch (dwvar[indx].vartype)
  {
  case tunsigned_char:
  case tsigned_char:
    *buf++='0';
    *buf++='x';
    *buf++=hex_to_ascii(*(char*)dwvar[indx].val>>4);
    *buf++=hex_to_ascii(*(char*)dwvar[indx].val);
    *buf++=0;
//    sprintf((char*)buf,"%d",*((unsigned char*)dwvar[indx].val));
    break;
//    sprintf((char*)buf,"%d",*((signed char*)dwvar[indx].val));
//    break;
  case tunsigned_int:
  case tsigned_int:
//    sprintf((char*)buf,"%d",*((unsigned int*)dwvar[indx].val));
    *buf++='0';
    *buf++='x';
    *buf++=hex_to_ascii(*(int*)dwvar[indx].val>>12);
    *buf++=hex_to_ascii(*(int*)dwvar[indx].val>>8);
    *buf++=hex_to_ascii(*(int*)dwvar[indx].val>>4);
    *buf++=hex_to_ascii(*(int*)dwvar[indx].val);
    *buf++=0;

    break;
  case tunsigned_long:
  case tsigned_long:
    *buf++='0';
    *buf++='x';
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val>>28);
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val>>24);
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val>>20);
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val>>16);
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val>>12);
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val>>8);
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val>>4);
    *buf++=hex_to_ascii(*(unsigned long*)dwvar[indx].val);
    *buf++=0;
    break;
  case tfloat:
//    sprintf((char*)buf,"%f",*((float*)dwvar[indx].val));
    break;
  case tdouble:
//    sprintf((char*)buf,"%f",*((double*)dwvar[indx].val));
    break;
  case tstring:
//    sprintf((char*)buf,"%s",(char*)(dwvar[indx].val));
    break;
  }
}

/* ==========================================================
   Преобразовать строку в параметр
   ========================================================== */
void Str_to_param(unsigned char *buf,int indx)
{
  unsigned char uch_tmp;
  signed char sch_tmp;
  unsigned int uin_tmp;
  signed int sin_tmp;
  unsigned long ulg_tmp;
  signed long slg_tmp;
  float f_tmp;
  double d_tmp;
  switch (dwvar[indx].vartype)
  {
  case tunsigned_char:
    uch_tmp=strtol((char*)buf,0,0);
    if (uch_tmp>((unsigned char)dwvar[indx].max)) uch_tmp=(unsigned char)dwvar[indx].max;
    if (uch_tmp<((unsigned char)dwvar[indx].min)) uch_tmp=(unsigned char)dwvar[indx].min;
    *(unsigned char*)dwvar[indx].val=uch_tmp;
    break;
  case tsigned_char:
    sch_tmp=strtol((char*)buf,0,0);
    if (sch_tmp>((signed char)dwvar[indx].max)) sch_tmp=(signed char)dwvar[indx].max;
    if (sch_tmp<((signed char)dwvar[indx].min)) sch_tmp=(signed char)dwvar[indx].min;
    *(signed char*)dwvar[indx].val=sch_tmp;
    break;
  case tunsigned_int:
    uin_tmp=strtol((char*)buf,0,0);
    if (uin_tmp>((unsigned int)dwvar[indx].max)) uin_tmp=(unsigned int)dwvar[indx].max;
    if (uin_tmp<((unsigned int)dwvar[indx].min)) uin_tmp=(unsigned int)dwvar[indx].min;
    *(unsigned int*)dwvar[indx].val=uin_tmp;
    break;
  case tsigned_int:
    sin_tmp=strtol((char*)buf,0,0);
    if (sin_tmp>((signed int)dwvar[indx].max)) sin_tmp=(signed int)dwvar[indx].max;
    if (sin_tmp<((signed int)dwvar[indx].min)) sin_tmp=(signed int)dwvar[indx].min;
    *(signed int*)dwvar[indx].val=sin_tmp;
    break;
  case tunsigned_long:
    ulg_tmp=strtol((char*)buf,0,0);
    if (ulg_tmp>((unsigned long)dwvar[indx].max)) ulg_tmp=(unsigned long)dwvar[indx].max;
    if (ulg_tmp<((unsigned long)dwvar[indx].min)) ulg_tmp=(unsigned long)dwvar[indx].min;
    *(unsigned long*)dwvar[indx].val=ulg_tmp;
    break;
  case tsigned_long:
    slg_tmp=strtol((char*)buf,0,0);
    if (slg_tmp>((signed long)dwvar[indx].max)) slg_tmp=(signed long)dwvar[indx].max;
    if (slg_tmp<((signed long)dwvar[indx].min)) slg_tmp=(signed long)dwvar[indx].min;
    *(signed long*)dwvar[indx].val=slg_tmp;
    break;
  case tfloat:
    f_tmp=atof((char*)buf);
    if (f_tmp>((float)dwvar[indx].max)) f_tmp=(float)dwvar[indx].max;
    if (f_tmp<((float)dwvar[indx].min)) f_tmp=(float)dwvar[indx].min;
    *(float*)dwvar[indx].val=f_tmp;
    break;
  case tdouble:
    d_tmp=atof((char*)buf);
    if (d_tmp>((double)dwvar[indx].max)) d_tmp=(double)dwvar[indx].max;
    if (d_tmp<((double)dwvar[indx].min)) d_tmp=(double)dwvar[indx].min;
    *(double*)dwvar[indx].val=d_tmp;
    break;
  }
}

// ==========================================================
//  Перезаписать в EEPROM установки по умолчанию. Если нет ошибки то возвращает 0
// ==========================================================
void Restore_default_settings(void)
{
  unsigned int i;

  // Загрузить параметры значениями по умолчанию
  for (i=0;i<get_params_num();i++)
  {
    switch (dwvar[i].vartype)
    {
    case tunsigned_char:
      *(unsigned char*)dwvar[i].val=(unsigned char)dwvar[i].defval; break;
    case tsigned_char:
      *(signed char*)dwvar[i].val=(signed char)dwvar[i].defval; break;
    case tunsigned_int:
      *(unsigned int*)dwvar[i].val=(unsigned int)dwvar[i].defval; break;
    case tsigned_int:
      *(signed int*)dwvar[i].val=(signed int)dwvar[i].defval; break;
    case tunsigned_long:
      *(unsigned long*)dwvar[i].val=(unsigned long)dwvar[i].defval; break;
    case tsigned_long:
      *(signed long*)dwvar[i].val=(signed long)dwvar[i].defval; break;
    case tfloat:
      *(float*)dwvar[i].val=dwvar[i].defval; break;
    case tdouble:
      *(double*)dwvar[i].val=dwvar[i].defval; break;
    }
  }

  // Выполнение инициализационных функций параметров
  for (i=0;i<get_params_num();i++)
  {
    if (dwvar[i].func!=0) dwvar[i].func();
  }

  Save_Params_To_EEPROM();
}
// ==========================================================
//  Сохранить всю область рабочих параметров в EEPROM
// ==========================================================
void Save_Params_To_EEPROM(void)
{
  wp.version = PRG_VERSION;
  ee_wp      = wp;
  ee_crc     = GetBlockCRC((unsigned char*)&wp,sizeof(wp));

}


// ==========================================================
//  Восстановить область рабочих параметров из EEPROM
// ==========================================================
unsigned char Restore_settings_from_eeprom(void)
{
  wp = ee_wp;
  if (ee_crc != GetBlockCRC((unsigned char*)&wp,sizeof(wp)))
    return 0;
  else
    return 1;
}

