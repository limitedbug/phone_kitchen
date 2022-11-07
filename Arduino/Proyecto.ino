#include <SoftwareSerial.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTPIN A5
#define DHTTYPE DHT11
#define Humo A4

SoftwareSerial BT(3,2);
DHT dht(DHTPIN, DHTTYPE);
int pinBuzzer = 9;
char op;
int luces = 0;
int venti = 0;
int horno = 0;
int venta = 0;
void setup() {
  BT.begin(9600);
  pinMode(pinBuzzer, OUTPUT);
  Serial.begin(9600);
  Serial.println("\nCodigo Funcionando\n");
  dht.begin();
}
int i=0;
char str[] = {0};
void loop() {
  
  if(luces == 1){
    digitalWrite(pinBuzzer, HIGH);   // Ponemos en alto(5V) el pin del buzzer
    delay(1000);                     // Esperamos 1 segundo
    digitalWrite(pinBuzzer, LOW);
  }else{
      digitalWrite(pinBuzzer, LOW);
  }
  if (BT.available()) {
    while (BT.available()) {
      op = BT.read();
      Serial.write(op);
    }
    switch(op){
      case 'x'://info general
        sendGeneralData();
        break;
      case 'v'://ventanal
        venta = 0;
        Serial.println("Se cerraron ventanas");
        break;
      case 'V'://ventanal
        venta = 1;
        Serial.println("Se abrieron ventanas");
        break;
      case 'j'://iluminacion
        luces = 0;
        Serial.println("Se apagaron luces");
        break;
      case 'J'://iluminacion
        luces = 1;
        Serial.println("Se prendieron luces");
        break;
      default:
        break;
   }
  } 
}
void sendGeneralData(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float s = analogRead(Humo);
  BT.print("data");
  BT.print(",");
  BT.print(h);
  BT.print(",");
  BT.print(t);
  BT.print(",");
  BT.print(s);
  BT.print(",");
  BT.print(s);
  BT.print(",");
  BT.print(luces);
  BT.print(",");
  BT.print(venta);
  BT.print(",");
  BT.print(venti);
  BT.print(",");
  BT.print(horno);
  BT.write(0xA);
  Serial.println("datos enviados");
}

void luz(){
  
}
