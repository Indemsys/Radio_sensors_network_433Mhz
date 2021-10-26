#include <ioavr.h>
#include <inavr.h>
#include <string.h>
#include "DS1Wire.h"

/*! \brief  Perform a 1-Wire search
*
*  This function shows how the D1W_SearchRom function can be used to
*  discover all slaves on the bus. It will also CRC check the 64 bit
*  identifiers.
*
*  \param  devices Pointer to an array of type D1W_device. The discovered
*                  devices will be placed from the beginning of this array.
*
*  \param  len     The length of the device array. (Max. number of elements).
*
*
*  \retval SEARCH_SUCCESSFUL   Search completed successfully.
*  \retval SEARCH_CRC_ERROR    A CRC error occured. Probably because of noise
*                              during transmission.
*/
unsigned char DS1W_SearchBuses(D1W_device * devices, unsigned char len)
{
  unsigned char i, j;
  unsigned char presence;
  unsigned char * newID;
  unsigned char * currentID;
  unsigned char lastDeviation;
  unsigned char numDevices;

  // Initialize all addresses as zero, on bus 0 (does not exist).
  for (i = 0; i < len; i++)
  {
    for (j = 0; j < 8; j++)
    {
      devices[i].id[j] = 0x00;
    }
  }

  // Find the buses with slave devices.
  presence = D1W_DetectPresence();

  numDevices = 0;
  newID = devices[0].id;

  // Go through all buses with slave devices.
  lastDeviation = 0;
  currentID = newID;
  if (presence) // Devices available on this bus.
  {
    // Do slave search  and place identifiers in the array.
    do
    {
      memcpy(newID, currentID, 8);
      D1W_DetectPresence();
      lastDeviation = D1W_SearchRom(newID, lastDeviation);
      currentID = newID;
      numDevices++;
      newID=devices[numDevices].id;
    }
    while(lastDeviation != D1W_ROM_SEARCH_FINISHED);
  }

  // Go through all the devices and do CRC check.
  for (i = 0; i < numDevices; i++)
  {
    // If any id has a crc error, return error.
    if(D1W_CheckRomCRC(devices[i].id) != D1W_CRC_OK)
    {
      return SEARCH_CRC_ERROR;
    }
  }
  // Else, return Successful.
  return SEARCH_SUCCESSFUL;
}

/*! \brief  Find the first device of a family based on the family id
*
*  This function returns a pointer to a device in the device array
*  that matches the specified family.
*
*  \param  familyID    The 8 bit family ID to search for.
*
*  \param  devices     An array of devices to search through.
*
*  \param  size        The size of the array 'devices'
*
*  \return A pointer to a device of the family.
*  \retval NULL    if no device of the family was found.
*/
D1W_device * DS1W_FindFamily(unsigned char familyID, D1W_device * devices, unsigned char size)
{
  unsigned char i = 0;

  // Search through the array.
  while (i < size)
  {
    // Return the pointer if there is a family id match.
    if ((*devices).id[0] == familyID)
    {
      return devices;
    }
    devices++;
    i++;
  }
  // Else, return NULL.
  return NULL;
}


/*! \brief  Read the temperature from a DS1820 temperature sensor.
*
*  This function will start a conversion and read back the temperature
*  from a DS1820 temperature sensor.
*
*
*  \param  id  The 64 bit identifier of the DS1820.
*
*  \return The 16 bit signed temperature read from the DS1820.
*/
signed int DS1820_ReadTemperature(unsigned char * id)
{
  signed int temperature;

  // Reset, presence.
  if (!D1W_DetectPresence())
  {
    return DS1820_ERROR; // Error
  }
  // Match the id found earlier.
  D1W_MatchRom(id);
  // Send start conversion command.
  D1W_SendByte(DS1820_START_CONVERSION);
  // Wait until conversion is finished.
  // Bus line is held low until conversion is finished.
  while (!D1W_ReadBit())
  {

  }
  // Reset, presence.
  if(!D1W_DetectPresence())
  {
    return -1000; // Error
  }
  // Match id again.
  D1W_MatchRom(id);
  // Send READ SCRATCHPAD command.
  D1W_SendByte(DS1820_READ_SCRATCHPAD);
  // Read only two first bytes (temperature low, temperature high)
  // and place them in the 16 bit temperature variable.
  temperature = D1W_ReceiveByte();
  temperature |= (D1W_ReceiveByte() << 8);

  return temperature;
}


/*! \brief  Set the wiper position of a DS2890.
*
*  This function initializes the DS2890 by enabling the charge pump. It then
*  changes the wiper position.
*
*  \param  position    The new wiper position.
*
*  \param  id          The 64 bit identifier of the DS2890.
*/
void DS2890_SetWiperPosition(unsigned char position, unsigned char * id)
{
  // Reset, presence.
  if(!D1W_DetectPresence())
  {
    return;
  }
  //Match id.
  D1W_MatchRom(id);

  // Send Write control register command.
  D1W_SendByte(DS2890_WRITE_CONTROL_REGISTER);

  // Write 0x4c to control register to enable charge pump.
  D1W_SendByte(0x4c);

  // Check that the value returned matches the value sent.
  if (D1W_ReceiveByte() != 0x4c)
  {
    return;
  }

  // Send release code to update control register.
  D1W_SendByte(DS2890_RELEASE_CODE);

  // Check that zeros are returned to ensure that the operation was
  // successful.
  if (D1W_ReceiveByte() == 0xff)
  {
    return;
  }

  // Reset, presence.
  if (!D1W_DetectPresence())
  {
    return;
  }

  // Match id.
  D1W_MatchRom(id);

  // Send the Write Position command.
  D1W_SendByte(DS2890_WRITE_POSITION);

  // Send the new position.
  D1W_SendByte(position);

  // Check that the value returned matches the value sent.
  if (D1W_ReceiveByte() != position)
  {
    return;
  }

  // Send release code to update wiper position.
  D1W_SendByte(DS2890_RELEASE_CODE);

  // Check that zeros are returned to ensure that the operation was
  // successful.
  if (D1W_ReceiveByte() == 0xff)
  {
    return;
  }
}

/*! \brief  Sends one byte of data on the 1-Wire(R) bus(es).
*
*  This function automates the task of sending a complete byte
*  of data on the 1-Wire bus(es).
*
*  \param  data    The data to send on the bus(es).
*
*/
void D1W_SendByte(unsigned char data)
{
  unsigned char temp;
  unsigned char i;

  // Do once for each bit
  for (i = 0; i < 8; i++)
  {
    // Determine if lsb is '0' or '1' and transmit corresponding
    // waveform on the bus.
    temp = data & 0x01;
    if (temp)
    {
      D1W_WriteBit1();
    }
    else
    {
      D1W_WriteBit0();
    }
    // Right shift the data to get next bit.
    data >>= 1;
  }
}


/*! \brief  Receives one byte of data from the 1-Wire(R) bus.
*
*  This function automates the task of receiving a complete byte
*  of data from the 1-Wire bus.
*
*  \return     The byte read from the bus.
*/
unsigned char D1W_ReceiveByte(void)
{
  unsigned char data;
  unsigned char i;

  // Clear the temporary input variable.
  data = 0x00;

  // Do once for each bit
  for (i = 0; i < 8; i++)
  {
    // Shift temporary input variable right.
    data >>= 1;
    // Set the msb if a '1' value is read from the bus.
    // Leave as it is ('0') else.
    if (D1W_ReadBit())
    {
      // Set msb
      data |= 0x80;
    }
  }
  return data;
}


/*! \brief  Sends the SKIP ROM command to the 1-Wire bus(es).
*
*/
void D1W_SkipRom(void)
{
  // Send the SKIP ROM command on the bus.
  D1W_SendByte(D1W_ROM_SKIP);
}


/*! \brief  Sends the READ ROM command and reads back the ROM id.
*
*  \param  romValue    A pointer where the id will be placed.
*
*/
void D1W_ReadRom(unsigned char * romValue)
{
  unsigned char bytesLeft = 8;

  // Send the READ ROM command on the bus.
  D1W_SendByte(D1W_ROM_READ);

  // Do 8 times.
  while (bytesLeft > 0)
  {
    // Place the received data in memory.
    *romValue++ = D1W_ReceiveByte();
    bytesLeft--;
  }
}


/*! \brief  Sends the MATCH ROM command and the ROM id to match against.
*
*  \param  romValue    A pointer to the ID to match against.
*
*/
void D1W_MatchRom(unsigned char * romValue)
{
  unsigned char bytesLeft = 8;

  // Send the MATCH ROM command.
  D1W_SendByte(D1W_ROM_MATCH);

  // Do once for each byte.
  while (bytesLeft > 0)
  {
    // Transmit 1 byte of the ID to match.
    D1W_SendByte(*romValue++);
    bytesLeft--;
  }
}


/*! \brief  Sends the SEARCH ROM command and returns 1 id found on the
*          1-Wire(R) bus.
*
*  \param  bitPattern      A pointer to an 8 byte char array where the
*                          discovered identifier will be placed. When
*                          searching for several slaves, a copy of the
*                          last found identifier should be supplied in
*                          the array, or the search will fail.
*
*  \param  lastDeviation   The bit position where the algorithm made a
*                          choice the last time it was run. This argument
*                          should be 0 when a search is initiated. Supplying
*                          the return argument of this function when calling
*                          repeatedly will go through the complete slave
*                          search.
*
*  \return The last bit position where there was a discrepancy between slave addresses the last time this function was run. Returns D1W_ROM_SEARCH_FAILED if an error was detected (e.g. a device was connected to the bus during the search), or D1W_ROM_SEARCH_FINISHED when there are no more devices to be discovered.
*
*  \note   See main.c for an example of how to utilize this function.
*/
unsigned char D1W_SearchRom(unsigned char * bitPattern, unsigned char lastDeviation)
{
  unsigned char currentBit = 1;
  unsigned char newDeviation = 0;
  unsigned char bitMask = 0x01;
  unsigned char bitA;
  unsigned char bitB;

  // Send SEARCH ROM command on the bus.
  D1W_SendByte(D1W_ROM_SEARCH);

  // Walk through all 64 bits.
  while (currentBit <= 64)
  {
    // Read bit from bus twice.
    bitA = D1W_ReadBit();
    bitB = D1W_ReadBit();

    if (bitA && bitB)
    {
      // Both bits 1 (Error).
      newDeviation = D1W_ROM_SEARCH_FAILED;
      return newDeviation;
    }
    else if (bitA ^ bitB)
    {
      // Bits A and B are different. All devices have the same bit here.
      // Set the bit in bitPattern to this value.
      if (bitA)
      {
        (*bitPattern) |= bitMask;
      }
      else
      {
        (*bitPattern) &= ~bitMask;
      }
    }
    else // Both bits 0
    {
      // If this is where a choice was made the last time,
      // a '1' bit is selected this time.
      if (currentBit == lastDeviation)
      {
        (*bitPattern) |= bitMask;
      }
      // For the rest of the id, '0' bits are selected when
      // discrepancies occur.
      else if (currentBit > lastDeviation)
      {
        (*bitPattern) &= ~bitMask;
        newDeviation = currentBit;
      }
      // If current bit in bit pattern = 0, then this is
      // out new deviation.
      else if ( !(*bitPattern & bitMask))
      {
        newDeviation = currentBit;
      }
      // IF the bit is already 1, do nothing.
      else
      {

      }
    }


    // Send the selected bit to the bus.
    if ((*bitPattern) & bitMask)
    {
      D1W_WriteBit1();
    }
    else
    {
      D1W_WriteBit0();
    }

    // Increment current bit.
    currentBit++;

    // Adjust bitMask and bitPattern pointer.
    bitMask <<= 1;
    if (!bitMask)
    {
      bitMask = 0x01;
      bitPattern++;
    }
  }
  return newDeviation;
}

unsigned char D1W_ComputeCRC8(unsigned char inData, unsigned char seed)
{
  unsigned char bitsLeft;
  unsigned char temp;

  for (bitsLeft = 8; bitsLeft > 0; bitsLeft--)
  {
    temp = ((seed ^ inData) & 0x01);
    if (temp == 0)
    {
      seed >>= 1;
    }
    else
    {
      seed ^= 0x18;
      seed >>= 1;
      seed |= 0x80;
    }
    inData >>= 1;
  }
  return seed;
}


unsigned char D1W_CheckRomCRC(unsigned char * romValue)
{
  unsigned char i;
  unsigned char crc8 = 0;

  for (i = 0; i < 7; i++)
  {
    crc8 = D1W_ComputeCRC8(*romValue, crc8);
    romValue++;
  }
  if (crc8 == (*romValue))
  {
    return D1W_CRC_OK;
  }
  return D1W_CRC_ERROR;
}

/*! \brief Initialization of the one wire bus(es). (Software only driver)
*
*  This function initializes the 1-Wire bus(es) by releasing it and
*  waiting until any presence sinals are finished.
*
*  \param  pins    A bitmask of the buses to initialize.
*/
void D1W_Init(void)
{
  D1W_RELEASE_BUS;
  // The first rising edge can be interpreted by a slave as the end of a
  // Reset pulse. Delay for the required reset recovery time (H) to be
  // sure that the real reset is interpreted correctly.
  __delay_cycles(D1W_DELAY_H_STD_MODE);
}


/*! \brief  Write a '1' bit to the bus(es). (Software only driver)
*
*  Generates the waveform for transmission of a '1' bit on the 1-Wire
*  bus.
*
*  \param  pins    A bitmask of the buses to write to.
*/
void D1W_WriteBit1(void)
{
  unsigned char intState;

  // Disable interrupts.
  intState = __save_interrupt();
  __disable_interrupt();

  // Drive bus low and delay.
  D1W_PULL_BUS_LOW;
  __delay_cycles(D1W_DELAY_A_STD_MODE);

  // Release bus and delay.
  D1W_RELEASE_BUS;
  __delay_cycles(D1W_DELAY_B_STD_MODE);

  // Restore interrupts.
  __restore_interrupt(intState);
}


/*! \brief  Write a '0' to the bus(es). (Software only driver)
*
*  Generates the waveform for transmission of a '0' bit on the 1-Wire(R)
*  bus.
*
*  \param  pins    A bitmask of the buses to write to.
*/
void D1W_WriteBit0(void)
{
  unsigned char intState;

  // Disable interrupts.
  intState = __save_interrupt();
  __disable_interrupt();

  // Drive bus low and delay.
  D1W_PULL_BUS_LOW;
  __delay_cycles(D1W_DELAY_C_STD_MODE);

  // Release bus and delay.
  D1W_RELEASE_BUS;
  __delay_cycles(D1W_DELAY_D_STD_MODE);

  // Restore interrupts.
  __restore_interrupt(intState);
}


/*! \brief  Read a bit from the bus(es). (Software only driver)
*
*  Generates the waveform for reception of a bit on the 1-Wire(R) bus(es).
*
*  \return A bitmask of the buses where a '1' was read.
*/
unsigned char D1W_ReadBit(void)
{
  unsigned char intState;
  unsigned char bitsRead;

  // Disable interrupts.
  intState = __save_interrupt();
  __disable_interrupt();

  // Drive bus low and delay.
  D1W_PULL_BUS_LOW;
  __delay_cycles(D1W_DELAY_A_STD_MODE);

  // Release bus and delay.
  D1W_RELEASE_BUS;
  __delay_cycles(D1W_DELAY_E_STD_MODE);

  // Sample bus and delay.
  bitsRead = D1W_PIN & D1W_BUS;
  __delay_cycles(D1W_DELAY_F_STD_MODE);

  // Restore interrupts.
  __restore_interrupt(intState);

  return bitsRead;
}


/*! \brief  Send a Reset signal and listen for Presence signal. (software
*  only driver)
*
*  Generates the waveform for transmission of a Reset pulse on the
*  1-Wire(R) bus and listens for presence signals.
*
*  \param  pins    A bitmask of the buses to send the Reset signal on.
*
*  \return A bitmask of the buses where a presence signal was detected.
*/
unsigned char D1W_DetectPresence(void)
{
  unsigned char intState;
  unsigned char presenceDetected;

  // Disable interrupts.
  intState = __save_interrupt();
  __disable_interrupt();

  // Drive bus low and delay.
  D1W_PULL_BUS_LOW;
  __delay_cycles(D1W_DELAY_H_STD_MODE);

  // Release bus and delay.
  D1W_RELEASE_BUS;
  __delay_cycles(D1W_DELAY_I_STD_MODE);

  // Sample bus to detect presence signal and delay.
  presenceDetected = ((~D1W_PIN) & D1W_BUS);
  __delay_cycles(D1W_DELAY_J_STD_MODE);

  // Restore interrupts.
  __restore_interrupt(intState);

  return presenceDetected;
}

