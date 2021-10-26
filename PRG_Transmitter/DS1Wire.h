#ifndef _DS1WIRE
#define _DS1WIRE

#define       CPU_FREQUENCY   16.000

#define       D1W_BUS         0x10
#define       D1W_DDR         DDRD
#define       D1W_PORT        PORTD
#define       D1W_PIN         PIND

#define       D1W_RELEASE_BUS  D1W_DDR &= ~D1W_BUS; D1W_PORT &= ~D1W_BUS;
#define       D1W_PULL_BUS_LOW D1W_DDR |=  D1W_BUS; D1W_PORT &= ~D1W_BUS;


/****************************************************************************
 ROM commands
****************************************************************************/
#define       D1W_ROM_READ    0x33    //!< READ ROM command code.
#define       D1W_ROM_SKIP    0xcc    //!< SKIP ROM command code.
#define       D1W_ROM_MATCH   0x55    //!< MATCH ROM command code.
#define       D1W_ROM_SEARCH  0xf0    //!< SEARCH ROM command code.
              
              
/****************************************************************************
 Return codes
****************************************************************************/
#define       D1W_ROM_SEARCH_FINISHED     0x00    //!< Search finished return code.
#define       D1W_ROM_SEARCH_FAILED       0xff    //!< Search failed return code.
              
              
/*****************************************************************************
 Timing parameters
*****************************************************************************/
              
#define       D1W_DELAY_OFFSET_CYCLES    13   //!< Timing delay when pulling bus low and releasing bus.
              
// Bit timing delays in clock cycles (= us*clock freq in MHz).
#define       D1W_DELAY_A_STD_MODE    ((6   * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_B_STD_MODE    ((64  * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_C_STD_MODE    ((60  * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_D_STD_MODE    ((10  * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_E_STD_MODE    ((9   * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_F_STD_MODE    ((55  * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
//#define       D1W_DELAY_G_STD_MODE  ((0   * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_H_STD_MODE    ((480 * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_I_STD_MODE    ((70  * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)
#define       D1W_DELAY_J_STD_MODE    ((410 * CPU_FREQUENCY) - D1W_DELAY_OFFSET_CYCLES)


// Defines used only in code example.
#define DS1820_FAMILY_ID                0x10 
#define DS1820_START_CONVERSION         0x44
#define DS1820_READ_SCRATCHPAD          0xbe
#define DS1820_ERROR                    -1000   // Return code. Outside temperature range.

#define DS2890_FAMILY_ID                0x2c
#define DS2890_WRITE_CONTROL_REGISTER   0X55
#define DS2890_RELEASE_CODE             0x96
#define DS2890_WRITE_POSITION           0x0f

#define SEARCH_SUCCESSFUL               0x00
#define SEARCH_CRC_ERROR                0x01

#define FALSE       0
#define TRUE        1



#define D1W_CRC_OK     1
#define D1W_CRC_ERROR  0


/*! \brief  Data type used to hold information about slave devices.
 *  
 *  The OWI_device data type holds information about what bus each device
 *  is connected to, and its 64 bit identifier.
 */
typedef struct
{
    unsigned char id[8];    //!< The 64 bit identifier.
} D1W_device;



signed int    DS1820_ReadTemperature(unsigned char * id);
void          DS2890_SetWiperPosition(unsigned char position, unsigned char * id);
unsigned char DS1W_SearchBuses(D1W_device * devices, unsigned char len);
D1W_device   *DS1W_FindFamily(unsigned char familyID, D1W_device * devices, unsigned char size);

void          D1W_SendByte(unsigned char data);
unsigned char D1W_ReceiveByte(void);
void          D1W_SkipRom(void);
void          D1W_ReadRom(unsigned char * romValue);
void          D1W_MatchRom(unsigned char * romValue);
unsigned char D1W_SearchRom(unsigned char * bitPattern, unsigned char lastDeviation);
unsigned char D1W_ComputeCRC8(unsigned char inData, unsigned char seed);
unsigned char D1W_CheckRomCRC(unsigned char * romValue);


void          D1W_Init(void);
void          D1W_WriteBit1(void);
void          D1W_WriteBit0(void);
unsigned char D1W_ReadBit(void);
unsigned char D1W_DetectPresence(void);

#endif
