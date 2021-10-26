#define    PRG_VERSION 106

#define    FOSC   16000000ul

// Порт D
#define    P_RXD  (0x01)
#define    P_TXD  (0x02)
#define    P_BT1  (0x04)  // Кнопка 1
#define    P_BT2  (0x08)  // Кнопка 2
#define    P_DSD  (0x10)  // Линия данных DS1820
#define    P_JM1  (0x20)  // Перемычка 1
#define    P_JM2  (0x40)  // Перемычка 2
#define    P_JM3  (0x80)  // Перемычка 3

// Порт B
#define    P_SWP  (0x01)  // Включение устройств на 1-Wire шине
#define    P_LED  (0x02)  // Управление светодиодом
#define    P_RFD  (0x04)  // Управление передатчиком
#define    P_MOSI (0x08)  //
#define    P_MISO (0x10)  //
#define    P_SCK  (0x20)  //


// Порт С
// PC0...PC5 - аналоговые входы


typedef struct
{
  unsigned char preamble:1;
  unsigned char   centre:1;
  unsigned char     done:1;
  unsigned char    lastb:1;

} tflags;

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


#define ADC_DISABLED        0x0001
#define DS1WIRE_DISABLED    0x0002
#define BUTTONS_DISABLED    0x0004
#define HEART_BEAT_DISABLED 0x0008
#define TRANSMIT_REFCOD     0x0010

typedef struct
{
  unsigned int  version;        // Версия программы
  unsigned char wdt_period;     // Оперативная переменная для периода WDT
  unsigned char wdt_div_const;  // Оперативная переменная для коэффициента прореживания
  unsigned char wdt_div_var;    // Оперативная переменная для вариации переменной части коэффициента прореживания
  unsigned int  rf_bitrate;     // Величина задающая скорость передачи по радиоканалу
                                // Равна числу тактов осцилятора формирующих интервал в половину бита 
  unsigned int  flags;          // Флаги управляющие работой передатчика
  unsigned int  preamble;       // Преамбула радио пакета
  unsigned int  refcod;         // Преамбула радио пакета
  unsigned long keycode;        // Ключ шифрования

} TPARAMS;


unsigned char get_pack_first_byte(unsigned char type);
