#include <EEPROM.h> 

union Float_Byte{
  float datoF;
  byte datoB[4];
} unionFB;

void setup() {
  EEPROMWriteint(0,0);    // Diametro de tuberia
  EEPROMWritelong(2,0);   // Acumulado final
  EEPROMWritelong(6,0);   // Adumulado dia anterio
  EEPROMWritelong(10,0);  // Acumulado del dia
  EEPROMWriteint(14,0);   // Numero de mediciones 
  EEPROMWriteint(16,0);   // Numero de mediciones dia anterior
  EEPROMWriteint(18,0);   // Bnadera de fecha
  EEPROMWriteint(20,0);   // Dia
  EEPROMWriteint(22,0);   // Mes
  EEPROMWriteint(24,0);   // Año
  EEPROMWritefloat(26,0.0); // Diametro en decimales
}

void loop() {
  // put your main code here, to run repeatedly:
}

void EEPROMWritelong(int address, long value){
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

long EEPROMReadlong(long address){
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EEPROMWriteint(int address, unsigned int value){
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, two);
  EEPROM.write(address + 1, one);
}

long EEPROMReadint(long address){
  //Read the 4 bytes from the eeprom memory.
  unsigned int two = EEPROM.read(address);
  unsigned int one = EEPROM.read(address + 1);
  
  //Return the recomposed long by using bitshift.
  return ((two << 0) & 0xFF) + ((one << 8) & 0xFFFF);
}

void EEPROMWritefloat(int address, float value){
  unionFB.datoF = value;
  EEPROM.write(address, unionFB.datoB[0]);
  EEPROM.write(address + 1, unionFB.datoB[1]);
  EEPROM.write(address + 2, unionFB.datoB[2]);
  EEPROM.write(address + 3, unionFB.datoB[3]);
}

float EEPROMReadfloat(int address){
  //Read the 4 bytes from the eeprom memory.
  unionFB.datoF = 0.0;
  unionFB.datoB[0] = EEPROM.read(address);
  unionFB.datoB[1] = EEPROM.read(address + 1);
  unionFB.datoB[2] = EEPROM.read(address + 2);
  unionFB.datoB[3] = EEPROM.read(address + 3);
  //Return the recomposed long by using bitshift.
  return unionFB.datoF;
}
