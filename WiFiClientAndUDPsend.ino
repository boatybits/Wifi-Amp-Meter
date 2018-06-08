// Wifi Ammeter

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_INA219.h>



const char* ssid     = "openplotter";
const char* password = "12345678";

const char* test  = "$WIMWV,28,R,4.7,N,A*04\r\n";

unsigned int localPort = 6666;      // local port to listen for UDP packets
byte packetBuffer[512]; //buffer to hold incoming and outgoing packets
char incomingPacket[255];  // buffer for incoming packets
IPAddress Rpi_IP(10, 10, 10, 92);

Adafruit_INA219 ina219;
WiFiUDP Udp;        // A UDP instance to let us send and receive packets over UDP



void setup() {
    Serial.begin(115200);
    delay(10);

    //************************************************
    // We start by connecting to a WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("[Connecting]");
    Serial.print(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Udp server starting at port ");
    Serial.println(localPort);
    Udp.begin(localPort);

    
    //**************************************************
    // Initialize the INA219.
    // By default the initialization will use the largest range (32V, 2A).  However
    // you can call a setCalibration function to change this range (see comments).
    ina219.begin();
    // To use a slightly lower 32V, 1A range (higher precision on amps):
    //ina219.setCalibration_32V_1A();
    // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
    //ina219.setCalibration_16V_400mA();
    //***************************************************

}

void loop() {

    int packetSize = Udp.parsePacket();
  if (packetSize)
    {
      // receive incoming UDP packets
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      int len = Udp.read(incomingPacket, 255);
      if (len > 0)
      {
        incomingPacket[len] = 0;
      }
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
    }

    float shuntvoltage = 0;
    float busvoltage = 0;
    float current_mA = 0;
    float loadvoltage = 0;
  
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    loadvoltage = busvoltage + (shuntvoltage / 1000);
    
    Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
    Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
    Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    Serial.println("");




    // Send UDP data ***************************************************

          int adc0;
          adc0 = millis();
          char result[10];
          
          dtostrf(current_mA, 7, 2, result);   //Change float to string 7 char long with 2 dec place
          
          // send back a reply, to the IP address and port we got the packet from
          Udp.beginPacket(Rpi_IP, 52525);
          Udp.write(result);    //Variable must be a char array
          Udp.endPacket();
          //delay(10);
          
    //  ******************************************************************

    delay(500);
}

