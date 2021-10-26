#include "bin_defines.h"

#ifndef _RFSETTINGS
#define _RFSETTINGS

#define PREAMBLE_LEN 16                 // Длина преамбулы должна быть четным числом


#define PREAMBLE 0xD9C2    // [1101 1001 1100 0010]



#define DECISLEV 52 // 48

#define HALFBIT_DURATION 0.001
#define PACK_LEN 11  // Длина пакета данных 


#define PACK_TEMPER  0      // Идентификатор пакета с температурой
#define PACK_BUTTONS 1      // Идентификатор пакета с состояниями кнопок 
#define PACK_ADC     2      // Идентификатор пакета с результатами АЦП

#define MAX_DEVICES  4      // Максимальное количество устройств на шине 1-Wire
#define MAX_NODES    4      // Максимальное количество сенсоров в сети

#endif
