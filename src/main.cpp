//-----------------------------------------------------------------------------------------------------
//                                 
// -Pluviometro DHT22 sensor BMP180 - calculo pressao relativa
// -Anemometro e biruta Invertido
// -Interface Bluetooth
// -Wifi : Ch modificado 

//  UV e Wm
//  SD

//  RTC corrige WiFi ou Bluet.

//  ATENCAO PORTAS ...

//-----------------------------------------------------------------------------------------------------
//                                     DEFINIÇÕES
//-----------------------------------------------------------------------------------------------------
#include <WiFi.h>
#include <AsyncTCP.h>
#include "RTClib.h"
#include <Wire.h>  //biblioteca para a configuração dos pinos SDA e SCL do sensor
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include <EEPROM.h>
#include <esp_task_wdt.h> //Biblioteca do watchdog
#include <Adafruit_BMP085.h>
#include "DHT.h"
#include "SSD1306.h"
#include "BluetoothSerial.h"
#include <WiFiManager.h>

#include <NTPClient.h> //Biblioteca NTPClient modificada
#include <WiFiUdp.h> //Socket UDP

#include <HTTPClient.h>
#include <ArduinoJson.h>

// Declaração das funções

int ReadWord(int endereco);
void connectWifi();
int ReadByte(int endereco);
void WriteByte(int endereco,int dado);
void writeFile(fs::FS &fs, const char * path, const char * message);
void setupNTP();
void WriteWord(int endereco,int dado);
void appendFile(fs::FS &fs, const char * path, const char * message);
bool readBlock();
void Pluv();
void ProcessaCadaSegundo();
void ProcessaCadaMinuto();
void ProcessaCadaHora();
void ProcessaCadaDia();
void tempo();
void Wtempo();
void RTCset();
void windvelocity();
void TestCh();
void TimeStamp();
void LeTempUmiPress();
float DirWind();
void MedLuz();
void logSDCard();
void TestWf();
void TestWf1();

//solar radiation
#include <BH1750.h>
BH1750 lightMeter;

//Solar
#include "Adafruit_SI1145.h"
Adafruit_SI1145 uv = Adafruit_SI1145(); //não ...

//---------Plataformas---------
//retiradas
const char* server = "api.thingspeak.com";

int H = 1035 ;  // altitude do local de instalacao em metros

boolean state = false;

boolean tx_thing=false;
boolean tx_wunderg=false;
boolean LogOk=false;
boolean tx_appwrf=false;
boolean ntpbol=false;  

//---------Eeprom-----------
#define ASSINATURA 0
#define mmHe2p 1
#define mmDe2p 3
#define hrAnte2p 5
#define diaAnte2p 6
#define crc_byte 7
#define bootCnte2p 8

//---------Display---------
SSD1306  display(0x3c,21, 22);

//---------DHT-----------
#define DHTPIN  15 // Pin 33 interfere no biruta 
#define DHTTYPE DHT22   // AM2301 
DHT dht(DHTPIN, DHTTYPE,20);


//---------BME--------------
Adafruit_BMP085 bmp;

// --------SD---------------
#define SD_CS 5
String dataMessage;
const int LEDpin = 26;   // 22 LOLIN  
const int prog_pin = 14;   // Botão Prog Bluet P14 ou  32.............>>>>>>>>>>>>>>>
const char turnON =1;  //0=on lolin 1=on DEV
const char turnOFF =0; //1=off lolin 0=off DEV

//---------RTC-------------
RTC_DS3231 rtc;
int hr=0;
int mn=0;
int sg=0;
int ano=0;
int mes=0;
int dia=0;

int sgAnt=0;
int minAnt=0;
int hrAnt=0;
int diaAnt=0;

String DataAtual="";

//----------------------------------

int Whr=0;
int Wmn=0;
int Wsg=0;
int Wano=0;
int Wmes=0;
int Wdia=0;

//Fuso Horário, no caso horário de verão de Brasília 
int timeZone = -3;
int dd=0;
 
//Struct com os dados do dia e hora
struct Date{
    int dayOfWeek;
    int day;
    int month;
    int year;
    int hours;
    int minutes;
    int seconds;
};
 
//Socket UDP que a lib utiliza para recuperar dados sobre o horário
WiFiUDP udp;
 
//Objeto responsável por recuperar dados sobre horário
NTPClient ntpClient(
    udp,                    //socket udp
    "0.br.pool.ntp.org",    //URL do servwer NTP
    timeZone*3600,          //Deslocamento do horário em relacão ao GMT 0
    60000);                 //Intervalo entre verificações online
 
//Nomes dos dias da semana
char* dayOfWeekNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};



//--------BlueTooth/Servidor-------
//BluetoothSerial SerialBT;
//char receivedChar;
//const char inic ='{';
//const char cr ='}';
//String buff = "";
//String ConfigBuffer;

//--------Contadores----------

int sent = 0;
int txwd =0;
int txtg =0;
int appw = 0; 

//----------Anemometro / biruta ------
//boolean state = false;
int NL=0;
int clicked;
unsigned long lastMillis = 0;

const int Anem = 35; //P 35 in anem  
int windirinp = 34;  //entrada biruta P34

int winddirstate = 0 ; 
int winddir_eu =0 ;
int wd=0;
int wds=0;

float winddir=90;
float windspeed=0;
float windgust= 0;

float RPM = 0;       // Revolutions per minute
float speedwind = 0;// Wind speed (km/h)
float vm=0;
float vmd=0;
float vmax=0;
float Vref=0;


//--------mediçoes---------
float h = 0;
float t = 0;
int p = 0; //pressao absoluta
float Pr =0 ; //pressao retativa
float ti=0;

int hs1 = 0;
int hs2 = 0;
int hs3 = 0;
int mf = 0;
float ts1 = 0;
float ts2 = 0;
float ts3 = 0;
float tr = 0; //temp de relva

int REEDCOUNT = 0;  //conta pulso / hora
int REEDh = 0;     //conta pulso / 24h
float rain = 0;       //armazena pulso hora
float rainh = 0;      //armazena até 24h

int vl = 0; 
int conta = 0; 
int Rg=0;
int aux=0;
int sTout=40 ;  //50 para ajuste de wifi   8 p test wifi
int dBm = 0 ;

float tempf = 0; 
float dewpout=0;
float rain1h = 0.00;
float rain24h = 0.00;
float baro= 0 ;

int solar=0 ;
int UVmax=0 ; 

//------Sensores--------
#define PIN_VL 4  // P4 entrada pluviometro
#define RSTp 2   // P2 RST pluviometro
#define Led1 12 //  P12  15 dht ... 14 ..........>>>>>>>>>>>>>
#define PIN_VLR 13  // Reset wifi 13 
#define Conec 25  //Saida p pulso  5


byte e2pBuffer[8]={0,0,0,0,0,0,0,0};

//-----------------------------------------------------------------------------------------------------
//                                      SETUP
//-----------------------------------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  while(!Serial);   

  Wire.begin();  //P21 SDA  - P22 SCL  
 
  Wire.setClock(100000); //Temporizacao entre dispositivos  4

//Habilita o watchdog configurando o timeout para 50 segundos
  esp_task_wdt_init(60, true);
  esp_task_wdt_add(NULL);
  
  pinMode(Anem, INPUT); //input anem
  pinMode (RSTp,OUTPUT); // reset pluviometro 
  pinMode (PIN_VL,INPUT); // entrada pluvio 
  digitalWrite(RSTp, HIGH);  // ativa pluviom
  pinMode (prog_pin,INPUT_PULLUP);
  pinMode(Led1, OUTPUT);    // LED 1
  digitalWrite(Led1, LOW);  // Apaga o Led 1
  pinMode(Conec,OUTPUT); // Piloto wifi
  digitalWrite(Conec,LOW); // Off
  pinMode (PIN_VLR,INPUT_PULLUP); // entrada chave rts wf 

  Serial.println(F(""));
  Serial.println(F("Inicializando WX WF SD 5 ..."));
  Serial.println(F(""));

  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16); 
  display.drawString(0, 0,"Iniciando..."); 
  display.display();
  display.setFont(ArialMT_Plain_10); 

  //int Push_button_state = digitalRead(prog_pin);
  //if ( Push_button_state != HIGH) {bmp.begin();dht.begin();LeTempUmiPress();BT();}

  for (int i=0 ; i < 5 ; i++) {
    digitalWrite(Led1, HIGH); // Acende o Led
    delay(200);
    digitalWrite(Led1, LOW); // Apaga o Led 
    delay(200);
  }

  Serial.print(F("Inicializando DHT: "));
  display.clear();
  display.drawString(0, 0,"BME:");
  display.display();
  delay(1000);
  dht.begin();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    display.drawString(60, 0,"ERRO");
  }else {
    display.drawString(60, 0,"OK"); 
    Serial.println(F("OK")); 
  }
  display.display();
    
  Serial.print(F("Inicializando RTC: "));
  display.drawString(0, 12,"RTC:");
  display.display();
  delay(1000);
  if (! rtc.begin()) {
    Serial.println(F("Não encontrado"));
    Serial.flush();
    display.clear();
    display.drawString(60, 12,"FALHA");
    display.display();
 //   abort();
  } else{
    display.drawString(60, 12,"OK");
    display.display();
    Serial.println(F("OK"));
  }

  Serial.print(F("Verificando alimentação RTC: "));
  display.drawString(0, 24,"RTC PW:");
  display.display();
  delay(1000);
  //rtc.adjust(DateTime(F(_DATE), F(TIME_)));
  if (rtc.lostPower()) {
    Serial.println(F("Perda de alimentação"));
    display.drawString(60, 24,"ERRO");
    display.display();
  }else{
    display.drawString(60, 24,"OK");
    display.display();
    Serial.println(F("OK"));
  }
  
  Serial.print(F("Iniciando SD_CS Card: "));
  SD.begin(SD_CS);  
  if(!SD.begin(SD_CS)) {
    Serial.println(F("Falha ao colocar Cartao !"));
  //return;
  } else {Serial.println(F("OK"));}

  Serial.print(F("Verificando presença SD_CS Card: "));
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println(F("SD card nâo encontrado"));
  //return;
  } else {
    Serial.println(F("OK"));
    LogOk=true;
    }
  
  Serial.print(F("Inicializando SD Card : "));
  if (!SD.begin(SD_CS)) {
    Serial.println(F("ERROR - SD Card initialização falhou ! "));
    LogOk=false;
  //return;    // init failed
  } else {Serial.println(F("OK"));}
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println(F("Arquivo não existe ! "));
    Serial.println(F("Criando arquivo ..."));
    writeFile(SD, "/data.txt", "Date ; Time ; mm/h ; mm/dia ; Temp ; Umidade ; Pressao ; Vmed ; Vmax ; dir ; UV ; W/m2 ; \r\n");
  }
  else {
    Serial.println(F("O arquivo já existe "));  
  }
  file.close();

  Serial.print(F("Inicializando BMP: "));
  display.drawString(0, 36,"BMP:");
  display.display();
  delay(1000);
  unsigned status;
  status =   bmp.begin();  
  if (!status) {
    display.drawString(60, 36,"ERRO");
    display.display();
    Serial.println(F("ERRO"));
  } else{
    Serial.println(F("OK"));
    display.drawString(60, 36,"OK");
    display.display();
  }

  /*esp_task_wdt_reset();

  Serial.print(F("Verificando EEPROM: "));
  display.drawString(0, 48,"EEPROM:");
  display.display();
  EEPROM.begin(512);
  int ass=EEPROM.read(0); 
  if (ass!=0xAA)
  {
    Serial.println(F(" EEPROM não inicializada . . . "));
    display.drawString(60, 48,"Err");
    WriteByte(0,0xAA);
    WriteWord(mmHe2p,0);
    WriteWord(mmDe2p,0);
    WriteByte(hrAnte2p,0);
    WriteByte(diaAnte2p,0);
    WriteByte(bootCnte2p,0);
  } else
  {
    Serial.println(F(" EEPROM Ok "));
    display.drawString(60, 48,"OK");
  }
  
  //-----------------------
  
  esp_task_wdt_reset(); */

  //-------------------

   //solar radiation
  lightMeter.begin();

  Serial.println(F("Adafruit SI1145 test"));
  if (! uv.begin()) {
    Serial.println(F("Didn't find Si1145"));
    //digitalWrite(Led1, HIGH); // Acende o Led
    //while (1);
  }

  //-------------------

  /*int btCnt=ReadByte(bootCnte2p);
  btCnt++;
  WriteByte(bootCnte2p,btCnt);
  Serial.print(F("N. de boots: "));
  Serial.println(F(btCnt));*/

  //---------Wifi------------

   connectWifi();
   
   esp_task_wdt_reset();

   setupNTP();

   Wtempo();

   //----------------
    if (ntpbol){
    Serial.println(F("Nao pode acertar RTC "));
    }else{  
    Serial.println(F("Acertar RTC "));
    RTCset(); //acertar RTC
  }
  //-----------------------
    //WriteByte(hrAnte2p,hr);  
    //WriteByte(diaAnte2p,dia);

    hrAnt=ReadByte(hrAnte2p);
    diaAnt=ReadByte(diaAnte2p);
    REEDCOUNT=ReadWord(mmHe2p);
    REEDh=ReadWord(mmDe2p);
    display.display();
    delay(1000);
  //-----------------------

   esp_task_wdt_reset();
   
  Serial.println(F(""));
  Serial.println(F("Fim da Inicialização"));

}
//-----------------------------------------------------------------------------------------------------
//                                               MAIN
//-----------------------------------------------------------------------------------------------------
void loop() {
  sTout=5;
  
  tempo();

  ProcessaCadaSegundo();

  ProcessaCadaMinuto();

  ProcessaCadaHora();

  ProcessaCadaDia();

  esp_task_wdt_reset();
}
//-----------------------------------------------------------------------------------------------------
//                                             Processos
//-----------------------------------------------------------------------------------------------------
void ProcessaCadaSegundo(){
  if (sg!=sgAnt){
    windvelocity(); // Le anemometro
    Pluv();         // le pluviometro
    //BT();           // Verifica BlueTooth
    TestCh();       // Verifica Reset Wifi
    TimeStamp();    // Atualiza display
  }
  sgAnt=sg;
}
//-----------------------------------------------------------------------------------------------------
void ProcessaCadaMinuto(){
  if (minAnt!=mn){
    
    LeTempUmiPress();

    //----Pluviometro---------
    REEDCOUNT=ReadWord(mmHe2p);
    REEDh=ReadWord(mmDe2p);
    rain=REEDCOUNT*0.1;  //PL3 0.10 ou PL1/2  0.25 ---------------------- >  >  >  >  >  >  >  >  >  >
    rainh=REEDh*0.1;     //.......................

    Serial.println(F(NL));
    
    vmd=vm/NL; // número de leituras do anemometro

    DirWind();  //verifica biruta

    MedLuz();   //Mede dados luz 

    if (LogOk){
   digitalWrite(Led1, HIGH); // Acende o Led
   logSDCard(); // Salva dados no SD Card
   delay(1500);
   digitalWrite(Led1, LOW); // Acende o Led
   }

    Serial.println(F("Testa WiFi .... "));
    
    TestWf();   //Verifica wifi

    TestWf1();  // Verifica roteador

    //----------------
    
    if (ntpbol){
    setupNTP();
    }
 
     Wtempo();
     
   
     WriteByte(hrAnte2p,hr);  
     WriteByte(diaAnte2p,dia);

 Pluv();         // le pluviometro

  vm=0;   //zera variaveis apos envio 
  vmax=0;
  NL=0;
  
  }
  minAnt=mn;  
}
//-----------------------------------------------------------------------------------------------------
void ProcessaCadaHora(){
  if (hrAnt!=hr){
    REEDCOUNT=0;
    WriteWord(mmHe2p,0);
    WriteByte(hrAnte2p,hr); 
    Serial.println(F("Resetando Contador milimetros por Hora")); 

     
  }
  hrAnt=hr;   
}
//-----------------------------------------------------------------------------------------------------
void ProcessaCadaDia(){
  if (diaAnt!=dia){
    REEDCOUNT=0;
    REEDh=0;
    WriteWord(mmHe2p,0);
    WriteWord(mmDe2p,0);
    WriteByte(diaAnte2p,dia);
    Serial.println(F("Resetando Contador milimetros por Hora e dia "));    

    //----------------
    if (ntpbol){
    Serial.println(F("Nao pode acertar RTC "));
    }else{  
    Serial.println(F("Acertar RTC "));
    RTCset(); //acertar RTC
  }
  //-----------------------
 
  }
  diaAnt=dia;   
}
//-----------------------------------------------------------------------------------------------------
//                                            Sensores
//-----------------------------------------------------------------------------------------------------
void Pluv(){

  if ( digitalRead(PIN_VL) == HIGH ) {
      
    REEDCOUNT=ReadWord(mmHe2p);
    REEDh=ReadWord(mmDe2p);
    REEDCOUNT++;
    REEDh++;
    WriteWord(mmHe2p,REEDCOUNT);
    WriteWord(mmDe2p,REEDh); 
     
    Serial.println(F(""));
    Serial.print(F("Pulso p hora : "));
    Serial.print(F(REEDCOUNT)); 
    Serial.print(F(" Pulso p dia : "));
    Serial.println(F(REEDh));

    delay(100);
    digitalWrite(RSTp,LOW ); // reseta pluviom
    Serial.println(F("Reseta hw externo "));
    delay(100);
    digitalWrite(RSTp, HIGH);  // ativa pluviom e finaliza leitura
  }
  Serial.print(F("."));
}
//-----------------------------------------------------------------------------------------------------
//                                              LOG
//-----------------------------------------------------------------------------------------------------
 // Write the sensor readings on the SD card
void logSDCard() {
  
  tempo();
  
  String dataString2="";
  
  dataString2=dataString2+";"+rain+";"+rainh+";"+t+";"+h+";"+Pr+";"+vmd+";"+vmax+";"+winddir_eu+";"+UVmax+";"+solar+";" + "\r\n";
  
  dataMessage =  DataAtual+dataString2 ;
  
  Serial.println(F(""));
  Serial.print(F("Save data : "));
  Serial.println(dataMessage);
  appendFile(SD, "/data.txt", dataMessage.c_str());
  
}
//-----------------------------------------------------------------------------------------------------
// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println(F("Falha ao abrir o arquivo para gravação..."));
    return;
  }
  if(file.print(F(message))) {
    Serial.println(F("Arquivo escrito..."));
  } else {
    Serial.println(F("Falha na gravação..."));
  }
  file.close();
}
//-----------------------------------------------------------------------------------------------------
// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf ("Anexando ao arquivo: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println (F("Falha ao abrir para anexar ... "));
    return;
  }
  if(file.print(F(message))) {
    Serial.println(F("Mensagem anexada..."));
  } else {
    Serial.println(F("Falha ao anexar ..."));
  }
  file.close();
}
//-----------------------------------------------------------------------------------------------------
//                                    COMUNICAÇÃO
//-----------------------------------------------------------------------------------------------------
  void TestWf()
{
WiFiClient client;

if (client.connect(server, 80)) { 
Serial.println(F(". . . WiFi Client Conectado ... "));

dBm = WiFi.RSSI();
Serial.print(F("WiFi nivel : "));
Serial.println(F(dBm));

}//end if
else {
  client.stop();
  Serial.println(F(". . .  WiFi Client Nao Conectado ... "));
  connectWifi();
}

client.stop();  // rever posição
esp_task_wdt_reset();
}//end send  

//--------------------------------------------------------------
 void TestWf1()
{

 if(WiFi.status()== WL_CONNECTED){
 Serial.println(F(" --- Conectado ao roteador --- "));
 }
   else{
   Serial.println(F(" --- Nao conectado ao roteador !!!"));
   //connectWifi();
}
}

//---------------------------------------------------------------------------------------------


   void setupNTP()
{
    //Inicializa o client NTP
    ntpClient.begin();
     
    //Espera pelo primeiro update online
    Serial.println(F("Waiting for first update"));
    if(!ntpClient.update())
    {
        Serial.print(F(" Forcar Update . . .  "));
        ntpClient.forceUpdate();
        delay(500);
    }

    if(!ntpClient.update())
    {
    ntpbol=true;
      
    }
    else {
    ntpbol=false; 
 
    //Serial.println(F());
    Serial.println(F("First Update Complete"));
}
}

  //----------------------------------------------------------------
Date getDate()
{
    //Recupera os dados de data e horário usando o client NTP
    char* strDate = (char*)ntpClient.getFormattedTime().c_str();
 
    //Passa os dados da string para a struct
    Date date;
    sscanf(strDate, "%d-%d-%dT%d:%d:%dZ", 
                    &date.year, 
                    &date.month, 
                    &date.day, 
                    &date.hours, 
                    &date.minutes,
                    &date.seconds);
 
    //Dia da semana de 0 a 6, sendo 0 o domingo
    date.dayOfWeek = ntpClient.getDay();
    return date;
}

//------------------------------------------------------------------------

  void Wtempo()
{
    //Recupera os dados sobre a data e horário
    Serial.println(F("  Data / Hora  web . . .  "));
    
    Date date = getDate();

        Wano=date.year;
        Wmes=date.month;
        Wdia=date.day;
        Whr=date.hours;
        Wmn=date.minutes;
        Wsg=date.seconds;

        //Serial.println(F());
        Serial.print(F("WiFi  Data / Hora  : "));
        Serial.print(F(Wano));
        Serial.print(F(":"));
        Serial.print(F(Wmes));
        Serial.print(F(":"));
        Serial.print(F(Wdia));
        Serial.print(F("::"));
        Serial.print(F(Whr));
        Serial.print(F(":"));
        Serial.print(F(Wmn));
        Serial.print(F(":"));
        Serial.print(F(Wsg));
        //Serial.println(F());
        
    delay(250);

}

//-------------------------------------------------------------------

   void RTCset() {

  rtc.adjust(DateTime(Wano,Wmes,Wdia,Whr,Wmn,Wsg));
    
  }
    
//-----------------------------------------------------------------------------------------------------
 /*void BT(){
  int Push_button_state = digitalRead(prog_pin);
  if ( Push_button_state != LOW) {return;} //**

  display.clear();
  display.setFont(ArialMT_Plain_24); 
  display.drawString(5, 15,"BlueTooth");
  display.display();

  esp_task_wdt_reset();
    
  SerialBT.begin("WRFComercial"); //Bluetooth device name
  digitalWrite(LEDpin, turnON);// turn the LED ON
  digitalWrite(Led1, HIGH); // Acende o Led
  delay(1000);
  
  Serial.println(F(""));
  Serial.println(F("BlueTooth"));
  String Pa1="";
  String Pa2="";
  String Pa3="";
  String Pa4="";
  String Pa5="";
  String Pa6="";
  int cnt=0;

  while (1){  
    receivedChar =(char)SerialBT.read(); 
    if (SerialBT.available()) 
    {
      digitalWrite(LEDpin, turnOFF);// turn the LED ON
      char ch=receivedChar; 
      if (ch==inic){buff="";}
      else
      {
        if (ch==cr)
        {
          Serial.print(F("Rx_Buffer: "));
          Serial.println(F(buff));
          
          int pt=buff.indexOf("]",1);
          String Len=buff.substring(1,pt);
          buff.remove(0,pt+1);
          Serial.println("");
          Serial.print("CMD Recebido: ");
          int ta1=buff.length();
          int ta2=Len.toInt();
          if (ta1!=ta2) {Serial.println("Erro de tamanho!!");SerialBT.println("Erro!!");}
          else
          {
            ConfigBuffer=buff;
//            SerialBT.println("OK");
            pt=buff.indexOf("]",1);
            String Cmd=buff.substring(1,pt);
            Serial.print("Cmd: ");
            Serial.println(Cmd);
            buff.remove(0,pt+1);
            
            if (Cmd=="0"){ // carrega setup na memoria
              Serial.println("Atualizando setup");
          
              pt=buff.indexOf("]",1);
              String Pa1=buff.substring(1,pt);
              Serial.print("RX  : ");
              Serial.println(Pa1);

              String str=Pa1.substring(0,2);
              int dd=str.toInt();
              str=Pa1.substring(3,5);
              int mm=str.toInt();
              str=Pa1.substring(6,10);
              int yyyy=str.toInt();
        
              str=Pa1.substring(11,13);
              int hh=str.toInt();
              str=Pa1.substring(14,16);
              int mn=str.toInt();
              str=Pa1.substring(17,19);
              int ss=str.toInt();
              rtc.adjust(DateTime(yyyy,mm,dd,hh,mn,ss));            

              tempo();
              String dataString="";
              if (dia<10){dataString+="0";} 
              dataString+=String(dia)+"-";
              if (mes<10){dataString+="0";}
              dataString+=String(mes)+"-";
              if (ano<10){dataString+="0";}
              dataString+=String(ano)+" ";
              if (hr<10){dataString+="0";}
              dataString+=String(hr)+":";
              if (mn<10){dataString+="0";}
              dataString+=String(mn)+":";
              if (sg<10){dataString+="0";}
              dataString+=String(sg)+" ";
              Serial.println("SET.: "+dataString);
              display.clear();
              display.setFont(ArialMT_Plain_24); 
              display.drawString(5, 15,"BlueTooth");
              display.setFont(ArialMT_Plain_10);
              display.drawString(10, 50,dataString);
              display.display();
              SerialBT.println("[00]Atualizado: "+dataString);             
            }
            
            if (Cmd=="1"){ // reinicializa ESP32
              display.clear();
              display.setFont(ArialMT_Plain_24); 
              display.drawString(5, 15,"Rebooting");
              display.display();              
              Serial.println("Rebooting ESP");
              SerialBT.println("[01]Rebooting");
              delay(3000);
              btStop();
              delay(1000);
              ESP.restart();  
            }
            
            if (Cmd=="2"){ // envia setup para o bluetooth
              
              tempo();
              
              Serial.println("Enviando Setup");
              delay(1000);
              SerialBT.print("[02]");
              SerialBT.print("Data/Hora: ");
              SerialBT.println(DataAtual);              
              SerialBT.print("mm (Hora): ");
              SerialBT.println(rain);
              SerialBT.print("mm (Dia) : ");
              SerialBT.println(rainh); 
              SerialBT.print("Temp.(*C): ");
              SerialBT.println(t);       
              SerialBT.print("Umind.(%): ");
              SerialBT.println(h);
              SerialBT.print("Pressão  : ");
              SerialBT.println(Pr);
              SerialBT.print("Vel Med (Kmh): ");
              SerialBT.println(vmd);
              SerialBT.print("Vel Max (Kmh): ");  
              SerialBT.println(vmax);
              SerialBT.print("Dir Vento (G): ");
              SerialBT.println(winddir_eu);
              SerialBT.print("Nivel UV : ");  
              SerialBT.println(UVmax);
              SerialBT.print("Solar (Wm2) : ");
              SerialBT.println(solar);
              SerialBT.print("Boots    : ");
              int btCnt=ReadByte(bootCnte2p);
              SerialBT.println(btCnt);
              SerialBT.println("");    
              cnt=0;       
            }
            if (Cmd=="3"){ // apaga assinatura da memorria do ESP32
              Serial.println("Apaga Assinatura");
              WriteByte(0,0);
              display.clear();
              display.setFont(ArialMT_Plain_24); 
              display.drawString(5, 15,"Rebooting");
              display.setFont(ArialMT_Plain_10);
              display.drawString(10, 50,"Contadores zerados");
              display.display();              
              SerialBT.println("[03]Rebooting com contadores zerados");
              delay(3000);
              btStop();
              delay(1000);
              ESP.restart(); 
            }  
                               
        /*            
        buff.remove(0,pt+1);
        
          }
          delay(2000);
          cnt=0;
        } 
        else {buff=buff+ch;} 
      } 
    }
    else digitalWrite(LEDpin, turnON);// turn the LED OFF

    cnt=cnt+1;
//    if (cnt==3000){SerialBT.println("?");cnt=0;}
    if (cnt==3000){SerialBT.println("[??]");cnt=0;}
    delay(1); 
  }
  digitalWrite(Led1, LOW); // Apaga o Led
  //end Bluet
}*/

//-----------------------------------------------------------------------------------------------------
//                                      SUB-ROTINAS
//-----------------------------------------------------------------------------------------------------
void tempo() {
  DateTime now = rtc.now();
  ano=now.year();
  mes=now.month();
  dia=now.day();
  hr=now.hour();
  mn=now.minute();
  sg=now.second();
  String dataString="";
  if (ano<10){dataString+="0";}
  dataString+=String(ano)+"/";
  if (mes<10){dataString+="0";}
  dataString+=String(mes)+"/";
  if (dia<10){dataString+="0";} 
  dataString+=String(dia)+" ";
  if (hr<10){dataString+="0";}
  dataString+=String(hr)+":";
  if (mn<10){dataString+="0";}
  dataString+=String(mn)+":";
  if (sg<10){dataString+="0";}
  dataString+=String(sg)+" ";
  DataAtual=dataString;  
}
//-----------------------------------------------------------------------------------------------------
void TimeStamp(){
  
  tempo();
  
  String dataString=DataAtual;

  display.clear();
  
  display.drawString(10, 0,dataString);
  for (int i = 0; i < 127; i++) {display.setPixel(i, 12);}
  display.drawString(0, 14,"Pluv/H.");
  display.drawString(55, 14,":");
  display.drawString(60, 14,String(rain)); //REEDCOUNT*0.25
  display.drawString(0, 26,"Pluv/D."); 
  display.drawString(55, 26,":");
  display.drawString(60, 26,String(rainh)); //REEDh*0.25
  display.drawString(0, 38,"Temp./Umi.");
  display.drawString(55, 38,":");
  display.drawString(60, 38,String(t)+" / "+String(h));
  display.drawString(0, 50,"Pressao");
  display.drawString(55, 50,":");
  display.drawString(60, 50,String(Pr));
  display.display();
  }
//-----------------------------------------------------------------------------------------------------
int ReadWord(int endereco)
{
  EEPROM.begin(512);
  int aux=EEPROM.read(endereco);
  aux=(aux*256)+EEPROM.read(endereco+1);
  EEPROM.end();
  return aux;
}
//-----------------------------------------------------------------------------------------------------
void WriteByte(int endereco,int dado)
{
  EEPROM.begin(512);
  EEPROM.write(endereco,dado);
  delay(10);
  EEPROM.commit();
  delay(10);
}
//-----------------------------------------------------------------------------------------------------
void WriteWord(int endereco,int dado)
{
  EEPROM.begin(512);
  EEPROM.write(endereco,dado/256);
  delay(10);
  EEPROM.write(endereco+1,dado%256);
  delay(10);
  EEPROM.commit();
  delay(10);
}
//-----------------------------------------------------------------------------------------------------
int ReadByte(int endereco){
  EEPROM.begin(512);
  int aux=EEPROM.read(endereco);
  EEPROM.end();
  return aux;
}
//-----------------------------------------------------------------------------------------------------
bool writeBlock(){
  e2pBuffer[1]=REEDCOUNT/256;
  e2pBuffer[2]=REEDCOUNT%256;
  e2pBuffer[3]=REEDh/256;
  e2pBuffer[4]=REEDh%256;
  e2pBuffer[5]=hrAnt;
  e2pBuffer[6]=diaAnt;
  byte crc=e2pBuffer[1]+e2pBuffer[1]+e2pBuffer[2]+e2pBuffer[3]+e2pBuffer[4]+e2pBuffer[5]+e2pBuffer[6];
  e2pBuffer[7]=crc;
  EEPROM.begin(512);
  for (int i=1;i<=7;i++){
    EEPROM.write(i,e2pBuffer[i]);
    delay(10);
  }
  EEPROM.commit();       
  if (readBlock) return true; else return false;
}
//-----------------------------------------------------------------------------------------------------
bool readBlock(){
  EEPROM.begin(512);
  e2pBuffer[1]=EEPROM.read(1);
  e2pBuffer[2]=EEPROM.read(2);
  e2pBuffer[3]=EEPROM.read(3);
  e2pBuffer[4]=EEPROM.read(4);
  e2pBuffer[5]=EEPROM.read(5);
  e2pBuffer[6]=EEPROM.read(6);
  e2pBuffer[7]=EEPROM.read(7);
  byte crc=e2pBuffer[1]+e2pBuffer[1]+e2pBuffer[2]+e2pBuffer[3]+e2pBuffer[4]+e2pBuffer[5]+e2pBuffer[6];
  EEPROM.end();
  if (crc==e2pBuffer[7]){
    REEDCOUNT=(e2pBuffer[1]/256)+e2pBuffer[2]%256;
    REEDh=(e2pBuffer[3]/256)+e2pBuffer[4]%256;
    hrAnt=e2pBuffer[5];
    diaAnt=e2pBuffer[6];
    return true;} else return false;
}
//-----------------------------------------------------------------------------------------------------
void LeTempUmiPress()
{
  h = dht.readHumidity();
  t = dht.readTemperature();
  p = bmp.readPressure();
  ti= bmp.readTemperature();
  
  if ((t >= 60) || (t <= -10) || (p < 30000) || (isnan(h)) || (isnan(t))) {
    Serial.println(" ATENCAO : Erro no DHT22 . . . . . . ");
    t= ti*0.8 ;
    h= 35 ;
    
  }

    if(h>=99){
     h=98.5 ;
  }

  Serial.print("Temperatura Ext: ");
  Serial.print(t);
  Serial.print(" C :: Temp. Inter. : ");
  Serial.print(ti);
  Serial.print("  C :: Pressao : ");
  Serial.println(p);

  Pr = (((p)/pow((1-((float)(H))/44330), 5.255))/100.0);
    Serial.print(" Pressao Rel = ");
    Serial.print(Pr);
    Serial.println(" Pa ");
}

//-------------------------------------------------------------------------
   void MedLuz()
   {
 //solar radiation
    float lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.println(lux);
    solar = (lux * 0.073) ; //(lux / 100) * 0.79  -- 0.0353
    Serial.print(" Radiacao Solar : "); 
    Serial.print(solar);  
    Serial.println("  W/mm2");

  // UV 
  Serial.print("Vis: "); Serial.println(uv.readVisible());
  Serial.print("IR: "); Serial.println(uv.readIR());
  
  UVmax = uv.readUV();
  UVmax /= 100.0; 
  UVmax = 0;  // ATENÇÃO . . . . . .
   
  Serial.print("UV: ");  
  Serial.println(UVmax);
   }
//-----------------------------------------------------------------------------------------------------
  void connectWifi()
{
 Serial.println("-- CONECTANDO WIFi ... ");
 delay(500);
 esp_task_wdt_reset();
 WiFiManager wifiManager;
 //wifiManager.autoConnect("Estacao Teste");
 wifiManager.setTimeout(sTout);

 if(!wifiManager.autoConnect("Conectar Estacao")) {
    Serial.println("failed to connect and hit timeout");
    digitalWrite(Conec, LOW);  //  off
 
 }
 else {

 digitalWrite(Conec, HIGH);  //  P27 ON
  
 delay(1500);

 }

 esp_task_wdt_reset();
    
  }
  /*  
    esp_task_wdt_reset(); {
    */
    

  //-----------Testa botao reset wf ---------
  void TestCh() {

  int vlr = digitalRead(PIN_VLR);
     //HIGH
    if( vlr != LOW ){return;}
     sTout=50 ;
     Serial.println("resetando");
     digitalWrite(Led1, HIGH); // Acende o Led
     delay(1000); 
     WiFiManager wifiManager;
     wifiManager.resetSettings();
     ESP.restart();
    
 //End TestCh
 }

 //------------------------------------------------------------

 float DirWind(){

    for (int i = 0; i < 20; i++) {
    wd = analogRead(windirinp);
    Serial.println(wd);
    wds=wds+wd; 
    delay(50); 
    }

    winddirstate = wds / 20;

  if ( (winddirstate >= 3150) && (winddirstate <= 5000) ) {
    winddir_eu = 0;
  }
  if ( (winddirstate >= 0) && (winddirstate <= 99) ) {
    winddir_eu = 315;  //N 45 I 315
  }
  if ( (winddirstate >= 100) && (winddirstate <= 599) ) {
    winddir_eu = 270; //N 90 I 270
  }
  if ( (winddirstate >= 600) && (winddirstate <= 1119) ) {
    winddir_eu = 225; //N 135 I 225
  }
  if ( (winddirstate >= 1120) && (winddirstate <= 1649) ) {
    winddir_eu = 180;
  }
  if ( (winddirstate >= 1650) && (winddirstate <= 2149) ) {
    winddir_eu = 135; //N 225  I 135
  }
  if ( (winddirstate >= 2150) && (winddirstate <= 2599) ) {
    winddir_eu = 90;  // N 270  I 90
  }
  if ( (winddirstate >= 2600) && (winddirstate <= 3149) ) {
    winddir_eu = 45;  //N 315  I 45
  }

  wd=0;
  wds=0;

  Serial.print(" -- Pin Status: ");
  Serial.println(winddirstate);
  //}
  return winddir_eu;

}


// Measure wind speed  -------------------------------------------------
void windvelocity() {
    NL++;

    lastMillis = xTaskGetTickCount();
    while(xTaskGetTickCount() - lastMillis < 3000){
        if(digitalRead(Anem) == HIGH) if(state == false){
            delay(50);
            clicked++;
            state = true;
        }
        if(digitalRead(Anem) == LOW) if(state == true) state = false;
    }
    
    RPM = clicked * 20;
    speedwind = ((30.77 *RPM)/1000) * 3.6;

  vm=vm+speedwind;
  
  if(speedwind > vmax ){
   vmax = speedwind;
   }

   Serial.print(" - Veloc : ");
   Serial.print(speedwind);
   Serial.print(" - Vmd : ");
   Serial.println(vm);
   
   clicked=0;
   RPM=0;
   
  //esp_task_wdt_reset(); 

  speedwind = 0;
}