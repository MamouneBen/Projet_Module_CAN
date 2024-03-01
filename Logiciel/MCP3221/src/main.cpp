#include <Arduino.h>
#include <Wire.h> // pour communication I2C
#include "MCP3X21.h"
#include <ESP32-TWAI-CAN.hpp>

//Constantes et variables globales
unsigned long dernierTemps = 0;
const byte adresseCAN = 0x4D; // Adresse du MCP3221A5T-E/OT 
const double vRef = 5.0;  // valeur de la référence de tension interne en Volts
const int printInfosSerie = 1;      // 1 pour imprimer les informations sur le port série, 0 pour ne pas afficher

//delcaration des fonctions
double lireMCP3221(int adresseMCP3221, int printInfo);
double convertDbyte_Double(byte msb, byte lsb);
void printINFO(byte msb, byte lsb);

void setup() {
  Wire.begin(8, 9); // Configure les broches SDA = 8 et SCL = 9 // Initialisation de la communication I2C
  Serial.begin(115200); // initialisation du port série
  dernierTemps = millis(); // initialisation du temps
}


double lireMCP3221(int adresseMCP3221, int printInfo) {
  Wire.beginTransmission(adresseMCP3221);
  Wire.endTransmission();

  delayMicroseconds(150); // Pause avant de lire les données du MCP3221

  Wire.requestFrom(adresseMCP3221, 2); // Lecture de 2 octets de données
  byte octetMSB = Wire.read();          // Octet MSB, premier octet de données
  byte octetLSB = Wire.read();          // Octet LSB, deuxième octet de données

  if (printInfo) {
    printINFO(octetMSB, octetLSB);
  }

  return convertDbyte_Double(octetMSB, octetLSB);
}

double convertDbyte_Double(byte msb, byte lsb) {
  int value = ((msb & 0x0F) << 8) | lsb; // Combinaison de l'octet MSB et de l'octet LSB en une valeur entière sur 12 bits.

  double v = (double)value * vRef / 4096.0; // Conversion de la valeur entière en tension en volts en utilisant la référence de tension et la résolution ADC.

  return v; // Retourne la valeur convertie en volts.
}

void printINFO(byte msb, byte lsb) {
  Serial.print("Valeur ADC (12 bits) = ");

  int valeurADC = ((msb & 0x0F) << 8) | lsb; // Conversion de l'octet MSB et de l'octet LSB en une valeur ADC sur 12 bits
  Serial.print(valeurADC);
  Serial.println();

  double tension_mV = convertDbyte_Double(msb, lsb) * 1000; // Conversion de la tension en mV
  Serial.print("Tension (mV) = ");
  Serial.println(tension_mV);
}


void loop() {
  double valeurCanal = lireMCP3221(adresseCAN, printInfosSerie); // mesurer et afficher les valeurs des canaux
  delay(1000); //1 seconde avant la prochaine mesure
}
