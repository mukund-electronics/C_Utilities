#include "udpMulticast.h"
#include "timerFw.h"
#include "eventFw.h"

#define BUFFER_LENGTH 256 //max buffer length for the multicast message

WiFiUDP Udp_receiver;
WiFiUDP Udp_sender;
//WiFiClient tcpClient_discovery;  //to send response to tcp server upon receiving udp multicast msg

WiFiClient tcpClient_Cloud;  //to send response to tcp server upon receiving udp multicast msg

static unsigned int tcpClientPort = 7654;  //multicast sender(tcp server) port address

IPAddress multicastRxAddress(224,0,0,10); //multicast reception ip group
IPAddress multicastTxAddress(224,0,0,11); //multicast transmission ip group

//IPAddress CloudIpAddress(192,168,88,150); //multicast transmission ip group
IPAddress CloudIpAddress(13, 234, 116, 116); //multicast transmission ip group

static unsigned int CloudIpPort = 1337;

static unsigned int multicastReceivingPort = 8001;  //multicast listening port
static unsigned int multicastSendingPort = 9001;  //multicast sending port

//char incomingPacket[BUFFER_LENGTH]; //udp multicast rx message 
//char *responseMessage = "Yup";  //udp multicast response string
//char *responseMessage = "{\"cmd\":\"prsnt\",\"cl\":{\"dN\":\"deviceName\"}}";

//char *multicastMessage = "Hello";  //multicast msg sent for showing presence 
//char *multicastMessage = "{\"cmd\":\"prsnt\",\"cl\":{\"dN\":\"deviceName\"}}";
void initMulticastReceiver()
{
  //Serial.println(WiFi.localIP());
  Udp_receiver.begin(WiFi.localIP(), multicastReceivingPort);
  Udp_receiver.beginMulticast(multicastRxAddress, multicastReceivingPort);  //start multicast receiver
  Serial.println("initMulticastReceiver: begin multicast");
}

void initMulticastSender()
{
  //Serial.println(WiFi.localIP());
  Udp_sender.begin(WiFi.localIP(), multicastSendingPort);
  Serial.println("initMulticastSender: begin multicast");
}

void transmitMulticastMessage(char *udpMessage, uint32_t msgLen)
{
  Serial.printf("UDP Multicast message : %s\n", udpMessage);
  // For ESP8266
  //Udp_sender.beginPacketMulticast(multicastTxAddress, multicastSendingPort, WiFi.localIP(), 1); //start multicast sender
  
  // For ESP32
  Udp_sender.beginPacket(multicastTxAddress, multicastSendingPort);
  Udp_sender.beginPacket(multicastTxAddress, multicastSendingPort);  Udp_sender.write((const uint8_t *)udpMessage, msgLen);
  Udp_sender.endPacket();
}

int receiveMulticastMessage()
{
  int packetLength = Udp_receiver.parsePacket(); 
//  if(packetLength > 0)
//  {
//    Serial.printf("packetLength : %d \n", packetLength);
//  }
  if(packetLength) {
      char incomingPacket[BUFFER_LENGTH];
      //char *incomingPacket = (char *)malloc(packetLength+1);
      //Serial.printf("Address of incomingPacket : %p \n", incomingPacket);
      //Check for NULL, out of memory
      int len = Udp_receiver.read(incomingPacket, packetLength); //read the udp multicast message

      //Serial.printf("len : %d \n", len);
      
      if (len > 0) {
          incomingPacket[len] = 0;
          Serial.println(incomingPacket);
          //sendIdentityResponse();                                 //send the identity resposne back to multicaster

          // Generate the Parse Udp event
          eventGenerate(EVT_PAR_UDP_MSG, (void*) incomingPacket, len);
          return 0;
      }
  }
  return 1;
}

// make a event for UDP message
// Parse UDP message
// make a event for type of UDP message i.e 1. Discovery


int sendIdentityResponse(char *responseMessage)
{
//  Serial.printf("IpAddress of Server : %s\n", Udp_receiver.remoteIP());
  Serial.printf("1. udpMulticast.ino :: sendIdentityResponse :: responseMessage : %s\n\n", responseMessage);
  Serial.print("2. Ip Address of Mobile APP : "); Serial.println(Udp_receiver.remoteIP());
  WiFiClient tcpClient_discovery;
  if(tcpClient_discovery.connect(Udp_receiver.remoteIP(), tcpClientPort)) { //connect to multicast sender over tcp
    tcpClient_discovery.write((uint8_t *)responseMessage, strlen(responseMessage));  //send the response message to tcp server
    tcpClient_discovery.stop();
    Serial.println("3. Response sent successfully to Mobile APP");
  } else {
    Serial.println("3. Tcp Connection failed with Mobile APP");
    return 1;
  }
  return 0;
}
static unsigned int tcpServerPort_Cloud = 1337;  //listening port for tcp server CLOUD
WiFiServer tcpServer_Cloud(tcpServerPort_Cloud);  //tcp server listening for control messages
WiFiClient tcpClient_control_Cloud;  //to read messages from tcp client meant for hardware control

int sendIdentityResponse_Cloud(char *responseMessage)
{
  Serial.print("1. IpAddress of Server Cloud : "); Serial.println(CloudIpAddress);
  Serial.printf("2. udpMulticast.ino :: sendIdentityResponse_Cloud :: responseMessage : %s\n", responseMessage);
  if(WiFi.status() == WL_CONNECTED){
    if(tcpClient_Cloud.connected()){
       Serial.println("3. Server (Cloud) Connected");
       tcpClient_Cloud.write((uint8_t *)responseMessage, strlen(responseMessage));  //send the response message to tcp server
       Serial.println("4. Response sent successfully to cloud");
    } else {
      tcpClient_Cloud.setTimeout(1);
      if(tcpClient_Cloud.connect(CloudIpAddress, CloudIpPort)) { 
        Serial.println("3. Server (Cloud) Connected");
        tcpClient_Cloud.write((uint8_t *)responseMessage, strlen(responseMessage));  //send the response message to tcp server
        Serial.println("4. Response sent successfully to cloud");
      } else {
        Serial.println("4. Tcp Connection failed with Cloud");
        return 1;
      }
    } 
  } else {
   Serial.println("Not Sending Data to Cloud as Wifi Not Connected");
  }
  return 0;
}

int handleTcpControlMessages_CLoud()
{
  if((tcpClient_Cloud.connected() || tcpClient_Cloud.available()))
  {
if (tcpClient_Cloud.available())
    {
//      String line = tcpClient_Cloud.readStringUntil('\n');
//      Serial.println(line);

      char dataByte = tcpClient_Cloud.read();
      if(dataByte != 0xff) { //discard the garbage character read after connecting to client 
        //Serial.print(dataByte, HEX);
        storeMessage_Cloud(dataByte);
      }
    
      //if message is ready, decode and process it
      if(1 == messageReady_Cloud) {
        Serial.println("\n\n\n ********Data Received on TCP from CLOUD******  \n\n\n");      
        commandDecoder_Cloud();  //decode command 
        processCommand_Cloud();  //respond to command
        reInitMessenger_Cloud();  //reInit message buffer         
      }
    }
  }
}

//void initiateMulticastUdpMessage()
//{
//  transmitMulticastMessage(); 
//}
