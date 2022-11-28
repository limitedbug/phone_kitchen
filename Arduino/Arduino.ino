#include "SoftwareSerial.h"
#include "DHT.h"
#include "Adafruit_Sensor.h"
#include "LiquidCrystal.h"
#include "Stepper.h"
#include "GFButton.h"

//banderas y variables generales
char data;
int bandera_luz = 0;
int advertencia_luz = 0;
int bandera_disi = 0;
int bandera_extra = 0;
int bandera_venta = 0;
int bandera_venta_interna = 0;
float valor_horno = 0;
float valor_luz = 0;
double stepsPerRevolution = 1024;

//Sensores
#define PIN_MQ6_GAS A12 //1
#define PIN_MQ2_HUMO A14 //2
#define PIN_DHT_HUME A1 //3
#define PIN_LM35_TEMP A0 //4
#define PIN_FLAMA 53 //5
#define PIN_PHOTOR A0 //6

//BLUETOOTH
#define PIN_RXD 17
#define PIN_TXD 16

//Actuadores
#define LED_LUZ_1 27 //1
#define LED_LUZ_2 29 //1
#define LED_LUZ_3 31 //1
#define LED_LUZ_4 33 //1

#define LED_HORNO_R 10 //1
#define LED_HORNO_G 11 //1
#define LED_HORNO_B 12 //1

#define PIN_BUZZER 8 //2

#define PIN_MOTOR_PASOS_1 45//3
#define PIN_MOTOR_PASOS_2 47//3
#define PIN_MOTOR_PASOS_3 49//3
#define PIN_MOTOR_PASOS_4 51//3
#define PIN_MOTOR_DC_1  22//4
#define PIN_MOTOR_DC_2  23//4

// //Inicializar funcionamiento
LiquidCrystal lcd(3, 2, 6,7,4,5); //(RS, E, D4,D5, D6, D7) //5
Stepper myStepper(stepsPerRevolution, PIN_MOTOR_PASOS_1,PIN_MOTOR_PASOS_2,PIN_MOTOR_PASOS_3,PIN_MOTOR_PASOS_4);  // Pin 
// SoftwareSerial BT(PIN_RXD, PIN_TXD);
DHT dht(PIN_DHT_HUME, DHT11);


//Simuladores
GFButton PIN_BOTON(9); //luz
#define PIN_POTENCIOMETRO A3 //Horno


void setup() {
  pinMode(PIN_POTENCIOMETRO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(LED_LUZ_1, OUTPUT);
  pinMode(LED_LUZ_2, OUTPUT);
  pinMode(LED_LUZ_3, OUTPUT);
  pinMode(LED_LUZ_4, OUTPUT);
  pinMode(LED_HORNO_R, OUTPUT);
  pinMode(LED_HORNO_G, OUTPUT);
  pinMode(LED_HORNO_B, OUTPUT);
  pinMode(PIN_MOTOR_DC_1, OUTPUT); 
  pinMode(PIN_MOTOR_DC_2, OUTPUT); 
  pinMode(PIN_FLAMA, INPUT);
  pinMode(PIN_MQ2_HUMO, INPUT);
  pinMode(PIN_MQ6_GAS, INPUT);
  dht.begin();
  myStepper.setSpeed(10); 
  Serial.begin(9600);
  Serial1.begin(9600);
Serial.print("Iniciado");
}


void loop() {
  
  //Control de luz
  if(bandera_luz == 1){
    encenderLuces();
    
  }else{
    apagarLuces();
  }
  //Control de ventanal
  if(bandera_venta == 1 && bandera_venta_interna == 1){
    myStepper.step(stepsPerRevolution); 
    delay(500);
    bandera_venta_interna = 0;
    Serial.println("al reloj");
  }else if(bandera_venta == 0 && bandera_venta_interna == 2){
    myStepper.step(-1024);
    delay(500);
    bandera_venta_interna = 0; 
    Serial.println("contrareloj");
  }


  //Control de ventilacion
  if(bandera_disi == 1){
    digitalWrite(PIN_MOTOR_DC_1, HIGH);
    digitalWrite(PIN_MOTOR_DC_2, LOW);
  }else if(bandera_extra == 1){
    digitalWrite(PIN_MOTOR_DC_1, LOW);
    digitalWrite(PIN_MOTOR_DC_2, HIGH);
  }else{
    digitalWrite(PIN_MOTOR_DC_1, LOW);
    digitalWrite(PIN_MOTOR_DC_2, LOW);
  }


  //Control de funcionamiento del boton para la simulacion LUZ
  // Serial.println(PIN_BOTON.isPressed());
  if(PIN_BOTON.wasPressed()){
    if(bandera_luz == 1){
      bandera_luz = 0;
    }else{
      bandera_luz = 1;
    }
  }


  //Control de funcionamiento del boton para la simulacion de HORNO
  int potencia = map(analogRead(PIN_POTENCIOMETRO), 0, 1023, 0, 100);
  
  if(potencia > 4){
    valor_horno = potencia;
  }
  
  // Serial.println(valor_horno);
  controlarHorno(valor_horno);



  
  if (digitalRead(PIN_FLAMA) == LOW)  //if it is low
  {
    sendAlert("¡Fuego!", "Se ha detectado un incendio dentro del edificio.");
    tone(PIN_BUZZER, 440);
  }
  
  
  //deteccion de humo
  float humo = analogRead(PIN_MQ2_HUMO);
  // Serial.println(humo);
  if(humo > 300 && humo < 400){
    sendAlert("¡Humo!", "Se ha detectado una gran cantidad de Humo, Se abrió la ventana.");
    if(bandera_venta == 0 && bandera_venta_interna == 0){
       bandera_venta = 1;
       bandera_venta_interna = 1;    
     }
  }else if(humo > 400){
    activarDisipador();
    sendAlert("¡Humo!", "Se ha detectado una cantidad excesiva de Humo. Se abrió la ventana y se prendio el ventilador como disipador.");
    if(bandera_venta == 0 && bandera_venta_interna == 0){
       bandera_venta = 1;
       bandera_venta_interna = 1;    
     }
  }
  

  //Deteccion de gas
  float gas = analogRead(PIN_MQ6_GAS);
  // Serial.println(gas);
  if(gas > 360 && gas < 440){
    sendAlert("¡Gas!", "Se ha detectado una gran cantidad de Gas, Se abrio la ventana.");
    if(bandera_venta == 0 && bandera_venta_interna == 0){
       bandera_venta = 1;
       bandera_venta_interna = 1;    
     } 
    tone(PIN_BUZZER, 440);
  }else if(gas > 440){
    // activarExtractor();
    sendAlert("¡Gas!", "Se ha detectado una cantidad peligrosa de Gas, Se abrio la ventana y se prendio el ventilador como extractor.");
    if(bandera_venta_interna == 2 && bandera_venta == 0){
      bandera_venta = 1;
      bandera_venta_interna = 1;
    }
    tone(PIN_BUZZER, 440);
  }


  //Deteccion de Luminosidad
  // valor_luz = analogRead(PIN_PHOTOR);
  //   Serial.println(valor_luz);
  // if(digitalRead(LED_LUZ_1) == HIGH){
  //   if(advertencia_luz == 0){
  //     if(valor_luz < 15){
  //       sendAlert("¡Luz!","Se ha detectado que tienes la luz encendida en horas donde se obtiene mucha luz solar, se recomienda apagarlas.");
  //     }
  //   }
  // }

  //Deteccion de humedad
  float h = dht.readHumidity();
  if(h > 30 && h <= 50){
    sendAlert("¡Humedad!", "Se ha detectado una cantidad moderada de humedad dentro de la habitacion.");
  }else if(h > 80){
    sendAlert("¡Humedad!", "Se ha detectado una gran cantidad de humedad dentro de la habitacion.");
  if(bandera_venta == 0 && bandera_venta_interna == 0){
     bandera_venta = 1;
     bandera_venta_interna = 1;    
   }
  }

  noTone(PIN_BUZZER);
  //Control Bluetooth
  if (Serial1.available() > 0) {
    data = Serial1.read();
    Serial.write(data);
    switch(data){
      case 'x'://info general
        sendGeneralData();
        break;
      case 'v'://ventanal
        bandera_venta = 0;
        bandera_venta_interna = 2;
        Serial.println("Se cerraron ventanas");
        break;
      case 'V'://ventanal
        bandera_venta = 1;
        bandera_venta_interna = 1;
        Serial.println("Se abrieron ventanas");
        break;
      case 'j'://iluminacion
        bandera_luz = 0;
        Serial.println("Se apagaron luces");
        break;
      case 'J'://iluminacion
        bandera_luz = 1;
        Serial.println("Se prendieron luces");
        break;
      case 'k'://apagado de ventilador
        bandera_disi = 0;
        bandera_extra = 0;
        Serial.println("Se apago el ventilador");
        break;
      case 'K'://ventilacion disipador
        activarDisipador();
        Serial.println("Se prendio el ventilador en modo disipador");
        break;
      case 'H'://ventilacion extractor
        activarExtractor();
        Serial.println("Se prendio el ventilador en modo extractor");
        break;
      case 'h'://prender horno en amarillo
        valor_horno = 30;
        Serial.println("Se prendio el horno con llama baja");
        break;
      case 'w'://prender horno en rojo
        valor_horno = 50;
        Serial.println("Se prendio el horno con llama media");
        break;
      case 'z'://prender horno en azul
        valor_horno = 70;
        Serial.println("Se prendio el horno con llama alta");
        break;
      case 'Z'://apagar horno 
        valor_horno = 0;
        Serial.println("Se apago el horno ");
        break;
      default:
        break;
   }
  } 
}
void controlarHorno(float valor_horno){  
  if(valor_horno >= 10){
    if(valor_horno <= 30){
      prenderFuegoAmarillo();
    }else if(valor_horno >30 && valor_horno <=60){
      prenderFuegoRojo();
    }else if(valor_horno >60){
      prenderFuegoAzul();
    } 
  }
  else{
    apagarFuego();
  }
}
void activarDisipador(){
  bandera_disi = 1;
  bandera_extra = 0;
}
void activarExtractor(){
  bandera_extra = 1;
  bandera_disi = 0;
}


void sendGeneralData(){
  float h = dht.readHumidity();
  float s = analogRead(PIN_MQ2_HUMO);
  float g = analogRead(PIN_MQ6_GAS);
  float t = 1;//analogRead(PIN_LM35_TEMP); 
  t = (5.0 * t * 100.0)/1024.0; 
  Serial1.print("data");
  Serial1.print(",");
  Serial1.print(h);
  Serial1.print(",");
  Serial1.print(t);
  Serial1.print(",");
  Serial1.print(s);
  Serial1.print(",");
  Serial1.print(g);
  Serial1.print(",");
  Serial1.print(bandera_luz);
  Serial1.print(",");
  Serial1.print(bandera_venta);
  Serial1.print(",");
  Serial1.print(valor_horno);
  Serial1.print(",");
  Serial1.print(bandera_disi);
  Serial1.print(",");
  Serial1.print(bandera_extra);
  Serial1.write(0xA);
  Serial.println("Datos generales enviados");
}

void sendAlert(String titulo, String texto){
  Serial1.print("alert");
  Serial1.print(",");
  Serial1.print(titulo);
  Serial1.print(",");
  Serial1.print(texto);
  Serial1.write(0xA);
  Serial.println("Alerta generada y enviada");
}

void encenderLuces(){
  digitalWrite(LED_LUZ_1, HIGH);
  digitalWrite(LED_LUZ_2, HIGH);
  digitalWrite(LED_LUZ_3, HIGH);
  digitalWrite(LED_LUZ_4, HIGH);
}
void apagarLuces(){
  digitalWrite(LED_LUZ_1, LOW);
  digitalWrite(LED_LUZ_2, LOW);
  digitalWrite(LED_LUZ_3, LOW);
  digitalWrite(LED_LUZ_4, LOW);
}

void prenderFuegoAmarillo(){
  digitalWrite(LED_HORNO_R, LOW);
  digitalWrite(LED_HORNO_G, LOW);
  digitalWrite(LED_HORNO_B, HIGH);
  lcd.setCursor(0, 0);         // Coloca el cursor en las coordenadas (0,0)   
  lcd.print("Horno: Encendido"); // Escribe no LCD   
  lcd.setCursor(0, 1);         // Coloca el cursor en las coordenadas (3,1) 
  lcd.print("Temp: 30 C");      // Escreve no LCD 
}
void prenderFuegoRojo(){
  digitalWrite(LED_HORNO_R, HIGH);
  digitalWrite(LED_HORNO_G, LOW);
  digitalWrite(LED_HORNO_B, LOW);
  lcd.setCursor(0, 0);         // Coloca el cursor en las coordenadas (0,0)   
  lcd.print("Horno: Encendido"); // Escribe no LCD   
  lcd.setCursor(0, 1);         // Coloca el cursor en las coordenadas (3,1) 
  lcd.print("Temp: 50 C");      // Escreve no LCD 
}
void prenderFuegoAzul(){
  digitalWrite(LED_HORNO_R, HIGH);
  digitalWrite(LED_HORNO_G, HIGH);
  digitalWrite(LED_HORNO_B, LOW);
  lcd.setCursor(0, 0);         // Coloca el cursor en las coordenadas (0,0)   
  lcd.print("Horno: Encendido"); // Escribe no LCD   
  lcd.setCursor(0, 1);         // Coloca el cursor e}n las coordenadas (3,1) 
  lcd.print("Temp: 70 C");      // Escreve no LCD 
}
void apagarFuego(){
  digitalWrite(LED_HORNO_R, LOW);
  digitalWrite(LED_HORNO_G, LOW);
  digitalWrite(LED_HORNO_B, LOW);
  // lcd.setCursor(0, 0);         // Coloca el cursor en las coordenadas (0,0)   
  // lcd.print("Horno: Apagado"); // Escribe no LCD 
}
