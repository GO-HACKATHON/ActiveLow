#include<SPI.h>
#include<MFRC522.h>
#include<Wire.h>
#include<math.h>
#include<SoftwareSerial.h>
#include<EEPROM.h>
SoftwareSerial hc05(D3, D4);

#define SS_PIN D8
#define RST_PIN D0

#define  Led_addr      0x3F
#define  ON   0
#define  OFF  1

byte id[10];
char NFC[3][4]={{0xDD, 0xAE, 0x3A, 0xD5},
                {0xC5, 0x59, 0xAE, 0x75},
                {0x35, 0x94, 0x2A, 0x77}};

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
int rfid_no;
int x;
int mode_Navi;

void setup() { 
  Serial.begin(9600);
  hc05.begin(9600);
  EEPROM.begin(1000);
  SPI.begin(); // Init SPI bus
  Wire.begin();    // Join I2C bus
  deviceWrite(Led_addr,0xFF);
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) 
  {
    key.keyByte[i] = 0xFF;
  }

  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
int i;
 
void loop() 
{
read_rf();
//
//  for(i=2;i<=8;i++)
//  {
//    deviceWrite(Led_addr,led(1, ON));
//    delay(100);
//    deviceWrite(Led_addr,led(1, OFF));
//    delay(100);
//  }
//  i=2;
navi();
//while(mode_Navi==2){
//  read_rf();
//}
}

void read_rf()
{
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));
  
  Serial.print("NO= ");
  Serial.println(rfid_no);
  
  deviceWrite(Led_addr,led(1, ON));
  delay(100);
  deviceWrite(Led_addr,led(1, OFF));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

int y;

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    id[i]=buffer[i];
    
    if(id[i]==NFC[0][i]){
      x++;
      if(x == 4){rfid_no=1; mode_Navi=1;}
      else rfid_no=0;
    }
    else if(id[i]==NFC[1][i]){
      y++;
      if(y == 4){rfid_no=2; mode_Navi=1;}
      else rfid_no=0;
    }
    else if(id[i]==NFC[2][i]){
      x++;
      if(x == 4){rfid_no=3; mode_Navi=1;}
      else rfid_no=0;
    }
    else rfid_no=0;
  }
  x=0;
  hc05.print(rfid_no);
}

void deviceWrite( byte addr, byte txData)
{
  Wire.beginTransmission(addr);
  Wire.write(txData);
  Wire.endTransmission();
}

byte deviceRead(byte addr)
{
    Wire.requestFrom(addr, 2);    // request 6 bytes from slave device #8

  while (Wire.available()) { // slave may send less than requested
  byte c = Wire.read(); // receive a byte as character
    return c;
  }
}

byte led(int no, int stat)
{
  if(stat==ON)
  {
    byte nilai = 0xFF << no;
    byte hasil = (pow(2,no-1))-1;
    byte status_Led = (nilai | hasil) & deviceRead(Led_addr);
    return status_Led; 
  }
  else if(stat==OFF)
  {
    byte hasil = (pow(2,no-1));
    byte status_Led = hasil | deviceRead(Led_addr);;
    return status_Led; 
  }
}

int bt;
void navi()
{
    while (hc05.available() > 0) {
    bt=hc05.read();
    Serial.println(bt);
  }
  
  if(bt==70){
    deviceWrite(Led_addr,led(2, ON));
  }
  else if(bt==66){
    deviceWrite(Led_addr,led(6, ON));
  }
  else if(bt==82){
    deviceWrite(Led_addr,led(3, ON));
    deviceWrite(Led_addr,led(5, ON));
  }
  else if(bt==76){
    deviceWrite(Led_addr,led(8, ON));
    deviceWrite(Led_addr,led(7, ON));
  }
  else if(bt==73){
    deviceWrite(Led_addr,led(2, ON));
    deviceWrite(Led_addr,led(3, ON));
    deviceWrite(Led_addr,led(5, ON));
  }
  else if(bt==71){
    deviceWrite(Led_addr,led(2, ON));
    deviceWrite(Led_addr,led(8, ON));
    deviceWrite(Led_addr,led(7, ON));
  }
  else if(bt==74){
    deviceWrite(Led_addr,led(5, ON));
    deviceWrite(Led_addr,led(3, ON));
    deviceWrite(Led_addr,led(6, ON));
  }
  else if(bt==72){
    deviceWrite(Led_addr,led(6, ON));
    deviceWrite(Led_addr,led(7, ON));
    deviceWrite(Led_addr,led(8, ON));
  }
  else if(bt==84){
    mode_Navi=2;
  }
  else{
    deviceWrite(Led_addr,led(2, OFF));
    deviceWrite(Led_addr,led(3, OFF));
    deviceWrite(Led_addr,led(5, OFF));
    deviceWrite(Led_addr,led(6, OFF));
    deviceWrite(Led_addr,led(7, OFF));
    deviceWrite(Led_addr,led(8, OFF));
  }
}
