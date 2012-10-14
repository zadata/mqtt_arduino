/*
  Turning led on/off and publish photo resistor value by mqtt publish subscribe messages
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "ledControler"
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = { 0x90, 0xA2, 0xDA, 0x04, 0x00, 0xEE };
byte server[] = {132,72,110,213};
byte ip[]     = {132,72,110,214};
// BGU network
//byte server[] = {132,72,110,213};
//byte ip[]     = {132,72,110,214};

#define photoResistorPublishCycle 10000U
#define redLedBlinkCycle 100U
#define temperatureCycle 5000U

unsigned long photoResistorPinLastMillis = 0;
unsigned long redLedBlinkLastMillis = 0;
unsigned long temperatureLastMillis = 0;

boolean lightPinState = false, redLedBlink = false;
int redLedStatus = LOW;

int photoLedPin = 6; // The pin the red led is conected to
int redLedPin = 7; // The pin the red led is conected to
int yellowLedPin = 8; // The pin the yellow led is conected to
int photoResistorPin = 0; // The analog pin the photo resistor is conected to
int temperaturePin = 1;

PubSubClient client(server, 1883, callback);

void setup()
{
  // initialize the digital pin as an output.
  // Pin 7 has an LED connected on most Arduino boards:
  pinMode(redLedPin, OUTPUT); 
  // initialize the digital pin as an output.
  // Pin 8 has an LED connected on most Arduino boards:
  pinMode(yellowLedPin, OUTPUT); 
  // initialize the digital pin as an output.
  // Pin 8 has an LED connected on most Arduino boards:
  pinMode(photoLedPin, OUTPUT); 
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(2000);
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Configure manually:
    Ethernet.begin(mac, ip);
  }
  if (client.connect("ArduinoClient")) 
  {
    client.publish("outTopic","outTopic - hello world");    
    client.subscribe("ledControler");
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{ 
  if (strcmp(topic,"ledControler") == 0)
  {
    String result, responseMessage;
    char charBuf[70];
    for(int index = 0; index<length; index++, payload++)
    {
      result += (char)*payload;  
    }
    responseMessage = "Arduino get message - '"+result+"' in '"+topic+"' topic.";
    responseMessage.toCharArray(charBuf, 70);
    Serial.println(responseMessage);  
    client.publish("Response",charBuf);     
    Serial.print("Message Length: "); 
    Serial.println(length); 
    if(result == "red-on")
    {
       redLedStatus = HIGH;
       redLedBlink = false;
       Serial.println("Turning the redLed on"); 
       digitalWrite(redLedPin, HIGH);   // set the LED on 
    }
    else if (result == "red-off")
    {
       redLedStatus = LOW;
       redLedBlink = false;
       Serial.println("Turning the redLed off"); 
       digitalWrite(redLedPin, LOW);   // set the LED off 
    }
    else if (result == "red-blink")
    {
       Serial.println("Blinking the redLed"); 
       redLedBlink = true;
    }
    else if (result == "yellow-on")
    {
       Serial.println("Turning the yellowLed off"); 
       digitalWrite(yellowLedPin, HIGH);   // set the LED off 
    }
    else if (result == "yellow-off")
    {
       Serial.println("Turning the yellowLed off"); 
       digitalWrite(yellowLedPin, LOW);   // set the LED off 
    }
    else if (result == "photo-on")
    {
       Serial.println("Turning the photoLed off"); 
       digitalWrite(photoLedPin, HIGH);   // set the LED off 
    }
    else if (result == "photo-off")
    {
       Serial.println("Turning the photoLed off"); 
       digitalWrite(photoLedPin, LOW);   // set the LED off 
    }
  } 
}

boolean cycleCheck(unsigned long *lastMillis, unsigned int cycle)
{
  unsigned long currentMillis = millis();
  if(currentMillis - *lastMillis >= cycle)
  {
    *lastMillis = currentMillis;
    return true;
  }
  else
    return false;
}

void loop()
{
  if(cycleCheck(&photoResistorPinLastMillis, photoResistorPublishCycle))
  {
    String photoResistorMessage = "Light level is: ";
    char charBuf[50];
    int lightLevel = analogRead(photoResistorPin);                               
    lightLevel = map(lightLevel, 0, 900, 0, 255); //adjust the value 0 to 900 to 0 to 255
    photoResistorMessage += lightLevel;
    lightLevel = constrain(lightLevel, 0, 255);  //
    analogWrite(photoLedPin, lightLevel);             //
    Serial.println(photoResistorMessage);
    photoResistorMessage.toCharArray(charBuf, 50);
    client.publish("Response",charBuf);
    lightPinState = !lightPinState;
  }
  
  if(cycleCheck(&redLedBlinkLastMillis, redLedBlinkCycle)&&redLedBlink)
  {
    if(redLedStatus==HIGH)
    {
      redLedStatus = LOW;
      digitalWrite(redLedPin, LOW);   // set the LED off 
    }
    else 
    {
      redLedStatus = HIGH;      
      digitalWrite(redLedPin, HIGH);   // set the LED off 
    }
  }
  
  if(cycleCheck(&temperatureLastMillis, temperatureCycle))
  {
    String temperatureMessage = "Temperature: ", temp;
    char charBuf[50];
    float temperature = getVoltage(temperaturePin);  //getting the voltage reading from the temperature sensor
    temperature = (temperature - .5) * 100;
    dtostrf(temperature,1,2,charBuf);   
    Serial.println(temperature);
    temperatureMessage += charBuf;
    temperatureMessage.toCharArray(charBuf, 50);
    client.publish("Response",charBuf);
  }
  client.loop();
}

float getVoltage(int pin)
{
  return (analogRead(pin) * .004882814);
}

