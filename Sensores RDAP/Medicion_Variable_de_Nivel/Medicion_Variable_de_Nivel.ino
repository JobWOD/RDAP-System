//#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

//Inicializacion del los pines de control del Display
//LiquidCrystal lcd(6,7,8,9,10,11);    //LiquidCrystal(rs, enable, d4, d5, d6, d7
SoftwareSerial x_bee(2,3);
int n1,n2,n3,n4,n5,n6,j=0;
int s1,s2,s3,s4,s5,s6,por;
double r,dis,discm,NA,Am = 2.0 ,V;    /*0.5*/
String cad;
char a;
char con[6];

void setup() {
  delay(1000);
  //Definicion de las columnas y filas del Display 
  //lcd.begin(16,2);
  // Print a message to the LCD.
  //lcd.print("Ini");
  Serial.begin(9600);
  x_bee.begin(9600);
  x_bee.print("bb");
}
 
void loop() {
  j = 0;
  if (Serial.available()>0 && j == 0){
    n1=Serial.read();
    j = 1;
  }
  if (Serial.available()>0 && j == 1){
    n2=Serial.read();
    j = 2;
  }
  if (Serial.available()>0 && j == 2){
    n3=Serial.read();
    j = 3;
  }
  if (Serial.available()>0 && j == 3){
    n4=Serial.read();
    j = 4;
  }
  if (Serial.available()>0 && j == 4){
    n5=Serial.read();
    j = 5;
  }
  if (Serial.available()>0 && j == 5){
    n6=Serial.read();
    j = 6;
  }
  if (j == 6){
    s1 = n1 - 48;
    s2 = n2 - 48;
    s3 = n3 - 48;
    s4 = n4 - 48;
    s5 = n5 - 48;
    r = (s5*1)+(s4*10)+(s3*100)+(s2*1000)+(s1*10000);
    dis = r * 0.003384;
    discm = (dis * 2.54)/100.0;   //Distancia en cemtimetros, a continuacion se quito               
    /*lcd.clear();                 //codigo que transformaba a volumen, guardado en archivo
    lcd.setCursor(0,0);          //respaldo de nivel.
    lcd.print("Nivel: ");
    lcd.setCursor(7,0);
    lcd.print(discm);
    lcd.print(" cm");*/  
    
    if (x_bee.available() > 0 ){
      
      a = x_bee.read();
      if(a == 88 || a == 120){    // X en ascii
        cad="";
        cad = "N";
        if (discm < 10)
          dtostrf(discm,4,2,con);
        else if (discm >= 10 && discm < 100)
          dtostrf(discm,5,2,con);
        else 
          dtostrf(discm,6,2,con);
        cad.concat(con);
        x_bee.print(cad);
      }
      
      if(a == 67 || a == 99){    // C en ascii
        x_bee.print("CN");
      }
      
    }
  } 
}
