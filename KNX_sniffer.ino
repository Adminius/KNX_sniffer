/*
KNX sniffer 1.0 (2020)
Author: E.Burkowski
*/


#ifdef __AVR_ATmega32U4__       //Universal Flush Interface, ProMicro, Leonardo
#define DEBUGSERIAL Serial      //USB
#define KNX_SERIAL Serial1      //D0,D1
#define LED_PIN LED_BUILTIN     //UFI STD, Aruino Leonardo
//#define LED_PIN 8               //UFI PWM
#define LED_PIN LED_BUILTIN_RX  //ProMicro (Attention: LOW = ON, HIGH = OFF)

#elif __SAMD21G18A__            //Multi Interface, Arduino Zero
#define DEBUGSERIAL SerialUSB   //USB
//#define KNX_SERIAL Serial       //Multi Interface 
#define KNX_SERIAL Serial1      //D0,D1
//#define LED_PIN A5              //Multi Interface
#define LED_PIN LED_BUILTIN     //Zero

#elif __AVR_ATmega2560__        //MEGA
#define DEBUGSERIAL Serial      //USB
#define KNX_SERIAL Serial2      
#define LED_PIN LED_BUILTIN     

#elif ESP32            
#define DEBUGSERIAL Serial      //USB
HardwareSerial Serial1(2);      //17=TX, 16=RX
#define KNX_SERIAL Serial1      //ESP32 uses Serial1
#define LED_PIN LED_BUILTIN
#endif



//Print settings
#define PRINT_PA_GA       //print PA and GA
#define PRINT_CRC         //CRC result
#define PRINT_LENGTH      //payload length
#define PRINT_PAYLOAD     //payload it self
#define PRINT_RAW         //raw telegram
//#define PRINT_NORMAL_ONLY //print normal case only (0xBC), without repeats and so on (0xB0, 0xB4, 0xB8, 0x90, 0x94, 0x98, 0x9C)



//just in case...
#if !(defined(PRINT_PA_GA) && defined(PRINT_CRC) && defined(PRINT_LENGTH) && defined(PRINT_PAYLOAD) && defined(PRINT_RAW))
#define PRINT_RAW
#endif

#define KNX_TELEGRAM_MAX_SIZE 23

/*Byte sequence stadard frame:
Header (6 bytes):
    Byte 0 Control Field 
    Byte 1 Source Address High byte
    Byte 2 Source Address Low byte
    Byte 3 Destination Address High byte
    Byte 4 Destination Address Low byte
    Byte 5 Routing field
Payload (from 1..15 bytes):
    Byte 6 Command field High
    Byte 7 Command field Low + 1st payload data (6bits) (used only if payload length = 1 or 15)
    Byte 8..n-1 payload bytes (optional)
Checksum (1 byte)
    Byte n
*/

byte datalength = 0;
bool dataLengthKnown = false;
bool dataReceived = false;
bool telegramReceived = false;
byte telegram[KNX_TELEGRAM_MAX_SIZE];
byte counter = 0;
byte headerCounter = 20;

void setup() {
    KNX_SERIAL.begin(19200,SERIAL_8E1);
    DEBUGSERIAL.begin(115200);
    while(!DEBUGSERIAL);
//    delay(5000);
    DEBUGSERIAL.print("KNX initialisation");
    KNX_SERIAL.write(0x01); //reset request
    byte counter = 19;
    while(counter){
        delay(500);
        if(KNX_SERIAL.available() > 0 && KNX_SERIAL.read() == 0x03){
            DEBUGSERIAL.println(" OK");
            break;
        }else{
            DEBUGSERIAL.print(".");
            counter--;
        }
        if (!counter) {
            DEBUGSERIAL.println(" failed");
            while(true); //do nothing
        }
    }
    pinMode(LED_PIN, OUTPUT);
    clearBuffer();
    printHeader();
}

void loop() {
    if(KNX_SERIAL.available() > 0){
        byte temp = KNX_SERIAL.read();
        telegram[counter] = temp;
        if (counter == 5) {
            datalength = telegram[5] & B00001111;
            dataLengthKnown = true;
        }
/* 
 * let's asume we know 5th byte in telegram.
 * last 4 bits tell us the payload length
 * we have 9 byte overhead (0..7 + CRC)
 * Example 1:  1 byte payload length + 9 bytes overhead =  9 bytes telegram length, but for "1 byte" we using is "6 bit magic" in 7th byte, so the length is still 9 bytes: counter 0..8
 * Example 2:  2 byte payload length + 9 bytes overhead = 10 bytes telegram length, first byte is "6 bit magic" (but we don't use it) + 1 byte payload, so real payload is only 1 byte counter 0..9
 * Example 3:  3 byte payload length + 9 bytes overhead = 11 bytes telegram length, first byte is "6 bit magic" (but we don't use it) + 2 byte payload, so real payload is only 2 byte counter 0..10
 * Example 4: 15 byte payload length + 9 bytes overhead = 23 bytes telegram length, because first byte is "6 bit magic" + 14 bytes payload, counter 0..22
 */

        if(dataLengthKnown && counter == (datalength + 7)){
            telegramReceived = true;
            dataLengthKnown = false;
        }
        counter++;
        //in case we started some were in the middele of telegram transmition, drop all bytes until control byte
        if(counter == 1 && !(temp == 0xBC 
#ifndef PRINT_NORMAL_ONLY
        || temp == 0xB0 || temp == 0xB4 || temp == 0xB8 || temp == 0x90 || temp == 0x94 || temp == 0x98 || temp == 0x9C)){
#else
        )){
#endif
            counter = 0;
        }
        if(!dataReceived){
            dataReceived = true;
            digitalWrite(LED_PIN, HIGH); //LED is ON => data received
        }
    }
    if(telegramReceived){
        dataReceived = false;
        telegramReceived = false;
        if(headerCounter == 0){
            printHeader();
            headerCounter = 19;
        }else{
            headerCounter--;
        }
#ifdef PRINT_NORMAL_ONLY
        if(telegram[0] == 0xBC){
#endif
        printData();
#ifdef PRINT_RAW
        for (byte i = 0; i < counter; i++){
            if(telegram[i] < 16) DEBUGSERIAL.print("0"); //print '0' for 2 digit HEX
            DEBUGSERIAL.print(telegram[i], HEX);
            DEBUGSERIAL.print(" ");
        }
#endif
        DEBUGSERIAL.print("\t");
        DEBUGSERIAL.println("");
        
        counter = 0;
        clearBuffer();
        digitalWrite(LED_PIN, LOW); //turn LED off
 #ifdef PRINT_NORMAL_ONLY       
        }
#endif        
    }
}

void clearBuffer(){
    memset(telegram, 0, sizeof(telegram));
}

bool checkCRC(byte length){
    byte crc = 0xFF;
    for (byte i = 0; i < length-1; i++){
        crc ^= telegram[i];
    }
    return crc == telegram[length-1];
}

void printHeader(){
#ifdef PRINT_PA_GA  
    DEBUGSERIAL.print("PA:    \tGA:      ");
#endif
#ifdef PRINT_CRC
    DEBUGSERIAL.print("CRC:\t");
#endif
#ifdef PRINT_LENGTH
    DEBUGSERIAL.print("Length:\t");
#endif
#ifdef PRINT_PAYLOAD
    DEBUGSERIAL.print("Payload:\t");
#endif
#ifdef PRINT_RAW
    DEBUGSERIAL.println("RAW:");
#endif
}

void printData(){
    byte pa1 = telegram[1] >> 4;
    byte pa2 = telegram[1] & B00001111;
    byte ga1 = telegram[3] >> 3;
    byte ga2 = telegram[3] & B00000111;
    byte firstDataByte = telegram[7] & B00111111; //use only last 6 bits
#ifdef PRINT_PA_GA
    DEBUGSERIAL.print(pa1, DEC);
    DEBUGSERIAL.print(".");
    DEBUGSERIAL.print(pa2, DEC);
    DEBUGSERIAL.print(".");
    DEBUGSERIAL.print(telegram[2], DEC);
    DEBUGSERIAL.print("\t");
    DEBUGSERIAL.print(ga1, DEC);
    DEBUGSERIAL.print("/");
    DEBUGSERIAL.print(ga2, DEC);
    DEBUGSERIAL.print("/");
    DEBUGSERIAL.print(telegram[4], DEC);
    if(ga1 > 9 && telegram[4] > 99){
        DEBUGSERIAL.print(" ");
    }else{
        DEBUGSERIAL.print("\t ");
    }
#endif
#ifdef PRINT_CRC
    if(checkCRC(counter)){
      DEBUGSERIAL.print("OK\t");
    }else{
      DEBUGSERIAL.print("nOK\t");
    }
#endif    
#ifdef PRINT_LENGTH
    switch (datalength){
        case 0:
          DEBUGSERIAL.print(""); //repeat or something?!
          break;
        case 1:
          DEBUGSERIAL.print(1, DEC); //if 1 then usable data max 6 bits in byte number 7 DPT 1.xxx, DPT 2.xxx, DPT 3.xxx, DPT 23.xxx ...
          break;
        case 15:
          DEBUGSERIAL.print(15, DEC); //special case, 14 bytes payload + 6 bits in byte number 7
          break;
        default:
          DEBUGSERIAL.print(datalength - 1, DEC); //ignore first byte because data doesn't fit in 6 bits (this 6 bits are all '0')
          break;
    }
#if !defined(PRINT_CRC) && defined(PRINT_PA_GA)
    DEBUGSERIAL.print(" \t");
#endif    
    DEBUGSERIAL.print("  \t");
#endif
#ifdef PRINT_PAYLOAD
    if(datalength == 1 || datalength == 15){ //print "6 magic bits of data" from 7th byte
        if(firstDataByte < 16) DEBUGSERIAL.print("0"); //print '0' for 2 digit HEX
        DEBUGSERIAL.print(firstDataByte, HEX);
        DEBUGSERIAL.print(" ");
    }
    if(datalength == 15) datalength = 14; //if length is 15, the first "byte" is only 6 bit long fitted in byte number 7 and 14 payload bytes.
    for(byte i = 1; i < datalength; i++){
        if(telegram[7+i] < 16) DEBUGSERIAL.print("0"); //print '0' for 2 digit HEX
        DEBUGSERIAL.print(telegram[7+i], HEX);
        DEBUGSERIAL.print(" ");
    }
    DEBUGSERIAL.print("\t");
    if(datalength < 4) DEBUGSERIAL.print("\t");
#endif
}
