#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <SD.h>
#include <LoRa.h>
#include <Streaming.h>
#include <Q2HX711.h>

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
#define CS_SD   29

// Lecturas Galgas
Q2HX711 celda1(DOUT_1,SCK_1); // Galga 1
long lectura_ADC1;
float lectura_kgf1;
float letura_N1;
float a1;
float b1;

Q2HX711 celda2(DOUT_2,SCK_2); // Galga 2
long lectura_ADC2;
float lectura_kgf2;
float letura_N2;
float a2;
float b2;

Q2HX711 celda3(DOUT_1,SCK_1); // Galga 3
long lectura_ADC3;
float lectura_kgf3;
float letura_N3;
float a3;
float b3;

// Objetos MLX90614
Adafruit_MLX90614 mlx1;
Adafruit_MLX90614 mlx2;

// Variables
float trama[9];

// Archivo sd
File datos;

void setup() {
  Serial.begin(9600);

  // Calibracion de las galgas
  //40 kg
  a1 = 1.42185e-05;
  b1 = -121.061;
  //Aquí pongan la calibracion de la de 1000
  a2 = 0.000229151;
  b2 = -1925.19;

  a3 = 0.000229151;
  b3 = -1925.19;

  // Configurar Sensores de temperatura infrarojos MLX90614
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

  // Creamos el archivo sd
  if (!SD.begin(CS_SD)) {
    Serial.println("Error al inicializar la tarjeta SD");
    return;
  }
  // Creamos el encabezado
  datos = SD.open("/datos.txt", FILE_WRITE);
  if (datos)
    datos.println("tiempo,peso1,peso2,peso3,presion1,presion2,temp1,temp2,temp3");
  datos.close();

}

void loop() {
  // Lectura galga 1
  lectura_ADC1 = celda1.read();
  lectura_kgf1 = a1 * lectura_ADC1 + b1 ;

  // Lectura galga 2
  lectura_ADC2 = celda2.read();
  lectura_kgf2 = a2 * lectura_ADC2 + b2 ;

  // Lectura galga 3
  lectura_ADC3 = celda3.read();
  lectura_kgf3 = a3 * lectura_ADC3 + b3 ;

  // Leer los datos del sensor de temperatura ADC
  float temp1 = read_temperature_ADC(ADC_LM35);

  // Leer los datos de los LMX90614
  float temp2 = mlx1.readObjectTempC();
  float temp3 = mlx2.readObjectTempC();

  // Leer los datos de los sensores de presión ADC
  float pressure1 = read_pressure_ADC(ADC_Presion_1);
  float pressure2 = read_pressure_ADC(ADC_Presion_2);

  // Guardamos los datos en la trama
  trama[0] = micros()/1000;
  trama[1] = lectura_kgf1;
  trama[2] = lectura_kgf2;
  trama[3] = lectura_kgf3;
  trama[4] = pressure1;
  trama[5] = pressure2;
  trama[6] = temp1;
  trama[7] = temp2;
  trama[8] = temp3;

  // Enviamos por telemetria Lora la Trama
  lora_send(trama);

  // Guardamos los datos en una sd
  datos = SD.open("/data.txt", FILE_WRITE);
  if (datos) {
    datos.print(trama[0]);
    datos.print(",");
    datos.print(trama[1]);
    datos.print(",");
    datos.print(trama[2]);
    datos.print(",");
    datos.print(trama[3]);
    datos.print(",");
    datos.print(trama[4]);
    datos.print(",");
    datos.print(trama[5]);
    datos.print(",");
    datos.print(trama[6]);
    datos.print(",");
    datos.print(trama[7]);
    datos.print(",");
    datos.println(trama[8]);
    datos.flush();
  }
  datos.close();
}

// Funcion para dar formato y enviar por medio del LoRa
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

// Funcion para leer la presion en las entradas analogicas
float read_pressure_ADC(byte pin) {
  float voltage = analogRead(pin) * (5.0 / 4096.0);
  float value = map(voltage, 0.5, 4.5, 0, 1600);
  return value;
}

// Funcion para leer las lecturas de las entradas de temperatura
float read_temperature_ADC(byte pin) {
  float voltage = analogRead(pin) * (3.3 / 4096.0);
  float temperature = (voltage) * 100;
  return temperature;
}
