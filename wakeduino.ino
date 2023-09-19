#include <Ethernet.h>
#include <SD.h>


// Constants
const byte chipSelect = 4;
const byte broadcastIp[] = { 192, 168, 1, 255 };
const short wolPort = 9;
const byte remoteMaxCount = 4;

// Variables
IPAddress ip(0, 0, 0, 0);
int port = 23;
byte mac[] =  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
EthernetServer server = NULL;
String serverKey = ""; // 20 chars key
String remoteDevices[remoteMaxCount];
String remoteMacs[remoteMaxCount];
short remoteCount = 0;
bool freeMode = true;
bool alreadyConnected = false;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  if(initSD()){
    loadConfig();
  }
  runServer();
}

void loop() {
  EthernetClient client = server.available();
  if (client && !alreadyConnected) {
    alreadyConnected = true;
    Serial.println(F("Client connected"));
    String receivedString =  client.readStringUntil(0x0D); // 0x0D return char
    String receivedMac = "";
    String receivedDevice = "";
    int splitIndex = receivedString.indexOf("||");
    if(splitIndex==-1){
      splitIndex = receivedString.indexOf('|');
      if(splitIndex==-1){
        client.stop();
        alreadyConnected = false;
        return;
      }
      else{
        receivedMac = receivedString.substring(splitIndex+1,receivedString.length());
      }
    }
    else{
      receivedDevice = receivedString.substring(splitIndex+2,receivedString.length());
    }
    String receivedKey = receivedString.substring(0,splitIndex);
    // Server key check
    if (receivedKey.equals(serverKey)) {
      Serial.println(F("Correct Key"));
      bool readyToSend = false;
      String destMac = "";
      byte remoteMac[] =  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
      if(receivedDevice.length()>0){
        int deviceIndex = -1;
        for(int i=0;i<remoteCount;i++){
          if(remoteDevices[i].equals(receivedDevice)){
            deviceIndex = i;
            break;
          }
        }
        if(deviceIndex==-1){
          client.stop();
          alreadyConnected = false;
          return;
        }
        else{
          String deviceMac = remoteMacs[deviceIndex];
          Serial.print(F("MAC address for device "));
          Serial.println(receivedDevice);
          Serial.println(deviceMac);
          if(macStringToByteArray(deviceMac.c_str(),remoteMac)){
            destMac = deviceMac;
            readyToSend = true;
          }
        }
      }
      else if(receivedMac.length()>0){
        Serial.print(F("MAC address "));
        Serial.println(receivedMac);
        if(macStringToByteArray(receivedMac.c_str(),remoteMac)){
          destMac = receivedMac;
          if(freeMode){
            readyToSend = true;
          }
        }
      }
      byte magicPacket[102];
      int i,c1,j=0;
      for(i = 0; i < 6; i++,j++){
          magicPacket[j] = 0xFF;
      }
      for(i = 0; i < 16; i++){
          for( c1 = 0; c1 < 6; c1++,j++)
            magicPacket[j] = remoteMac[c1];
      }
      EthernetUDP Udp;
      if(readyToSend){
        client.println(F("\nOk"));
        client.stop(); 
        Udp.begin(wolPort);
        Udp.beginPacket(broadcastIp, wolPort);
        Udp.write(magicPacket, sizeof magicPacket);
        Udp.endPacket();
        Udp.flush();
        Udp.stop();
        Serial.println(F("Ok"));
        
      }
      delay(1000);
      
    } else {
      Serial.println(F("Wrong Key"));
      delay(1000);
      client.stop();
    }
    alreadyConnected = false;
    Serial.println(F("Client disconnected"));
  }
}
bool initSD(){
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(chipSelect)) {
    //Serial.println(F("ERR1"));
    /*
    is a card inserted?
    is your wiring correct?
    did you change the chipSelect pin to match your shield or module?
    */
    return false;
  }
  else return true;
}
void loadConfig(){
  File configFile = SD.open(F("init.cfg"));
  if (configFile) {
    short i = 0;
    while(configFile.available()){
      String data = configFile.readStringUntil('\n');
      data.remove(data.lastIndexOf(0x0D));
      switch(i){
        case 0:
          ip = stringToIpAddress(data.c_str());
          break;
        case 1:
          port = data.toInt();
          break;
        case 2:
          macStringToByteArray(data.c_str(),mac);
          break;
        case 3:
          serverKey = data;
          break;
        case 4:
          if(data.equals(F("remoteOnly"))){
            freeMode = false;
          }
          break;
        default:
          if(remoteCount<remoteMaxCount){      
            int splitIndex = data.indexOf('|');
            String remoteDevice = data.substring(0,splitIndex);
            String remoteMac = data.substring(splitIndex+1);
            remoteDevices[remoteCount] = remoteDevice;
            remoteMacs[remoteCount] = remoteMac;
            remoteCount++;
          }   
          break;
      }  
      i++;
    }
    configFile.close();
  } else {
    Serial.println(F("Can't load settings from SD. Using default settings."));
  }
}
void runServer(){
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println(F("Ethernet cable is not connected."));
    while (Ethernet.linkStatus() == LinkOFF) {
      delay(1000); 
    }
  }
  Ethernet.begin(mac, ip);
  server = EthernetServer(port);
  server.begin();
  Serial.print(F("LISTENING "));
  Serial.print(Ethernet.localIP());
  Serial.println(':'+String(port));
}
/* UTILITY FUNCTIONS */
IPAddress stringToIpAddress(const char* ipAddressString) {
  byte ipAddressBytes[4];
  IPAddress result;
  if (sscanf(ipAddressString, "%d.%d.%d.%d", &ipAddressBytes[0], &ipAddressBytes[1], &ipAddressBytes[2], &ipAddressBytes[3]) == 4) {
    result = IPAddress(ipAddressBytes);
  } else {
    Serial.println(F("ERRU2"));
  }
  return result;
}
bool macStringToByteArray(const char* macString, byte* macArray) {
   if (strlen(macString) != 17) {
    Serial.println(F("ERRU1"));
    return false;
  }
  char* token = strtok(macString, ":");
  int byteIndex = 0;

  while (token != NULL && byteIndex < 6) {
    macArray[byteIndex] = strtol(token, NULL, 16); // Converts hex in byte
    token = strtok(NULL, ":");
    byteIndex++;
  }
  return true;
}

