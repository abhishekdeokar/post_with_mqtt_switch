#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WIFI_SSID "OnePlus3"
#define WIFI_PASS "8550912599"

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "abdeokar23"
#define MQTT_PASS "2ea31ca6e6744c5fa7bceb72166f1367"

#define LED_BUILTIN 16
long randNum;

//Set up MQTT and WiFi clients
WiFiClient client1;
Adafruit_MQTT_Client mqtt(&client1, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe swit = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/edgeswitch"); //enter your feed name accordingly

void setup()
{
	Serial.begin(9600);
	randomSeed(analogRead(0));

	//Connect to WiFi
	Serial.print("\n\nConnecting to Wifi... ");
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
	}
	Serial.println("OK!");

	//Subscribe to the onoff feed
	mqtt.subscribe(&swit);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
	bool send_state;
	MQTT_connect();
   
	//Read from our subscription queue until we run out, or
	//wait up to 5 seconds for subscription to update
	Adafruit_MQTT_Subscribe * subscription;
	while ((subscription = mqtt.readSubscription(5000)))
	{
		//If we're in here, a subscription updated...
  
		if (subscription == &swit)
		{
			//Print the new value to the serial monitor
			/*Serial.print("value: ");
			Serial.println((char*) swit.lastread);*/
			do
			{
				randNum = random(20,30);
				Serial.println("MQTT Switch on");
				//Serial.println((char*)swit.lastread);
				digitalWrite(LED_BUILTIN,LOW);
				StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
				JsonObject& JSONencoder = JSONbuffer.createObject(); 
					 
				JSONencoder["sender"] = "d4ee26eee15148ee92c6cd394edd974e";
				JSONencoder["recipient"] = "some-other-address";
				JSONencoder["data"] = randNum;    
				   
				char JSONmessageBuffer[300];
				JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
				Serial.println(JSONmessageBuffer);
					  
				HTTPClient http;    //Declare object of class HTTPClient
					  
				http.begin("http://192.168.43.3:5000/upload/new");      //Specify request destination
				http.addHeader("Content-Type", "application/json");  //Specify content-type header
					   
				int httpCode = http.POST(JSONmessageBuffer);   //Send the request
				http.writeToStream(&Serial);
				String payload = http.getString();                                        //Get the response payload
					   
				Serial.println(httpCode);   //Print HTTP return code
				Serial.println(payload);    //Print request response payload

				if(strcmp((char *)swit.lastread,"OFF")==0)
				Serial.println("stop received");
				delay(5000);
				http.end();  //Close connection
					  
			}while(reader());
		}
	}

	// ping the server to keep the mqtt connection alive
	if (!mqtt.ping())
	{
	mqtt.disconnect();
	}
}

bool reader()
{
	bool send_state;
	Adafruit_MQTT_Subscribe * subscription;
	while ((subscription = mqtt.readSubscription(5000)))
	{
		//If we're in here, a subscription updated...
    	
		if (subscription == &swit)
		{
			if(strcmp((char *)swit.lastread,"ON")==0)
				send_state = true;
			else{
      send_state = false;
      Serial.println("MQTT Switch off");
      digitalWrite(LED_BUILTIN,HIGH);
			}
				
		}
	}
	return send_state;
}

void MQTT_connect() 
{
	int8_t ret;
	// Stop if already connected.
	if (mqtt.connected()) 
	{
		return;
	}

	Serial.print("Connecting to MQTT... ");

	uint8_t retries = 3;
	while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
	{ 
		Serial.println(mqtt.connectErrorString(ret));
		Serial.println("Retrying MQTT connection in 5 seconds...");
		mqtt.disconnect();
		delay(5000);  // wait 5 seconds
		retries--;
		if (retries == 0) 
		{
			// basically die and wait for WDT to reset me
			while (1);
		}
	}
	Serial.println("MQTT Connected!");
}