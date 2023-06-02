/**
 * cuentaChiles
 * Leemos el archivo contador.txt que contiene el número de archivos en la SD
 */
void cuentaChiles() {
  archivo = SD.open("contador.txt", FILE_WRITE);
  if(archivo) {
    archivo.seek(archivo.position()-1);
    N_archivos = ((int) archivo.read())-48;
    archivo.print(N_archivos+1);
    archivo.close();
  } else
    N_archivos = 0;    
}

/**
 * loraSend
 * Funcion para dar formato y enviar por medio del LoRa
 * @param data Arreglo con los datos a mandar 
 */
void loraSend(float data[]) {
  LoRa.beginPacket();
  for (int i = 0; i < NUM_VARS; i++) {
    LoRa.print(data[i]);
    LoRa.print(",");
  }
  LoRa.endPacket();
}

/**
 * serialShow
 * Funcion para mostrar por serial
 * @param data Arreglo con los datos a mostrar 
 */
void serialShow(float data[]) {
  Serial.print("<");
  for (int i = 0; i < NUM_VARS; i++) {
    Serial.print(data[i]);
    Serial.print(",");
  }
  Serial.println(">");
}

/**
 * sd
 * Funcion para escribir los datos a la SD
 * @param data Arreglo con los datos a escribir
 */
void sdWrite(float data[]){
  archivo = SD.open("D"+String(N_archivos)+".csv", FILE_WRITE);
  if (archivo) {
    for (int i = 0; i < NUM_VARS-1; i++) {
      archivo.print(data[i]);
      archivo.print(",");
    }
    archivo.println(data[NUM_VARS-1]);
    archivo.flush();
  }
  archivo.close();
}

/**
 * read_pressure
 * Funcion para leer la presion en las entradas analogicas
 */
void readPressure() {
  ADC_Presion_1 = analogRead(presionPin1);
  float voltage = (float) ADC_Presion_1 * (5.0/1023.0);
  presion1 =  mapeo(voltage,0.5,4.5,0,6.8948);

  ADC_Presion_2 = analogRead(presionPin2);
  voltage = (float) ADC_Presion_2 * (5.0/1023.0);
  presion2 =  mapeo(voltage,0.5,4.5,0,6.8948);
}

/**
 * readTempADC
 * Funcion para leer las lecturas de las entradas de temperatura
 * @param pin valor del pin a leer
 * @return ADC de la temper
 */
float readTempADC(byte pin) {
  float voltage = analogRead(pin) * (3.3 / 4096.0);
  float temperature = (voltage) * 100;
  return temperature;
}

/**
 * mapeo
 * Dado una voltaje de entrada y un rango, deducimos la presion
 * @param x Voltaje a mapear
 * @param in_main valor minimo de voltaje de entrada [V]
 * @param in_max valor maximo de voltaje de entrada  [V]
 * @param out_min valor minimo de presion de salida  [MPa]
 * @param out_max valor maximo de presion de salida  [MPa]
 * @return Valor de presion en MPa
 */
float mapeo(float X, float in_min, float in_max, float out_min, float out_max)
{
  return out_min + ((X - in_min)*(out_max-out_min)/(in_max-in_min));
}

/**
 * leerGalgas
 * Leemos los Hx de cada galga
 */
void leerGalgas() {
  // Lectura galga 1
  galgaADC1 = celda1.read();
  galgaKgf1 = A1 * galgaADC1 + B1;

  // Lectura galga 2
  galgaADC2 = celda2.read();
  galgaKgf2 = A2 * galgaADC2 + B2;

  // Lectura galga 3
  galgaADC1 = celda3.read();
  galgaKgf3 = A3 * galgaADC3 + B3;
}