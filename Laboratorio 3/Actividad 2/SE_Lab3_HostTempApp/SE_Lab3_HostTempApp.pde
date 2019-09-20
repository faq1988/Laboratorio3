
/**
 * Sistemas Embebidos - 1º Cuatrimestre de 2012
 *
 * Este skecth implementa un programa en el host que tiene la capacidad de
 * graficar funciones que evolucionan en el tiempo y además permite la
 * presentación de datos simples mediante Labels, y la captura de eventos
 * del usuario mediante la implementación de botones simples.
 *
 * La aplicación divide la ventana en 2 regiones, una de dibujado y otra 
 * donde se ubican los botones y etiquetas de información.
 *
 * Autor: Sebastián Escarza
 * Modificaciones introducidas por: Lorena Assef - Nahuel Sanabria
 */

import processing.serial.*;

// El puerto serie...
Serial myPort;  

// Declaraciones para graficar funciones...
int cosVal;
int cantValues;
ScrollingFcnPlot fAct, fMax, fMin, fProm; // Se define una funcion para cada temperatura

//Botones...
RectButton rectBtnMax, rectBtnMin, rectBtnAct, rectBtnProm;
PFont myFont;

//Etiquetas textuales...
boolean alert = false;
Label lbl1, lbl2, lbl3, lbl4, lbl5, lbl6, lbl7, lbl8, lbl9, lbl10, lbl11;
Label yVal1,yVal2,yVal3;

//Ventana y viewports...
int pgFunctionViewportWidth = 480;
int pgControlViewportWidth = 300;
int pgViewportsHeight = 240;

//variables auxiliares que contendran las temperaturas...
int actual = 0;
int maxima = 0;
int minima = 0;
int promedio = 0;
int pedido = 0;  //se utilizara para manejar los pedidos desde la placa arduino (cuando se presione un pulsador)
byte boton = 1;

void setup() {

  //Se define el tamaño de la ventana de la aplicación... 
  size(pgFunctionViewportWidth+pgControlViewportWidth, pgViewportsHeight);
  
  //Se inicializan los arreglos para almacenar las funciones...
  cantValues = pgFunctionViewportWidth;
  fAct = new ScrollingFcnPlot(cantValues,color(0,100,0),0,height);
  fMax = new ScrollingFcnPlot(cantValues,color(100,0,0),0,height);
  fMin = new ScrollingFcnPlot(cantValues,color(0,0,100),0,height);
  fProm = new ScrollingFcnPlot(cantValues,color(50,50,0),0,height);
  
  //Se inicializan los botones...
  rectBtnMax = new RectButton(pgFunctionViewportWidth+10,10,70,40,color(102),color(50),color(255),"Maxima");
  rectBtnMin = new RectButton(pgFunctionViewportWidth+10,60,70,40,color(102),color(50),color(255),"Minima");
  rectBtnAct = new RectButton(pgFunctionViewportWidth+90,10,70,40,color(102),color(50),color(255),"Actual");
  rectBtnProm = new RectButton(pgFunctionViewportWidth+90,60,70,40,color(102),color(50),color(255),"Promedio");
  
  //Se inicializan los labels...
  lbl1 = new Label(pgFunctionViewportWidth+10,110,color(255),"Status: ");
  lbl2 = new Label(pgFunctionViewportWidth+60,110,color(255),"-");
  lbl3 = new Label(pgFunctionViewportWidth+10,130,color(255),"T.Actual: ");
  lbl4 = new Label(pgFunctionViewportWidth+70,130,color(255),"-");			//4: ACTUAL
  lbl5 = new Label(pgFunctionViewportWidth+10,150,color(255),"T.Max: ");
  lbl6 = new Label(pgFunctionViewportWidth+70,150,color(255),"-");			//6: MAXIMA
  lbl7 = new Label(pgFunctionViewportWidth+10,170,color(255),"T.Min: ");
  lbl8 = new Label(pgFunctionViewportWidth+70,170,color(255),"-");			//8: MINIMA
  lbl9 = new Label(pgFunctionViewportWidth+10,190,color(255),"T.Prom: ");
  lbl10 = new Label(pgFunctionViewportWidth+70,190,color(255),"-");			//10: PROMEDIO
  lbl11 = new Label(pgFunctionViewportWidth+10,210,color(255,0,0),"ALERTA!!!");
  
  yVal1 = new Label(10,5,color(255),"150");
  yVal2 = new Label(10,(pgViewportsHeight-20)/2,color(255),"75");
  yVal3 = new Label(10,pgViewportsHeight-25,color(255),"0");
  
  //Inicializa el font de la GUI...
  myFont = createFont("FFScala", 14);
  textFont(myFont);
  println(Serial.list());
   myPort = new Serial(this, Serial.list()[1], 9600); // 0:COM1  1:COM4
}

void draw() {

  
  //Se actualizan las funciones...
  fAct.updateValue(actual);
  fMax.updateValue(maxima);
  fMin.updateValue(minima);
  fProm.updateValue(promedio);
     
  //Rendering de la interface...
  background(125);
  stroke(0);
  noFill();
 
  //Dibuja las funciones...
  fAct.displayIntoRect(30,10,pgFunctionViewportWidth-10,pgViewportsHeight-10);
  fMax.displayIntoRect(30,10,pgFunctionViewportWidth-10,pgViewportsHeight-10);
  fMin.displayIntoRect(30,10,pgFunctionViewportWidth-10,pgViewportsHeight-10);
  fProm.displayIntoRect(30,10,pgFunctionViewportWidth-10,pgViewportsHeight-10);
  
  //Procesa eventos de MouseOver...
  rectBtnMax.update();
  rectBtnMin.update();
  rectBtnAct.update();
  rectBtnProm.update();  

  //Si hay algo para leer, y esto es la temperatura actual, se lee y se muestra en la interfaz
  if ((myPort.available() > 0) && (myPort.read()==200)) {    
    if (myPort.available() > 0){
        actual = myPort.read();
        lbl4.caption = actual + " °C";
      }
  }

//Si hay algo para leer, y esto es la temperatura maxima, se lee y se muestra en la interfaz
if ((myPort.available() > 0) && (myPort.read()==201)) {  
  if (myPort.available() > 0){
        maxima = myPort.read();
        lbl6.caption = maxima + " °C";
      }
}

//Si hay algo para leer, y esto es la temperatura minima, se lee y se muestra en la interfaz
if ((myPort.available() > 0) && (myPort.read()==202)) {  
  if (myPort.available() > 0){
        minima = myPort.read();
        lbl8.caption = minima + " °C";
    }
}

//Si hay algo para leer, y esto es la temperatura promedio, se lee y se muestra en la interfaz
if ((myPort.available() > 0) && (myPort.read()==203)) {  
  if (myPort.available() > 0){
        promedio = myPort.read();
        lbl10.caption = promedio + " °C";
     }
}

//Si hay algo para leer, y esto es el modo actual de la placa, se lee y se muestra en la interfaz
if ((myPort.available() > 0) && (myPort.read()==204)) {  
  if (myPort.available() > 0){
    pedido = myPort.read();
    if(pedido == 0)
        lbl2.caption = "Maxima";
    if(pedido == 2)
        lbl2.caption = "Minima";
    if(pedido == 3)
        lbl2.caption = "Actual";
    if(pedido == 4)
        lbl2.caption = "Promedio";
  }  
}
  
  //si la temperatura actual es mayor a los 99 °C, se indica estado de alerta
  if(actual > 99)
    alert = true;
  else alert = false;
  
  
  //Procesa las entradas (botones)
  if(mousePressed) {
    if(rectBtnMax.pressed()) {
      rectBtnMax.currentcolor = color(0,100,0);
      boton = 0; //indico que quiero la temperatura maxima       
    } 
    else if(rectBtnMin.pressed()) {
      rectBtnMin.currentcolor = color(0,100,0);
      boton = 2; //indico que quiero la temperatura minima      
    }
     else if(rectBtnAct.pressed()) {
      rectBtnAct.currentcolor = color(0,100,0);
      boton = 3; //indico que quiero la temperatura actual     
    }
     else if(rectBtnProm.pressed()) {
      rectBtnProm.currentcolor = color(0,100,0);
      boton = 4; //indico que quiero la temperatura promedio      
    }
    myPort.write(boton);
  }  
  
  //Dibuja el eje X y el recuadro de los gráficos...
  stroke(0);
  line(30, pgViewportsHeight/2, pgFunctionViewportWidth-10, pgViewportsHeight/2);
  rect(30,10,pgFunctionViewportWidth-40,pgViewportsHeight-20);
  
  //Se dibujan los botones...
  rectBtnMax.display();
  rectBtnMin.display();
  rectBtnAct.display();
  rectBtnProm.display();
  
  //Se dibuja texto adicional...(labels, etc)
  lbl1.display();
  lbl2.display();
  lbl3.display();
  lbl4.display();
  lbl5.display();
  lbl6.display();
  lbl7.display();
  lbl8.display();
  lbl9.display();
  lbl10.display();

  if (alert) lbl11.display();
  
  yVal1.display();
  yVal2.display();
  yVal3.display();  
}

void mouseReleased()
{}
