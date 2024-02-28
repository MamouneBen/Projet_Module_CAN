#include <Arduino.h>
#include <Wire.h> // pour communication I2C
#include "MCP3X21.h"
#include <MCP342x.h>

unsigned long dernierTemps = 0;
const byte adresseCAN = 0x68; // Adresse du MCP3428
const double vRef = 2.048;   //2.048       // valeur de la référence de tension interne en Volts
const int printInfosSerie = 1;      // 1 pour imprimer les informations sur le port série, 0 pour ne pas afficher

// Prototypes des fonctions
double lire3428(int adresseCAN, int noCANAL, int printInfo);
double convertDbyte_Double(byte gauche, byte droit, int noBITS, int gain);
void printINFO(byte gauche, byte droit, int canal, int noBits, int gain);

void setup() {
  Wire.begin(8, 9); // Configure les broches SDA = 8 et SCL = 9 // Initialisation de la communication I2C
  Serial.begin(115200); // initialisation du port série
  dernierTemps = millis(); // initialisation du temps
}

void loop() {
  // mesurer et afficher les valeurs des canaux
  double valeurCanal1 = lire3428(adresseCAN, 0, printInfosSerie); // Mesure uniquement le canal CH1
  delay(1000); //1 seconde avant la prochaine mesure
}

double lire3428(int adresseCAN, int noCANAL, int printInfo) {
  byte configCH_CAN = 0;
  Wire.beginTransmission(adresseCAN);
  Wire.write(configCH_CAN);
  Wire.endTransmission();

  delay(1 + pow(2, 2 + 2 * ((configCH_CAN >> 2) & B00000011))); //insérer une pause avant de lire les données à partir du MCP3428

  Wire.requestFrom(adresseCAN, 3); //adresse du mcp3428, 3 octets de données
  byte octetGauche = Wire.read(); // Octet MSB, premier octet de donnés
  byte octetDroit = Wire.read();  // Octet LSB, deuxieme octet de donnés
  byte configCAN = Wire.read();   // paramètres de configuration (numéro de canal, le nombre de bits de conversion, le gain), troisieme octets de données

  int channelNUM = configCAN >> 5 & B00000011; // Numéro du canal analogique converti
  int channelBITS = 12 + 2 * ((configCAN >> 2) & B00000011); // Nombre de bits de conversion= 12 bits
  int channelGAIN = 1 << (configCAN & B00000011); //gain 

  if (printInfo) {
    printINFO(octetGauche, octetDroit, noCANAL, channelBITS, (channelGAIN));
  }

  return convertDbyte_Double(octetGauche, octetDroit, channelBITS, (channelGAIN));
}

// Brief: Cette fonction convertit un octet MSB (gauche) et un octet LSB (droit) en une valeur double représentant une tension en volts.

double convertDbyte_Double(byte gauche, byte droit, int noBITS, int gain) {

  long demiePlage = (1 << (noBITS)) / 2;// Calcul de la moitié de la plage de valeurs du convertisseur ADC en fonction du nombre de bits de conversion.

  if (noBITS == 16) {
    demiePlage = 32768; // Si le nombre de bits est égal à 16, alors la plage de valeurs est spécifiquement de 32768.
  }
  
  long t = gauche << 8 | droit;// Combinaison de l'octet MSB et de l'octet LSB en une valeur entière.
  
  if (t > demiePlage) t = ((1 << (noBITS)) * 10 + 1) - t; // Si la valeur est supérieure à la moitié de la plage, alors elle est convertie en une valeur négative.

  double v = (double)t * vRef / (double)demiePlage / (double)gain; // Conversion de la valeur entière en tension en volts en utilisant la référence de tension et le gain.
  
  return v; // Retourne la valeur convertie en volts.
}

// Brief: Cette fonction affiche les informations sur le canal ADC, y compris les bits MSB et LSB, le nombre de bits de conversion, et la valeur convertie en volts et en ADC.


void printINFO(byte gauche, byte droit, int canal, int noBits, int gain) {
  
  long demiePlage = (1 << (noBits)) / 2; // Calcul de la moitié de la plage de valeurs du convertisseur ADC en fonction du nombre de bits de conversion.
  
  if (noBits == 16) {
    demiePlage = 32768; // Si le nombre de bits est égal à 16, alors la plage de valeurs est spécifiquement de 32768.
  }

  // Affichage du numéro du canal.
  Serial.print("CH");
  Serial.print(canal + 1); //canal CH1
  Serial.print(" ");

  
  for (int cpt = 7; cpt >= 0; cpt-- ) {
    Serial.print(bitRead(gauche, cpt)); // Affichage des bits MSB.
  }
  Serial.print("-");

  
  for (int cpt = 7; cpt >= 0; cpt-- ) {
    Serial.print(bitRead(droit, cpt)); // Affichage des bits LSB.
  }

  Serial.println("");
  
  Serial.print("nombre de bits="); // Affichage du nombre de bits de conversion.
  Serial.println(noBits);
  
  /*Serial.print("demie plage ="); // Affichage de la moitié de la plage de valeurs du convertisseur ADC.
  Serial.println((double)demiePlage);*/

  double conversionVolt = convertDbyte_Double(gauche, droit, noBits, gain); // Conversion de la valeur brute en volts.
 
  Serial.print("mV = "); // Affichage de la valeur convertie en volts.
  Serial.println(conversionVolt * 1000);

  
  int valeurADC = static_cast<int>((conversionVolt / vRef) * 4096); // Conversion de la valeur brute en ADC (de 0 à 4096).
  
  Serial.print("Valeur ADC = "); // Affichage de la valeur ADC.
  Serial.println(valeurADC);
}




/*
#define MCP3428_ADDR 0x68 // MCP3428 I2C address
#define ESP32_ADDR 0x10   // ESP32 I2C address

void setup() {
  Wire.begin(8, 9); // SDA on pin 8, SCL on pin 9
  Serial.begin(115200);
}

void loop() {
  // Request conversion on Channel 1, 12-bit resolution
  Wire.beginTransmission(MCP3428_ADDR);
  Wire.write(0x10); // Configuration byte: 12-bit, Channel 1, Continuous Conversion Mode
  Wire.endTransmission();

  delay(100); // Wait for conversion to complete

  // Request 3 bytes of data from MCP3428
  Wire.requestFrom(MCP3428_ADDR, 3);
  while (Wire.available() < 3); // Wait until data is received

  // Read the received bytes
  byte msb = Wire.read();
  byte midb = Wire.read();
  byte lsb = Wire.read();

  // Combine bytes to form the 12-bit ADC value
  int adcValue = ((msb & 0x0F) << 8) | (midb << 4) | (lsb >> 4);

  // Send ADC value to ESP32 via I2C
  Wire.beginTransmission(ESP32_ADDR);
  Wire.write(adcValue >> 8); // Send MSB
  Wire.write(adcValue & 0xFF); // Send LSB
  Wire.endTransmission();

  Serial.print("ADC Value: ");
  Serial.println(adcValue);

  delay(1000); // Delay for readability
}*/


/*
const uint8_t adresseCAN = 0x68; // Adresse I2C du MCP3428
const uint8_t configRegistreCH1 = B10011000; // Configuration pour le canal 1, 12 bits de résolution
const double vRef = 5.0; // Valeur de la référence de tension en Volts (plage de 0 à 5 volts)
//(Vref) est la tension maximale que le convertisseur analogique-numérique (CAN) peut mesurer

// Déclaration de la fonction de conversion
double convertDbyte_Double(byte gauche, byte droit, int noBITS, int gain);

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // Utilisation des broches SDA (8) et SCL (9) sur ESP32-C3
}

void loop() {
  // Configuration du MCP3428 pour le canal 1
  Wire.beginTransmission(adresseCAN);
  Wire.write(configRegistreCH1);
  Wire.endTransmission();

  // Attente de la conversion (ajuster si nécessaire en fonction de la fréquence de conversion)
  delay(100);

  // Lecture des données
  Wire.requestFrom((uint8_t)adresseCAN, (uint8_t)3); // Utilisation des types de données corrects

  byte octetGauche = Wire.read();
  byte octetDroit = Wire.read();
  byte configCAN = Wire.read();

  // Extraction des paramètres de la mesure
  int channelBITS = 12 + 2 * ((configCAN >> 2) & B00000011);
  int channelGAIN = 1 << (configCAN & B00000011);

  // Conversion des octets en tension en volts
  double tension = convertDbyte_Double(octetGauche, octetDroit, channelBITS, channelGAIN);

  // Envoi de la tension vers le port série
  Serial.print("Tension sur CH1: ");
  Serial.print(tension);
  Serial.println(" V");

  delay(1000); // Attendre une seconde entre chaque mesure (ajuster selon vos besoins)
}

// Définition de la fonction de conversion
double convertDbyte_Double(byte gauche, byte droit, int noBITS, int gain) {
  long demiePlage = (1L << (noBITS - 1)) - 1;
  long t = (gauche << 8) | droit;
  if (t > demiePlage) t -= (1L << noBITS);
  double v = (double)t * vRef / (double)demiePlage / (double)gain;
  return v;
}*/


/*
#define Addr 0x68 // MCP3428 I2C address

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  // Configure MCP3428 for continuous conversion mode, Channel-1, 12-bit resolution, gain 1
  Wire.beginTransmission(Addr);
  Wire.write(0x10); // Configuration command byte
  Wire.endTransmission();
}

void loop() {
  // Start I2C Transmission to request data from MCP3428
  Wire.requestFrom(Addr, 3); // Request 3 bytes of data (2 bytes for ADC value, 1 byte for configuration)

  if (Wire.available() >= 3) { // Check if data is available
    byte data[3];
    for (int i = 0; i < 3; i++) {
      data[i] = Wire.read();
    }

    // Extract ADC value from the received data
    int raw_adc = ((data[0] & 0x0F) << 8) | data[1];

    // Convert raw ADC value to voltage (assuming a 3.3V reference voltage)
    float voltage = raw_adc * (3.3 / 4095.0); // 12-bit resolution and 3.3V reference

    // Print the ADC value and corresponding voltage to the serial monitor
    Serial.print("ADC Value: ");
    Serial.print(raw_adc);
    Serial.print(", Voltage: ");
    Serial.println(voltage, 4); // Print voltage with 4 decimal places
  }

  delay(1000); // Delay for 1 second before next reading
}
*/


/*
const uint8_t address = 0x68; // MCP3221 I2C adresse
const uint16_t ref_voltage = 5000; // 3.3V

MCP3221 mcp3221(address);

void setup() {
    Serial.begin(115200);
    Wire.begin(8, 9); // initialise I2C: SDA pin 8 // SCL pin 9
    mcp3221.init(&Wire);
}

void loop() {
    uint16_t result = mcp3221.read();

    Serial.print(F("ADC: "));
    Serial.print(result);
    Serial.print(F(", mV: "));
    Serial.println(mcp3221.toVoltage(result, ref_voltage));

    delay(1000);
}
*/