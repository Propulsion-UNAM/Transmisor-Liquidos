#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <HX711.h>
#include <SD.h>

// Configuración de los pines de presión
#define ADC_Presion_1 27
#define ADC_Presion_2 26

//Configuración del pin de temperatura ADC
#define ADC_LM35 28

//Configuración de los pines de temperatura IC2
#define SCL_1 5
#define SDA_1 4
#define SCL_2 7
#define SDA_2 6

//Configuración de los pines de los HX711
#define DOUT_1 7
#define SCK_1 8
#define DOUT_2 10
#define SCK_2 11
#define DOUT_3 13
#define SCK_3 14

#define NUM_GALGAS 3

// Definir los pines de conexión de los HX711
const int DOUT_PINS[] = {DOUT_1, DOUT_2, DOUT_3};  // Pines de datos de las galgas
const int SCK_PINS[] = {SCK_1, SCK_2, SCK_3};   // Pines de reloj de las galgas

// Crear un arreglo de objetos HX711 para las galgas
HX711 scales[NUM_GALGAS];

// Objetos MLX90614
Adafruit_MLX90614 mlx1;
Adafruit_MLX90614 mlx2;

void setup() {
  Serial.begin(9600);
  
  // Inicializar los módulos HX711
  for (int i = 0; i < NUM_GALGAS; i++) {
    scales[i].begin(DOUT_PINS[i], SCK_PINS[i]);

    //CALIBRACIÓN
    //scales[i].set_scale(1000);  // Factor de escala para cada galga
    //scales[i].tare();
  }
  
  // Inicializar el ADC

  // Configurar MLX90614

  Wire.begin();
  Wire.setSDA(SDA_1);  // Pin SDA para el primer bus
  Wire.setSCL(SCL_1);  // Pin SCL para el primer bus
  mlx1.begin();  // Inicializar el primer sensor MLX90614

  Wire1.begin();
  Wire1.setSDA(SDA_2);  // Pin SDA para el segundo bus
  Wire1.setSCL(SCL_2);  // Pin SCL para el segundo bus
  mlx2.begin();  // Inicializar el segundo sensor MLX90614
  
}

void loop() {

  // Leer los datos de las galgas conectadas

  float weights[NUM_GALGAS];

  int connectedScales = 0;
  for (int i = 0; i < NUM_GALGAS; i++) {
    if (scales[i].is_ready()) {
      connectedScales++;
      float weight = scales[i].get_units();
      
    }
  }

  // Leer los datos de los sensores de presión
  float pressure1 = read_pressure_ADC(ADC_Presion_1);
  float pressure2 = read_pressure_ADC(ADC_Presion_2);

  // Leer los datos del sensor de temperatura ADC
  float temp = read_temperature_ADC(ADC_LM35);

  Serial.print("Galga(s) detectada(s): ");
  Serial.println(connectedScales);

  delay(1000); // Esperar 1 segundo antes de la siguiente lectura
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
