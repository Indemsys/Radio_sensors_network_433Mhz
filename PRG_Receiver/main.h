#define    PRG_VERSION 103
#define    FOSC        16000000ul

// PORTD
#define    P_RXD     (0x01)
#define    P_TXD     (0x02)
#define    P_LED     (0x04) // Управление светодиодом 
#define    P_DIN     (0x08) // Вход сигнала с приемника
#define    P_DIN_OFS (0x03) // Смещение P_DIN относительно разряда 0

#define    P_D0      (0x10) // Сигнал данных LCD
#define    P_D1      (0x20) // Сигнал данных LCD
#define    P_D2      (0x40) // Сигнал данных LCD
#define    P_D3      (0x80) // Сигнал данных LCD

// PORTB

#define    P_OUT1    (0x01)  // Управляемый выход 1  
#define    P_OUT2    (0x02)  // Управляемый выход 2  
#define    P_BT1     (0x04)  // Кнопка 1  
#define    P_BT2     (0x08)  // Кнопка 2  
#define    P_BT3     (0x10)  // Кнопка 3 
#define    P_BT4     (0x20)  // Кнопка 4 
                     
                     
// PORTC             
#define    P_RS      (0x01)  // Сигнал выбора регистра LCD
#define    P_RW      (0x02)  // Сигнал чтения-записи   LCD
#define    P_E       (0x04)  // Сигнал разрешения      LCD
#define    P_TST     (0x20)  // Вспомогательный сигнал !!! поменять определеня


#define    INBUF_LEN 100
#define    MAX_AN_IN  6 // Количество принимаемых в пакете кодов аналоговых входов


typedef struct 
{
  unsigned char preamble:1;
  unsigned char   centre:1;
  unsigned char     done:1;
  unsigned char    lastb:1;
  
} tflags;  


typedef struct
{
    unsigned char id[8];    // The 64 bit identifier.
    unsigned int  temperature;
} D1W_device;

enum vartypes {tunsigned_char, tsigned_char, tunsigned_int, tsigned_int, tunsigned_long, tsigned_long, tfloat, tdouble, tstring};

typedef struct
{
  char*                  name;          // Строковое описание
  __eeprom void*         ee_val;        // Указатель на значение переменной в EEPROM
  void*                  val;           // Указатель на значение переменной в RAM
  enum  vartypes         vartype;       // Идентификатор типа переменной
  double                 defval;        // Значение по умолчанию
  double                 min;           // Минимальное возможное значение
  double                 max;           // Максимальное возможное значение
  char                   precision;     // Количество знаков после запятой
  unsigned char          attr;          // Аттрибуты переменной
  void                   (*func)(void); // Указатель на функцию выполняемую после редактирования
  unsigned char          varlen;        // Длинна переменной
} DWAR_TYPE;



typedef struct
{
  unsigned int  version;        // Версия программы
  unsigned int  rf_bitrate;     // Величина задающая скорость передачи по радиоканалу
                                // Равна числу тактов осцилятора формирующих интервал в половину бита 
  unsigned int  flags;          // Флаги управляющие работой передатчика
  unsigned int  preamble;       // Преамбула радио пакета
  unsigned long keycode;        // Ключ шифрования 
  unsigned char devaddr;        // Адрес устройства в сети MODBUS
  
} TPARAMS;

void          crlf(void);
