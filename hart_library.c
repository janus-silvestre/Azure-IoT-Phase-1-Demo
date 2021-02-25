#include "hart_library.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <applibs/gpio.h>


/*
Author: Janus Silvestre
HART Protocol stack library functions
This library is to be used for the Azure Sphere MCU to emulate a 
secondary master to poll a HART device for additional diagnostics
through the use of Command 48.
*/

//Configures the bitfield in the Delimiter field of the HART PDU
//The Delimiter field is 1 byte long
//Address Type is assumed to be unique, Frame Type will always be MasterToFieldDevice
uint8_t* buildDelimiterField()
{
    uint8_t* delimField = (uint8_t*)malloc(sizeof(uint8_t));
    *delimField = 0x00;

    *delimField |= UNIQUEADDRESS; 

    //Leave expansion bytes bits as 0
    //Leave physical layer bits as 0 (asynchronous FSK)

    //Frame Type
    *delimField |= BACK;

    return delimField;
}

/*
Configures a 5 byte HART Unique Address Field
Parameters:
deviceTypeCode needs to be the 2 bytes
deviceID needs to be 3 bytes
*/

uint8_t* buildAddressField(uint8_t* deviceTypeCode, uint8_t* deviceID)
{
    uint8_t* addressField = (uint8_t*)malloc(5 * sizeof(uint8_t));

    //Create a buffer for the first two bytes so they can be set
    uint8_t firstTwoBytes[2];
    memcpy(firstTwoBytes, deviceTypeCode, 2 * sizeof(uint8_t)); 

    //Copy the first byte so that the 2 most signifcant bits can be set
    uint8_t firstByte = firstTwoBytes[0];

    //Set bit 7 and 6 to 0 (Secondary Master and not a Field Device)
    firstByte &= ~(1<<7);
    firstByte &= ~(1<<6);

    //Then copy back the modified first byte
    memcpy(&firstTwoBytes[0], &firstByte, sizeof(uint8_t));

    //Finally build the actual 5 byte address field
    memcpy(&addressField[0], &firstTwoBytes, 2 * sizeof(uint8_t));
    memcpy(&addressField[2], deviceID, 3 * sizeof(uint8_t));

    return addressField;
}

/*
Configures the Command field 
Parameters:
Takes in the command number and converts to the respective binary for the 
1 byte Command field
*/
uint8_t* buildCommandField(int command)
{
    uint8_t* commandField = (uint8_t*)malloc(sizeof(uint8_t));
    *commandField = command;
    return commandField;
}

//Configures the Byte Count field which specifies
//how long the Data field is, the data field for specific commands
//are known so no need to count them
//May refactor this in the future to use a lookup table instead
uint8_t* buildByteCountField(int byteCount)
{
    uint8_t* byteCountField = (uint8_t*)malloc(sizeof(uint8_t));
    *byteCountField = byteCount;
    return byteCountField;
}

//Checks if the kth bit is set in byte n 
bool checkBits(uint8_t* n,uint8_t k)
{
    if(*n & (1 << (k)))
    {
        return true;
    }
    else
    {
        return false;
    }  
}

//Builds Command 48 Data field byte stream
//STT700 returns 8 bytes as specified in its documentation
//Based on the interpretation of the docs, the subfields are organised
//as follows:
//0 1 2 3 4 5 6 0
//Tested bit shifting algo on a sample and it seems to work
uint8_t* configureCommand48Response(uint8_t* byte1, uint8_t* byte2, uint8_t* byte3,
                                uint8_t* byte4, uint8_t* byte5, uint8_t* byte6)
{
    char* command48 = malloc(8 * sizeof(char));
    // *command48 = 0x0000000000000000;

    // *command48 |= *byte1 << (8 * 6);
    // *command48 |= *byte2 << (8 * 5);
    // *command48 |= *byte3 << (8 * 4);
    // *command48 |= *byte4 << (8 * 3);
    // *command48 |= *byte5 << (8 * 2);
    // *command48 |= *byte6 << (8 * 1);

    //Refactoring this
    char unused = 0x00;
    memcpy(&command48[0], &unused, sizeof(char));
    memcpy(&command48[1], byte1, sizeof(char));
    memcpy(&command48[2], byte2, sizeof(char));
    memcpy(&command48[3], byte3, sizeof(char));
    memcpy(&command48[4], byte4, sizeof(char));
    memcpy(&command48[5], byte5, sizeof(char));
    memcpy(&command48[6], byte6, sizeof(char));
    memcpy(&command48[7], &unused, sizeof(char));

    return command48;
}

//Build Byte 1 of Command 48 response
//This function was originally to be used for a simulated slave using the 
//Arduino, building the actual data request bytes is not required by the master
//as it will simply send out the previous response byte (as per Command 48 
//specification)
char* buildByte1(Command48_Byte1 byte1)
{
    char* byte1Field = (char*)malloc(sizeof(char));
    *byte1Field = 0x00;

    if(byte1.configCorrupt)
    {
        *byte1Field |= 0x10;
    }

    if(byte1.charTableCRC)
    {
        *byte1Field |= 0x08;
    }

    if(byte1.sensInputFailure)
    {
        *byte1Field |= 0x04;
    }

    if(byte1.DACFailure)
    {
        *byte1Field |= 0x02;
    }

    if(byte1.diagFailure)
    {
        *byte1Field |= 0x01;
    }
}

//Not writing the functions for the other bytes as it doesnt matter for now
//but same concept applies

/*
Calculates the checksum for entire frame
Parameters:
frame - framefields combined from delimiter to last data byte
length - number of bytes from delim to data field
*/
uint8_t checksum(uint8_t* frame, int length)
{
    int i;
    uint8_t checksum = 0x00;
    for(i = 0; i < length; i++)
    {
        uint8_t buffer = frame[i];
        checksum ^= buffer;
    }
    return checksum;
}

//Frame before adding the checksum
uint8_t* addFrames(uint8_t* delim, uint8_t* address, uint8_t* command,
                        uint8_t* byteCount, uint8_t* data)
{
    uint8_t* finalFrame = (char*)malloc(16 * sizeof(uint8_t));
    memcpy(&finalFrame[0], delim, sizeof(uint8_t));
    memcpy(&finalFrame[1], address, 5 * sizeof(uint8_t));
    memcpy(&finalFrame[6], command, sizeof(uint8_t));
    memcpy(&finalFrame[7], byteCount, sizeof(uint8_t));
    memcpy(&finalFrame[8], data, 8 * sizeof(uint8_t));

    return frame;
}

//Builds the final HART frame to send off through UART by Master MAC
//state machine
uint8_t* buildResponseFrame(uint8_t* delim, uint8_t* address, uint8_t* command,
                        uint8_t* byteCount, uint8_t* data, uint8_t* checkByte)
{ 
    //Assuming 5 byte unique address field, Command 48 has 8 bytes of data
    //and no expansion field
    uint8_t* finalFrame = (uint8_t*)malloc(17 * sizeof(uint8_t));

    memcpy(&finalFrame[0], delim, sizeof(uint8_t));
    memcpy(&finalFrame[1], address, 5 * sizeof(uint8_t));
    memcpy(&finalFrame[6], command, sizeof(uint8_t));
    memcpy(&finalFrame[7], byteCount, sizeof(uint8_t));
    memcpy(&finalFrame[8], data, 8 * sizeof(uint8_t));
    memcpy(&finalFrame[16], checkByte, sizeof(uint8_t));

    //Can clean up here as we have a copy of the entire frame
    free(delim);
    free(address);
    free(command);
    free(byteCount);
    free(data);
    free(checkByte);

    return finalFrame;
}



//NOTE: Data types for function parameters will need to be adapted/changed
//NOTE: The Physical Layer SAPS will now have to be implemented in the real time app
//      for the Master MAC 


/*Data Link Layer SAPs for Master Application layer*/

//Message SAPS

/*
This SAP is to be used by the Master Device to request transfer of information to a Field Device
at a given address. 

Parameters:
preambleLength : usually set to 5, unsure yet what to do with preamble
address : unsure yet, probably going to be the 5 byte address field which includes the device type code
and unique device identifier
cmd : one byte representing the command - in this use case it will 48 or 0x30
data : data request bytes for the given command

Frames the given address, command, data with its delimiter, bytecount and checksum into a HART PDU
NOTE: still unsure of where preamble is added
*/
Cmd48Response transmitRequest(int preambleLength, uint8_t* address, uint8_t* cmd, uint8_t* data)
{
    uint8_t* delim = buildDelimiterField();
    uint8_t* byteCount = buildByteCountField(*cmd);
    uint8_t* checkByte = addFrames(delim, address, cmd, byteCount, data);
    uint8_t* responseFrame = buildResponseFrame(delim, address, cmd, byteCount, data, checkByte);
    
    //Pass frame to Master MAC state machine to be sent by XMT_MSG here
    //by sending to RTApp through shared buffer

}

/*
This SAP is returned to the Master Application Layer to communicate the results of a previously executed
transmitRequest. The localStatus parameter carries the status of the communication routines in the master.
Master DataLink performs retries as needed. Local status returns either "Success" or "Communication Failure".

The function will set the given parameters. 
*/
void transmitConfirm(int* localStatus, uint8_t* address, uint8_t* cmd, uint8_t* data)
{

}

/*Physical layer SAPS for Data Link Layer*/
//These will be in reference to using the DAC8740H in Hart-UART mode
//Does not include setup such as opening file descriptors,etc

//Passed to Physical layer to undergo physical reset
void resetRequest()
{
    //rstFd is the file descriptor from opening the pin as output
    GPIO_SetValue(rstFd, GPIO_Value_Low);
    //delay
    GPIO_SetValue(rstFd, GPIO_Value_High);
}

//Enables are used to assert carrier and release network

/*
This SAP is passed to Physical Layer to request it to prepare for data transmission or terminate data transmission
depending on whether the parameter state is HIGH or LOW. Implemented as request to send (RTS) signal in modem.
May only be issued if the interface is in an idle state (no pending transmitIndicate or transmitConfirm). 
*/
void enableRequest()
{
    //Enables the modulator for transmitting
    GPIO_SetValue(rtsFd, GPIO_Value_Low);
}

/*
A receive SAP by the Physical Layer to indicate establishment of communication with peer Physical Layer entity or
indicate termination of currently established connection depending on whether state is HIGH or LOW.
SAP is implemented as Carrier Detect (CD) signal from modem. 

Continually polled by RCV or used in interrupt?
Returns true if a valid carrier is detecting on the line
*/
bool enableIndicate()
{
    GPIO_Value validCarrier;
    GPIO_GetValue(cdFd, &carrierDetect);
    if(validCarrier == GPIO_Value_High)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
This SAP is passed back by Physical Layer to confirm ability to accept data for transmission or indicate 
that it will accept no further data. Usually mapped to clear to send (CTS) signal from modem. 
*/
void enableConfirm(State* state)
{
    //Need to read bit 0 of MODEM_STATUS Register (offset = 20h)

}

/*
SAP to actually transfer a unit of data (byte in this case) to Physical Layer for transmission.
Usually implemented as a write to transmit data register. In this case, Azure Sphere UART would use
the posix write() to the UART interface.
*/
void dataRequest(uint8_t* data)
{

}

/*
SAP is passed back from Physical Layer to acceptance of data from a dataRequest and readiness to process
more requests. Implementation of this SAP may be setting a status bit n a register for simple polling.
*/
void dataConfirm(uint8_t* data)
{

}

/*
SAP is passed back from Physical Layer to indicate availability of data unit (byte). 
Implemented and mapped into a "receive data register full" status bit.
This should be triggered after an interrupt on the mod_status_reg.
*/
void dataIndicate(uint8_t* data)
{

}

/*
SAP returned by Physical Layer to indicate error in received data. 
