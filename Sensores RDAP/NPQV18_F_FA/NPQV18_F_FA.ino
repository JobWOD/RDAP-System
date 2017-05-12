#include <Keypad.h>                           // IC Job Daniel Rodriguez Hernandez
#include <Wire.h>                             // Primera version Oficial del sistema RDAP alias NPQV18
#include <LiquidCrystal_I2C.h>                // Pruebas de laboratorio exitosas - mide presion y flujo 
#include <SoftwareSerial.h>                   // Fecha de actualizacion 5/Ene/2017 2:18 PM
#include <EEPROM.h>                           // Envio de datos Correcto
#include <avr/wdt.h>                          // Nombre del archivo NPQV18_P_FA

SoftwareSerial x_bee(2,3);                    // Creación del puerto serial de comunicacion con x_bee RX TX
//Variables control de flujo frecuencias.
int pin = 12,z,c,fin,flu;
double per,per2,per3,frec,frecs,frecf,flujo,RF;
//Variables control de flujo Amp.
double valf,res,pf,reff,fl,sfl,flp;           // valf = valor analogico 0, res = promedio de valf, pf = sumatoria
int b,redf;                                   // reff = ajuste de curvas, fl = valor de flijo, b = contador, redf = redondeo
//Variables control de nivel
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
double areaf, arean;                          // areaf = area del flujo, arean = area de nivel  
byte inmode;                                  // Variable que indica que hay datos de entrada.
String cadnum;                                // cadnum = cadena de numeros de valores guardados.
int canu;                                   // area de la tuberia guardada
double art,dm;
byte concam, bancl;                           // Variable de contador para mostrar la informacion en display
byte concon, banres;                          // Contado de control = concon     // bancl = bandera de clean
byte conres;                                  // Conres = contador de reset      banres = bandera de reset 
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
LiquidCrystal_I2C lcd(0x27,16,2);
 
void setup(){   
  // initialize timer1 
  valf = b = z = c = areaf = arean = inmode = concam = conmp = 0;
  bancl = concon = banres = conres = bancon = banpress = 0, conenv = 0, banenv = 0;
  sel = 2;
  wdt_disable();                            // Deshabilita el watch dog
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
  wdt_enable(WDTO_1S);
}

ISR(TIMER1_OVF_vect)        // Interrupt service routine, vector de interrupcion, se ejecuta al activar.
{
  TCNT1 = timer1_counter;   // Preload timer, es el tiempo precargado para que se cuente un segundo exacto.

  //Bloque de calculo de la variable de presion 

  //Bloque de calculo de la variable de flujo Amp

  res = pf/b;                               // Se calcula el promedio de lod valores 
  redf = res * 1000;                        // Se redondea para mejorar la manipulacion de los datos
  reff = 0.0022 * redf - 1.905;            // el ajuste de curvas hace referenca a un archivo excel
  canu = EEPROM.read(0);                    // se lee la memoria EEPROM segun el dato que guardaron
  dm = canu/10.0;                           // conversion de los milimetros
  art = (3.14159 * dm * dm)/4.0;            // calculo del area de la tuberia
  if (reff <= 0.37)                         // si la velocidad es muy poca muestra 0
    fl = 0;
  else
    fl = (reff * art)/10.0;                 // calculo final de flujo
  conmf ++;
  pf=0;                                     // se inicia todo en 0 para un mejor calculo posterior
  b=0;
  res=0;
  sfl += fl;
  if (conmf == 60){
    flp = sfl / conmf;                    
    sfl = 0;
    conmf = 0;  
  }

  //Bloque de calculo de la variable de flujo Frec.

  z++;          // codigo para extender el tiempo de los valores promedio 
  if (z == 60){
     z=0;
    frecf = frecs/c;
    fin = frecf;
    flujo = fin/9.9;
    flu = flujo*10;
    flujo = flu/10.0;
    RF = 0.5612 * flujo + 0.5582;
    if (flujo == 0)
      RF = 0;
    c=0;  
    frecs = 0;
  }

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

  if (conres == 3600){        // Esta variable en segundos marca cuando el micro se resetea solo , por ahora lo dejare en 1 hora.
    banres = 1;
  }

  if (banenv == 1)           // bandera de envio se activa para mandar los datos de ambos sensores, defasados uno del otro.
    conenv++;

  if (conenv == 1){
    cad="";                               // Se envia la cadena de flujo
    cad = "F";                            // cad es el string que se envia
    if (RF < 10)                         // 
      dtostrf(RF,4,2,con);
    else if (RF >= 10 && RF < 100)
      dtostrf(RF,5,2,con);
    else 
      dtostrf(RF,6,2,con);
    cad.concat(con);
    x_bee.print(cad);
  }

  if (conenv == 11){
    cad="";                     // Se envia la cadena de flujo
    cad = "F";
    if (fl < 10)
      dtostrf(fl,4,2,con);
    else if (fl >= 10 && flp < 100)
      dtostrf(fl,5,2,con);
    else 
      dtostrf(fl,6,2,con);
    cad.concat(con);
    x_bee.print(cad);
    conenv = 0;
    banenv = 0;
  }
  
}

void loop()
{      
  if (banres == 1){
    delay(1001);  
  }
  else
    wdt_reset();

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
      lcd.print("1.Flow F");
      lcd.setCursor ( 0, 1 );
      lcd.print("2.Flow");
      break;       
    case 2:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Current value:");
      lcd.setCursor ( 0, 1 );
      lcd.print("Flow F: ");
      lcd.print(RF);        // RF     //Variable de flujo frecuencias
      lcd.print(" LPS");   
      break;
    case 3:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Current value:");
      lcd.setCursor ( 0, 1 );
      lcd.print("Flow: ");
      lcd.print(flp);      // flp     //Variable de flujo   //envio
      lcd.print(" LPS");
      break;           
    case 4:  
      lcd.setCursor ( 0, 0 );
      lcd.print("1.Flow"); 
      break;      
    case 5:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Pipe area: ");
      canu = EEPROM.read(0);
      lcd.print(canu);
      lcd.setCursor ( 0, 1 );
      lcd.print("Change: 1.Y 2.N ");  
      break;    
    case 6:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Pipe area (mm):");
      lcd.setCursor ( 0, 1 );
      lcd.print("Area-> ");  
      lcd.print(cadnum);    
      break;    
    case 7:  
      lcd.setCursor ( 0, 0 );
      lcd.print("Data saved!");
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

  // Sensor de flujo frecuencias

  per = pulseIn(pin, HIGH);
  per2 = pulseIn(pin, LOW);
  per3 = per + per2;
  per3 = per3/1000;
  if (per3 == 0)
    frec = 0;
  else
    frec = (1/per3)*1000;
  frecs = frecs + frec;   //Suma de frecuancias para promedios en la medicion
  c++;

  // Termina bloque de sensores 
  
  // Area de envio de datos y recepcion de Xbee
  if (x_bee.available() > 0 ){   // si es mayor a 0 se recibio un dato  
    a = x_bee.read();              
    if(a == 88 || a == 120){      // X en ascii
      banenv = 1;
    } 
    if(a == 67 || a == 99){    // C en ascii
      x_bee.print("CFF");     // Se envia la cadena de reconocimiento de sensores
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
        case 'A': if (sel == 6)           // estamos en pantalla de ingreso estamos en pantalla de ingreso guarda valor del area de tuberia
                    sel = 7;              // guarda valor del area de tanque
                  bancl = 1;              // se limpia la pantalla
                  EEPROM.write(0,cadnum.toInt());
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
    else{                                 // Si no esta en modo de entrada
      switch(sel){                        // Este switch maneja la navegacion por todos los menus
        case 0:
          switch (eKey){
            case '1': sel = 1; break;
            case '2': sel = 4; break;
          }
          bancl = 1;
          break;    
        case 1:
          switch (eKey){
            case '1': sel = 2; break;
            case '2': sel = 3; break;
          }
          bancl = 1;
          break; 
        case 4:
          switch (eKey){
            case '1': sel = 5; break;
          }
          bancl = 1; 
          break;
        case 5:
          switch (eKey){
            case '1': sel = 6; inmode = 1; break;
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
