#include <Keypad.h>                           // IC Job Daniel Rodriguez Hernandez
#include <Wire.h>                             // Primera version Oficial del sistema RDAP alias NPQV18
#include <LiquidCrystal_I2C.h>                // Pruebas de laboratorio exitosas - mide presion y flujo 
#include <SoftwareSerial.h>                   // Fecha de actualizacion 5/Ene/2017 2:18 PM
#include <EEPROM.h>                           // Envio de datos Correcto
#include <avr/wdt.h>                          // Nombre del archivo NPQV18_P_FA
#include "RTClib.h"                           // Libreria para el manejo de reloj en tiempo real

SoftwareSerial x_bee(2,3);                    // Creación del puerto serial de comunicacion con x_bee RX TX
//Varibles del Reloj
DateTime now;
int ann,mes,dia,hora,minuto,ord;              // ann = año, ord = orden de entrada
char an,an1,m,m1,d,d1,h,h1,mi,mi1;            // an = primer digito del año, an1 = segundo digito; m = primer digito del mes
byte setclock;                                // banacum = bandera de acumulados
//Variables del flujo acumulado               // m1 = segundo dogito y asi los dias, horas y minutos.
//Variables control de flujo frecuencias.
//Variables control de flujo Amp.
//Variables control de nivel
int n1,n2,n3,n4,n5,n6,j=0;
int s1,s2,s3,s4,s5,s6,por;
double r,dis,discm,NA,Am = 2.0 ,V;
//Variables de control de presion
//Variables control de envio de datos
String cad;                                   // Cadena que se forma antes de ser enviada al Xbee
char a;                                       // Lectura del caracter que manda el Xbee
char con[6];                                  // Cadena que se envia al Xbee
//Variables de control de teclado
const byte Filas = 4;                         // Cuatro filas
const byte Cols = 4;                          // Cuatro columnas
int timer1_counter;                       
byte sel;                                 // Seleccion de lo que se imprime
//Variables de control del flujo del sistema
String cadnum;                                // cadnum = cadena de numeros de valores guardados.
byte concam, bancl;                           // Variable de contador para mostrar la informacion en display
byte concon, banres;                          // Contado de control = concon     // bancl = bandera de clean
unsigned int conres;                                  // Conres = contador de reset      banres = bandera de reset 
byte bancon, banpress;                        // Bandera de control = bancon     //banpress = bandera de tecleo 
byte conmp, conmf;
byte conenv, banenv; 
// Mas variables del control del teclado
byte Pins_Filas[] = {11, 10, 9, 8};           // Pines Arduino para las filas
byte Pins_Cols[] = {7, 6, 5, 4};              // Pines Arduinopara las columnas
char Teclas [ Filas ][ Cols ] =
{
     {'1','2','3','A'},
     {'4','5','6','B'},
     {'7','8','9','C'},
     {'*','0','#','D'}
};

Keypad keypad = Keypad(makeKeymap(Teclas), Pins_Filas, Pins_Cols, Filas, Cols);
RTC_DS3231 RTC;
LiquidCrystal_I2C lcd(0x27,16,2);
 
void setup(){   
  // initialize timer1 
  r = concam = conmp = j = 0;
  bancl = concon = banres = conres = bancon = banpress = 0, conenv = 0, banenv = 0;
  sel = 2;
  restaura();
  wdt_disable();                            // Deshabilita el watch dog
  Wire.begin();                             // Inicia el puerto I2C
  RTC.begin();                              // Inicia la comunicacion con el reloj
  noInterrupts();                           // Disable all interrupts
  TCCR1A = 0;                               // registros para el preescalador de contador 
  TCCR1B = 0;
  timer1_counter = 3036;                    // Preload timer 65536-16MHz/256/2Hz
  TCNT1 = timer1_counter;                   // Preload timer
  TCCR1B |= (1 << CS12);                    // 1 prescaler 
  TIMSK1 |= (1 << TOIE1);                   // Enable timer overflow interrupt
  interrupts();                             // Enable all interrupts
  Serial.begin(9600);                       // Inicializacion de puetos seriales
  x_bee.begin(9600);                        
  x_bee.print("bb");
  keypad.addEventListener(keypadEvent);     // Agragar evento de presion de teclas       
  lcd.init();                               // Inicia LCD
  lcd.noBacklight();
  lcd.setCursor(0,0);
  lcd.print("Iniciando...");
  wdt_enable(WDTO_250MS);
}

ISR(TIMER1_OVF_vect)        // Interrupt service routine, vector de interrupcion, se ejecuta al activar.
{
  TCNT1 = timer1_counter;   // Preload timer, es el tiempo precargado para que se cuente un segundo exacto.

  //Bloque de calculo de la variable de presion 

  //Bloque de calculo de la variable de flujo Amp

  //Bloque de calculo de la variable de flujo Frec.

  // Conteo de cambio de datos en el display
  concam++;               // Contador de cambio aumenta cada segundo
  if (concam > 10){       // a los 10 segundos cambia al siguiente menu
    concam = 0;           // y reinicia el conteo.
    if (bancon == 0){     // Si el la bandera de control es 0 no hay teclas presionadas,
      bancl = 1;          // se limpia el Display.
      if (sel == 3){      // Si el menu esta en 4(dependiendo de los menus)(este caso 3)
        sel = 2;          // que es maximo, regresa al inicio automatico.
      }
      else{               // Si no, este avanza al siguiente menu automatico.
        sel++;
      }
    }
  }

  // controlador para saber si se han presionado o no teclas
  if (banpress == 1){       // si se presiono una tecla
    concon = 0;             // el contador de control es igual con 0 
    banpress = 0;           // y la bandera cambia 0, aqui se reinicial el contador al presionar una tecla
  }

  // Si pasa 1 minuto sin que se presione una tecla se reinica el menu
  concon++;               // Estamos en el bloque de interrupcion que se ejecuta cada segundo
  if (concon > 60){       // entonces el contador de control incrementa cada segundo
    concon = 0;           // si llega al minuto sin presionar nada regresa a 0 y
    if (bancon == 1){     // si bandera de control es igual a 1, entra y reinicia el conteo automatico.
      bancl = 1;          // se limpia el Display.
      sel = 2;            
      bancon = 0;         // y el sitema se inactiva.
    }  
  }

  //Contador de reset 
  conres++;

  if (conres == 36000){        // Esta variable en segundos marca cuando el micro se resetea solo , por ahora lo dejare en 1 hora.
    banres = 1;
  }

  if (banenv == 1)           // bandera de envio se activa para mandar los datos de ambos sensores, defasados uno del otro.
    conenv++;

  if (conenv == 1){
    cad="";                               // Se envia la cadena de nivel
    cad = "N";                            // cad es el string que se envia
    if (discm < 10)                         // 
      dtostrf(discm,4,2,con);
    else if (discm >= 10 && discm < 100)
      dtostrf(discm,5,2,con);
    else 
      dtostrf(discm,6,2,con);
    cad.concat(con);
    x_bee.print(cad);
  }
  
}

void loop()
{      
  if (banres == 1){
    delay(251);  
  }
  else
    wdt_reset();

  //Obtencion de los datos del reloj cada segundo
  now = RTC.now(); // Obtiene la fecha y hora del RTC

  if (bancl == 1){    // Bandera de clear para limpiar el display
    lcd.clear();      // Se limpia el Display y se reinicia la bandera
    bancl = 0;  
  }

  if (bancon == 1)
    lcd.backlight();
  else
    lcd.noBacklight();

  //Deteccion del teclado 
  
  keypad.getKey();    // funcion que detecta el cambio en el teclado.

  //Area de impresion
  
  switch (sel){
    case 0:    
      lcd.setCursor ( 0, 0 );
      lcd.print("1.Show Data");
      lcd.setCursor ( 0, 1 );
      lcd.print("2.Configure Data");
      break;
    case 1:  
      lcd.setCursor ( 0, 0 );
      lcd.print("1.Level");
      break;       
    case 2:  
      lcd.setCursor(0,0);
      lcd.print("Date: ");
      if (now.day() < 10)
        lcd.print("0");
      lcd.print(now.day(), DEC); // Día
      lcd.print('/');
      if (now.month() < 10)
        lcd.print("0");
      lcd.print(now.month(), DEC); // Mes
      lcd.print('/');
      lcd.print(now.year(), DEC); // Año
      lcd.setCursor(0,1);
      lcd.print("Time: ");
      lcd.print(now.hour(), DEC); // Horas
      lcd.print(':');
      if (now.minute() < 10)
        lcd.print("0");
      lcd.print(now.minute(), DEC); // Minutos
      lcd.print(':');
      if (now.second() < 10)
        lcd.print("0");
      lcd.print(now.second(), DEC); // Segundos   
      break;
    case 3:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Current value:");
      lcd.setCursor ( 0, 1 );
      lcd.print("Lev.: ");
      lcd.print(discm);        // discm     //Variable de nivel
      lcd.print(" m");  
      break;
    case 4:  
      lcd.setCursor ( 0, 0 );
      lcd.print("1.Set clock");
      break;          
    case 5:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Data saved!");
      break;   
    case 6: 
      lcd.setCursor ( 0, 0 );
      lcd.print("Date: ");
      lcd.setCursor ( 0, 1 );
      lcd.print(d);
      lcd.print(d1);
      lcd.print('/');
      lcd.print(m);
      lcd.print(m1);
      lcd.print('/');
      lcd.print(an);
      lcd.print(an1);
      break;             
    case 7:
      lcd.setCursor ( 0, 0 );
      lcd.print("Time (24 Hrs): ");
      lcd.setCursor ( 0, 1 );
      lcd.print(h);
      lcd.print(h1);
      lcd.print(':');
      lcd.print(mi);
      lcd.print(mi1);
      break;            
  } 

  //Area de sensores 

  noInterrupts();

  //Sensor de nivel

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
    j = 0; 
  }
  //Sensor de FLujo Amper

  // Seonsor de Presion

  // Termina bloque de sensores 
  
  // Area de envio de datos y recepcion de Xbee
  if (x_bee.available() > 0 ){   // si es mayor a 0 se recibio un dato  
    a = x_bee.read();              
    if(a == 88 || a == 120){      // X en ascii
      banenv = 1;
    } 
    if(a == 67 || a == 99){    // C en ascii
      x_bee.print("CN");     // Se envia la cadena de reconocimiento de sensores
    }
    if(a == 82 || a == 114){    // R en ascii
      banres = 1;              // Se activa la bndera de reset, Reset REMOTO
    }
  }

  interrupts();  
}

//Funcion del teclado cuando se presiona cualquier tecla.
void keypadEvent(KeypadEvent eKey){      //aquientra al presionar cualquier tecla
  banpress = 1;                          //al entrar sabemos que se presiono entonces la bandera es 1       
  if (bancon == 0){                      //si la bandera de control el 0 el sistema esta inactivo 
    sel = 0;                             //pero al presionar una tecla se activa y muetra el menu 0 
    bancon = 1;                          //entonces la bandera de control es 1 pues alguien presiono una tecla
    bancl = 1;                           // con bandera de clear en 1 se limpia el display.
  }
  else if(keypad.getState() == PRESSED){  // aqui se asegura que solo entre una vez en presionar el boton 
    if (setclock == 1){
      switch (eKey){
        case 'A': if (sel == 6)
                    sel = 7;
                  else if (sel == 7){
                    sel = 5;
                    setclock = 0;
                    RTC.adjust(DateTime(ann,mes,dia,hora,minuto,00)); // Establece la fecha y hora
                  }
                  bancl = 1;
                  break;              
        case 'B': bancl = 1; cadnum = ""; ord = 0; break;
        case 'C': break;
        case 'D': sel = 0; bancl = 1; setclock = 0; ord = 0; cadnum = ""; break;
        case '#': break;
        case '*': break;
        default: ord++;
                 if (sel == 6){
                    switch (ord){
                      case 1: cadnum += eKey; d = eKey; break;
                      case 2: cadnum += eKey; d1 = eKey; dia = cadnum.toInt(); cadnum = ""; break;
                      case 3: cadnum += eKey; m = eKey; break;
                      case 4: cadnum += eKey; m1 = eKey; mes = cadnum.toInt(); cadnum = ""; break;
                      case 5: cadnum = "20"; cadnum += eKey; an = eKey; break;
                      case 6: cadnum += eKey; an1 = eKey; ann = cadnum.toInt(); cadnum = ""; ord = 0; break;  
                    } 
                 }
                 else if (sel == 7){
                    switch (ord){
                      case 1: cadnum += eKey; h = eKey; break;
                      case 2: cadnum += eKey; h1 = eKey; hora = cadnum.toInt(); cadnum = ""; break;
                      case 3: cadnum += eKey; mi = eKey; break;
                      case 4: cadnum += eKey; mi1 = eKey; minuto = cadnum.toInt(); cadnum = ""; ord = 0; break;  
                    }
                 }
                 break;
      }
    }
    else{                                 // Si no esta en modo de entrada
      switch(sel){                        // Este switch maneja la navegacion por todos los menus
        case 0:
          switch (eKey){
            case '1': sel = 1; break;
            case '2': sel = 4; break;
            case 'C': sel = 2; break;
          }
          bancl = 1;
          break;    
        case 1:
          switch (eKey){
            case '1': sel = 3; break;
          }
          bancl = 1;
          break; 
        case 4:
          switch (eKey){
            case '1': sel = 6; setclock = 1; break;
          }
          bancl = 1; 
          break;
      } 
      if (eKey == 'D'){             
        sel = 0;
        bancl = 1;
      }
    }
  }  
}

void restaura(){
  an = an1 = 'a';
  m = m1 = 'm';
  d = d1 = 'd';
  h = h1 = 'h';
  mi = mi1 = 'm';
}
