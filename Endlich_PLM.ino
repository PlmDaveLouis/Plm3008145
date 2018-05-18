// To do Programm: Zum laufen bringen ==> Switchcase genauer betrachten (Vergleich mit Nyfennegger Programm & Andre)

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <Servo.h>

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif

/*=========================================================================*/
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/



Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
Servo myservo;

enum jobs {
  JOB_IDLE,
  JOB_PICKUP,
  JOB_DROP_L,
  JOB_DROP_R
};

int moduleJob = JOB_IDLE;

/**************************************************************************/
void setup()
{
  myservo.attach(9);
  myservo.write(68);
  pinMode(6, INPUT);

  Serial.begin(9600);
  initBLE();
  ble.verbose(false);  // debug info is a little annoying after this point!
}

void loop()
{
  switch (moduleJob)
  {
    case JOB_IDLE : {
        listenBLE();
      } break;

    case  JOB_DROP_L : {
        Serial.println("JOB_DROP");
        myservo.write(180);             // tell servo to go to position in variable 'pos'
        delay(1000);
        myservo.write(65);              // tell servo to go to position in variable 'pos'
        SensorCheckEntladen();
        moduleJob = JOB_IDLE;
      } break;

    case  JOB_DROP_R : {
        myservo.write(0);               // tell servo to go to position in variable 'pos'
        delay(1000);
        myservo.write(65);              // tell servo to go to position in variable 'pos'
        SensorCheckEntladen();
        moduleJob = JOB_IDLE;
      } break;

    case  JOB_PICKUP : {
        int Sensor = digitalRead(6);
        SensorCheckLaden();
        moduleJob = JOB_IDLE;
      } break;

    default : sendBLE("Falscher Befehl");
  }

  delay(10);
  //*********************************************************************************************************************************************************************************
}

// SensorCheckLaden

void SensorCheckLaden() {
  int Sensor = digitalRead(6);
  delay(100);
  if (Sensor == 1) sendBLE("Success(0)");
  else sendBLE("Success(1)");
}

// SensorCheckEntladen

void SensorCheckEntladen() {
  int Sensor = digitalRead(6);
  delay(100);
  if (Sensor == 1) sendBLE("Success(1)");
  else sendBLE("Success(0)");
}



// Handle API commands
void handleApiCommands(String command) {

  if (command == "pickup();") {
    moduleJob = JOB_PICKUP;
  }
  if (command == "drop(1);") {
    moduleJob = JOB_DROP_R;
  }
  if (command == "drop(-1);") {
    moduleJob = JOB_DROP_L;
  }
}

// Listen to incomminc commands from Bluetooth
void listenBLE() {
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  handleApiCommands(ble.buffer);
}

// Send message over Bluetooth
void sendBLE(String msg) {
  ble.print("AT+BLEUARTTX=");
  ble.println(msg);

  // check response stastus
  if (! ble.waitForOK() ) {
    Serial.println(F("Failed to send?"));
  }
}

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void initBLE() {
  Serial.println(F("Adafruit Bluefruit Command Mode Example"));
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */

  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();
}


