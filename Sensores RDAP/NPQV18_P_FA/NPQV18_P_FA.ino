#include <Keypad.h>                           // IC Job Daniel Rodriguez Hernandez
#include <Wire.h>                             // Primera version Oficial del sistema RDAP alias NPQV18
#include <LiquidCrystal_I2C.h>                // Pruebas de laboratorio exitosas - mide presion y flujo 
#include <SoftwareSerial.h>                   // Fecha de actualizacion 5/Ene/2017 2:18 PM
#include <EEPROM.h>                           // Envio de datos Correcto
#include <avr/wdt.h>                          // Nombre del archivo NPQV18_P_FA
#include "RTClib.h"                           // Libreria para el manejo de reloj en tiempo real

SoftwareSerial x_bee(2,3);                    // Creación del puerto serial de comunicacion con x_bee RX TX
//Guardar diametro
union Float_Byte{
  float datoF;
  byte datoB[4];
} unionFB;
//Varibles del Reloj
DateTime now;
int ann,mes,dia,hora,minuto,ord;              // ann = año, ord = orden de entrada
char an,an1,m,m1,d,d1,h,h1,mi,mi1;            // an = primer digito del año, an1 = segundo digito; m = primer digito del mes
byte setclock, banacum;                       // banacum = bandera de acumulados
//Variables del flujo acumulado               // m1 = segundo dogito y asi los dias, horas y minutos.
unsigned int nummed, nummedda, acumdia, acumdiaan;      // nummed = numero de mediciones o sumas (minutos), nummedda = mediciones del dia anterior
unsigned long acumfin;                        // acumdia = acumulado de flujo del dia. acumdiaan = acumulado del dia anterior.
int banfecha;
//Variables control de flujo frecuencias.     // acum fin = acumulado final
//Variables control de flujo Amp.
double valf,res,pf,reff,fl,sfl,flp;           // valf = valor analogico 0, res = promedio de valf, pf = sumatoria , sfl = sumatoria de flujo
int b,redf;                                   // reff = ajuste de curvas, fl = valor de flujo, b = contador, redf = redondeo, flp = flujo promedio
//Variables control de nivel
//Variables de control de presion
double val,r,p,ref,rkc,rca,srca,rcap;         // val = valor analogico 1, r = promedio de val, p = sumatoria, ref = ajuste de curvas
int i,red;                                    // rkc = resultado en kg , rca = resultado en MCL, i = contador, red = redondeo
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
double areaf, arean;                          // areaf = area del flujo, arean = area de nivel  
byte inmode;                                  // Variable que indica que hay datos de entrada.
String cadnum;                                // cadnum = cadena de numeros de valores guardados.
float canu;                                     // canu = area de la tuberia guardada
float dm;
float art;                                   // dm = diametro en mm, canu = valor del area
byte concam, bancl;                           // Variable de contador para mostrar la informacion en display
byte concon, banres;                          // Contado de control = concon     // bancl = bandera de clean
unsigned int conres;                          // Conres = contador de reset      banres = bandera de reset 
byte bancon, banpress;                        // Bandera de control = bancon     //banpress = bandera de tecleo 
byte conmp, conmf;                            // conmp = contador medicion de presion, conmf = contador medicion de flujo
byte conenv, banenv;                          // conenv = contador de envio, banenv = bandera de envio
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
  valf = b = val = r = p = i = areaf = arean = inmode = concam = conmp = banacum = 0;
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
  r = p/i;                                  // Se calcula el promedio de lod valores leidos
  red = r * 1000;                           // Se redondea para mejorar la manipulacion de los datos
  ref = 0.0114 * red - 9.8864;              // el ajuste de curvas hace referenca a un archivo excel /Users/REX/Documents/PBB's
  if (red <= 870)                           // 820 es el valor real minimo en campo.                 /curvas caracterisitcas.xlsx
    ref = 0;                        
  rca = ref*10;                             // calcula el valor en metros columna de agua
  conmp ++;                                 // sumatoria de mediciones
  rkc = ref*100;                            // calcula el valor en kg
  p=0;                                      // se inicia todo en 0 para un mejor calculo posterior
  i=0;
  r=0;
  srca += rca;                              // sumatoria de metro columna de agua
  if (conmp == 60){                         // al pasar un minuto
    rcap = srca / conmp;                    // promedio de metros columna de agua
    srca = 0;                               // reinicia sumatoria
    conmp = 0;                              // reinicia numero de mediciones
  }

  //Bloque de calculo de la variable de flujo Amp

  res = pf/b;                               // Se calcula el promedio de lod valores 
  redf = res * 1000;                        // Se redondea para mejorar la manipulacion de los datos
  reff = 0.0022 * redf - 1.905;            // el ajuste de curvas hace referenca a un archivo excel
  canu = EEPROMReadfloat(26);               // se lee la memoria EEPROM segun el dato que guardaron
  dm = canu/10.0;                           // conversion de los milimetros
  art = (3.14159 * dm * dm)/4.0;            // calculo del area de la tuberia
  if (reff <= 0.37)                         // si la velocidad es muy poca muestra 0
    fl = 0;
  else
    fl = (reff * art)/10.0;                 // calculo final de flujo
  conmf ++;                                 // sumatoria de mediciones de flujo
  pf=0;                                     // se inicia todo en 0 para un mejor calculo posterior
  b=0;
  res=0;
  sfl += fl;                                // sumatoria del valor de flujo
  if (conmf == 60){                         // en un minuto
    flp = sfl / conmf;                      // promedio de medicion de flujo
    sfl = 0;                                // reinicia la sumatoria de valores 
    conmf = 0;                              // reinicia el contador de mediciones
    acumdia = EEPROMReadlong(10);
    acumdia += (flp * 60);                    // variable = flp // el 1 es para pruebas
    EEPROMWritelong(10,acumdia);
    nummed = EEPROMReadint(14);
    nummed ++;
    EEPROMWriteint(14,nummed);
  }

  //Bloque de calculo de la variable de flujo Frec.

  // Conteo de cambio de datos en el display
  concam++;               // Contador de cambio aumenta cada segundo
  if (concam > 10){       // a los 10 segundos cambia al siguiente menu
    concam = 0;           // y reinicia el conteo.
    if (bancon == 0){     // Si el la bandera de control es 0 no hay teclas presionadas,
      bancl = 1;          // se limpia el Display.
      if (sel == 6){      // Si el menu esta en 4(dependiendo de los menus)(este caso 6)
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
  conres++;                    // contador de reset cada segundo aumenta 

  if (conres == 36000){        // Esta variable en segundos marca cuando el micro se resetea solo , por ahora lo dejare en 1 hora.
    banres = 1;
  }

  if (banenv == 1)           // bandera de envio se activa para mandar los datos de ambos sensores, defasados uno del otro.
    conenv++;

  if (conenv == 1){
    cad="";                               // Se envia la cadena de presion
    cad = "P";                            // cad es el string que se envia
    if (rcap < 10)                         // 
      dtostrf(rcap,4,2,con);
    else if (rcap >= 10 && rcap < 100)
      dtostrf(rcap,5,2,con);
    else 
      dtostrf(rcap,6,2,con);
    cad.concat(con);
    x_bee.print(cad);
  }

  if (conenv == 11){
    cad="";                     // Se envia la cadena de flujo
    cad = "F";
    if (flp < 10)
      dtostrf(flp,4,2,con);
    else if (flp >= 10 && flp < 100)
      dtostrf(flp,5,2,con);
    else 
      dtostrf(flp,6,2,con);
    cad.concat(con);
    x_bee.print(cad);
    conenv = 0;
    banenv = 0;
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

  if (now.hour() == 0 && now.minute() == 0 && now.second() == 0 && banacum == 0){     // Cadena real = now.hour() == 0 && now.minute() == 0 && now.second() == 0
    acumdia = EEPROMReadlong(10);
    acumfin = EEPROMReadlong(2);
    acumfin += acumdia;
    EEPROMWritelong(2,acumfin);
    acumdiaan = acumdia;
    EEPROMWritelong(6,acumdiaan);
    acumdia = 0;
    EEPROMWritelong(10,acumdia);
    nummed = EEPROMReadint(14);
    nummedda = nummed;
    nummed = 0;
    EEPROMWriteint(14,nummed);
    EEPROMWriteint(16,nummedda);
    banacum = 1;
  }

  if (now.hour() == 0 && now.minute() == 1 && now.second() == 0 && banacum == 1)
    banacum = 0;
  
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
      lcd.print("1.Press. 3.Tot A");
      lcd.setCursor ( 0, 1 );
      lcd.print("2.Flow   4.Tot D");
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
      lcd.print("Total ");
      if (EEPROMReadint(20) < 10)
        lcd.print("0");
      lcd.print(EEPROMReadint(20));
      lcd.print('/');
      if (EEPROMReadint(22) < 10)
        lcd.print("0");
      lcd.print(EEPROMReadint(22));
      lcd.print('/');
      lcd.print(EEPROMReadint(24));
      lcd.setCursor ( 0, 1 );
      lcd.print(EEPROMReadlong(2));        // Total de acumulado final
      lcd.print(" m3");   
      break;
    case 4:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Accum. yesterday:");
      lcd.setCursor ( 0, 1 );
      lcd.print(EEPROMReadlong(6));        // Total de acumulado final
      lcd.print(" m3/day");   
      break;       
    case 5:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Current value:");
      lcd.setCursor ( 0, 1 );
      lcd.print("Pres.: ");
      lcd.print(rcap);        // rcap     //Variable de presion
      lcd.print(" MCA");   
      break;
    case 6:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Current value:");
      lcd.setCursor ( 0, 1 );
      lcd.print("Flow: ");
      lcd.print(flp);      // flp     //Variable de flujo   //envio
      lcd.print(" LPS");
      break;           
    case 7:  
      lcd.setCursor ( 0, 0 );
      lcd.print("1.Flow"); 
      lcd.setCursor ( 0, 1 );
      lcd.print("2.Set clock");
      break;      
    case 8:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Pipe Diam: ");
      lcd.print(EEPROMReadfloat(26));
      lcd.setCursor ( 0, 1 );
      lcd.print("Change: 1.Y 2.N ");  
      break;    
    case 9:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Pipe Diam. (mm):");
      lcd.setCursor ( 0, 1 );
      lcd.print("Diam.-> ");  
      lcd.print(cadnum);    
      break;    
    case 10:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Data saved!");
      break;  
    case 11: 
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
    case 12:
      lcd.setCursor ( 0, 0 );
      lcd.print("Time (24 Hrs): ");
      lcd.setCursor ( 0, 1 );
      lcd.print(h);
      lcd.print(h1);
      lcd.print(':');
      lcd.print(mi);
      lcd.print(mi1);
      break;
    case 13:
      lcd.setCursor ( 0, 0 );
      lcd.print("NM: ");
      lcd.print(EEPROMReadint(16));
      lcd.setCursor ( 0, 1 );
      lcd.print("AD: ");
      lcd.print(EEPROMReadlong(6));
      break;
    case 14:
      lcd.setCursor ( 0, 0 );
      lcd.print("banacum: ");
      lcd.print(banacum);  
      break;
    case 15:
      lcd.setCursor ( 0, 0 );
      lcd.print("M: ");
      lcd.print(EEPROMReadint(14));
      lcd.setCursor ( 0, 1 );
      lcd.print("A: ");
      lcd.print(EEPROMReadlong(10));  
      break;       
  } 

  //Area de sensores 

  noInterrupts();

  //Sensor de nivel

  //Sensor de FLujo Amper

  valf = (5.000 * analogRead(0))/1024.000;
  pf += valf;
  b++;

  // Seonsor de Presion

  val = (5.000 * analogRead(1))/1024.000;
  p += val;
  i++;

  // Termina bloque de sensores 
  
  // Area de envio de datos y recepcion de Xbee
  if (x_bee.available() > 0 ){   // si es mayor a 0 se recibio un dato  
    a = x_bee.read();              
    if(a == 88 || a == 120){      // X en ascii
      banenv = 1;
    } 
    if(a == 67 || a == 99){    // C en ascii
      x_bee.print("CPF");     // Se envia la cadena de reconocimiento de sensores
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
    if (inmode == 1){                     // y no al soltarlo, si inmode esta en 1 quiere decir 
      switch (eKey){                      // que se ingresara un dato.
        case 'A': if (sel == 9)           // estamos en pantalla de ingreso estamos en pantalla de ingreso guarda valor del area de tuberia
                    sel = 10;              // guarda valor del area de tanque
                  bancl = 1;              // se limpia la pantalla
                  EEPROMWritefloat(26,cadnum.toFloat());
                  cadnum = "";
                  inmode = 0;             // al guadar los datos cierra el inmode (modo de ingreso)
                  break;              
        case 'B': cadnum = "" ; bancl = 1; break;     // Limpia el dato ingresado Borra
        case 'C': break;                  // NADA
        case 'D': sel = 0; bancl = 1; inmode = 0; cadnum = ""; break;     //cancela la operación
        case '#': break;                  // NADA
        case '*': cadnum += '.'; break;   // agrega el punto a la cadena.
        default: cadnum += eKey; break;   // agraga cada numero a la cadena cadnum
      }
    }
    else if (setclock == 1){
      switch (eKey){
        case 'A': if (sel == 11)
                    sel = 12;
                  else if (sel == 12){
                    sel = 10;
                    setclock = 0;
                    RTC.adjust(DateTime(ann,mes,dia,hora,minuto,00)); // Establece la fecha y hora
                    banfecha = EEPROMReadint(18);
                    if (banfecha == 0){
                      EEPROMWriteint(20,dia);
                      EEPROMWriteint(22,mes);
                      EEPROMWriteint(24,ann);
                      EEPROMWriteint(18,1);
                    }
                    restaura();
                    ord = 0;
                  }
                  bancl = 1;
                  break;              
        case 'B': bancl = 1; cadnum = ""; ord = 0; break;
        case 'C': break;
        case 'D': sel = 0; bancl = 1; setclock = 0; ord = 0; cadnum = ""; break;
        case '#': break;
        case '*': break;
        default: ord++;
                 if (sel == 11){
                    switch (ord){
                      case 1: cadnum += eKey; d = eKey; break;
                      case 2: cadnum += eKey; d1 = eKey; dia = cadnum.toInt(); cadnum = ""; break;
                      case 3: cadnum += eKey; m = eKey; break;
                      case 4: cadnum += eKey; m1 = eKey; mes = cadnum.toInt(); cadnum = ""; break;
                      case 5: cadnum = "20"; cadnum += eKey; an = eKey; break;
                      case 6: cadnum += eKey; an1 = eKey; ann = cadnum.toInt(); cadnum = ""; ord = 0; break;  
                    } 
                 }
                 else if (sel == 12){
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
            case '2': sel = 7; break;
            case 'C': sel = 2; break;
            case '*': sel = 13; break;
            case '#': sel = 14; break;
            case '0': sel = 15; break;
          }
          bancl = 1;
          break;    
        case 1:
          switch (eKey){
            case '1': sel = 5; break;
            case '2': sel = 6; break;
            case '3': sel = 3; break;
            case '4': sel = 4; break;
          }
          bancl = 1;
          break; 
        case 7:
          switch (eKey){
            case '1': sel = 8; break;
            case '2': sel = 11; setclock = 1; break;
          }
          bancl = 1; 
          break;
        case 8:
          switch (eKey){
            case '1': sel = 9; inmode = 1; break;
            case '2': sel = 0;   
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
