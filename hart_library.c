#include "HARTtoUART.h"
#include <stdlib.h>

//Configures the bits in the Delimiter field of the HART PDU
//The Delimiter field is comprised of 1 byte represented as a char
char* buildDelimiterField(Delimiter delim)
{
    char* delimField = (char*)malloc(sizeof(char));
    *delimField = 0x00;

    if(delim.addressType == 1) //Unique 5 byte addressfield
    {
        *delimField |= 0x80; //set the bit to 1   
    }
    //else it remains a 0

    //Leave expansion bytes bits as 0

    //Leave physical layer bits as 0 (asynchronous FSK)

    //Frame Type
    if(delim.frameType == BACK)
    {
        *delimField |= 0x01;
    }
    else if (delim.frameType = STX)
    {
        *delimField |= 0x02;
    }
    else
    {
        *delimField |= 0x06;
    }
    
    return delimField;
}

//Configures a 5 byte HART Unique Address Field
char* buildAddressField(UniqueAddress address)
{
    char* addressField = (char*)malloc(5 * sizeof(char));
    // *addressField = 0x0000000000;

    // if(address.masterAddress == 1)
    // {
    //     //Set MSB to 1
    //     *addressField |= 0x8000000000;
    // }

    // if(address.burstMode == 1)
    // {
    //     *addressField |= 0x4000000000;
    // }

    // //Not sure if this will work
    // *addressField |= address.deviceTypeCode << 24; 

    // //Not sure if this will work either
    // //Trying to set the last 3 bytes to the device ID, not sure if it automatically pads it
    // *addressField |= address.deviceID; 

    //Refactoring this as well

    char* firstTwoBytes = memcpy(firstTwoBytes, address.deviceTypeCode, 2 * sizeof(char)); 

    //Copy the first byte only so that the 2 most signifcant bits can be set
    char firstByte = firstTwoBytes[0];

    if(address.masterAddress == 1)
    {
        firstByte |= 0x80;
    }

    if(address.burstMode == 1)
    {
        firstByte |= 0x40;
    }

    //Then copy back the modified first byte
    memcpy(&firstTwoBytes[0], firstByte, sizeof(char));

    //Finally build the actual 5 byte address field
    memcpy(&addressField[0], firstTwoBytes, 2 * sizeof(char));
    memcpy(&addressField[2], address.deviceID, 3 * sizeof(char));

    return addressField;
}

//Configures the Command field 
//Pass COMMAND_48 to this
char* buildCommandField(int command)
{
    char* commandField = (char*)malloc(sizeof(char));
    *commandField = command;
    return commandField;
}

//Configures the Byte Count field which specifies
//how long the Data field is
char* buildByteCountField(int byteCount)
{
    char* byteCountField = (char*)malloc(sizeof(char));
    *byteCountField = byteCount;
    return byteCountField;
}

//Checks if certain bits in a byte stream are
//set to 1 by checking it against a mask
//For example:
//1010 (byte stream to test)
//0010 (mask)
//0010 (result)
//might need to change datatypes? 
bool checkBits(int mask, char* byteStream)
{
    if((*byteStream & mask) == mask)
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
char* configureCommand48Response(char* byte1, char* byte2, char* byte3,
                                char* byte4, char* byte5, char* byte6)
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

//Calculates the checksum for entire frame
char* checksum(char* frame, int length)
{
    int i;
    char checksum = 0x00;
    for(i = 0; i < length; i++)
    {
        char buffer = frame[i];
        checksum ^= buffer;
    }
    return checksum;
}

//Frame before adding the checksum
char* buildResponseFrame(char* delim, char* address, char* command,
                        char* byteCount, char* data)
{
    char* frame = (char*)malloc(16 * sizeof(char));
    memcpy(&finalFrame[0], delim, sizeof(char));
    memcpy(&finalFrame[1], address, 5 * sizeof(char));
    memcpy(&finalFrame[6], command, sizeof(char));
    memcpy(&finalFrame[7], byteCount, sizeof(char));
    memcpy(&finalFrame[8], data, 8 * sizeof(char));

    return frame;
}


//Builds the final HART frame to send off through UART
char* buildResponseFrame(char* delim, char* address, char* command,
                        char* byteCount, char* data, char* checkByte)
{
    //Assuming 5 byte unique address field, Command 48 has 8 bytes of data
    //and no expansion field
    char* finalFrame = (char*)malloc(17 * sizeof(char));

    memcpy(&finalFrame[0], delim, sizeof(char));
    memcpy(&finalFrame[1], address, 5 * sizeof(char));
    memcpy(&finalFrame[6], command, sizeof(char));
    memcpy(&finalFrame[7], byteCount, sizeof(char));
    memcpy(&finalFrame[8], data, 8 * sizeof(char));
    memcpy(&finalFrame[16], checkByte, sizeof(char));

    //Clean up 
    free(delim);
    free(address);
    free(command);
    free(byteCount);
    free(data);
    free(checkByte);

    return finalFrame;
}

//NOTE: WORK TO DO
//NEED TO ADD COMMAND STATUS BYTES before Data field

//NOTE: WORK TO DO
//MAY NEED TO REFACTOR char* to uint8_t*


//NOTE: Data types for function parameters will need to be adapted/changed

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
void transmitRequest(int preambleLength, uint8_t* address, uint8_t* cmd, uint8_t* data)
{

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

//Passed to Physical layer to undergo physical reset
void resetRequest()
{

}

//Enables are used to assert carrier and release network

/*
This SAP is passed to Physical Layer to request it to prepare for data transmission or terminate data transmission
depending on whether the parameter state is HIGH or LOW. Implemented as request to send (RTS) signal in modem.
May only be issued if the interface is in an idle state (no pending transmitIndicate or transmitConfirm). 
*/
void enableRequest(State state)
{

}

/*
A receive SAP by the Physical Layer to indicate establishment of communication with peer Physical Layer entity or
indicate termination of currently established connection depending on whether state is HIGH or LOW.
SAP is implemented as Carrier Detect (CD) signal from modem. 
*/
void enableIndicate(State* state)
{

}

/*
This SAP is passed back by Physical Layer to confirm ability to accept data for transmission or indicate 
that it will accept no further data. Usually mapped to clear to send (CTS) signal from modem. 
*/
void enableConfirm(State* state)
{

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
*/
void dataIndicate(uint8_t* data)
{

}

/*
SAP returned by Physical Layer to indicate error in received data. 
Examples include: parity errors, framing errors, receive data overruns.
Determined by status registers in the modem.
*/
void errorIndicate(PHYERROR status, uint8_t* data)
{

}
