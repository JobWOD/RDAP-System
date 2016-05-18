#include <LiquidCrystal.h>    // ST = sin temperatura  (sufijos del nombre de archivo)
                              // RF = Relacion del Fabricante
//Inicializacion del los pines de control del Display
LiquidCrystal lcd(6,7,8,9,10,11);
double val = 0,r = 0,qa = 0,q = 0,tem = 0, p = 0, pro = 0;
int timer1_counter,i=0,b = 0,j = 0,z = 0, cp = 0;
char a;
String cad;
char con[6];

void setup() {
  //Definicion de las columnas y filas del Display 
  lcd.begin(16, 2);
  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  timer1_counter = 3036;   // preload timer 65536-16MHz/256/2Hz
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 1 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  // Print a message to the LCD.
  lcd.print("INI");
  Serial.begin(9600);
  Serial.print("bb");
}

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  lcd.clear();
  TCNT1 = timer1_counter;   // preload timer
  //if (b == 1){
    pro = p/cp;
    r = (1.4063 * pro) - 1.1875;
    if (r <= 0.05)
      r = 0; 
    i++;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("V = ");
    lcd.print(r);        //caudal 
    lcd.print(" m/s");
    lcd.setCursor(0,1);      //Impresiones de pruebas
    lcd.print(pro);
    p = 0;
    cp = 0;
  //} 
}

void loop() {   
  noInterrupts();  
  val = ((5.0 * analogRead(0))/1024.0);
  p = p + val;
  cp++;
  
  if (Serial.available() > 0 ){
    a = Serial.read();
    if(a == 88 || a == 120){
        cad="";
        cad = "F";
        if (r < 10)
          dtostrf(r,5,3,con);
        else
          dtostrf(r,6,3,con);
        cad.concat(con);
        Serial.print(cad);
    }
    if(a == 67 || a == 99){
      Serial.print("CF");
    }
    //if(a == 73 || a == 105 ){
    //  b = 1;
    //}  
  }
  interrupts(); 
}
