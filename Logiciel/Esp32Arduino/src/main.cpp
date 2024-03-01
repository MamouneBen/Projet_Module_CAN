//Brief: Ce code est conçu pour un microcontrôleur esp32-c3-devkitc-02, utilisant le langage ARDUINO et les bibliothèques Wire pour la communication I2C et ESP32-TWAI-CAN pour la 
//communication CAN. L'objectif principal est la lecture d'une valeur analogique à partir d'un convertisseur analogique-numérique (CAN) 
//MCP3221 et l'envoi de cette valeur sur le bus CAN.

//Les Includes
#include <Arduino.h>
#include <Wire.h> // pour communication I2C
#include "MCP3X21.h" //librairie pour MP3221
#include <ESP32-TWAI-CAN.hpp> //TWAI Esp32

//broches TX RX pour la communication CAN
#define CAN_TX 1 
#define CAN_RX 3 

//Constantes et variables globales
unsigned long dernierTemps = 0;
const byte adresseCAN = 0x4D; // Adresse du MCP3221A5T-E/OT 
const double vRef = 5.0;       // valeur de la référence de tension interne en Volts
const int printInfosSerie = 1;      // 1 pour imprimer les informations sur le port série, 0 pour ne pas afficher

//delcaration des fonctions
double lireMCP3221(int adresseMCP3221, int printInfo);
double convertDbyte_Double(byte msb, byte lsb);
void printINFO(byte msb, byte lsb);

void setup() {
  Wire.begin(8, 9); // Configure les broches SDA = 8 et SCL = 9 // Initialisation de la communication I2C
  Serial.begin(115200); // initialisation du port série
  dernierTemps = millis(); // initialisation du temps

    ESP32Can.setPins(CAN_TX, CAN_RX); // Initialisation de la communication CAN
    ESP32Can.setRxQueueSize(5);
    ESP32Can.setTxQueueSize(5);
    ESP32Can.setSpeed(ESP32Can.convertSpeed(500));

    if (ESP32Can.begin()) {
        Serial.println("Bus CAN démarré !");
    } else {
        Serial.println("Échec du démarrage du bus CAN !");
    }
}



// Cette fonction communique avec le MCP3221 via le protocole I2C pour lire deux octets de données (MSB et LSB),
// les convertit en une valeur de tension et les retourne en double. Elle prend en compte un paramètre pour décider d'imprimer les informations ou non.
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



//Convertit deux octets en une valeur double en tenant compte de la référence de tension et de la résolution ADC.
double convertDbyte_Double(byte msb, byte lsb) {
  int value = ((msb & 0x0F) << 8) | lsb; // Combinaison de l'octet MSB et de l'octet LSB en une valeur entière sur 12 bits.

  double v = (double)value * vRef / 4096.0; // Conversion de la valeur entière en tension en volts en utilisant la référence de tension et la résolution ADC.

  return v; // Retourne la valeur convertie en volts.
}


// Imprime les informations sur la valeur ADC et la tension correspondante en mV.
void printINFO(byte msb, byte lsb) {
  Serial.print("Valeur ADC (12 bits) = ");

  int valeurADC = ((msb & 0x0F) << 8) | lsb; // Conversion de l'octet MSB et de l'octet LSB en une valeur ADC sur 12 bits
  Serial.print(valeurADC);
  Serial.println();

  double tension_mV = convertDbyte_Double(msb, lsb) * 1000; // Conversion de la tension en mV
  Serial.print("Tension (mV) = ");
  Serial.println(tension_mV);
}


// Prépare et envoie la valeur du potentiomètre via le bus CAN en créant un message CAN avec l'ID approprié et les données correspondantes.
void sendPotentiometerValue(uint16_t potValue) {
    CanFrame potFrame = { 0 };
    potFrame.identifier = 0xB; // ID du message CAN
    potFrame.extd = 0;
    potFrame.data_length_code = 8;

    uint8_t* bufferPotValue = reinterpret_cast<uint8_t*>(&potValue); // Copie chaque octet du buffer dans le message CAN
    for (int i = 0; i < 2; i++) {
        potFrame.data[i] = bufferPotValue[i];
    }

    potFrame.data[2] = 0; 
    potFrame.data[3] = 0;
    potFrame.data[4] = 0;
    potFrame.data[5] = 0;
    potFrame.data[6] = 0;
    potFrame.data[7] = 0;

    ESP32Can.writeFrame(potFrame);
}


void loop() {
  double valeurCanal = lireMCP3221(adresseCAN, printInfosSerie); // mesurer et afficher les valeurs des canaux
  delay(1000); //1 seconde avant la prochaine mesure
  sendPotentiometerValue(valeurCanal); //Envoie la valeur lue via le bus CAN à l'aide de la fonction sendPotentiometerValue
}
