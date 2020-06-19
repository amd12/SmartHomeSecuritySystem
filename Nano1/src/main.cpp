
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <Keypad.h> 
#include <SD.h>
#include <SoftwareSerial.h>
SoftwareSerial SIM800(A6, A7); 

File myFile;

int T1 = 1;
int T2 = 1;
 //Enter a MAC address and IP address for your controller below.
 //The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

const int PIN_direction_TX_RX = A0;
unsigned long timing; // Переменная для хранения точки отсчета

// Объявляем переменные и константы:
           // объявляем переменную для работы с LCD дисплеем, указывая параметры дисплея (адрес I2C = 0x27, количество столбцов = 16, количество строк = 2)
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
char keypressed;
boolean armed = false;
int T =1;
unsigned long lastTame;
//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]=
{
   {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {9, 8, 7, 6};//Rows 0 to 3
byte colPins[numCols] = {5, 4, 3,2};//Columns 0 to 3                
//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);    


       void systemIsUnarmed();
       void systemIsArmed();
        void WebUnarmed();
        void WebArmed();
        void logDis();
        void WebFire();
         void logFire();
         void fireAlarm ();
         void WebWeter();
         void sendSMS(String phone, String message);
         void logArmed();
         void keypressedEnt();



void setup(){

  SIM800.begin(9600);               // Скорость обмена данными с модемом
  SIM800.println("AT");             // Автонастройка скорости

SPI.begin();
  
pinMode(A1, INPUT);
pinMode(A2, INPUT);  // A3 как выход
 pinMode(A3, INPUT);  // A5 как выход (Nano/UNO)

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();


  pinMode(PIN_direction_TX_RX,   OUTPUT);    // устанавливаем режим работы вывода PIN_direction_TX_RX, как "выход"
  digitalWrite(PIN_direction_TX_RX, HIGH);    // устанавливаем уровень логического «0» на выводе PIN_direction_TX_RX (переводим модуль в режим приёма данных)
  Serial.begin(9600);                        // открываем последовательный порт на скорости 9600 бод
                           // инициируем клавиатуру (KB1 - эластичная матричная клавиатура 4х4)

  
}
void loop(){ 
    if (armed){
		systemIsArmed(); 	//Run function to activate the system
		
	    }
	     else if (!armed){
		systemIsUnarmed(); 	//Run fuction to de activate the system
	}

  

 char keypressed = myKeypad.getKey();
  if( keypressed  ){                    // если нажимается кнопка (KEY_DOWN - нажимается, KEY_UP - отпускается, KEY_PRESS - удерживается), то ...
    digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
    delay(10);
    Serial.write(keypressed);                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
    delay(10);
    digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
    delay(1);
  }
  
  
  if(Serial.available()>0){                  // если в последовательном порту есть данные для чтения, то ...
    char n = Serial.read();
    if(   n =='G' ){
     Serial.print(n); 
     armed = true;
    }
    if(   n =='T' ){ 
     armed = false;
    }
  }
  }


void systemIsUnarmed(){
   T1 = 1;
  
  int water = analogRead(A5);
   if(water < 500){
     for(int i = 0;i<1000;i++){
         timing = millis(); 
     WebWeter();
     delay(50);
     keypressedEnt();
     }
   }

    int fire = analogRead(A3);
   if (fire < 500){
      digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
    delay(1);
    Serial.write('F');                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
    delay(1);
    digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
    delay(1);
     while(T2){ 
       keypressedEnt();
      if (millis() - timing > 5000){
        timing = millis(); 
       WebFire();
      }
        char n = Serial.read();
       if(   n =='T' ){ 
         Serial.print(n);
         T2 = 0;
       }
      
     }
    }else{
    if (millis() - timing > 5000){
        timing = millis(); 
        WebUnarmed();
    }
    }
}


void systemIsArmed(){

    char n = Serial.read();
      if(   n =='T' ){ 
     armed = false;
    }

  int sensorGo = analogRead(A1);
  if (sensorGo > 49 ){

   digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
    delay(1);
    Serial.write('S');                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
    delay(1);
    digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
    delay(1);
    while(T1){ 
       keypressedEnt();
      if (millis() - timing > 5000){
        timing = millis(); 
    WebArmed();
      }
    char n = Serial.read();
    if(   n =='T' ){ 
     armed = false;
     T1 = 0;
      break;
    }
    }
    
  }
    int gercon = analogRead(A2);
    if (gercon > 1020){
     
    digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
    delay(1);
    Serial.write('L');                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
    delay(1);
    digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
    delay(1);
    
      
    }
       int fire = analogRead(A3);
       if (fire < 500){
      digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
       delay(1);
       Serial.write('F');                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
      delay(1);
     digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
     delay(1);
     while(true){ 
       keypressedEnt();
      if (millis() - timing > 5000){
        timing = millis(); 
       WebFire();
      }
       char n = Serial.read();
      if(   n =='T' ){ 
       armed = false;
       break;
    }
     }
    }

      int water = analogRead(A5);
   if(water < 500){
     for(int i = 0;i<1000;i++){
     WebWeter();
     }
   }
}

 
 void WebUnarmed(){
   T2 = 1;
   // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
           client.print("<h1>Security system</h1>");
            client.print("System disabled ");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
    logDis();
  }
 }


  void WebArmed(){
   // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("<h1>Security system</h1>");
          // output the value of each analog input pin
            client.print("Security system Attact ");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
    logArmed();
  }
  }


void WebFire(){
  //Serial.println("WeFire");
   // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
           client.print("<h1>Security system</h1>");
            client.print("FIRE ");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    delay(10);// give the web browser time to receive the data
    // close the connection:
    client.stop();
   // Serial.println("client disconnected");
    logFire();
  }
 }

  void logDis() {
    digitalWrite(10 , HIGH);
    digitalWrite(A4, LOW);
    SD.begin(A4);
      myFile = SD.open("Logg.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("------------");
    myFile.print( "Seconds ");
    myFile.print( millis());
    myFile.println("");
    myFile.close();
    delay(10);
    WebUnarmed();
    }
    digitalWrite(10 , LOW);
  }

 void logFire() {
    SD.begin(A4);
      myFile = SD.open("LogFire.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("------------");
    myFile.print( "Seconds ");
    myFile.print( millis());
    myFile.println("");
    myFile.close();
    delay(10);
    WebUnarmed();
    for(int i = 0 ;i<1;i++){
      sendSMS("", "Fire");
    }
    }
  }

  void logArmed() {
    SD.begin(A4);
      myFile = SD.open("LogArmed.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("------------");
    myFile.print( "Seconds ");
    myFile.print( millis());
    myFile.println("");
    myFile.close();
    delay(10);
    WebUnarmed();
    for(int i = 0 ;i<1;i++){
     sendSMS("","Attact");
    }
    }
  }

  void WebWeter(){
   // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
           client.print("<h1>Security system</h1>");
            client.print("Water leak ");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
  }
 }


void keypressedEnt(){
char keypressed = myKeypad.getKey();
  if( keypressed  ){                    // если нажимается кнопка (KEY_DOWN - нажимается, KEY_UP - отпускается, KEY_PRESS - удерживается), то ...
    digitalWrite(PIN_direction_TX_RX, HIGH); // переводим модуль в режим передачи данных
    delay(10);
    Serial.write(keypressed);                // отправляем символ кнопки в последовательный порт (для вывода цифры кнопки используйте KB.getNum)
    delay(10);
    digitalWrite(PIN_direction_TX_RX, LOW);  // переводим модуль в режим приёма данных
    delay(1);
  }
  }

 void sendSMS(String phone, String message) 
{
  SIM800.print("AT+CMGF=1\r");      // Устанавливаем текстовый (не PDU) формат сообщений
  delay(100);                       // Даем модулю отработать команду
  SIM800.println("AT+CMGS=\"" + phone + "\"");  // Задаем номер телефона адресата
  delay(100);
  SIM800.println(message);          // Вводим сообщение
  delay(100);
  SIM800.println((char)26);         // Уведомляем GSM-модуль об окончании ввода
}

