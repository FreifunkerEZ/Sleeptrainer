//wemos D1 mini

//#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <math.h>
#define NUM_LEDS 64
#define DATA_PIN 4 // https://escapequotes.net/esp8266-wemos-d1-mini-pins-and-diagram/
#define __max(a,b) ((a)>(b)?(a):(b)) 

CRGB leds[NUM_LEDS];


#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h> 

#include <Credentials.h>
const char *ssidMinor     = SSID_MINOR;
const char *passwordMinor = PASSWORD_MINOR;

const char *ssidMajor     = SSID_MAJOR;
const char *passwordMajor = PASSWORD_MAJOR;

WiFiUDP ntpUDP;
WiFiServer server(80);

// Variable to store the HTTP request
String header;
word WINTERTIME = 1;  // set 1 or 0

NTPClient timeClient(
  ntpUDP, 
  "europe.pool.ntp.org", 
  3600 + WINTERTIME * 3600, // UTC-offset
  3600 * 1000               // update frequency in ms.
);

const int ledPin = LED_BUILTIN;// the number of the LED pin
const byte INIT  = 0;
const byte SLEEP = 1;
const byte SOON  = 2;
const byte WAKE  = 3;
const byte KIGA  = 4;


// times are in "minutes of the day"
word wakeTime  =  7 * 60;
word soonTime  = wakeTime - 15;
word kigaTime  =  7 * 60 + 50;
word playTime  = 15 * 60;
word sleepTime = 19 * 60;

////////////////////////////////////
// C L O C K 
////////////////////////////////////



void setClockStatus(byte newState) {
/*  blank pic
    boolean pic[NUM_LEDS] = {
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
    };
*/
  boolean closedEyes[NUM_LEDS] = {
      0,0,0,0,0,0,0,0,
      1,0,0,1,1,0,0,1,
      0,1,1,0,0,1,1,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      1,0,0,0,0,0,0,1,
      0,1,1,1,1,1,1,0,
      0,0,0,0,0,0,0,0,
    };
  boolean sun[NUM_LEDS] = {
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      1,0,0,0,1,0,0,0,
      0,1,0,0,1,0,0,1,
      1,0,1,0,1,0,1,0,
      0,1,1,1,1,1,0,1,
      1,1,1,1,1,1,1,0,
      1,1,1,1,1,1,1,1,
    };
  boolean happy[NUM_LEDS] = {
      0,1,1,0,0,1,1,0,
      1,0,0,1,1,0,0,1,
      1,1,1,1,1,1,1,1,
      0,0,0,0,0,0,0,0,
      1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,
      0,1,1,1,1,1,1,0,
      0,0,1,1,1,1,0,0,
    };
    boolean bag[NUM_LEDS] = {
      0,0,0,0,0,0,0,0,
      0,0,1,1,1,1,0,0,
      0,0,1,0,0,1,0,0,
      1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,
    };


  if (newState == SLEEP) {
    FastLED.setBrightness(1);
    displayPix(closedEyes, CRGB::Red, CRGB::Black, false);
  }
  if (newState == SOON) {
    CHSV yellow( HUE_YELLOW , 255, 255);
    FastLED.setBrightness(10);
//    startFade(closedEyes, sun, yellow);
//    startFade(sun, closedEyes, yellow);
    displayPix(sun, CRGB::Yellow, CRGB::Black, false);
  }
  if (newState == WAKE) {
    FastLED.setBrightness(25);
    displayPix(happy, CRGB::Green, CRGB::Black, false);
  }
  if (newState == KIGA) {
    FastLED.setBrightness(25);
    displayPix(bag, CRGB::Blue, CRGB::Black, false);
  }
}


void checkClockState() {
  timeClient.update();

  word minuteOfDay = (timeClient.getEpochTime() / 60) % (24 * 60);
  byte desiredStatus = SLEEP;
  if (minuteOfDay > soonTime) {
    desiredStatus = SOON;
  }
  if (minuteOfDay > wakeTime) {
    desiredStatus = WAKE;
  }
  if (minuteOfDay > kigaTime) {
    desiredStatus = KIGA;
  }
  if (minuteOfDay > playTime) {
    desiredStatus = WAKE;
  }
  if (minuteOfDay > sleepTime) {
    desiredStatus = SLEEP;
  }
  setClockStatus(desiredStatus);
}

////////////////////////////////////
// L E D   S T U F F 
////////////////////////////////////

void all(CRGB color) {
  for(int dot = 0; dot < NUM_LEDS; dot++) { 
      leds[dot] = color;
  }
  FastLED.show();
}

byte fadeUp = 0;
byte fadeDown = 255;
byte curStep = 0;
byte maxStep = 100;
byte durationSec = 3;

//start a fade between two boolean pic-arrays.
//color is CHSV
void startFade(boolean picStart[], boolean picEnd[], CHSV color) {
  //init vars
  fadeUp = 0;
  fadeDown = 255;
  curStep = 0;

  /*
  //init to the correct color before starting to mess with brightness.
  for(int dot = 0; dot < NUM_LEDS; dot++) { 
      leds[dot] = color;
  }
  */
  
  //and GO!
  while (curStep < maxStep) 
    _fadePix(picStart,picEnd, color);
}

void _fadePix(boolean picStart[], boolean picEnd[], CHSV color) {
  byte val = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (picStart[i] == 0 and picEnd[i] == 0) val = 0;
    if (picStart[i] == 1 and picEnd[i] == 1) val = __max(fadeUp, fadeDown);
    if (picStart[i] == 0 and picEnd[i] == 1) val = fadeUp;
    if (picStart[i] == 1 and picEnd[i] == 0) val = fadeDown;
    CHSV pixel = color;
    pixel.val = val;
  hsv2rgb_rainbow(pixel, leds[i]);
  //hsv2rgb_spectrum(pixel, leds[i]);
  }
  snakeify(false);
  FastLED.show();
  int wait = (durationSec * 1000) / maxStep;
//Serial.println(wait);
  delay(wait);

  // calc nex step
  curStep += 1;
  float percent = ((float) curStep / (float) maxStep);
  float up = 255.0 * percent;
/*Serial.print("next Step "); Serial.print(curStep);
  Serial.print(" of "); Serial.print(maxStep);
  Serial.print(" % "); Serial.print(percent);
  Serial.print(" up "); Serial.print(up);
  Serial.print(" fadeUp "); Serial.print(fadeUp);
  Serial.print(" fadeDown "); Serial.println(fadeDown);       */
  fadeUp = round(up);
  fadeDown = 255 - fadeUp;
}


// turn the leds-array of N x M pixels that looks normal for humans
// into a snakified version, which has the values in the correct places
// for a snaked matrix to display them.
// use this after setting the matrix, before displaying.
void snakeify(bool mirror) {
  CRGB park;
  int lineCount = sqrt(NUM_LEDS);
  bool snakeNow = !mirror;
  for (int i=0; i < NUM_LEDS; i++) {
    if (snakeNow) {
      // calculate where we are and which places to swap
      int currentRow = ((i+1) / lineCount) + 1;
      int goBack = (i+1) % lineCount;
      if (goBack == 0)  goBack = 2 * lineCount;
      int s = currentRow * lineCount - goBack;

      //swap only first half with last half places. ignore rest.
      if (goBack <= lineCount / 2) {
        park = leds[i];
        leds[i] = leds[s];
        leds[s] = park;
      }
    }
    
    //at the end of the row, toggle snakeify
    if ((i+1) % lineCount == 0) snakeNow = !snakeNow;
  }
}


void blueDot() {
  for(int dot = 0; dot < NUM_LEDS; dot++) { 
      leds[dot] = CRGB::Blue;
      FastLED.show();
      // clear this led for the next time around the loop
      leds[dot] = CRGB::Black;
      delay(20);
  }
}

void displayPix(boolean pic[],CRGB colorOn, CRGB colorOff, bool mirror) {
  for(int dot = 0; dot < NUM_LEDS; dot++) {
    if (pic[dot] == 0) {
      leds[dot] = colorOff;
    }
    else {
      leds[dot] = colorOn;
    }
  }
  snakeify(mirror);
  FastLED.show();
    
}

////////////////////////////////////
// H T T P   S E R V E R
////////////////////////////////////

void process_http_request(WiFiClient client) {
  Serial.println("New Client.");          // print a message out in the serial port
  int currentLine = 0;                // make a String to hold incoming data from the client
  while (client.connected()) {            // loop while the client's connected
    if (client.available()) {             // if there's bytes to read from the client,
      char c = client.read();             // read a byte, then
      Serial.write(c);                    // print it out the serial monitor
      header += c;
      if (c == '\n') {                    // if the byte is a newline character
        // if the current line is blank, you got two newline characters in a row.
        // that's the end of the client HTTP request, so send a response:
        if (currentLine == 0) {
          parse_http_header();
          
          // Display the HTML web page
          print_http_response(client);
          // Break out of the while loop
          break;
        } else { // if you got a newline, then clear currentLine
          currentLine = 0;
        }
      } else if (c != '\r') {  // if you got anything else but a carriage return character,
        currentLine++;      // add it to the end of the currentLine
      }
    }
  }
  // Clear the header variable
  header = "";
  // Close the connection
  client.stop();
  Serial.println("Client disconnected.");
  Serial.println("");
}

void parse_http_header() {
  // turns the GPIOs on and off
  if (header.indexOf("GET /WAKE/earlier") >= 0) {
    Serial.println("Waking earlier");
    wakeTime -= 30;
    soonTime -= 30;
  } else if (header.indexOf("GET /WAKE/later") >= 0) {
    Serial.println("waking later");
    wakeTime += 30;
    soonTime += 30;
  } else if (header.indexOf("GET /SLEEP/earlier") >= 0) {
    Serial.println("sleep earlier");
    sleepTime -= 30;
  } else if (header.indexOf("GET /SLEEP/later") >= 0) {
    Serial.println("sleep later");
    sleepTime += 30;
  }
}

void print_http_response(WiFiClient client) {
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  
  client.println("<!DOCTYPE html><html><head>");
//  client.println(F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
//  client.println(F("<link rel=\"icon\" href=\"data:,\">"));
  /*
  // CSS to style the on/off buttons 
  // Feel free to change the background-color and font-size attributes to fit your preferences
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: #77878A;}</style>");
  */
  client.println("</head>");
  
  // Web Page Heading
  client.println("<body><h1>Sleeptrainer Web Server</h1>");
  
  // Display current state
  client.println("<p>Current time is " + timeClient.getFormattedTime() + "</p>");
  
  client.println("<p>SOON = " + formatMoD(soonTime) + "</p>");
  client.println("<p>WAKE = " + formatMoD(wakeTime) + "</p>");
  client.println("<p><a href=\"/WAKE/earlier\"> <button class=\"button\">earlier</button></a>");
  client.println("<a href=\"/WAKE/later\"><button class=\"button\">later</button></a></p>");

  client.println("<p>KIGA = " + formatMoD(kigaTime) + "</p>");

  client.println("<p>SLEEP = " + formatMoD(sleepTime) + "</p>");
  client.println("<p><a href=\"/SLEEP/earlier\"> <button class=\"button\">earlier</button></a>");
  client.println("<a href=\"/SLEEP/later\"><button class=\"button\">later</button></a></p>");
     
  client.println("</body></html>");
  
  // The HTTP response ends with another blank line
  client.println();  
}

String formatMoD(word minutesOfTheDay) {
  byte hours = 0;
  byte minutes = 0;
  hours = minutesOfTheDay / 60;
  minutes = minutesOfTheDay - hours * 60;
  return (String) hours + ":" + (String) minutes;
}


////////////////////////////////////
// S E T U P S 
////////////////////////////////////

void wifisetup() {
  // try emergency debuggingn wifi first for short time
  byte minorWaitSec = 15;
  Serial.println ( F("WIFI setup: ") );
  Serial.print   ( F("WIFI init Minor. Timeout in secs: ") ); Serial.println(minorWaitSec);
  Serial.println ( ssidMinor );
  Serial.println ( passwordMinor );
  WiFi.begin(ssidMinor, passwordMinor);
  for (int i = 0; i < minorWaitSec; i++) {
    leds[1] = CRGB::Yellow; FastLED.show();
    delay ( 500 );
    leds[1] = CRGB::Red; FastLED.show();
    delay ( 500 );
    Serial.print('.');
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print( F(" -> WIFI minor connected. IP address ") );
      Serial.println(WiFi.localIP());
      return;
    }
  }

  // still here? try next.
  Serial.println (F( "No minor WIFI. Now trying Major: " ));
  Serial.println ( ssidMajor );
  Serial.println ( passwordMajor );
  WiFi.begin(ssidMajor, passwordMajor);
  while ( WiFi.status() != WL_CONNECTED ) {
    leds[2] = CRGB::Yellow; FastLED.show();
    delay ( 500 );
    Serial.print ( "." );
    leds[2] = CRGB::Red; FastLED.show();
    delay ( 500 );
  }
  Serial.print( F(" -> WIFI major connected. IP address ") );
  Serial.println(WiFi.localIP());
}

void ledTest() {
  int ms = 3000;
  blueDot();
  
  setClockStatus(SLEEP);
  delay(ms);
  setClockStatus(SOON);
  delay(ms);
  setClockStatus(WAKE);
  delay(ms);
  setClockStatus(KIGA);
  delay(ms);

  all(CRGB::Black);
}


void setup() {
  Serial.begin(115200);  
  Serial.println ( F("LED init ") );

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); 
  FastLED.setBrightness(25);
  
  Serial.println ( F("LED test") );
  ledTest();
  Serial.println ( F("LED test done") );
  leds[0] = CRGB::Yellow; FastLED.show();
   
  
  wifisetup();

  Serial.println(F("Starting NTP-client. "));
  timeClient.begin();
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  Serial.println(F("Done."));
  
  Serial.print(F("Starting Web-server. "));
  server.begin();
  Serial.println(F("Done."));

  if (!MDNS.begin("sleep")) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started for sleep.local");
 
  Serial.println('clock boot complete');
}

void loop() {
  checkClockState();
  delay(10);
  WiFiClient client = server.available();

  //https://randomnerdtutorials.com/esp8266-web-server-with-arduino-ide/
  if (client) {                             // If a new client connects,
    process_http_request(client);
  }
  
}
