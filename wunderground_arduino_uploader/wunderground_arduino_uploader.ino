
//Serial reading and parsing code based on code by JHaskell - http://jhaskellsblog.blogspot.com
//

#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Cosm.h>

// MAC address forEthernet shield
<<<<<<< HEAD
byte mac[] = { YOUR MAC HERE };
=======
byte mac[] = { YOUR MAC ADDRESS HERE };
>>>>>>> abe62735f12f06e9ecd909e5fa2cae1128ceed97

// Cosm key 
char cosmKey[] = "YOUR COSM KEY HERE";



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

boolean report=false;
float mbarcheck=990.0;

// Define the strings for our datastream IDs
char temp[] = "Temp";
char humidity[]="Humidity";
char dew_point[]="Dewpoint";
char pressure[]="Pressure";
char wind_speed[]="windspeed";
char wind_direction[]="Wind_Direction";

const int bufferSize = 140;
char bufferValue[bufferSize]; // enough space to store the string we're going to send
CosmDatastream datastreams[] = {
  CosmDatastream(temp, strlen(temp), DATASTREAM_FLOAT),
  CosmDatastream(humidity, strlen(humidity), DATASTREAM_FLOAT),
  CosmDatastream(dew_point, strlen(dew_point), DATASTREAM_FLOAT),
  CosmDatastream(pressure, strlen(pressure), DATASTREAM_FLOAT),
  CosmDatastream(wind_speed, strlen(wind_speed), DATASTREAM_FLOAT),
  CosmDatastream(wind_direction, strlen(wind_direction), DATASTREAM_FLOAT),
 
};
// Finally, wrap the datastreams into a feed
CosmFeed feed(84186, datastreams, 6 /* number of datastreams */);

EthernetClient client;
CosmClient cosmclient(client);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  Serial.println("Starting multiple datastream upload to Cosm...");
  Serial.println();

  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(15000);
  }
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
  delay(15000);
   }
  }
 }
void printComma() // we do this a lot, it saves two bytes each time we call it
{
  Serial.print(",");
}


void uploadData()  // actually uploading to COSM
{
  datastreams[0].setFloat(Temp);
  datastreams[1].setFloat(Humidity);
  datastreams[2].setFloat(dewpoint);
  datastreams[3].setFloat(mbar);
  datastreams[4].setFloat(windspeed);
  datastreams[5].setFloat(winddirection);

Serial.println("Uploading it to Cosm");
  int ret = cosmclient.put(feed, cosmKey);
  Serial.print("cosmclient.put returned ");
  Serial.println(ret);

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
  
  if (mbar<=mbarcheck)
  {
    report=false;
    zeroData();
  }
  else
  {
    report=true;
  }

if (rel_light>=0.1)
  {
    report=false;
    zeroData();
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

