/*********************************************************************
This is a simple clock written for 128x64 OLED display. 
Hardware used:
Arduino UNO
64x128 OLED display
DHT22 temp/humidity sensor
Chronodot RTC

Code written by George Timmermans
Libraries provided by other sources. 

Special thanks to:
Adafruit (for there hardware, tutorials and libraries)
Stephanie (for writing the chronodot library)
I could not have done it without them. Thank you.
*********************************************************************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include "Chronodot.h"
//********************************************************************
#define OLED_DC 11
#define OLED_CS 12
#define OLED_CLK 10
#define OLED_MOSI 9
#define OLED_RESET 13
Adafruit_SSD1306 display( OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, 
                          OLED_CS );

#if ( SSD1306_LCDHEIGHT != 64 )
#error( "Height incorrect, please fix Adafruit_SSD1306.h!" );
#endif

#define DHTPIN 4
#define DHTTYPE DHT22 
DHT dht( DHTPIN, DHTTYPE );

Chronodot RTC;

const int MAX = 60;
int tempArray[ MAX ];
int humArray[ MAX ];
//********************************************************************
uint32_t storeDataTimer = millis();
uint32_t refreshTimer = millis();
uint32_t printTimer = millis();
//********************************************************************
void setup()   
{   
  display.begin( SSD1306_SWITCHCAPVCC );
  dht.begin();
  Wire.begin();
  RTC.begin();
  
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  
  display.setTextSize( 1 );
  display.setTextColor( WHITE );
  display.clearDisplay();
  
  
  // Initialise arrays
  for ( int i = 0; i < MAX; i++ )
  {
    tempArray[ i ] = 0;
    humArray[ i ] = 0;
  }
}
//********************************************************************
void loop() 
{
  DateTime now = RTC.now();
  
  if ( storeDataTimer > millis() )  
    storeDataTimer = millis();
  if ( refreshTimer > millis() )  
    refreshTimer = millis();
  if ( printTimer > millis() )  
    printTimer = millis();  
  
  if ( millis() - storeDataTimer > 60000 ) 
  { 
    storeDataTimer = millis();
    storeTemp();
  }
  
  if ( millis() - printTimer <= 20000 ) 
  {
    if ( millis() - refreshTimer > 1000 ) 
    { 
      refreshTimer = millis();
      
      display.clearDisplay();
      printClock( RTC.now() );
      display.display();
    }  
  }
  if ( millis() - printTimer > 20000 && millis() - printTimer <= 40000 ) 
  {
    if ( millis() - refreshTimer > 1000 ) 
    { 
      refreshTimer = millis();
      
      display.clearDisplay();
      drawTempGraph();
      display.display();
    }  
  }
  if ( millis() - printTimer > 40000 && millis() - printTimer <= 60000 ) 
  {
    if ( millis() - refreshTimer > 1000 ) 
    { 
      refreshTimer = millis();
      
      display.clearDisplay();
      drawHumGraph();
      display.display();
    }  
  }
  if ( millis() - printTimer > 60000 )
    printTimer = millis();
}
//********************************************************************
void printClock( DateTime now )
{
  display.drawCircle( 31,32,31,WHITE );
  display.drawFastVLine( 30, 3, 4, WHITE );
  display.drawFastVLine( 32, 3, 4, WHITE );
  display.drawFastHLine( 57, 32, 4, WHITE );
  display.drawFastVLine( 31, 58, 4, WHITE );
  display.drawFastHLine( 2, 32, 4, WHITE );
  display.drawPixel( 45, 7, WHITE );
  display.drawPixel( 55, 18, WHITE);
  display.drawPixel( 55, 46, WHITE );
  display.drawPixel( 45, 56, WHITE );
  display.drawPixel( 17, 56, WHITE );
  display.drawPixel( 6, 46, WHITE );
  display.drawPixel( 6, 18, WHITE );
  display.drawPixel( 17, 7, WHITE );
  if ( ! RTC.isrunning() ) 
  {
    display.setCursor( 67, 2 );
    display.println( "RTC NOT" );
    display.setCursor( 67, 10 );
    display.println( "running!" );
  }
  else
  {
  drawSeconds( now.second() );
  drawMinutes( now.minute() );
  drawHours( now.hour(), now.minute() );
  printDateTime( RTC.now() );
  }
}
//********************************************************************  
void printDateTime( DateTime now )
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity(); 
  
  display.setCursor( 67, 2 );
  if( now.day() < 10 ) 
    display.print( "0" );
  display.print( now.day(), DEC );
  display.print( '/' );
  if( now.month() < 10 ) 
    display.print( "0" );
  display.print( now.month(), DEC );
  display.print( '/' );
  display.println( now.year(), DEC );
  
    display.setCursor( 67, 10 );
  if ( now.hour() < 10 )
    display.print( '0' );  
  display.print( now.hour(), DEC );
  display.print( ':' );
  if ( now.minute() < 10 )
    display.print('0');
  display.print( now.minute(), DEC );
  display.print( ':' );
  if ( now.second() < 10 )
    display.print( '0' );
  display.println( now.second(), DEC );
  
  if ( isnan( ( float ) temp ) || isnan( ( float ) hum ) ) 
  {
    display.setCursor( 67, 18 );
    display.println( "No" );  
    display.setCursor( 67, 26 );
    display.println( "Temp/Hum!" );
  }
  else
  {
    display.setCursor( 67, 18 );
    display.print( "Temp:" );
    display.print( ( float ) temp, 1 );
    display.println( "C" );
    display.setCursor( 67, 26 );
    display.print( "Hum: " );
    display.print( ( float ) hum, 1 );
    display.println( "%" );
  }
}
//********************************************************************
void drawSeconds( int seconds )
{
  display.drawLine( 31, 32, 31 + 25 * cos( ( 270 + seconds * 6 ) * 3.14159 / 180 ), 
                    32 + 25 * sin( ( 270 + seconds * 6 ) * 3.14159 / 180 ), WHITE );    
}
//********************************************************************
void drawMinutes( int minutes )
{
  display.drawLine( 31, 32, 31 + 20 * cos( ( 270 + minutes * 6 ) * 3.14159 / 180 ), 
                    32 + 20 * sin( ( 270 + minutes * 6 ) * 3.14159 / 180 ), WHITE );    
}
//********************************************************************
void drawHours( int hours, int minutes )
{
  display.drawLine( 31, 32, 31 + 15 * cos( ( 270 + hours % 12 * 30 + minutes * 0.5 ) * 3.14159 / 180 ),
                    32 + 15 * sin( ( 270 + hours % 12 * 30 + minutes * 0.5 ) * 3.14159 / 180 ), WHITE );    
}
//********************************************************************
void storeTemp()
{
  int temp = dht.readTemperature();
  int hum = dht.readHumidity(); 
  static int i = 0;
  if ( isnan( ( int ) temp ) )
    if ( i < MAX )
    {
      tempArray[ i ] = 0;
      humArray[ i ] = 0;
      i++;
    }
    else
    {
      for ( int j = 0; j < MAX - 1; j++ )
      {
        tempArray[ j ] = tempArray[ j + 1 ];
        tempArray[ MAX - 1 ] = 0;
        humArray[ j ] = humArray[ j + 1 ];
        humArray[ MAX - 1 ] = 0; 
      }
    }
  else
  {
    if ( i < MAX )
    {
      tempArray[ i ] = temp;
      humArray[ i ] = hum;
      i++;
    }
    else
    {
      for ( int j = 0; j < MAX - 1; j++ )
      {
        tempArray[ j ] = tempArray[ j + 1 ];
        tempArray[ MAX - 1 ] = temp;
        humArray[ j ] = humArray[ j + 1 ];
        humArray[ MAX - 1 ] = hum; 
      }
    }
  }
}
//********************************************************************
void drawTempGraph()
{
  drawGraph();
  display.setCursor( 9, 54 );
  display.print( "Temp:" );
  display.print( ( float ) dht.readTemperature(), 1 );
  display.println( "C" );
  display.setCursor( 0, 0 );
  display.write( 24 ); 
  display.setCursor( 0, 8 );
  display.print( 'T' ); 
  for (int i = 0; i < MAX; i++ )
    display.drawFastHLine( 128 - MAX * 2 + i * 2, 64 - tempArray[ i ] * 2, 2, WHITE ); 
}
//********************************************************************
void drawHumGraph()
{
  drawGraph();
  display.setCursor( 9, 54 );
  display.print( "Hum: " );
  display.print( ( float ) dht.readHumidity(), 1 );
  display.println( "%" );
  display.setCursor( 0, 0 );
  display.write( 24 ); 
  display.setCursor( 0, 8 );
  display.print( 'H' ); 
  for (int i = 0; i < MAX; i++ )
    display.drawFastHLine( 128 - MAX * 2 + i * 2, 64 - humArray[ i ] / 2, 2, WHITE ); 
}  
//********************************************************************

void drawGraph()
{
  display.drawPixel( 6, 13, WHITE ); 
  display.drawPixel( 6, 23, WHITE ); 
  display.drawPixel( 6, 33, WHITE ); 
  display.drawPixel( 6, 43, WHITE ); 
  display.drawPixel( 6, 53, WHITE ); 
  display.drawPixel( 27, 62, WHITE ); 
  display.drawPixel( 47, 62, WHITE ); 
  display.drawPixel( 67, 62, WHITE ); 
  display.drawPixel( 87, 62, WHITE ); 
  display.drawPixel( 107, 62, WHITE ); 
  display.drawFastVLine( 7, 0, 100, WHITE );
  display.drawFastHLine( 7, 63, 120, WHITE );
}
