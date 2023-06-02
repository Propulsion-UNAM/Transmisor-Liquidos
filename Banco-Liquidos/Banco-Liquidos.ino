/**
 * Banco de pruebas para motor liquido
 * Propulsion UNAM
 * 
 * @author Santiago Arroyo
 * @author Carlos Andrade
 * @author Rodrigo Liprandi
 * 
 * @date 02/06/2023
 * @version 1.0
 * 
 * https://aafi.space/coheteria
 * 
 */
#include <SD.h>
#include <LoRa.h>
#include <Q2HX711.h>
// #include <Wire.h>
// #include <Adafruit_MLX90614.h>

// [-----------> PINES <-----------]
// I2C
#define SCL_1        2
#define SDA_1        3
#define SCL_2        4
#define SDA_2        5
// Galgas
#define HX711_DOUT_1 7 
#define HX711_SCK_1  8 
#define HX711_DOUT_2 10
#define HX711_SCK_2  11
#define HX711_DOUT_3 13
#define HX711_SCK_3  14
// SPI
#define MISO         16
#define CS_LORA      17
#define MOSI         19
#define DIO0         20 
#define RTS          21 
#define SCK          24
#define CS_SD        29
// Presion ADC
#define presionPin2  26
#define presionPin1  27
// Temp ADC
#define ADC_LM35     28


// [-----------> CONSTANES <-----------]
//[Galga 40Kg]
#define A1 1.42185e-05
#define B1 -121.061  

//[Galga 1000Kg]
#define A2 0.000229151
#define B2 -1925.19

//[Galga Extra]
#define A3 0
#define B3 0

#define NUM_VARS 10

// [-----------> VARIABLES <-----------]
//[Variables Lecturas Galgas]
Q2HX711 celda1(HX711_DOUT_1,HX711_SCK_1); // Galga 1
unsigned long galgaADC1;
float galgaKgf1;

Q2HX711 celda2(HX711_DOUT_2,HX711_SCK_2); // Galga 2
unsigned long galgaADC2;
float galgaKgf2;

Q2HX711 celda3(HX711_DOUT_3,HX711_SCK_3); // Galga 3
unsigned long galgaADC3;
float galgaKgf3;

//Presiones
unsigned long ADC_Presion_1;
float presion1;

unsigned long ADC_Presion_2;
float presion2;

// Objetos MLX90614
/*
  Adafruit_MLX90614 mlx1;
  Adafruit_MLX90614 mlx2;
*/

// Variables
float trama[NUM_VARS];

// SD
File archivo;
byte N_archivos;
unsigned int N;

// [-----------> SETUP <-----------]
// [-----------> SETUP <-----------]
// [-----------> SETUP <-----------]
void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  // [--- TARJETA SD ---]
  if(!SD.begin(CS_SD))
    Serial.println("No funciona la SD!");       
  N = 0;

  // [--- CSV ---]
  cuentaChiles();
  archivo = SD.open("D"+String(N_archivos)+".csv", FILE_WRITE);//abrimos o creamos el archivo
  if(archivo) {
    archivo.print("t[s],galga1[ADC],galga2[ADC],presion1[ADC],");
    archivo.println("presion2[ADC],kgf1,kgf2,kPa1,kPa2");
    archivo.close();
  } else {
    Serial.println("Error al abrir el archivo");
  }

  // Configurar LoRa
  LoRa.setPins(CS_LORA, RTS, DIO0);
  if (!LoRa.begin(915E6)) { 
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Error al iniciar LoRa");
    // while (1);
  }
  // SF -> 7
  // CR -> 4
  LoRa.setSyncWord(0xF3);
}

// [-----------> LOOP <-----------]
// [-----------> LOOP <-----------]
// [-----------> LOOP <-----------]
void loop() {
  N++;
  /*
    // Leer los datos del sensor de temperatura ADC
    float temp1 = readTempADC(ADC_LM35);

    // Leer los datos de los LMX90614
    float temp2 = mlx1.readObjectTempC();
    float temp3 = mlx2.readObjectTempC();
  */

  leerGalgas();
  readPressure();

  // Guardamos los datos en la trama
  trama[0] = N;
  trama[1] = millis();
  trama[2] = galgaADC1;
  trama[3] = galgaADC2;
  trama[4] = ADC_Presion_1;
  trama[5] = ADC_Presion_2;
  trama[6] = galgaKgf1;
  trama[7] = galgaKgf2;
  trama[8] = presion1;
  trama[9] = presion2;
  // Largo de TRAMA debe ser siempre igual a NUM_VARS!!!!

  loraSend(trama);
  serialShow(trama);
  sdWrite(trama);
}
