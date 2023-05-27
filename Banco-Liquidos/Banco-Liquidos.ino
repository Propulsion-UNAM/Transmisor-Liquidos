#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <HX711.h>
#include <SD.h>
#include <LoRa.h>
#include <Streaming.h>

// Configuración de los pines de presión
#define ADC_Presion_1 27
#define ADC_Presion_2 26

//Configuración del pin de temperatura ADC
#define ADC_LM35 28

//Configuración de los pines de temperatura IC2
//CONFIGURADOS EN pins_arduino.h
#define SCL_1 2
#define SDA_1 3
#define SCL_2 4
#define SDA_2 5

//Configuración de los pines de los HX711
#define DOUT_1 7
#define SCK_1 8
#define DOUT_2 10
#define SCK_2 11
#define DOUT_3 13
#define SCK_3 14

#define NUM_GALGAS 3

//Configuración de los pines de LoRa
#define MISO    16
#define MOSI    19
#define SCK     24
#define RTS     21
#define DIO0    20
#define CS_LORA 17

// Definir los pines de conexión de los HX711
const int DOUT_PINS[] = {DOUT_1, DOUT_2, DOUT_3};  // Pines de datos de las galgas
const int SCK_PINS[] = {SCK_1, SCK_2, SCK_3};   // Pines de reloj de las galgas

// Crear un arreglo de objetos HX711 para las galgas
HX711 scales[NUM_GALGAS];

// Objetos MLX90614
Adafruit_MLX90614 mlx1;
Adafruit_MLX90614 mlx2;

// Variables
float trama[8];

void setup() {
  Serial.begin(9600);

  //TODO: Poner los pinModes

  //TODO: Controles para ver si se detectan los sensores
  
  // Inicializar los módulos HX711
  for (int i = 0; i < NUM_GALGAS; i++) {
    scales[i] = HX711();
    scales[i].begin(DOUT_PINS[i], SCK_PINS[i]);

    //TODO: CALIBRACIÓN
    
    //scales[i].set_scale(1000);  // Factor de escala para cada galga
    //scales[i].tare();
    
  }

  // Configurar MLX90614

  Wire.begin();
  mlx1.begin(0x5A,&Wire);  // Inicializar el primer sensor MLX90614 -> (default addr, pointer to wire)

  Wire1.begin();
  mlx2.begin(0x5A,&Wire1);  // Inicializar el segundo sensor MLX90614 -> (default addr, pointer to wire)

  // Configurar LoRa

  LoRa.setPins(CS_LORA, RTS, DIO0);
  if (!LoRa.begin(433E6)) { //AJUSTAR FRECUENCIA
    Serial.println("Error al iniciar LoRa");
    while (1);
  }
  
}

void loop() {

  // Leer los datos de las galgas conectadas

  float weights[NUM_GALGAS];

  int connectedScales = 0;
  for (int i = 0; i < NUM_GALGAS; i++) {
    if (scales[i].is_ready()) {
      connectedScales++;
      weights[i] = scales[i].get_units();
    } else {
      weights[i] = 0.0;
    }
  }

  // Leer los datos de los sensores de presión ADC
  float pressure1 = read_pressure_ADC(ADC_Presion_1);
  float pressure2 = read_pressure_ADC(ADC_Presion_2);

  // Leer los datos del sensor de temperatura ADC
  float temp1 = read_temperature_ADC(ADC_LM35);

  // Leer los datos de los LMX90614
  float temp2 = mlx1.readObjectTempC();
  float temp3 = mlx2.readObjectTempC();

  trama[0] = weights[0];
  trama[1] = weights[1];
  trama[2] = weights[2];
  trama[3] = pressure1;
  trama[4] = pressure2;
  trama[5] = temp1;
  trama[6] = temp2;
  trama[7] = temp3;

  lora_send(trama);

  delay(1000); // Esperar 1 segundo antes de la siguiente lectura
}

void lora_send(float data[]) {
  LoRa.beginPacket();
  LoRa.print("<");
  for (int i = 0; i < sizeof(data); i++) {
    if (data[i] != NULL) {
      if (i == sizeof(data) -1) {
        LoRa.print(data[i]);
      }
      LoRa.print(data[i]);
      LoRa.print(",");
    } else {
      LoRa.print(0.0);
    }
  }
  LoRa.print(">");
  LoRa.endPacket();
}

float read_pressure_ADC(byte pin) {
  float voltage = analogRead(pin) * (5.0 / 4096.0);
  float value = map(voltage, 0.5, 4.5, 0, 1600);
  return value;
}

float read_temperature_ADC(byte pin) {
  float voltage = analogRead(pin) * (3.3 / 4096.0);
  float temperature = (voltage) * 100;
  return temperature;
}

long mapFloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//TODO: SD
