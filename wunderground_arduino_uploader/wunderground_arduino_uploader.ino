//Receives data from the Sparkfun USB Weather Board V3 and uploads via http put using the HttpClient library.
//Wunderground upload and variables code modified from the original WIMP Weatherboard project by Nathan Seidle, which was itself based on Weatherboard
//code by Mike Grusin.
//https://learn.sparkfun.com/tutorials/weather-station-wirelessly-connected-to-wunderground

//Serial reading and parsing code based on code by JHaskell - http://jhaskellsblog.blogspot.com
//



//UdpNtpTime code modified from the Arduino tutorial written by Michael Margolis/Tom Igoe - http://arduino.cc/en/Tutorial/UdpNtpClient

#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <EthernetUdp.h>


// MAC address for Ethernet shield. This is a "dummy" or default MAC that should work, but check to see if your shield has one already assigned.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const char WunderStation[]= "KCORVA39"; //Wunderground Station ID, replace with your own value

//Define Sensor Values
float Temp;
int Humidity;
float dewpoint;
float mbar;  // Barometric pressure
float rel_light;  //TEMT6000 light, a relative value (%)
float windspeed;
int winddirection;
float rainfall;
float batt_power;

//Some variables for the sanity checking code
boolean report=false;
float mbarcheck=990.0;

//Defines for the Time check code. Uses Network time to check for midnight when resetting daily rain etc.
unsigned int localPort = 8888;
IPAddress timeServer(132,163,4,101); // time-a.timefreq.bldrdoc.gov NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

const unsigned long GMT_offset = 25200;  //For Pacific Daylight Time. 7 hrs x 60 minutes x 60 seconds. Need to add code to determine is PST or PDT
byte hour; //hour of the day, to check for reset 
byte minutes; //minute of the hour
byte seconds; //second in the minute
// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  Serial.println("Starting data upload to Weather Underground");
  Serial.println();

  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(15000);
  }
  Udp.begin(localPort);
}

void loop() {
 
     zeroData();
  
   // read the sensor data in from the X-bee:
 
    while (Serial.available() > 0) {
      
     if (Serial.read()== '$'){  // Look for start of data payload 
      
       Temp = Serial.parseFloat();
       Humidity = Serial.parseInt();
       dewpoint = Serial.parseFloat();
       mbar = Serial.parseFloat();
       rel_light = Serial.parseFloat();
       windspeed = Serial.parseFloat();
       winddirection = Serial.parseInt();
       rainfall = Serial.parseFloat();
       batt_power = Serial.parseFloat();
     } 
      // look for newline character to indicate end of the sequence
  if (Serial.read()=='\n'){
  validate();

sendNTPpacket(timeServer);  //check the time
delay(1000);
if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
 //  Serial.print("Seconds since Jan 1 1900 = " );
  //  Serial.println(secsSince1900);              

    // now convert NTP time into everyday time:
  // Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;    
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;  
    // print Unix time:
//   Serial.println(epoch);                              

/*
   // print the hour, minute and second:
    Serial.print("The GMT time is ");       // UTC is the time at Greenwich Meridian (GMT), PDT is GMT -7hrs
    Serial.print((epoch % 86400L) / 3600); // print the hour (86400 equals secs per day), adjust for GMT offset in other time zones
    Serial.print(':');  
    if ( ((epoch % 3600) / 60) < 10 ) {
       In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch %60); // print the second
*/
hour = (((epoch-GMT_offset) % 86400L)/ 3600 ); //what hour of the day is it? GMT offset of 25200 seconds for PDT (7 hours behind)
minutes = ((epoch %3600)/60);  // minute in the hour
seconds = (epoch %60);// seconds in the minute
Serial.print("It is ");
Serial.print(hour);
Serial.print(":");
Serial.print(minutes);
Serial.print(":");
Serial.print(seconds);
Serial.println(" Hrs PDT");

if (hour == 24 && minutes == 1 && seconds < 20)
{
  Serial.println("It is midnight, time to reset");
  Serial.println();
  Serial.println("@"); //This is the sensor board reset code
}
 }



if (report==true)  
  {
    uploadData(); //data to COSM
    localReport();//data to local display
  }
  else
  {
    zeroData();
    Serial.println("Data was invalid, zeroed");
  }
  
  //  Now print out the values via the Xbee as a CSV for the separate display
  //  

  
  

  Serial.println();
  delay(14000);
   }
  }
 }
void printComma() // we do this a lot, it saves two bytes each time we call it
{
  Serial.print(",");
}


void uploadData()  // actually uploading to Weather Underground
{
 

}

void validate()  //sanity checking the data
{
  Serial.println("Validating Data Payload");
  Serial.print("mbar variable = ");
  Serial.println(mbar);
  Serial.println("mbarcheck =");
  Serial.print (mbarcheck);
  Serial.println();
  Serial.print("Relative light variable =");
  Serial.println(rel_light);
  Serial.println();
  
  if (mbar<=mbarcheck||rel_light>=0.1)
  {
    report=false;
    zeroData();
  }
  else
  {
    report=true;
  }
}


void localReport()
{
    
   
      Serial.print("Q"); printComma(); //Stream starts with Q to distinguish from weatherboard inbound data
      Serial.print(Temp); printComma();
      Serial.print(Humidity); printComma();
      Serial.print(dewpoint); printComma();
      Serial.print(mbar); printComma();
      Serial.print(rel_light); printComma();
      Serial.print(windspeed); printComma();
      Serial.print(winddirection); printComma();
      Serial.print(rainfall); printComma();
      Serial.print(batt_power); printComma();
      Serial.print('\n');
      
// Then values in 'human readable' form for debugging
      Serial.println();
      Serial.println("------------------");
      Serial.print("Temp= "); Serial.println(Temp);
      Serial.print("Humidity= "); Serial.println(Humidity);
      Serial.print("Dewpoint= "); Serial.println(dewpoint);
      Serial.print("mBar= "); Serial.println(mbar);
      Serial.print("Relative light= "); Serial.println(rel_light);
      Serial.print("Wind speed= "); Serial.println(windspeed);
      Serial.print("Wind Direction= "); Serial.println(winddirection);
      Serial.print("Rainfall= "); Serial.print(rainfall); Serial.println(" Units");
      Serial.print("Batt Power= "); Serial.println(batt_power);
      Serial.println("------------------");
}

void zeroData()
{
    Temp = 0;
       Humidity = 0;
       dewpoint = 0;
       mbar = 0;
       rel_light = 0;
       windspeed = 0;
       winddirection = 0;
       rainfall = 0;
       batt_power = 0;
       report= false;
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

//When the timeserver tells us it's midnight, reset the total amount of rain and gusts
void reset_counts()
{
/*  dailyrainin = 0; //Reset daily amount of rain

  windgustmph = 0; //Zero out this minute's gust
  windgustdir = 0; //Zero out the gust direction

  minutes = 0; //Reset minute tracker
  seconds = 0;
  lastSecond = millis();
  
  These are placeholders for WUnderground variables
  
*/  
}
