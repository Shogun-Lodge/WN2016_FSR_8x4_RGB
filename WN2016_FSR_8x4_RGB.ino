/*
  -- Shogun Lodge Services --

  Test Code for White Night 2016
  
  FSR Reader

  Individual RGB Section
  
  Reads 8x4 FSR w/ 2 x 4051

  Needs to configured for each 'section'

  R = 'fsr_data/1' / 192.168.0.110 / 8888 (port) / 0xED (last byte of mac) / /fireOSCdata/1
  G = 'fsr_data/2' / 192.168.0.111 / 8889 (port) / 0xEE (last byte of mac) / /fireOSCdata/2
  R = 'fsr_data/3' / 192.168.0.112 / 8890 (port) / 0xEF (last byte of mac) / /fireOSCdata/3
  
  Question - Why can't 64 bytes be added to OSC message? 63 can...
  
  6/1/2016
 
  shogunlodgeservices@gmail.com 

*/

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>    
#include <OSCMessage.h>

EthernetUDP Udp;

//the Arduino's IP
IPAddress ip(192, 168, 0, 110);
//destination IP
IPAddress outIp(192, 168, 0, 125);
const unsigned int outPort = 7000;

 byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // you can find this written on the board of some Arduino Ethernets or shields

// Multiplexer code set up
int r0 = 0;      //value of select pin at the 4051 (s0)
int r1 = 0;      //value of select pin at the 4051 (s1)
int r2 = 0;      //value of select pin at the 4051 (s2)
int countA = 0;   //which y pin we are selecting

// Analog Pin & FSR Data
int fsrData[64];
int fireOSC = 0;

void setup() 
{
  Ethernet.begin(mac,ip);
    Udp.begin(8888);
    
  pinMode(2, OUTPUT);    // s0
  pinMode(3, OUTPUT);    // s1
  pinMode(4, OUTPUT);    // s2
}

void loop() {
  
  for (countA = 0; countA<=7; countA++) {
    // select the bit
    Serial.println(countA);  
    r0 = bitRead(countA,0);        
    r1 = bitRead(countA,1);        
    r2 = bitRead(countA,2);

    // Write the FSR select code to 4051
    digitalWrite(2, r0);
    digitalWrite(3, r1);
    digitalWrite(4, r2);    
    
    fsrData[countA] = analogRead(0); // Load fsrdata with fsr value currently 'pointed' at
    fsrData[countA+8] = analogRead(1); // Load fsrdata with fsr value currently 'pointed' at
    fsrData[countA+16] = analogRead(2); // Load fsrdata with fsr value currently 'pointed' at
    fsrData[countA+24] = analogRead(3); // Load fsrdata with fsr value currently 'pointed' at
    //fsrData[countA+32] = analogRead(1); // Load fsrdata with fsr value currently 'pointed' at
    //fsrData[countA+40] = analogRead(1); // Load fsrdata with fsr value currently 'pointed' at
    //fsrData[countA+48] = analogRead(1); // Load fsrdata with fsr value currently 'pointed' at
    //fsrData[countA+56] = analogRead(1); // Load fsrdata with fsr value currently 'pointed' at
  } 
    
  while (fireOSC==0) {
    OSCMessage msgIN;
    int size;
    if( (size = Udp.parsePacket())>0){
      while(size--)
      msgIN.fill(Udp.read());
    } 

    if(!msgIN.hasError()){
      fireOSC = msgIN.fullMatch("/fireOSCdata/1", 0);
    }
  }

  fireOSC = 0;
  
  OSCMessage msg("/fsr_data/1");
   
  for (int x = 0; x<=62; x++) {    // Why NOT 64 bytes?
    msg.add(fsrData[x]);
  }
     
  Udp.beginPacket(outIp, outPort);
    msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message    
}

//-----END OF CODE-----
