#include <LiquidCrystal.h>
#include "device.h"
#include "fnqueue.h"
#include "driverADC.h"

#define Vcc 5.0f
#define Vref 5.0f
#define pinSensorTemp 1 

//Inicializa la library con los numeros de interfaces de los pines.
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int brightness = 204; //brillo del display LCD
volatile int dimmer; //cuenta tiempo transcurrido ante inactividad o al presionar select
volatile int pressingSelect; //indica si se esta presionando la tecla select
volatile int positionCounter=0;// scroll del mensaje inicial

//Identifican las celdas del lcd
const int numRows = 2;
const int numCols = 16;

//Posiciones para imprimir cronometro
volatile int posCronX = 4;
volatile int posCronY = 1;

int ms,s,m,cs, cs_aux; //representan milisegundos, segundos, minutos, centesimas de segundo
volatile int tiempos[10][3]; //tiempos guardados
volatile int savepos; //posición donde se almacena el ultimo tiempo a guardar
volatile int visorpos; //posición del tiempo visualizado en MVT
const int analogOutPin = 10; // Analog output pin that the LED brightness is attached to

int estadoActual = 0; //de 0 a 4, el 0 es para mostrar el mensaje inicial, el 1 MCA, 2 MP, 3 MVT, 4 MAD.
volatile int timerON = 0; //estado del timer
int cont=0;

//presionar tecla select
void select_key_down()
{
  pressingSelect=1;      
}

//presionar tecla left
void left_key_down(){}
//presionar tecla right
void right_key_down(){}
//presionar tecla up
void up_key_down(){}
//presionar tecla down
void down_key_down(){}

//soltar tecla down
void down_key_up(){
switch(estadoActual){
  case 0:
    estadoActual = 2;
    cleanDisplay();
    mostrarEstado();
    break;
  case 1:
    tiempos[savepos][0]=m;
    tiempos[savepos][1]=s;
    tiempos[savepos][2]=cs;
    savepos=(savepos+1)%10;
    break;  
  case 2:
    tiempos[savepos][0]=m;
    tiempos[savepos][1]=s;
    tiempos[savepos][2]=cs;
    savepos=(savepos+1)%10;
    ms=0;
    cs=0;
    m=0;
    s=0; 
    break;
  case 3:
    visorpos=visorpos-1;
    if(visorpos<0)
       visorpos=9;
    break;
  case 4:
    dimmer=0;
    if (brightness>0)
       brightness -= 51;
    delay(100);
    analogWrite(10, brightness);
    break;
}
}

//soltar tecla up
void up_key_up(){
switch(estadoActual){
  case 0:
    estadoActual = 2;
    cleanDisplay();
    break;
  case 1:
    estadoActual = 2;
    break;
  case 2:
    estadoActual = 1;
    break;
  case 3:
    visorpos=(visorpos+1)%10;  
    break;
  case 4:
    dimmer=0;
    if (brightness<255)
        brightness += 51;
        delay(100);
    analogWrite(10, brightness);
    break;   
}
  mostrarEstado();
}

//soltar tecla left
void left_key_up(){}
//soltar tecla right
void right_key_up(){}

//soltar tecla select
void select_key_up()
{

 pressingSelect=0;
 
if (estadoActual == 0)
{
  estadoActual = 2;
  cleanDisplay();
}
else  
  if(estadoActual == 2){
    if (dimmer<=2)
    { 
      estadoActual=3;
      dimmer=0;
    }
    else
    {
      estadoActual = 4;  
      dimmer=0;
    }    
  }
  else
  {
    if (estadoActual==3)
    {
      estadoActual = 2;
      cleanDisplay();
    }
    else
    {
      if (estadoActual==4)
      {
        estadoActual = 2;
        resetTimer();
      }
    }      
  }
  mostrarEstado();
}

//limpia el display LCD
void cleanDisplay()
{ 
  lcd.clear();
}

void setup() {
  //Setup timer2
  cli();
  //set timer2 interrupt at 100Hz (Interrupciones cada 0,01s)
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 100Hz increments (0,01s)
  OCR2A = 155.25;// = (16*10^6) / (1024*100) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM01);  
  // Set CS21 bit for 1024 prescaler
  TCCR2B |= (1 << CS20) | (1 << CS21)| (1 << CS22);  
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
 sei();

  // Setup LCD
  pinMode(analogOutPin, OUTPUT);
  lcd.begin(numCols,numRows);
  analogWrite(analogOutPin, brightness); //Controla intensidad backlight  
  
  //Setup driverADC
  adc_cfg cfg_sensorTemp;
  cfg_sensorTemp.canal = pinSensorTemp;
  cfg_sensorTemp.func_callback = adcToTemp;
  adc_init(&cfg_sensorTemp);

  //Inicialización driverTeclado
  teclado_init();
  //Inicialización de cola de funciones
  fnqueue_init();

  //Inicialización de contadores de tiempo
  cs_aux=0;
  ms=0;
  cs=0;
  m=0;
  s=0;
  dimmer=0;
 
  //inicialización de flags  
  pressingSelect=0;
  savepos=0;
  visorpos=0;

  //Inicialización del arreglo que mantiene tiempos guardados
  int i,j;
  for(i=0;i<10;i++)
  {
    for(j=0;j<3;j++)
    tiempos[i][j]=0;
  } 

  //Asociación de funciones al callback
  key_down_callback(select_key_down, TECLA_SELECT);
  key_down_callback(left_key_down, TECLA_LEFT);
  key_down_callback(right_key_down, TECLA_RIGHT);
  key_down_callback(up_key_down, TECLA_UP);
  key_down_callback(down_key_down, TECLA_DOWN);
  
  key_up_callback(select_key_up, TECLA_SELECT);
  key_up_callback(left_key_up, TECLA_LEFT);
  key_up_callback(right_key_up, TECLA_RIGHT);
  key_up_callback(up_key_up, TECLA_UP);
  key_up_callback(down_key_up, TECLA_DOWN);

  imprimirInicio();
    
	Serial.begin(9600);
}


void adcToTemp(int adc_value){
  float Vin = adcToVin(adc_value);
  Serial.println(adc_value);
  valorTemp = VinToTemp(Vin);
  
  agregarMedicion(valorTemp);
  
  if(valorTemp < minTemp){
    minTemp = valorTemp;
  }
  if(valorTemp > maxTemp){
    maxTemp = valorTemp;
  }
  
  // Print the results to the Serial Monitor:
  Serial.println("sensor = " + String(adc_value));
  Serial.println("digital = " + String(Vin));
  Serial.println("temp = " + String(valorTemp));
}

float adcToVin(float adcValue){
  return (adcValue*Vref)/1024.0f;
}


int VinToTemp(float valor){
  int out = 0;
  firulito = true;
  //We transform the map of LRD values to Lux into a Vin (analog) to Lux map.
  if(valor > 4.54f){
    firulito = false;
    return 100;
  }
  else if(valor > 3.84f){
    out = 100;
  }else if(valor > 3.3f){
    out = 80;
  }else if(valor > 2.94f){
    out = 35;
  }else if(valor > 2.5f){
    out = 15;
  }else if(valor > 1.923f){
    out = 10;
  }else if(valor > 1.47f){
    out = 6;
  }else if(valor > 0.98f){
    out = 3;
  }else if(valor > 0.49f){
    out = 1;
  }else{
    out = 1;
    firulito = false;
  }
  return out;
}

void agregarMedicion(int tempValue){
  mediciones[posLibre] = tempValue;
  posLibre += 1;
  if(posLibre == 200){
    posLibre = 0;
  }
  if(cantMediciones < 200){
    cantMediciones += 1;
  }
  
}

float obtenerPromedio(){
  float suma = 0.0f;
  for(int i=0; i < cantMediciones; i++){
    suma += mediciones[i];
  }
  if (cantMediciones == 0){
    return 0;
  }
  else{
    return suma/(float)cantMediciones;
  }
}


//Reinicia el timer
void resetTimer(){
  cs = 0;
  s = 0;
  cli();
  TIMSK2 &= ~(1 << OCIE2A); //Disable timer compare interrupt
  TCNT2  = 0;//initialize counter value to 0
  TIMSK2 |= (1 << OCIE2A); //Enable timer compare interrupt
  sei();
  timerON = 1;
}

//Imprime en display los números del cronómetro
void imprimirNumero(int num,int x) {
  if (num <= 9) {
    lcd.setCursor(x, posCronY);
    lcd.print("0");
    lcd.setCursor(x+1, posCronY);
    lcd.print(num);
  } else {
    lcd.setCursor(x, posCronY);
    lcd.print(num);
  }
}

//Imprime en display el cronómetro con sus unidades de tiempo
void imprimirCronometro() {   
  imprimirNumero(m,posCronX);
  lcd.setCursor(posCronX+2, posCronY);
  lcd.print(":");
  imprimirNumero(s,posCronX+3);
  lcd.setCursor(posCronX+5, posCronY);
  lcd.print(":");
  imprimirNumero(cs,posCronX+6);
}

//Imprime en display el tiempo guardado en la posición i
void imprimirTiempoGuardado(int i)
{
  lcd.print(i);
  imprimirNumero(tiempos[i][0],posCronX);
  lcd.setCursor(posCronX+2, posCronY);
  lcd.print(":");
  imprimirNumero(tiempos[i][1],posCronX+3);
  lcd.setCursor(posCronX+5, posCronY);
  lcd.print(":");
  imprimirNumero(tiempos[i][2],posCronX+6);
  
}


//Imprime en display el mensaje inicial
void imprimirInicio(void)
{
  lcd.setCursor(0, 0);
  lcd.print("Sistemas Embebidos-2do Cuatrimestre 2019");
  lcd.setCursor(0, 1);
  lcd.print("Laboratorio 2 - Com: Fraysse / Carignano");
  
  if (positionCounter <24)
  {    
      // scroll one position left:
      lcd.scrollDisplayLeft();     
      positionCounter++;
      // wait a bit:
      delay(200);   
  }else if(estadoActual==0){
    if (positionCounter<48 && estadoActual==0){
      lcd.scrollDisplayRight();
      positionCounter++;
      delay(200);
    }
  }
  
}

//Imprime en display información del estado actual del sistema
void mostrarEstado(){
  switch(estadoActual){
    case 1: // MCA      
      lcd.setCursor(0,0);      
      lcd.print("   Ascendente   ");
      timerON = 0;
      break;
    case 2: // MP
      lcd.setCursor(0,0);      
      lcd.print("     Pausa      ");      
      break;
    case 3: // MVT
      lcd.setCursor(0,0);      
      lcd.print(" Visor Tiempos  ");      
      break;
    case 4: // MAD
      lcd.setCursor(0,0);      
      lcd.print(" Ajuste dimmer  ");      
      break;
  }
}

//Imprime en display información sobre el brillo del mismo
void imprimirBrillo(){  
  lcd.setCursor(0, 1);
  //7 espacios en blanco
  lcd.print("       ");
  //1, 2 o 3
  lcd.print(brightness*100/255);
  //Si son 3 cifras
  if(brightness == 255){
    lcd.setCursor(10, 1);
    //6 caracteres mas
    lcd.print("%     ");
  }
   else//son 2 cifras
     if(brightness*100/255 > 9){
       lcd.setCursor(9, 1);
       lcd.print("%     ");
     }
     else{//es de 1 cifra
       lcd.setCursor(8, 1);
       lcd.print("%     ");
     }
}

//Funcion llamada por la rutina de interrupciones.
void procesarTimer(){
  if(timerON){
        cs += 1;
        if(cs >= 100){
          cs = 0;
          s += 1;
    
          if(estadoActual == 4){
            dimmer++;
        }
    
        if (s >=60)
        {
          s=0;
          m += 1;       
        }
      }
  }
  else{
        if (pressingSelect ==1 )
        {
            cs_aux+=1;    
            if (cs_aux>=100)
            {
              cs_aux=0;
              dimmer++;
            }          
        }      
      }   
}

//Rutina del timer2
ISR(TIMER2_COMPA_vect){
  fnqueue_add(modoActual);
  fnqueue_add(procesarTimer);
}

void modoActual(){

  // Modo mensaje inicial
  if (estadoActual == 0)
  {
      //Imprime en display el mensaje inicial  
      imprimirInicio();      
  }
   // Modo conteo ascendente
  else if(estadoActual == 1){    
    lcd.setCursor(0,1);
    timerON=1;
    imprimirCronometro();    
  }
  // Modo pausa
  else if (estadoActual == 2){
    lcd.setCursor(0,1);
    timerON=0;
    imprimirCronometro();
  }
  //Modo visor de tiempos
  else if(estadoActual == 3){
    lcd.setCursor(0,1);
    imprimirTiempoGuardado(visorpos);
  }
  //Modo ajuste dimmer
  else if (estadoActual == 4){
    lcd.setCursor(0,1);
    imprimirBrillo();
    timerON=1;

  //si no presiona tecla durante mas de 5 segundos vuelve a modo pausa
    if(dimmer>=5)
         {
           estadoActual = 2;           
           dimmer=0;
           resetTimer();
           mostrarEstado();
         }
  }
  
}

//  principal
void loop() {
   fnqueue_run();


}
