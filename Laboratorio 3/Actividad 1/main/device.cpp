#include "Arduino.h"
#include "device.h"
#include "stdio.h"
#include "fnqueue.h"
#include "driverADC.h"


volatile int adcValue;
int canal;

//Handler key down functions
void (*handler_key_down_tecla_up)();
void (*handler_key_down_tecla_down)();
void (*handler_key_down_tecla_right)();
void (*handler_key_down_tecla_left)();
void (*handler_key_down_tecla_select)();
//Handler key up functions
void (*handler_key_up_tecla_up)();
void (*handler_key_up_tecla_down)();
void (*handler_key_up_tecla_right)();
void (*handler_key_up_tecla_left)();
void (*handler_key_up_tecla_select)();

//Used to convert adcValue to key number
int adc_key_val[5] ={30, 180, 360, 535, 760};
int NUM_KEYS = 5;
static int deboucing=0;
// Variables auxiliares
int lastKeyDown = -1; //variable utilizada para recordar la ultima tecla presionada
int teclaPresionada = -1;

//Encabezado de funciones

void key_down_function(int teclaPresionada);
void key_up_function(int lastKeyDown);
void procesarAdc(int adcValue); // func callback for driverADC
int get_key(int adcValue);

/*
int adc_init(void){
    int exito = 0;
    canal = 0;
    int tensionRef =1;
   
        ADMUX |= canal;
     
        ADMUX |= (tensionRef << 6);
           
        //PRR – Power Reduction Register
        //Bit 0 – PRADC: Power Reduction ADC
        //Writing a logic one to this bit shuts down the ADC. The ADC must be disabled before shut down.
        PRR &= ~(1 << PRADC);
        
        //ADCSRA – ADC Control and Status Register A
        //Bit 7 – ADEN: ADC Enable
        //Por defecto el adc esta apagado para consumir menos,entonces lo prende poniendo el bit a 1.  
        ADCSRA |= (1 << ADEN);
        //Configura el prescaler en 128 para que trabaje con la frecuencia maxima del ADC que es de 125khz
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
        //Bit 3 – ADIE:habilita las intrerrupciones del adc.
        ADCSRA |= (1 << ADIE);
        //Bit 5 – ADATE: ADC Auto Trigger Enable para que en modo free running comience la conversión.
        ADCSRA |= (1 << ADATE);

        
        ADCSRB &= ~(1 << ADTS2) & ~(1 << ADTS1) & ~(1 << ADTS0);

        //Inicializar Conversion
        //Bit 6 – ADSC: ADC Start Conversion
        ADCSRA |= (1 << ADSC);
		
		exito = 1;
    
    return exito;
}
*/

void teclado_init(void){    	
	  adc_cfg cfg_A0;
    cfg_A0.canal = 0;    
	  cfg_A0.func_callback = procesarAdc;
    adc_init(&cfg_A0);    
}



// Convert ADC value to key number
int get_key(int input){
    int k;
    for (k = 0; k < NUM_KEYS; k++)
        if (input < adc_key_val[k])
            return k;

    if (k >= NUM_KEYS)
        k = -1;     // No valid key pressed

    return k;
}


void key_down_function(int teclaPresionada){
    switch(teclaPresionada){
    case TECLA_UP:
      handler_key_down_tecla_up();
      break;
    case TECLA_DOWN:
      handler_key_down_tecla_down();
      break;
    case TECLA_LEFT:
      handler_key_down_tecla_left();
      break;
    case TECLA_RIGHT:
      handler_key_down_tecla_right();
      break;
    case TECLA_SELECT:
      handler_key_down_tecla_select();
      break;

  }
}

void key_up_function(int lastKeyDown){
  switch(lastKeyDown){
    case TECLA_UP:
      handler_key_up_tecla_up();
      break;
    case TECLA_DOWN:
      handler_key_up_tecla_down();
      break;
    case TECLA_LEFT:
      handler_key_up_tecla_left();
      break;
    case TECLA_RIGHT:
      handler_key_up_tecla_right();
      break;
    case TECLA_SELECT:
      handler_key_up_tecla_select();
      break;
  }
}

void procesarAdc(int adcValue){
  //Funcion que determina que tecla fue pulsada.


      int key = get_key(adcValue);
      //Serial.println("adcValue: "+String(adcValue));
      if (key != -1){
        key_down_function(key);
        lastKeyDown = key;
      }
      else{
        if(lastKeyDown != -1){
          key_up_function(lastKeyDown);
          lastKeyDown=-1;
        }
      }
      
 
}

/*
ISR(ADC_vect){ //ADC conversion complete
  deboucing++;
  if(deboucing==100)
  {
    uint8_t low, high;
    low = ADCL;
    high = ADCH;
    adcValue = (high << 8) | low;
  }
  if(deboucing==200){
    fnqueue_add(ProcesarAdc);
  }
  if(deboucing == 300){
    deboucing=0;
  }
}
*/

void key_down_callback(void (*handler)(), int tecla){
  switch(tecla){
    case TECLA_UP:
      handler_key_down_tecla_up = handler;
      break;
    case TECLA_DOWN:
      handler_key_down_tecla_down = handler;
      break;
    case TECLA_LEFT:
      handler_key_down_tecla_left = handler;
      break;
    case TECLA_RIGHT:
      handler_key_down_tecla_right = handler;
      break;
    case TECLA_SELECT:
      handler_key_down_tecla_select = handler;
      break;
  
  }
}

void key_up_callback(void (*handler)(), int tecla){
  switch(tecla){
    case TECLA_UP:
      handler_key_up_tecla_up = handler;
      break;
    case TECLA_DOWN:
      handler_key_up_tecla_down = handler;
      break;
    case TECLA_LEFT:
      handler_key_up_tecla_left = handler;
      break;
    case TECLA_RIGHT:
      handler_key_up_tecla_right = handler;
      break;
    case TECLA_SELECT:
      handler_key_up_tecla_select = handler;
      break;

  }
}
