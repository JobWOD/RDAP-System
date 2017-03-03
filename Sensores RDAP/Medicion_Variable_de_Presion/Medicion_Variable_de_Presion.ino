#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Inicializacion del los pines de control del Display

double val = 0,r = 0,p = 0,ref,rkc,rca;
int timer1_counter,i=0,red,j = 0,rcat,k=0;
char a;
String cad;
char con[6];

LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  //Definicion de las columnas y filas del Display 
  pinMode(5,INPUT);
  // initialize timer1 
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
  k++;
  r = p/i;
  red = r * 1000;
  ref = 0.0114 * red - 9.4773;
  if (red <= 835)
    ref = 0;
  rca = ref*10;
  rkc = ref*100;
  p=0;
  i=0;
  r=0;
}

void loop() {
  noInterrupts();
  val = (5.000 * analogRead(0))/1024.000;
  p += val;
  i++;
  
  if (Serial.available() > 0 ){
    
    a = Serial.read();
    if(a == 88 || a == 120){    // X en ascii
      cad="";
      cad = "P";
      if (rca < 10)
        dtostrf(rca,4,2,con);
      else
        dtostrf(rca,5,2,con);
      cad.concat(con);
      delay(20000);
      Serial.print(cad);
    }
    
    if(a == 67 || a == 99){    // C en ascii
      Serial.print("CP");
    }
  }
  interrupts();
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PRES.: ");
  lcd.setCursor(0,1);
  lcd.print(rca);
  lcd.print(" MCA");  
}
