/*Thiago S.  01/02/21
Adaptado do código do Daniel C.: <https://github.com/camposdp/emg-monitor>
Objetivo: coletar os dados com freq. de 1000 Hz e enviar via socket para python 
Código com multiprocessamento: loop e loop2
Foram utilizados dois buffers para leitura dos dados.
Usei apenas o delay para testes. Próximo passo: implementar o timer
*/

#include <Arduino.h>
#include "WiFi.h"
#include <ArduinoJson.h>
#include <math.h>


// Replace with your network credentials
const char* ssid     = "ESP32-Access-Poin";
const char* password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

int cont1 = 1;
int cont2 = 1;
int flag1 = 0;
int flag2 = 0;
int flag_buff = 1;
int bufferIndex1 = 0;
int bufferIndex2 = 0;
int  M1Value=0;
int  M2Value=0;
int  M3Value=0;
int  M4Value=0;

float Voltage1;
float Voltage2;
float Voltage3;
float Voltage4;
WiFiClient client;

hw_timer_t * timer = NULL;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#define BUFF_SIZE_ROWS 4   //configurar com o número de colunas + 1 
#define BUFF_SIZE_LINES 100 //configura com o número de linhas (linha 0 a 99)
float buf1[BUFF_SIZE_ROWS];
float buf2[BUFF_SIZE_ROWS];

String pacote1;
String pacote2;
const size_t CAPACITY = JSON_ARRAY_SIZE((BUFF_SIZE_ROWS + 1) * BUFF_SIZE_LINES);
StaticJsonDocument<CAPACITY> doc1;
StaticJsonDocument<CAPACITY> doc2;
JsonArray msg1 = doc1.to<JsonArray>();
JsonArray msg2 = doc2.to<JsonArray>();

void clear();

String prepararDado1(){
    String json1;
    serializeJson(msg1, json1);
    return json1;
}
String prepararDado2(){
    String json2;
    serializeJson(msg2, json2);
    return json2;
}

//Precisa configurar para limpar o array msg para começar da linha 1 novamente
// testas a função: void clear(); fonte arduino Json versão 6
void montarBuffer1(){
JsonArray nested1 = msg1.createNestedArray(); //Cria uma lista (buffer 1)
     buf1[0] = millis();
     buf1[1] = Voltage1;
     buf1[2] = Voltage1;
     buf1[3] = cont1;
  
     for (int i=0; i< BUFF_SIZE_ROWS; i++){
     nested1.add(buf1[i]); //adiciona os dados lidos como uma linha
     }

  bufferIndex1++;
  cont1++;

  if (bufferIndex1 > BUFF_SIZE_LINES-1){
    cont1 = 0;
    flag1 = 1;
    bufferIndex1 = 0;
    flag_buff = 2;
    }  
}

void montarBuffer2(){
JsonArray nested2 = msg2.createNestedArray(); //Cria uma lista (buffer 2)
     buf2[0] = millis();
     buf2[1] = Voltage1;
     buf2[2] = Voltage2;
     buf2[3] = cont2;
  
     for (int i=0; i< BUFF_SIZE_ROWS; i++){
     nested2.add(buf2[i]); //adiciona os dados lidos como uma linha da uma
     }

  bufferIndex2++;
  cont2++;

  if (bufferIndex2 > BUFF_SIZE_LINES-1){
    cont2 = 0;
    flag2 = 1;
    bufferIndex2 = 0;
    flag_buff = 1;
    }  
}

void leituraEMG(){
  M1Value = analogRead(34);
  Voltage1 = (((3.3 / 4096.0) *(M1Value)));
  
  M2Value = analogRead(34);
  Voltage2 = ((((3.3 / 4096.0) *(M2Value))));
  /*
  M3Value = analogRead(34);
  Voltage3 = ((((3.3 / 4096.0) *(M3Value))-1.6));
  
  M4Value = analogRead(34);
  Voltage4 = ((((3.3 / 4096.0) *(M4Value))-1.6));
  */
}

void setup() {

  Serial.begin(9600);
  Serial2.begin(9600);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();

 // timer = timerBegin(0, 80, true);
 // timerAttachInterrupt(timer, &onTimer, true);
 // timerAlarmWrite(timer, Ts, true);
 // timerAlarmEnable(timer);
  Serial.println("Timer configurado!");
  xTaskCreatePinnedToCore(loop2, "loop2", 8192, NULL, 1, NULL, 0);//Cria a tarefa "loop2()" com prioridade 1, atribuída ao core 0
  delay(1);
}

void loop() {
 
WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
   
    if(flag1 == 1){
       pacote1 = prepararDado1(); //prepara o dado para ser enviado via wifi   
       doc1.clear();
       flag_buff = 2;
       client.print(pacote1); //envia o buf-json por sokets
       pacote1 = "vazio";
       flag1 = 0;
       }
    if(flag2 == 1){
       pacote2 = prepararDado2(); //prepara o dado para ser enviado via wifi   
       doc2.clear();
       flag_buff = 1;
       client.print(pacote2); //envia o buf-json por sokets
       pacote2 = "vazio";
       flag2 = 0;
       }
     }
  client.stop();
  Serial.println("Client disconnected");
 }
}

void loop2(void*z)//Atribuímos o loop2 ao core 0, com prioridade 1
{
  while(1){
    leituraEMG(); //ok salva em Voltage
        if(flag_buff == 1){
          montarBuffer1();
          }
        else{
          montarBuffer2();
          }
    delay(1);
    }
}

  
