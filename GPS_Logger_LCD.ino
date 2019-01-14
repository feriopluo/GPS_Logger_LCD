
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define RX_PIN  8 // GPS TX
#define TX_PIN  9 // GPS RX
#define CHIP_SELECT 4  // uSD card 
#define GPS_BAUD  9600  // GP-735 default baud rate

LiquidCrystal lcd(7,6,5,4,3,2);


TinyGPSPlus gps;
File myFile;
SoftwareSerial ss(RX_PIN,TX_PIN);

void setup() {
  Serial.begin(115200); // Begin serial communication with computer
  ss.begin(GPS_BAUD); // Begin serial communication with GPS

  lcd.begin(16,2); // Initialize the 16x2 LCD
  lcd.clear(); // clear old data from LCD
  

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  myFile = SD.open("data.txt", FILE_WRITE); // Create or open a file called "data.txt" on the SD card
  if(myFile) 
  {
    if(myFile.size() == 0)  // Only create the header if there isn't any data in the file yet
    {
      myFile.println("Date , Time , Latitude , Longitude , Alt , Course , Speed");
    }
    myFile.close(); // Close the file to properly save the data
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  char gpsDate[10], gpsTime[10];

  if(gps.location.isValid()){ // GPS has a fix

    myFile = SD.open("GPS_data.csv", FILE_WRITE); // Open file "data.txt"
    if(myFile)
    {
      // Get date and time
      sprintf(gpsDate,"%d/%d/%d", gps.date.month(),gps.date.day(),gps.date.year());
      
      if(gps.time.hour() >= 8)
      {
        if(gps.time.second() < 10){
          sprintf(gpsTime,"%d:%d:0%d", (gps.time.hour() - 8),gps.time.minute(),gps.time.second());
        }
        else
        {
          sprintf(gpsTime,"%d:%d:%d", (gps.time.hour() - 8),gps.time.minute(),gps.time.second());
        }
      }
      else
      {
        if(gps.time.second() < 10){
          sprintf(gpsTime,"%d:%d:0%d", (gps.time.hour() + 16),gps.time.minute(),gps.time.second());
        }
        else
        {
          sprintf(gpsTime,"%d:%d:%d", (gps.time.hour() + 16),gps.time.minute(),gps.time.second());
        }
      }

      // Save data to SD card
      myFile.print(gpsDate);
      myFile.print(" , ");
      myFile.print(gpsTime);
      myFile.print(" , ");
      myFile.print(gps.location.lat(),6);
      myFile.print(" , ");
      myFile.print(gps.location.lng(),6);
      myFile.print(" , ");
      myFile.print((int)(gps.altitude.feet()));
      myFile.print(" , ");
      myFile.print(gps.course.deg(),2);
      myFile.print(" , ");
      myFile.println(gps.speed.mph(),0);

      // Print to LCD
      lcd.clear(); // clear old data from LCD
      lcd.setCursor(0,0); //Set invisible cursor to column 0 row 0
      lcd.print("Alti: ");
      lcd.print((int)(gps.altitude.feet()));
      lcd.print(" Ft");
      lcd.setCursor(0,1); // set to column 0 and row 1
      lcd.print("Spd: ");
      lcd.print((int)(gps.speed.mph()));
      lcd.print(" MPH");

      // Print GPS data to serial window
      Serial.print(gpsDate);
      Serial.print('\t');
      Serial.print(gpsTime);
      Serial.print('\t');
      Serial.print(gps.location.lat(),6);
      Serial.print('\t');
      Serial.print(gps.location.lng(),6);
      Serial.print('\t');
      Serial.print((int)(gps.altitude.feet()));
      Serial.print('\t');
      Serial.print(gps.course.deg(),2);
      Serial.print('\t');
      Serial.println(gps.speed.mph(),0);
    }

    myFile.close(); // Close the file to properly save the data
  }
  else  // GPS is looking for satellites, waiting on fix
  {

    Serial.print("Satellites in view: ");
    Serial.println(gps.satellites.value());
  }

  smartDelay(1000);
}

// Delay ms while still reading data packets from GPS
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while(ss.available())
    {
      gps.encode(ss.read());  
    }
  } while(millis() - start < ms);
}
