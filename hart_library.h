#ifndef HARTtoUART_H
#define HARTtoUART_H

#include <stdbool.h>

//Bit masks for Delimiter field
#define UNIQUEADDRESS 0x80
#define BACK 0x02 //Burst Frame
#define STX 0x06 //Master to Field Device
#define ACK 0x01 //Field Device to Master


//Timeout values for FSK
#define CHARACTER_TIME 9.167f //in ms
#define RT1 (41 * CHARACTER_TIME) //for secondary master
#define RT2 (8 * CHARACTER_TIME)
#define STO (28 * CHARATER_TIME)


typedef enum 
{
    RCV_SUCCESS = 1,
    RCV_ERR = 2
} RCVSTATUS;

typedef uint8_t RCVCODE;

typedef enum 
{
    XMT_SUCCESS = 1,
    XMT_ERR = 2
} XMTSTATUS;

typedef uint8_t XMTCODE;

typedef enum
{
    CMD_SUCCESS = 1,
    CMD_ERR = 2
} CMDSTATUS;

typedef uint8_t CMDCODE;

//Device type code for Honeywell STT700 
#define DEVICE_TYPE_CODE 0x2B;

//Placeholder for the actual transmitter device identifier
//3 bytes used for this 
#define DEVICE_IDENTIFIER 0xFFFFFF; 

//Command 48 to poll for Additional Field Device Status
#define COMMAND_48 0x30

//Bits used in Byte 1 of Command 48 Data field for critical status
typedef struct Command48_Byte1
{
    bool configCorrupt; //bit 4
    bool charTableCRC; //bit 3
    bool sensInputFailure; //bit 2
    bool DACFailure; //bit 1
    bool diagFailure; //bit 0 
} Command48_Byte1;

//Bits used in Byte 2 of Command 48 Data field for non-critical status
typedef struct Command48_Byte2
{
    bool userCorrectActive; //bit 7
    bool fixedCurrentMode;
    bool sens1ExcessURV;
    bool sens1ExcessLRV;
    bool coldJunctionTemp;
    bool pvOOR;
    bool noFactCalib;
    bool coreTempOOR; //bit 0
} Command48_Byte2;

//Bits used in Byte 3 of Command 48 Data field for non-critical status
typedef struct Command48_Byte3
{
    bool analogOutSat; //bit 6
    bool input2Fault; //bit 5 IMPORTANT
    bool input1Fault; //bit 4 IMPORTANT
    bool noDACComp; //bit 1
    bool inputSuspect; //bit 0
} Command48_Byte3;

//Bits used in Byte 4 of Command 48 Data field for non-critical status
typedef struct Command48_Byte4
{
    bool supplyVoltFault;
    bool watchdogReset;
    bool input2OOR; //bit 5 IMPORTANT
    bool input1OOR; //bit 4 IMPORTANT
    bool sens2ExcessURV;
    bool sens2ExcessLRV;
    bool ADCFault;
    bool excessDeltaDetect;
} Command48_Byte4;

//Bits used in Byte 5 of Command 48 Data field for non-critical status
typedef struct Command48_Byte5
{
    //all other bits are unused
    bool SILDiag; //bool 0
} Command48_Byte5;

//IMPORTANT note:
//Sensor input failure is reported when input sensor 
//is open/short/out of range (OOR) 
//Caused by Input1/2 Fault

//Bit used for Byte 6 Data field Extended Device Status 
typedef struct Command48_Byte6
{
    //Set when a critical or non critical error has been detected
    bool devVarAlert; //bit 1
    //All other bits are unused/undefined
} Command48_Byte6;


//Bit masks for setting indivual bits in a single byte
//From LSB (bit 0) to MSB (bit 7)
const uint8_t bitFieldTable[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20,0x40, 0x80 };

//For setting logic HIGH and LOW
typedef enum
{
    LOW = 0,
    HIGH = 1
} STATE;

typedef uint8_t State;

//Error status at Physical Layer level (UART)
typedef enum
{
    PARITYERROR = 1,
    FRAMEERROR = 2
} PHYERROR;

//Critical and non critical status faults for Cmd 48
typedef enum
{
    CONFICORRUPT,
    CRCFAILURE,
    DACFAILURE,
    DIAGNOSTICSFAILURE
} STATUS;

typedef uint8_t DeviceStatus;

//Returned from the APP layer request to send a command
//to field device with information regarding the response
//If communication failure is returned (status is set to err),
//all other fields should be set to null.
typedef struct Cmd48Response
{   
    //True if a response was received,
    //False if a communication error occured
    CMDCODE status;
    uint8_t* address;
    uint8_t* cmd;
    uint8_t* data;
} Cmd48Response;


void waitForCommand();
void sendResponse();
uint8_t* buildDelimiterField();
uint8_t* buildAddressField(uint8_t* deviceTypeCode, uint8_t* deviceID);
uint8_t* buildCommandField(int command);
uint8_t* buildByteCountField(int byteCount);
bool checkBits(uint8_t* n,uint8_t k);
uint8_t* configureCommand48Response(uint8_t* byte1, uint8_t* byte2, uint8_t* byte3,
                                uint8_t* byte4, uint8_t* byte5, uint8_t* byte6);
char* buildByte1(Command48_Byte1 byte1);
uint8_t checksum(uint8_t* frame, int length);
uint8_t* addFrames(uint8_t* delim, uint8_t* address, uint8_t* command,
                        uint8_t* byteCount, uint8_t* data);
uint8_t* buildResponseFrame(uint8_t* delim, uint8_t* address, uint8_t* command,
                        uint8_t* byteCount, uint8_t* data, uint8_t* checkByte);
                        
#endif