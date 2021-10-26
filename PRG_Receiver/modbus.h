#define array_of_byte 1

// Структура описания типа команды MODBUS
typedef struct
{
    unsigned char id;     // Номер функции
    unsigned char df;     // 1 - имеет поле данных
    void (*func)(void);   // Указатель на исполняемую функцию
} TMODB_tcmd;


typedef struct
{
  void*              val;        // Указатель на значение переменной в RAM
  unsigned char      type;       // Идентификатор типа регистра указывающего на его интерпретацию
  unsigned char      typelen;    // Количество байт в регистре
  unsigned char      indx1len;   // Количество элементов по первому индексу массива
  unsigned char      indx2len;   // Количество элементов по второму индексу массива
  void               (*func)(void); // Указатель на функцию выполняемую после записи регистра

} TMODB_reg;

#define MODB_TIMEOUT 10000

void modbus_task(char ch);
