#include <SoftwareSerial.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal.h> 
#include <Stepper.h> 

//banderas y variables generales
char data;
int bandera_luz = 0;
int advertencia_luz = 0;
int bandera_disi = 0;
int bandera_extra = 0;
int bandera_venta = 0;
float valor_horno = 0;
float valor_luz = 0;
double stepsPerRevolution = 2048;


//Sensores
#define PIN_MQ6_GAS A2 //1
#define PIN_MQ2_HUMO A3 //2
#define PIN_DHT_HUME A4 //3
#define PIN_LM35_TEMP A5 //4
#define PIN_FLAMA A6 //5
#define PIN_PHOTOR A7 //6

//BLUETOOTH
#define PIN_RXD 2
#define PIN_TXD 3

//Actuadores
#define LED_LUZ_1 4 //1
#define LED_LUZ_2 5 //1
#define LED_LUZ_3 6 //1
#define LED_LUZ_4 7 //1

#define LED_HORNO_R 8 //1
#define LED_HORNO_G 9 //1
#define LED_HORNO_B 10 //1

#define PIN_BUZZER 12 //2

#define PIN_MOTOR_PASOS_1 22//3
#define PIN_MOTOR_PASOS_2 23//3
#define PIN_MOTOR_PASOS_3 24//3
#define PIN_MOTOR_PASOS_4 25//3
#define PIN_MOTOR_DC_1  26//4
#define PIN_MOTOR_DC_2  27//4

//Inicializar funcionamiento
LiquidCrystal lcd(32, 33, 34, 35, 36, 37, 38); //(RS, RW, E, D4,D5, D6, D7) //5
Stepper myStepper(stepsPerRevolution, PIN_MOTOR_PASOS_1, PIN_MOTOR_PASOS_2, PIN_MOTOR_PASOS_3, PIN_MOTOR_PASOS_4);  // Pin 
SoftwareSerial BT(PIN_TXD,PIN_RXD);
DHT dht(PIN_DHT_HUME, DHT11);

//Simuladores
#define PIN_BOTON 13 //luz
#define PIN_POTENCIOMETRO A8 //Horno


void setup() {
  pinMode(PIN_BOTON, INPUT);
  pinMode(PIN_POTENCIOMETRO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(LED_LUZ_1, OUTPUT);
  pinMode(LED_LUZ_2, OUTPUT);
  pinMode(LED_LUZ_3, OUTPUT);
  pinMode(LED_LUZ_4, OUTPUT);
  pinMode(LED_HORNO_R, OUTPUT);
  pinMode(LED_HORNO_G, OUTPUT);
  pinMode(LED_HORNO_B, OUTPUT);
  pinMode(PIN_MOTOR_PASOS_1, OUTPUT); 
  pinMode(PIN_MOTOR_PASOS_2, OUTPUT); 
  pinMode(PIN_MOTOR_PASOS_3, OUTPUT); 
  pinMode(PIN_MOTOR_PASOS_4, OUTPUT); 
  pinMode(PIN_MOTOR_DC_1, OUTPUT); 
  pinMode(PIN_MOTOR_DC_2, OUTPUT); 
  
  dht.begin();
  myStepper.setSpeed(10); 
  Serial.begin(9600);
  BT.begin(9600);

  Serial.println("\nCodigo Funcionando\n");
}


void loop() {
  
  //Control de luz
  if(bandera_luz == 1){
    encenderLuces();
  }else{
    apagarLuces();
  }

  //Control de ventanal
  if(bandera_venta == 1){
    myStepper.step(stepsPerRevolution); 
  }else{
    myStepper.step(-stepsPerRevolution); 
  }


  //Control de ventilacion
  if(bandera_disi == 1){
    prenderDisipador();
  }else if(bandera_extra == 1){
    prenderExtractor();
  }else{
    apagarVentilador();
  }


  //Control de funcionamiento del boton para la simulacion LUZ
  if(digitalRead(PIN_BOTON) == HIGH){
    if(bandera_luz == 1){
      bandera_luz = 0;
    }else{
      bandera_luz = 1;
    }
  }


  //Control de funcionamiento del boton para la simulacion de HORNO
  valor_horno = analogRead(PIN_POTENCIOMETRO)
  controlarHorno(valor_horno);


  //deteccion de humo
  float humo = analogRead(PIN_MQ2_HUMO);
  if(humo > 300 && humo < 400){
    sendAlert("¡Humo!", "Se ha detectado una gran cantidad de Humo, Se abrió la ventana.");
    bandera_venta = 1;
  }else if(humo > 400){
    activarDisipador();
    sendAlert("¡Humo!", "Se ha detectado una cantidad excesiva de Humo. Se abrió la ventana y se prendio el ventilador como disipador.");
    bandera_venta = 1;
  }
  

  //Deteccion de gas
  float gas = analogRead(PIN_MQ6_GAS);
  if(gas > 300 && gas < 400){
    sendAlert("¡Gas!", "Se ha detectado una gran cantidad de Gas, Se abrio la ventana.");
    bandera_venta = 1;
  }else if(gas > 400){
    activarExtractor();
    sendAlert("¡Gas!", "Se ha detectado una cantidad peligrosa de Gas, Se abrio la ventana y se prendio el ventilador como extractor.");
    bandera_venta = 1;
  }


  //Deteccion de Luminosidad
  valor_luz = analogRead(PIN_PHOTOR);
  if(digitalRead(LED_LUZ_1) == HIGH){
    if(advertencia_luz == 0){
      if(valor_luz < 15){
        sendAlert("¡Luz!","Se ha detectado que tienes la luz encendida en horas donde se obtiene mucha luz solar, se recomienda apagarlas.");
      }
    }
  }


  //Control Bluetooth
  if (BT.available()) {
    data = BT.read();
    Serial.write(data);
    switch(data){
      case 'x'://info general
        sendGeneralData();
        break;
      case 'v'://ventanal
        bandera_venta = 0;
        Serial.println("Se cerraron ventanas");
        break;
      case 'V'://ventanal
        bandera_venta = 1;
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
      case 'k'://ventilacion disipador
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
      case 'h'://ventilacion extractor
        
        Serial.println("efe");
        break;
      
      default:
        break;
   }
  } 
}
void controlarHorno(float valor_horno){
  if(valor_horno != 0){
    if(valor_horno <= 30){
      prenderFuegoAmarillo();
    }else if(valor_horno >30 && valor_horno <=40){
      prenderFuegoRojo();
    }else if(valor_horno >40){
      prenderFuegoAzul();
    } 
  }else{
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
void prenderDisipador(){
  digitalWrite(PIN_MOTOR_DC_1, HIGH);
  digitalWrite(PIN_MOTOR_DC_2, LOW);
}
void prenderExtractor(){
  digitalWrite(PIN_MOTOR_DC_1, LOW);
  digitalWrite(PIN_MOTOR_DC_2, HIGH);
}
void apagarVentilador(){
  digitalWrite(PIN_MOTOR_DC_1, LOW);
  digitalWrite(PIN_MOTOR_DC_2, LOW);
}
void sendGeneralData(){
  float h = dht.readHumidity();
  float s = analogRead(PIN_MQ2_HUMO);
  float g = analogRead(PIN_MQ6_GAS);
  float t = analogRead(pinLM35); 
  t = (5.0 * t * 100.0)/1024.0; 
  BT.print("data");
  BT.print(",");
  BT.print(h);
  BT.print(",");
  BT.print(t);
  BT.print(",");
  BT.print(s);
  BT.print(",");
  BT.print(g);
  BT.print(",");
  BT.print(bandera_luz);
  BT.print(",");
  BT.print(bandera_venta);
  BT.print(",");
  BT.print("1");
  BT.print(",");
  BT.print(valor_horno);
  BT.print(",");
  BT.print(bandera_disi);
  BT.print(",");
  BT.print(bandera_extra);
  BT.write(0xA);
  Serial.println("Datos generales enviados");
}

void sendError(error){
  BT.write("error");
  BT.print(",");
  BT.write(error);
  BT.write(0xA);
}

void sendAlert(String titulo, String texto){
  BT.write("alert");
  BT.print(",");
  BT.write(titulo);
  BT.print(",");
  BT.write(texto);
  BT.write(0xA);
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
  digitalWrite(LED_HORNO_R, HIGH);
  digitalWrite(LED_HORNO_G, HIGH);
  digitalWrite(LED_HORNO_B, LOW);
}
void prenderFuegoRojo(){
  digitalWrite(LED_HORNO_R, HIGH);
  digitalWrite(LED_HORNO_G, LOW);
  digitalWrite(LED_HORNO_B, LOW);
}
void prenderFuegoAzul(){
  digitalWrite(LED_HORNO_R, LOW);
  digitalWrite(LED_HORNO_G, LOW);
  digitalWrite(LED_HORNO_B, HIGH);
}
void apagarFuego(){
  digitalWrite(LED_HORNO_R, LOW);
  digitalWrite(LED_HORNO_G, LOW);
  digitalWrite(LED_HORNO_B, LOW);
}