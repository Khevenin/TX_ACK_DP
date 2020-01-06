#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24l01.h>
#include <printf.h>

//#include "libbuf.h"
//#include "lib-adc-irq.h"

#define X_AXIS 0
#define Y_AXIS 1
#define SWITCH 2

#define CE 7
#define CSN 8

#define TX_BUF_SIZE 32     //Size of TX buffer
#define RX_BUF_SIZE 32     //Size of RX buffer
#define ADDRESS_BUF_SIZE 5 //Size of buffer with pipeline address

const uint8_t addressTX[ADDRESS_BUF_SIZE] = {'P', 'I', 'L', 'O', 'T'};
const uint8_t addressRX[ADDRESS_BUF_SIZE] = {'S', 'L', 'A', 'V', 'E'};

uint8_t bufferTX[TX_BUF_SIZE];
uint8_t bufferRX[RX_BUF_SIZE];

boolean statusTX = false;

RF24 TX(CE, CSN); //set CE, CSN pins

/* funtions's prototypes */
void resetShort(uint8_t *buf, size_t size);
void printBuffer(uint8_t *buf, size_t size);

void setup()
{

  Serial.begin(115200);
  Serial.println("Serial port init done.");

  /* RF config */
  Serial.println("RF init start.\n");
  TX.begin();
  TX.setPALevel(RF24_PA_LOW); //set Power Amp output power
  Serial.println("Set PA power.");
  TX.setDataRate(RF24_250KBPS); //set Data Rate of speed tramission
  Serial.println("Set data rate.");
  TX.setChannel(0x64); //set Channel
  Serial.println("Set RF channel.");

  TX.setRetries(4, 2); //set Retries of package trasmission
  Serial.println("Set retries feature.");
  TX.setCRCLength(RF24_CRC_8);
  Serial.println("Set CRC.");

  TX.setAutoAck(1);
  Serial.println("Enable ACK.");
  TX.enableDynamicPayloads() ;
  Serial.println("Enable Dynamic Payload size.");
  TX.enableAckPayload(); //enable transmission ACK signal with Payload
  TX.writeAckPayload(1, bufferTX, sizeof(bufferTX));
  Serial.println("Enable ACK payload.");
  TX.enableDynamicPayloads();
  Serial.println("Enable dynamic payload size.");
  Serial.println("nRF24 init done.");

  /* Open pipeline */
  TX.openWritingPipe(addressTX);    //enable pipeline to transmit on TX address
  TX.openReadingPipe(1, addressRX); //enable pipeline to receive from RX address

  Serial.println("nRF24 pipeline addresses done.");

  /* Reset MCU TX and RX buffers */
  resetShort(bufferTX, TX_BUF_SIZE);
  resetShort(bufferRX, RX_BUF_SIZE);
  printBuffer(bufferRX, RX_BUF_SIZE);
  printBuffer(bufferTX, TX_BUF_SIZE);
  Serial.println("\nTX and RX buffers reset done.\n");

  TX.stopListening();
}

void loop()
{
  bufferTX[0] = map(analogRead(X_AXIS), 0, 1023, 0, 255);
  bufferTX[1] = map(analogRead(Y_AXIS), 0, 1023, 0, 255);
  bufferTX[2] = map(analogRead(SWITCH), 0, 1023, 0, 255);
  Serial.print("\nADC conversion done.");
  delay(250);

  /* Sending data */
  statusTX = TX.write(bufferTX, TX_BUF_SIZE);
  Serial.print("\nTransmited data.");
  printBuffer(bufferTX, TX_BUF_SIZE);
  delay(250);
  /* Receive data */
  if (statusTX)
  {
    if (TX.isAckPayloadAvailable())
    {
     // uint8_t lenght = TX.getPayloadSize();
     uint8_t lenght = TX.getDynamicPayloadSize();
      Serial.print("\nACK payload lenght: ");
      Serial.print(lenght);
      TX.read(bufferRX, lenght);
      Serial.print("\nTX - Received data.");
      printBuffer(bufferRX, RX_BUF_SIZE);
    }
  }
}

void printBuffer(uint8_t *buf, size_t size)
{
  Serial.print("\nBuffer content.");
  Serial.print("\n");
  for (size_t i = 0; i < size; i++)
  {
    Serial.print(buf[i]);
    Serial.print(" ");
  }
}

void resetShort(uint8_t *buf, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    buf[i] = 0;
  }
}
