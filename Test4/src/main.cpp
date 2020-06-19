#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Adafruit_Keypad.h"
#include <NewPing.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <RtcDS3231.h>
#include <Buzzer.h>


	byte gradus[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

   ///////////////////////////////////////////////////////////////////////////////////////////	MQTT

   Adafruit_BME280 bme; // use I2C interface
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();

// the on off button feed turns this LIGHT_PIN on/off
#define LIGHT_PIN1 D5  
#define LIGHT_PIN2 D6
#define LIGHT_PIN3 D7  
#define LIGHT_PIN4 D8 
unsigned long lastTame;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "ASUS_MI"
#define WLAN_PASS       ""

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish valLight1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lampa-1");
Adafruit_MQTT_Publish valLight2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lampa-2");
Adafruit_MQTT_Publish valLight3 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lampa-3");
Adafruit_MQTT_Publish valLight4 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lampa-4");
Adafruit_MQTT_Publish valLight5 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish valLight6 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidit");


/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lampa-1");
Adafruit_MQTT_Subscribe light2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lampa-2");
Adafruit_MQTT_Subscribe light3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lampa-3");
Adafruit_MQTT_Subscribe light4 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lampa-4");
Adafruit_MQTT_Subscribe light5 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/disable-all");

/*************************** Sketch Code ************************************/
int Push = 1;
// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
    
   ///////////////////////////////////////////////////////////////////////////////////////////	MQTT

RtcDS3231<TwoWire> rtcObject(Wire); //Uncomment for version 2.0.0 of the rtc library
char keypressed;

/*--------------------------CONSTANTS-------------------------*/
Buzzer buzzer(D3);  //Buzzer/small speaker
const int doorMagSen = A0;    //Door magnetic sensor
const int windowMagSen = A0; //Window magnetic sensors

LiquidCrystal_I2C lcd(0x27,20,4); //lcd ((RS, E, D4, D5, D6, D7)
NewPing sonar(A0,A0,2000); // Trig Echo Max distance
/*--------------------------VARIABLES------------------------*/
String password= "1234" ; //Variable to store the current password
String tempPassword="";	//Variable to store the input password
int doublecheck;
boolean armed = false;	//Variable for system state (armed:true / unarmed:false)
boolean input_pass;		//Variable for input password (correct:true / wrong:false)
boolean storedPassword = true;
boolean changedPassword = false;
boolean checkPassword = false;
int distance;
 String esid;
int i = 1; //variable to index an array
char Sicrity =  'R' ;
char flag =  'G' ;
int T =1;
int T1 = 1;
int dT = 0;
 sensors_event_t temp_event, pressure_event, humidity_event;

/**********************************************************************************/
void test();
void systemIsArmed();
void systemIsUnarmed();
void changePassword();
void alarmFunction();
void newPassword();	
void unlockPassword();
void dataTemp ();
void alarmFunctionFire();

const uint8_t     PIN_direction_TX_RX = D0;
void setup() {

  ///////////////////////////////////////////////////////////////////////////////////////////	MQTT
    bme.begin();
	rtcObject.Begin();  // start RtcDS3231
	buzzer.begin(10);

	RtcDateTime currentTime = RtcDateTime(__DATE__, __TIME__);
  rtcObject.SetDateTime(currentTime); //configure the RTC with object
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  
  // Setup MQTT subscription for onoff.
  mqtt.subscribe(&light1);
  mqtt.subscribe(&light2);
  mqtt.subscribe(&light3);
  mqtt.subscribe(&light4);
  mqtt.subscribe(&light5);
  ///////////////////////////////////////////////////////////////////////////////////////////	MQTT


	
lcd.init();   
pinMode(PIN_direction_TX_RX,   OUTPUT);    // устанавливаем режим работы вывода PIN_direction_TX_RX, как "выход"
  digitalWrite(PIN_direction_TX_RX, LOW); 
  
 //EEPROM.begin(512);
lcd.setCursor(2,2);
lcd.print("Security system");
Serial.begin(9600);

	lcd.begin(20,4 );
								//Setup the LCD's number of columns and rows 
	//pinMode(doorMagSen,INPUT_PULLUP);				//Set all magnetic sensors as input withn internal pullup resistor
	//pinMode(windowMagSen,INPUT_PULLUP);
}

void loop() { //Main loop

	 MQTT_connect(); 

	if (armed){
		
		systemIsArmed(); 	//Run function to activate the system
		 digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
          delay(1);
           Serial.write(Sicrity);                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
          delay(1);
          digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
          delay(1);
	    }
	     else if (!armed){

		
		systemIsUnarmed(); 	//Run fuction to de activate the system
		 
	}
}

/********************************FUNCTIONS************************************/

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       mqtt.disconnect();
       //delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         while (1);
       }
  }
   if(Push==1)
  {
    valLight1.publish("OFF");
    valLight2.publish("OFF");
    valLight3.publish("OFF");
    valLight4.publish("OFF");
    Push++;
  }
}

//While system is unarmed
void systemIsUnarmed(){
	int screenMsg=0;
	lcd.clear();									//Clear lcd
	unsigned long previousMillis = 0;        		//To make a delay by using millis() function
	const long interval = 5000;						//delay will be 5 sec. 
													//every "page"-msg of lcd will change every 5 sec
	while(!armed){		
		            if (millis() - lastTame > 1000){
        lastTame = millis();
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  // MQTT_connect();


 //sensors_event_t temp_event, pressure_event, humidity_event;
  bme_temp->getEvent(&temp_event);
  bme_pressure->getEvent(&pressure_event);
  bme_humidity->getEvent(&humidity_event);


    valLight5.publish(temp_event.temperature);
    valLight6.publish(humidity_event.relative_humidity);
 

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {  
    
    // Check if its the onoff button feed Light1
    if (subscription == &light1) {
      
      if (strcmp((char *)light1.lastread, "OFF") == 0) {
        pinMode(LIGHT_PIN1, OUTPUT);
        digitalWrite(LIGHT_PIN1, HIGH); 
      }
      if (strcmp((char *)light1.lastread, "ON") == 0) {
        pinMode(LIGHT_PIN1, OUTPUT);
        digitalWrite(LIGHT_PIN1, LOW); 
      }
    }

    // Check if its the onoff button feed Light2
    if (subscription == &light2) {
      
      if (strcmp((char *)light2.lastread, "OFF") == 0) {
        pinMode(LIGHT_PIN2, OUTPUT);
        digitalWrite(LIGHT_PIN2, HIGH); 
      }
      if (strcmp((char *)light2.lastread, "ON") == 0) {
        pinMode(LIGHT_PIN2, OUTPUT);
        digitalWrite(LIGHT_PIN2, LOW); 
      }
    }

    // Check if its the onoff button feed Light3
    if (subscription == &light3) {
      
      if (strcmp((char *)light3.lastread, "OFF") == 0) {
        pinMode(LIGHT_PIN3, OUTPUT);
        digitalWrite(LIGHT_PIN3,HIGH); 
      }
      if (strcmp((char *)light3.lastread, "ON") == 0) {
        pinMode(LIGHT_PIN3, OUTPUT);
        digitalWrite(LIGHT_PIN3, LOW); 
      }
    }

    // Check if its the onoff button feed Light4
    if (subscription == &light4) {
      
      if (strcmp((char *)light4.lastread, "OFF") == 0) {
        pinMode(LIGHT_PIN4, OUTPUT);
        digitalWrite(LIGHT_PIN4, HIGH); 
      }
      if (strcmp((char *)light4.lastread, "ON") == 0) {
        pinMode(LIGHT_PIN4, OUTPUT);
        digitalWrite(LIGHT_PIN4,LOW); 
      }
    }

    if (subscription == &light5){
      if (strcmp((char *)light5.lastread, "OFF") == 0) {
        valLight1.publish("OFF");
        digitalWrite(LIGHT_PIN1,HIGH); 
        valLight2.publish("OFF");
        digitalWrite(LIGHT_PIN2, HIGH); 
        valLight3.publish("OFF");
        digitalWrite(LIGHT_PIN3, HIGH); 
        valLight4.publish("OFF");
        digitalWrite(LIGHT_PIN4, HIGH); 
      }
    }
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
 }
		yield();							//While system is unarmed do...
		if( T1 == 1  ){  
           digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
           delay(1);
          Serial.write('T');                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
           delay(1);
          digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
          delay(1);
		  T1++;
             }
		unsigned long currentMillis = millis();		//Store the current run-time of the system (millis function)
		T = 1; // create members nano
		
  		if (currentMillis - previousMillis >= interval) {
    		previousMillis = currentMillis;
			dataTemp ();
  			if(screenMsg==0){						//First page-message of lcd
				lcd.setCursor(0,2);
				lcd.print("  SYSTEM ALARM OFF");
				lcd.setCursor(0,3);
				lcd.print("  ----------------");
				screenMsg=1;
  			}
  			else{									//Second page-message of lcd
  				lcd.setCursor(0,2);
  				lcd.print("A. to arm          ");
  				lcd.setCursor(0,3);
				lcd.print("B. to change pass  ");
				screenMsg=0;
  			}
  		}
		keypressed = Serial.read(); 				//Read the pressed button
		if (keypressed =='A'){						//If A is pressed, activate the system
			buzzer.sound(500, 200); 
			systemIsArmed();						//by calling the systemIsArmed function
		}
		else if (keypressed =='B'){//If B is pressed, change current password
			doublecheck=0;
			buzzer.sound(500, 200); 
			storedPassword=false;
			if(!changedPassword){					//By calling the changePassword function
				changePassword();
			}
		}
		if (keypressed ==  'F' ){
			alarmFunctionFire(); //Call alarm!
		}
	}
}
 
//While system is armed
void systemIsArmed(){								
	lcd.clear();
	int count=10;								//Count 10sec before activate the system
	unsigned long previousMillis = 0;        	
	const long interval = 1000;	
	while(!armed){	
		yield();	
		distance = sonar.ping_cm(); //Store distance from sensor only for first time
		//While system is unarmed - for 10sed do...
		lcd.setCursor(0,2);
		lcd.print(" SYSTEM WILL BE ");			//Print message to lcd with 10 sec timer
		lcd.setCursor(0,3);
		lcd.print("   ARMED IN ");
		unsigned long currentMillis = millis();
  		if (currentMillis - previousMillis >= interval) {
    		previousMillis = currentMillis;
    		//Screen counter 10sec
    		if (count>1){
				count--;						//Countdown timer
    		}
    		else{
    			armed=true;						//Activate the system!
				break;
    		}
  		}
		lcd.setCursor(12,3);
		lcd.print(count);						//show the timer at lcd second line 13 possition 
	}
	while (armed){	
           if( T== 1  ){  
			                     // если нажимается кнопка (KEY_DOWN - нажимается, KEY_UP - отпускается, KEY_PRESS - удерживается), то ...
		   
           digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
           delay(1);
          Serial.write(flag);                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
           delay(1);
          digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
          delay(1);
		  T++;
             }
			 T1 = 1;
		yield();						//While system is armed do...
		delay(1);
		lcd.setCursor(0,2);
		lcd.print("SYSTEM IS ARMED!");
		lcd.setCursor(0,3);
		lcd.print("----------------");
		//int door = digitalRead(doorMagSen);		//Read magnetic sensros and ultrasonic sensor
		//int window = digitalRead(windowMagSen);
		//int curr_distanse = sonar.ping_cm();
         digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
          delay(1);
		char n = Serial.read();
		if (n == 'S' ){
			alarmFunction(); //Call alarm!
		} 
		if (n == 'L'){
			unlockPassword(); //Disarm the system with correct password
		} 
		if (n == 'F' ){
			alarmFunctionFire(); //Call alarm!
		} 
	}
}
//Door is opend, unlcok the system!
void unlockPassword() {
	int count=21;							//20 sec for alarm!
	retry: 									//label for goto, retry in case of wrong password
    tempPassword="";						//reset temp password (typing...)
	lcd.clear();							//clear lcd
	i=6;									//variable to put * while typing pass
	unsigned long previousMillis = 0;       
	const long interval = 1000;
	boolean buzzerState = false;			//variable to help us make  a beep tone
	while(!checkPassword){	
		yield();				//While waiting for correct password do...
		unsigned long currentMillis = millis();
  		if (currentMillis - previousMillis >= interval) {
    		previousMillis = currentMillis;	//play beep tone every 1 sec
    		if (!buzzerState){
    			buzzer.sound(196, 700);  
    			buzzerState=true;
    		}
    		else{
    			buzzer.sound(0, 1024); 
    			buzzerState=false;
    		}
    		if (count>0){    				//Screen counter 20sec
				count--;
    		}
    		else{
    			alarmFunction();			//Times is up, ALARM!
    			break;
    		}
  		}
		keypressed =Serial.read(); 
		lcd.setCursor(0,2);
		lcd.print("ALARM IN: "); 
		//For screen counter - 20sec
		if (count>=10){
			lcd.setCursor(14,2);
			lcd.print(count);				//print countdown timer at lcd
		}
		else{								//catch '0'bellow 10 (eg 09)
			lcd.setCursor(14,2);
			lcd.print(" ");
			lcd.print(count);
		}
		lcd.setCursor(0,2);
		lcd.print("PASS>");
		if (keypressed ){			//Accept only numbers and * from keypad
			if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
			keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
			keypressed == '8' || keypressed == '9' ){
				tempPassword += keypressed;
				lcd.setCursor(i,3);
				lcd.print("*");				//Put * on lcd
				i++;
				buzzer.sound(500, 200); 		//Button tone
			}
			else if (keypressed == '*'){	//Check for password
				if (password==tempPassword){//If it's correct unarmed the system
					armed=false;
					buzzer.sound(700, 500); 
					break;
				}
				else{						//if it's false, retry
					tempPassword="";
					buzzer.sound(200, 200); 
					delay(300);
					buzzer.sound(200, 200); 
					delay(300);
					goto retry;
				}
			}
		}
	}		
}

//Alarm
void alarmFunction(){
	retry: //label for goto
	tempPassword="";
	lcd.clear();
	i=6;
	unsigned long previousMillis = 0;       
	const long interval = 500;
	boolean buzzerState = false;
	while(!checkPassword){	
		yield();				//Waiting for password to deactivate the alarm...
		unsigned long currentMillis = millis();
  		if (currentMillis - previousMillis >= interval) {
    		previousMillis = currentMillis;	//Play a beep tone every 0.5 second
    		if (!buzzerState){
				buzzer.sound(196, 700);

    			buzzerState=true;
    		}
    		else{
				buzzer.sound(0, 700);
    			buzzerState=false;
    		}
  		}
		keypressed = Serial.read(); 
		lcd.setCursor(0,2);
		lcd.print("  !!! ALARM !!! "); 
		lcd.setCursor(0,3);
		lcd.print("PASS>");
		if (keypressed ){			//Accept only numbers and *
			if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
			keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
			keypressed == '8' || keypressed == '9' ){
				tempPassword += keypressed;
				lcd.setCursor(i,3);
				lcd.print("*");
				i++;
			}
			else if (keypressed == '*'){
				if (password==tempPassword){
					armed=false;
					buzzer.sound(700, 500);
					break;
				}
				else{
					tempPassword="";
					buzzer.sound(200, 200);
					delay(300);
					buzzer.sound(200, 200);
					delay(300);
					goto retry;
				}
			}
		}
	}	
}

void alarmFunctionFire(){
	retry: //label for goto
	tempPassword="";
	lcd.clear();
	i=6;
	unsigned long previousMillis = 0;       
	const long interval = 500;
	boolean buzzerState = false;
	while(!checkPassword){	
		yield();				//Waiting for password to deactivate the alarm...
		unsigned long currentMillis = millis();
  		if (currentMillis - previousMillis >= interval) {
    		previousMillis = currentMillis;	//Play a beep tone every 0.5 second
    		if (!buzzerState){
				buzzer.sound(196, 700);

    			buzzerState=true;
    		}
    		else{
				buzzer.sound(0, 700);
    			buzzerState=false;
    		}
  		}
		keypressed = Serial.read(); 
		lcd.setCursor(0,2);
		lcd.print("  !!! ALARM FIRE !!! "); 
		lcd.setCursor(0,3);
		lcd.print("PASS>");
		if (keypressed ){			//Accept only numbers and *
			if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
			keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
			keypressed == '8' || keypressed == '9' ){
				tempPassword += keypressed;
				lcd.setCursor(i,3);
				lcd.print("*");
				i++;
			}
			else if (keypressed == '*'){
				if (password==tempPassword){
					armed=false;
					buzzer.sound(700, 500);
					Serial.write('T'); 
					digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
                        delay(1);
                       Serial.write('T');                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
                       delay(1);
                      digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
                       delay(1);
					    
					break;
				}
				else{
					tempPassword="";
					buzzer.sound(200, 200);
					delay(300);
					buzzer.sound(200, 200);
					delay(300);
					goto retry;
				}
			}
		}
	}	
}

//Change current password
void changePassword(){
	retry: //label for goto
	tempPassword="";
	lcd.clear();
	i=1;
	while(!changedPassword){	
		yield();			//Waiting for current password
		keypressed = Serial.read(); 		//Read pressed keys
		lcd.setCursor(0,2);
		lcd.print("CURRENT PASSWORD");
		lcd.setCursor(0,3);
		lcd.print(">");
		if (keypressed ){
			if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
			keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
			keypressed == '8' || keypressed == '9' ){
				tempPassword += keypressed;
				lcd.setCursor(i,3);
				lcd.print("*");
				i++;
				//tone(buzzer,800,200);	
				buzzer.sound(800, 200);			
			}
			else if (keypressed=='#'){
				break;
			}
			else if (keypressed == '*'){
				i=1;
				if (password==tempPassword){ // 2580 - master password will be same time 
					storedPassword=false;
					buzzer.sound(500, 200);
					newPassword();					//Password is corrent, so call the newPassword function
					break;
				}
				else{								//Try again
					tempPassword="";
					buzzer.sound(500, 200);
					delay(300);
					buzzer.sound(500, 200);
					delay(300);
					goto retry;
				}
			}
		}
	}
}
String firstpass;
//Setup new password
void newPassword(){
	tempPassword="";
	changedPassword=false;
	lcd.clear();
	i=1;
	while(!storedPassword){
		yield();
		keypressed = Serial.read(); 	//Read pressed keys
		if (doublecheck==0){
			lcd.setCursor(0,2);
			lcd.print("SET NEW PASSWORD");
			lcd.setCursor(0,3);
			lcd.print(">");
		}
		else{
			lcd.setCursor(0,2);
			lcd.print("One more time...");
			lcd.setCursor(0,3);
			lcd.print(">");
		}
		if (keypressed ){
			if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
			keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
			keypressed == '8' || keypressed == '9' ){
				tempPassword += keypressed;
				lcd.setCursor(i,3);
				lcd.print("*");
				i++;
				buzzer.sound(800, 200);
			}
			else if (keypressed=='#'){
				break;
			}
			else if (keypressed == '*'){
				if (doublecheck == 0){
					firstpass=tempPassword;
					doublecheck=1;
					newPassword();
				}
				if (doublecheck==1){
					doublecheck=0;
					if (firstpass==tempPassword){
						i=1;
						firstpass="";
						password = tempPassword; // New password saved
						/*String qsid = password ;
						int charLength=qsid.length();
						EEPROM.begin(512);
						Serial.println("writing eeprom ssid:");
                         for (int i = 0; i < qsid.length(); ++i)
                         {
                          EEPROM.write(i, qsid[i]);
                           Serial.print("Wrote: ");
                            Serial.println(qsid[i]);
                         
						 }
						 Serial.println("Reading EEPROM ssid");
                         
						  for (int i = 0; i < charLength; ++i)
                          {
                            esid += char(EEPROM.read(i));
                         }
						 EEPROM.end();
                              esid.trim();
                               Serial.println(esid.length());
                                Serial.print("SSID: ");
                               Serial.println(esid);
							
						    password = esid ;
							*/
								
						tempPassword="";//erase temp password
						lcd.setCursor(0,2);
						lcd.print("PASSWORD CHANGED");
						lcd.setCursor(0,3);
						lcd.print("----------------");
  						storedPassword=true;
						   buzzer.sound(500, 400);
					     
  						delay(2000);
  						lcd.clear();
  						break;
					}
					else{
						firstpass="";
						newPassword();
					}
				}
			}	
		}
	}
}

void dataTemp (){
  RtcDateTime currentTime = rtcObject.GetDateTime();    //get the time from the RTC
  char str[20];   //declare a string as an array of chars
  sprintf(str, "%d/%d/%d       %d:%d",     //%d allows to print an integer to the string
          currentTime.Day(),   //get year method
          currentTime.Month(),  //get month method
          currentTime.Year(),    //get day method
          currentTime.Hour(),   //get hour method
          currentTime.Minute() //get minute method
         );
     while (dT == 0)
	 {
     lcd.setCursor(0,0);
     lcd.print(str); //print the string to the serial port
	 lcd.setCursor(0,1);
	 lcd.print(temp_event.temperature,1);
	 lcd.print((char)223);
	 lcd.print("C");
	 lcd.setCursor(8,1);
	 lcd.print(humidity_event.relative_humidity,0);
	 lcd.print("%");
	 lcd.setCursor(13,1);
	 lcd.print(pressure_event.pressure,0);
	 lcd.print("hPa");
	 dT++;
	 }
	 dT = 0;
}
