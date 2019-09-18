#include "driverADC2.h"
#include "Arduino.h"

volatile uint8_t canalActual = 0;
volatile uint8_t cantCanales = 0;
volatile uint8_t low, high;


struct data_canal{
  uint8_t canal;
  void (*func_callback)(int);
  int dataFromADC = 0;
};

struct data_canal *datos_canal[6];

uint8_t alreadyCall = 1;

bool reemplazarCanal(adc_cfg *cfg);

int adc_init(adc_cfg *cfg){
    int exito = 0;
    uint8_t canalAux = cfg->canal;
    if(canalAux >= 0 && canalAux <6){
        if(cfg->func_callback){
            if(!reemplazarCanal(cfg)){
              datos_canal[cantCanales] = (struct data_canal *) malloc(sizeof(struct data_canal));
              datos_canal[cantCanales]->canal = canalAux;
              datos_canal[cantCanales]->func_callback = cfg->func_callback;  
              if(cantCanales < 5){
                cantCanales += 1;
              }
            }
            exito = 1;
        }
    }
    if(alreadyCall && exito){    
        alreadyCall = 0; //Para no modificar los registros cada vez que se llama a adc_init().
        noInterrupts();
        ADMUX |= canalAux;
        //cantCanales += 1;
        
        //Tension de referencia por default = VCC
        ADMUX |= (1 << REFS0);
        ADMUX &= ~(1 << REFS1);
        
        //PRR – Power Reduction Register
        //Bit 0 – PRADC: Power Reduction ADC
        //Writing a logic one to this bit shuts down the ADC. The ADC must be disabled before shut down.
        PRR &= ~(1 << PRADC);
        
        //ADCSRA – ADC Control and Status Register A
        //Bit 7 – ADEN: ADC Enable
        //Writing this bit to one enables the ADC (this don't start de ADC). 
        ADCSRA |= (1 << ADEN);
        //Bits 2:0 – ADPS[2:0]: ADC Prescaler Select Bits
        //128 Preescaler
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
        //Bit 3 – ADIE: ADC Interrupt Enable
        ADCSRA |= (1 << ADIE);
    
        //Inicializar Conversion
        //Bit 6 – ADSC: ADC Start Conversion
        ADCSRA |= (1 << ADSC);
        interrupts();
    }
    return exito;
}

bool reemplazarCanal(adc_cfg *cfg){
  // Busca si ya existe un canal configurado con el mismo numero que cfg->canal y lo reemplaza. 
  // retorna true si lo encontro, false en caso contrario.
   int i = 0;
   uint8_t canal = cfg->canal;
   while(i < cantCanales){
    if(datos_canal[i]->canal == canal){
      datos_canal[i]->canal = canal;
      datos_canal[i]->func_callback = cfg->func_callback;
      return true;
    }
    i += 1;
   }
   return false;
}

uint8_t buscarPosCanal(uint8_t canal){
   int i = 0;
   while(i < cantCanales){
    if(datos_canal[i]->canal == canal){
      return i;
    }
    i += 1;
   }
   return 10;
}

void adc_loop(uint8_t canal){
  if(canal >= 0 && canal <6){
    int aux;
    uint8_t pos = buscarPosCanal(canal);
    if(pos != 10){
      //noInterrupts();
	  cli();
      aux = datos_canal[pos]->dataFromADC;
	  sei();
      //interrupts();
      datos_canal[pos]->func_callback(aux);
    }
  }
  /*
  switch(canal){
    case 0:
      if(hayConversionCanal_0){
        func_callback_canal_0(dataADC_0);
        hayConversionCanal_0 = 0;  
      }
      break;
    case 1:
      if(hayConversionCanal_1){
        func_callback_canal_1(dataADC_1);
        hayConversionCanal_1 = 0;  
      }
      break;
  }*/
  
}

ISR(ADC_vect){ //ADC conversion complete  
  low = ADCL;
  high = ADCH;

  datos_canal[canalActual]->dataFromADC = (high << 8) | low;
  canalActual = (canalActual + 1) % cantCanales;
  
  high = ADMUX & 0b11110000;
  ADMUX = high | datos_canal[canalActual]->canal;
  
  /*
  if(ADMUX & (1<<MUX0)){ //Canal 1
    dataADC_1 = (high << 8) | low;
    ADMUX &= ~(1<<MUX0);
  }
  else{ //Canal 0
    dataADC_0 = (high << 8) | low;
    ADMUX |= (1<<MUX0);
  }*/
  
  ADCSRA |= (1 << ADSC);
}



