#include <SPI.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include <axp20x.h>
#include "EspMQTTClient.h"

// Configuração Power chip
AXP20X_Class axp;

// Configuração MQTT
EspMQTTClient client(
  "SSID", // SSID Wifi
  "PASS", // Senha Wifi
  "IP",   // MQTT Broker IP
  "USER", // Usuário MQTT
  "PASS", // Senha MQTT
  "NAME", // Nome do dispositivo
  1883    // Porta MQTT
);

// Configuração Display
SSD1306 display(0x3c, 21, 22);

// Configuração Potenciômetro
const int analogIn = 36;
int sensor_value = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Habilita o debug do MQTT via porta Serial
  client.enableDebuggingMessages(); 
  
  // Configuração Power chip
  Wire.begin(21, 22);
  if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
    Serial.println("AXP192 Begin PASS");
  } else {
    Serial.println("AXP192 Begin FAIL");
  }
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
  
  // Configuração Display
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);
  delay(50); 
  digitalWrite(16, HIGH);
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
  delay(1500);
}


// Esse procedimento é executado toda vez que a conexão com o Broker MQTT é estabelecida
void onConnectionEstablished()
{
  // Se inscreve no topico (Interessande para fins de Debug)
  client.subscribe("latinoware/pot", [](const String & payload) {
    Serial.println(payload);
  });
}

void loop() {
  // Leitura do Potenciômetro com map de 0 a 100(Regra de 3).
  sensor_value = map(analogRead(analogIn), 0, 2040, 0, 100);

  // Gera info display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0,  "Valor lido: "+ String(sensor_value));
  display.drawString(0, 44, "V Bat: "+ String(axp.getBattVoltage()/1000) + " V");
  display.drawString(0, 54, "V Esp: "+ String(axp.getSysIPSOUTVoltage()/1000) + " V");
  display.display();

  // Publica via MQTT
  client.publish("latinoware/pot", String(sensor_value));
  client.publish("latinoware/vbat", String(axp.getBattVoltage()/1000));
  client.publish("latinoware/vesp", String(axp.getSysIPSOUTVoltage()/1000));
  client.loop();

  // Delay de 2 segundos
  delay(2000);
}
