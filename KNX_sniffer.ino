/*
KNX sniffer (no version, work in progress)
Author: E.Burkowski
*/


#ifdef __AVR_ATmega32U4__       //Universal Flush Interface, ProMicro, Leonardo
#define DEBUGSERIAL Serial      //USB
#define KNX_SERIAL Serial1      //D0,D1
//#define LED_PIN 13              //UFI STD, Aruino Leonardo
#define LED_PIN 8               //UFI PWM
//#define LED_PIN LED_BUILTIN_RX  //ProMicro (Attention: LOW = ON, HIGH = OFF)
#elif __SAMD21G18A__            //Multi Interface, Arduino Zero
#define DEBUGSERIAL SerialUSB   //USB
//#define KNX_SERIAL Serial       //Multi Interface 
#define KNX_SERIAL Serial1      //D0,D1
//#define LED_PIN A5              //Multi Interface
#define LED_PIN LED_BUILTIN     //Zero
#elif __AVR_ATmega2560__            //Multi Interface, Arduino Zero
#define DEBUGSERIAL Serial   //USB
#define KNX_SERIAL Serial2      
#define LED_PIN LED_BUILTIN     
#elif ESP32            //Multi Interface, Arduino Zero
#define DEBUGSERIAL Serial   //USB
HardwareSerial Serial1(2); //17=TX, 16=RX
#define KNX_SERIAL Serial1 // ESP32 use Serial1
#define LED_PIN LED_BUILTIN     //Zero
#endif

#define KNX_TELEGRAM_MAX_SIZE 23
#define KXN_STANDARD_TELEGRAM 0xBC



//Print settings
//#define PRINT_CRC         //CRC column
#define PRINT_BYTES       //payload length
#define PRINT_PAYLOAD     //payload it self
#define PRINT_RAW         //raw telegram
#define PRINT_NORMAL_ONLY //print standard frame only, without repeats and so on...

/*Byte sequence stadard frame:
--Header (6 bytes):
  Byte 0 Control Field
  Byte 1 Source Address High byte
  Byte 2 Source Address Low byte
  Byte 3 Destination Address High byte
  Byte 4 Destination Address Low byte
  Byte 5 Routing field
--Payload (from 2..16 bytes):
  Byte 6 Command field High
  Byte 7 Command field Low + 1st payload data (6bits)
  Byte 8..n-1 payload bytes (optional)
--Checksum (1 byte)
  Byte n
*/

bool dataReceived = false;
bool telegramReceived = false;
byte telegram[KNX_TELEGRAM_MAX_SIZE];
byte counter = 0;
byte  headerCounter = 20;

void setup() {
    KNX_SERIAL.begin(19200,SERIAL_8E1);
    DEBUGSERIAL.begin(115200);
    delay(5000);
    DEBUGSERIAL.println("Begin");
    pinMode(LED_PIN,OUTPUT);
    clearBuffer();
    printHeader();
}

void loop() {
    if(KNX_SERIAL.available()>0){

        byte temp = KNX_SERIAL.read();
        telegram[counter] = temp;
        
        // let's asume we know 5th byte in telegram.
        // last 4 bits tell us the payload length
        // we have 8 byte overhead (0..7 + CRC)
        // Example: 1 byte payload length + 8 byte overhead = 9 bytes telegram length
        // in this case counter will count from 0 to 8
        if(counter == (telegram[5] & B00001111) + 7){
            telegramReceived = true;
        }
        counter++;
        if(!dataReceived){
            dataReceived = true;
            digitalWrite(LED_PIN,HIGH); //LED is ON => data received
        }
    }
    if(telegramReceived){

        dataReceived = false;
        telegramReceived = false;

        if(headerCounter == 0){
            printHeader();
            headerCounter = 20;
        }else{
            headerCounter--;
        }
#ifdef PRINT_NORMAL_ONLY
        if(telegram[0] == KXN_STANDARD_TELEGRAM){
#endif
        printData();
#ifdef PRINT_RAW
        DEBUGSERIAL.print("   \t");
        for (byte i = 0; i < counter;i++){
            if(telegram[i]<16) DEBUGSERIAL.print("0"); //print '0' for 2 digit HEX
            DEBUGSERIAL.print(telegram[i],HEX);
            DEBUGSERIAL.print(" ");
        }
#endif
        DEBUGSERIAL.println("");
        
        counter = 0;
        clearBuffer();
        digitalWrite(LED_PIN,LOW); //turn LED off
 #ifdef PRINT_NORMAL_ONLY       
        }
#endif        
    }
}

void clearBuffer(){
    memset(telegram,0,sizeof(telegram));
}

bool checkCRC(byte length){
    byte crc = 0xFF;
    for (byte i = 0;i < length-1;i++){
        crc ^= telegram[i];
    }
    return crc == telegram[length-1];
}

void printHeader(){
    DEBUGSERIAL.print("PA:    \tGA:    ");
#ifdef PRINT_CRC
    DEBUGSERIAL.print("\tCRC:");
#endif
#ifdef PRINT_BYTES
    DEBUGSERIAL.print("\tBytes:");
#endif
#ifdef PRINT_BYTES
    DEBUGSERIAL.print("\tPayload:");
#endif
#ifdef PRINT_RAW
    DEBUGSERIAL.println("\tRAW:");
#endif
}

void printData(){
  byte temp = telegram[1];
  byte pa1 = telegram[1] >> 4;
  byte pa2 = telegram[1] & B00001111;
  byte ga1 = telegram[3] >> 3;
  byte ga2 = telegram[3] & B00000111;
  byte datalength = telegram[5] & B00001111;
  byte firstDataByte = telegram[7] & B00111111; //use only last 6 bits

  DEBUGSERIAL.print(pa1,DEC);
  DEBUGSERIAL.print(".");
  DEBUGSERIAL.print(pa2,DEC);
  DEBUGSERIAL.print(".");
  DEBUGSERIAL.print(telegram[2],DEC);
  DEBUGSERIAL.print("\t");

  DEBUGSERIAL.print(ga1,DEC);
  DEBUGSERIAL.print("/");
  DEBUGSERIAL.print(ga2,DEC);
  DEBUGSERIAL.print("/");
  DEBUGSERIAL.print(telegram[4],DEC);
#ifdef PRINT_CRC
   if(checkCRC(counter)){
      DEBUGSERIAL.print("\tOK ");
   }else{
      DEBUGSERIAL.print("\tnOK ");
   }
#endif
#ifdef PRINT_BYTES
  DEBUGSERIAL.print("\t");
  DEBUGSERIAL.print(datalength,DEC);
/*
  if(datalength == 1){
      DEBUGSERIAL.print(datalength,DEC); //if 1 then usable data max 6 bits, DPT 1.xxx, DPT 2.xxx, DPT 3.xxx, DPT 23.xxx ...
  }else{
      DEBUGSERIAL.print(datalength-1,DEC); //ignore first byte because data doesn't feet in 6 bits
  }
*/
#endif
#ifdef PRINT_PAYLOAD
    DEBUGSERIAL.print("\t");

  if(datalength == 1){
      if(firstDataByte < 16) DEBUGSERIAL.print("0"); //print '0' for 2 digit HEX
      DEBUGSERIAL.print(firstDataByte,HEX);
      DEBUGSERIAL.print(" ");
  }
  for(byte i = 1;i < datalength;i++){
      if(telegram[7+i]<16) DEBUGSERIAL.print("0");  //print '0' for 2 digit HEX
      DEBUGSERIAL.print(telegram[7+i],HEX);
      DEBUGSERIAL.print(" ");
  }
#endif  
}

