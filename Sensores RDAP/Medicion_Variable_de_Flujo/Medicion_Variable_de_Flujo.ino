#include <LiquidCrystal.h>

int pin = 12,b=0,c=0,timer1_counter,fin,flu;
double per,per2,per3,frec,frecs,frecf,u =1.0,y,x,flujo,RF,V;
String cad;
char a;
char con[6];
LiquidCrystal lcd(6,7,8,9,10,11);    //LiquidCrystal(rs, enable, d4, d5, d6, d7) 

void setup()
{
  lcd.begin(16, 2);
  pinMode(pin, INPUT);
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  timer1_counter = 3036;   // preload timer 65536-16MHz/256/2Hz
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 1 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  Serial.begin(9600);
  Serial.print("bb");
}

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  TCNT1 = timer1_counter;   // preload timer
  b++;          // codigo para extender el tiempo de los valores promedio 
  if (b == 60){
     b=0;
     frecf = frecs/c;
     fin = frecf;
     flujo = fin/9.9;
     flu = flujo*10;
     flujo = flu/10.0;       // Despues de esta linea se respalda codigo en el archivo 
     RF = 0.5612 * flujo + 0.5582;
     if (flujo == 0)
      RF = 0;
     c=0;                    // Respaldo de flujo pues era codigo espcifico para el canal
     frecs = 0;              // de pruebas. ahora solo envia la velocidad
     V = 0.0368 * RF + 0.2521;
     if (RF == 0)
      V = 0;
  }
}

void loop(){
  noInterrupts();
  per = pulseIn(pin, HIGH);
  per2 = pulseIn(pin, LOW);
  per3 = per + per2;
  per3 = per3/1000;
  if (per3 == 0)
    frec = 0;
  else
    frec = (1/per3)*1000;
  frecs = frecs + frec;   //Suma de frecuencias para promedios en la medicion
  c++;
 
  if (Serial.available() > 0 ){
    
    a = Serial.read();
    if(a == 88 || a == 120){    // X en ascii
        cad="";
        cad = "F";
        if (V < 10)
          dtostrf(V,5,3,con);
        else
          dtostrf(V,6,3,con);
        cad.concat(con);
        delay(10000);
        Serial.print(cad);
    }
    
    if(a == 67 || a == 99){    // C en ascii
        Serial.print("CF");
    }
  }
  interrupts();
   
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Flujo: ");
  lcd.setCursor(7, 0);
  lcd.print(V);
  lcd.print(" V"); 
}
