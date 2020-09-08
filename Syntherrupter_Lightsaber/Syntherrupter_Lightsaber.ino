/*
 * Code for transmitting data from an MPU-6050 IMU via WiFi.
 * 
 * If the code detects the IMU (GPIO0: SDA, GPIO2: SCL), it will act as slave 
 * and send the data. If it can't detect the IMU it will act as master, create
 * the WiFi AP and collect data from slaves. This means the same code can be 
 * flashed to both master and slave devices.
 *
 * Pairing (Master)
 *   * Master creates WiFi AP with credentials specified below. 
 *   * Master creates server on port specified below.
 *   * If a new device connects (assumed to be a slave) the master checks if 
 *     there's still space in the list of slaves and if so, adds it to the 
 *     list. Otherwise it sends 0x00 to the slave and it won't be added. 
 *   * Master waits for the slave to send it's ID. If this does not happen
 *     within 100ms, master sends 0x00 to the slave and removes it from the 
 *     the list. Otherwise master returns the ID to the slave. 
 * Pairing (Slave)
 *   * Slave waits for the WiFi AP to appear and connects.
 *   * Slave waits for the master server to come up and connects to it.
 *   * Slave immediately sends its ID (default: 1, can't be 0).
 *   * Slave waits for a response. If the response is not equal to its ID,
 *     slave will enter endless loop. User has to reset the slave for a new 
 *     try (preventing unecessary connection retries to the master).
 * 
 * (Slave) ID
 *   Each ESP8266 stores its ID in its EEPROM which it uses to identify its 
 *   data packets (see below). The ID defaults to 1 and can go up to 255. 
 * 
 * Slave data packets
 *   Data packets from the slave to the master look like this:
 *     * Byte  0:       Slave ID
 *     * Byte  1 -  4:  X-Acceleration in g,   float, little endian
 *     * Byte  5 -  8:  Y-Acceleration in g,   float, little endian
 *     * Byte  9 - 12:  Z-Acceleration in g,   float, little endian
 *     * Byte 13 - 16:  X-Angle Rate   in °/s, float, little endian
 *     * Byte 17 - 20:  Y-Angle Rate   in °/s, float, little endian
 *     * Byte 21 - 24:  Z-Angle Rate   in °/s, float, little endian
 *
 * Master data packets
 *   Master collects data from each slave. Once a complete packet from a slave
 *   has been received, it will be sent via the master's UART. This assures 
 *   that different slave packets won't be scrambled together. 
 * 
 * Serial configuration
 *   The code is listening to the serial port for commands. It does this if 
 *   it detected no IMU (operates as master) and if there are no clients. 
 *   This makes sure that the Serial port is not used for anything else.
 *   Command format is pretty simple: 1 address byte, 1 data byte, both must 
 *   not be 0.
 *   Configurable parameters: 
 *     * Address 1: Slave ID. It will be stored to the EEPROM. If the 
 *                  device operates as slave, it will be applied after a 
 *                  restart.
 *   After processing the command, the data byte is returned. A value of 0
 *   means the processing was not successful or the command was invalid. 
 *
 * Debugging
 *   This code is full of Serial.print commands. Since the code is assumed to
 *   work, they are commented out. To get debugging information, replace all
 *   occurences of "//Serial.print" with "Serial.print". Same thing for 
 *   turning it off again. Serial.print* is _only_ used for debugging.
 */


#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>
#include <TaskSchedulerSleepMethods.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <EEPROM.h>

#ifndef APSSID
#define APSSID "Syntherrupter"
#define APPSK  "ICanChange!IPromise!"
#endif
#define PORT     8888
#define MAX_CLIENTS 4
#define DATA_SIZE  24
#define DATA_PERIOD_MS 30
#define SAMPLE_PERIOD_MS 5
#define TIMEOUT_FACTOR  3

#define MPU_SDA_PIN 0 // GPIO0
#define MPU_SCL_PIN 2 // GPIO2
#define MPU_SIGNAL_PATH_RESET 0x68

WiFiServer masterServer(PORT);
WiFiClient masterClients[MAX_CLIENTS];
WiFiClient slaveClient;
IPAddress masterIP;
MPU6050 mpu(Wire);
bool serialCommandsEnabled = true;
bool slave = 0;
uint8_t slaveIDs[MAX_CLIENTS];
uint32_t masterClientsTimeout[MAX_CLIENTS];
float slaveData[6];

// Task callbacks
void slaveReadMPUCallback();
void slaveConnectToMasterServerCallback();
void slaveSendDataCallback();

// Tasks
Task slaveReadMPU(SAMPLE_PERIOD_MS, TASK_FOREVER, slaveReadMPUCallback);
Task slaveSendData(DATA_PERIOD_MS, TASK_FOREVER, slaveSendDataCallback);
Task slaveConnectToMasterServer(DATA_PERIOD_MS * TIMEOUT_FACTOR, TASK_FOREVER, slaveConnectToMasterServerCallback);

Scheduler scheduler;


void slaveReadMPUCallback()
{
  mpu.update();

  float newData[6];
  newData[0] = mpu.getAccX();
  newData[1] = mpu.getAccY();
  newData[2] = mpu.getAccZ();
  newData[3] = mpu.getGyroX();
  newData[4] = mpu.getGyroY();
  newData[5] = mpu.getGyroZ();

  for (uint32_t i = 0; i < 6; i++)
  {
    if (fabsf(newData[i]) > fabsf(slaveData[i]))
    {
      slaveData[i] = newData[i];
    }
  }
}

void slaveSendDataCallback()
{
  uint8_t* dataBytes = (uint8_t*) ((void*) slaveData);
  if (slaveClient.connected())
  {
    slaveClient.write(slaveIDs[0]);
    slaveClient.write(dataBytes, DATA_SIZE);
  }

  for (uint32_t i = 0; i < 6; i++)
  {
    slaveData[i] = 0;
  }
}

void slaveConnectToMasterServerCallback()
{
  if (!slaveClient.connected())
  {
    // Wait for the master TCP Server to appear and connect to it.
    //Serial.print("Waiting for Master Server to appear");
    while (slaveClient.connect(masterIP, PORT) != 1)
    {
      //Serial.print('.');
      delay(200);
    }
    //Serial.println("\r\nConnected!");
    //Serial.println("Sending ID");
    slaveClient.setNoDelay(true);
    slaveClient.write(slaveIDs[0]);
    while (!slaveClient.available());
    uint8_t data = slaveClient.read();
    if (data != slaveIDs[0])
    {
      while (42)
      {
          // nfg; there's no space left for us or the communication went wrong. 
          // Hence we won't do anything anymore (User has to reset for a new try).
          yield();
      }
    }
  }
}

void setup()
{
  EEPROM.begin(128);
  scheduler.init(); 
  Serial.begin(115200);

  // Check if EEPROM has been written. Otherwise overwrite with default values.
  if (EEPROM.read(0) != 42)
  {
    EEPROM.write(0, 42);
    EEPROM.write(1, 1);
    EEPROM.commit();
  }

  Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN);
  // Reset MPU (MPU6050 Register Map page 41
  delay(100);
  mpu.writeMPU6050(MPU6050_PWR_MGMT_1, 0b10000000);
  delay(100);  
  mpu.writeMPU6050(MPU_SIGNAL_PATH_RESET, 0b00000111);
  delay(100);
  // Search for MPU 6050. Its WHO_AM_I 
  byte whoAmI = mpu.readMPU6050(MPU6050_WHO_AM_I);
  //Serial.print("MPU6050 Who Am I: ");
  //Serial.println(whoAmI);
  
  // Depending on the MPU variant the WHO AM I register must contain either the value 0x68 or 0x72
  if (whoAmI == 0x68 || whoAmI == 0x72)
  {
    // If an MPU6050 has been found this device operates as a slave (since master and slave run the same firmware)
    //Serial.println("Entering slave setup.");
    slave = true;
    serialCommandsEnabled = false;

    // Initialize MPU
    mpu.begin();

    // Read slaveID from EEPROM
    slaveIDs[0] = EEPROM.read(1);
    //Serial.print("SlaveID is ");
    //Serial.println(slaveIDs[0]);

    // Initialize slaveData
    for (uint32_t i = 0; i < 6; i++)
    {
      slaveData[i] = 0;
    }
    
    // Wait for the masters AP to appear and connect to it.
    //Serial.print("Waiting for Master AP to appear");
    WiFi.mode(WIFI_STA);
    WiFi.begin(APSSID, APPSK);
    while (WiFi.status() != WL_CONNECTED)
    {
      //Serial.print('.');
      delay(500);
    }
    WiFi.setAutoReconnect(true);
    masterIP = WiFi.gatewayIP();
    //Serial.print("\r\nConnected! Master IP: ");
    //Serial.print(masterIP);
    //Serial.print("  Slave IP: ");
    //Serial.println(WiFi.localIP());
    slaveConnectToMasterServerCallback();
    
    // Add slave tasks to scheduler
    scheduler.addTask(slaveReadMPU);
    slaveReadMPU.enable();
    scheduler.addTask(slaveSendData);
    slaveSendData.enable();
    scheduler.addTask(slaveConnectToMasterServer);
    slaveConnectToMasterServer.enable();
  }
  else
  {
    //Serial.println("Entering master setup");
    slave = false;
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APSSID, APPSK);
    masterIP = WiFi.softAPIP();
    //Serial.print("Master AP up and running at ");
    //Serial.println(masterIP);

    // Start server
    //Serial.print("Starting Server... ");
    masterServer.begin();
    masterServer.setNoDelay(true);
    //Serial.println("Done!");
 }
}

void loop()
{
  scheduler.execute();

  if (serialCommandsEnabled)
  {
    if (Serial.available() >= 2)
    {
      uint8_t address = Serial.read();
      uint8_t data    = Serial.read();
      if (address == 1 && data)
      {
        EEPROM.write(address, data);
        EEPROM.commit();
        Serial.write(EEPROM.read(address));
      }
      else
      {
        Serial.write(0);
      }
    }
  }

  if (!slave)
  {
    // Master
    
    // Handle new clients if there are any.
    if (masterServer.hasClient())
    {
      serialCommandsEnabled = false;
      //Serial.println("There's something...");
      bool clientLimitReached = true;
      for (uint32_t i = 0; i < MAX_CLIENTS; i++)
      {
        if (!masterClients[i])
        {
          masterClients[i] = masterServer.available();
          masterClients[i].setNoDelay(true);

          // Wait for successful transmission of the slaveID
          uint32_t timeMS = millis();
          slaveIDs[i] = 0;
          while (!slaveIDs[i] && (millis() - timeMS) < 100)
          {
            if (masterClients[i].available())
            {
              slaveIDs[i] = masterClients[i].read();
            }
          }

          if (!slaveIDs[i])
          {
            // No slaveID received. Remove from list.
            masterClients[i].write(0);
            masterClients[i].flush();
            masterClients[i].stop();
            //Serial.println("Refused new client (invalid ID)");
          }
          else
          {
            masterClients[i].write(slaveIDs[i]);
            masterClientsTimeout[i] = millis() + DATA_PERIOD_MS * TIMEOUT_FACTOR;
            //Serial.print("New Client (id: ");
            //Serial.print(i);
            //Serial.println(")");
            clientLimitReached = false;
          }
          break;
        }
      }
      if (clientLimitReached)
      {
        WiFiClient noSpaceForYou = masterServer.available();
        noSpaceForYou.write(0);
        noSpaceForYou.flush();
        noSpaceForYou.stop();
        //Serial.println("Refused new client (no space)");
      }
    }

    // Check clients for new data
    for (uint32_t clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
    {
      uint32_t timeMS = millis();
      if (masterClients[clientNum].available() >= DATA_SIZE + 1)
      {
        uint8_t dataByte =  masterClients[clientNum].read();
        if (dataByte == slaveIDs[clientNum])
        {
          masterClientsTimeout[clientNum] = timeMS + DATA_PERIOD_MS * TIMEOUT_FACTOR;
          Serial.write(dataByte);
          for (uint32_t i = 0; i < DATA_SIZE; i++)
          {
            Serial.write(masterClients[clientNum].read());
          }
        }
      }
      else if (timeMS > masterClientsTimeout[clientNum])
      {
        masterClients[clientNum].flush();
        masterClients[clientNum].stop();
      }
      yield();
    }
  }
  else
  {
    // Slave
  }
}
