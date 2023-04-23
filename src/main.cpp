//-----------------------------------------------------------------------------------------------------
//                                     Refatoração
#include <biblioteca_solar.h>
#include <biblioteca_sdcard.h>
#include <biblioteca_biruta.h>
#include <biblioteca_pressao.h>
#include <biblioteca_rtc.h>
#include <biblioteca_ntp.h>
#include <biblioteca_display.h>
#include <biblioteca_eeprom.h>
#include <parametros.h>
#include <testwf1.h>

//-----------------------------------------------------------------------------------------------------

#include <AsyncTCP.h>
#include <Wire.h> //biblioteca para a configuração dos pinos SDA e SCL do sensor
#include <SPI.h>
#include "FS.h"
#include <esp_task_wdt.h> //Biblioteca do watchdog
#include "BluetoothSerial.h"
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//------Porta física dos Sensores--------
#define PIN_VL 4   // P4 entrada pluviometro
#define RSTp 2     // P2 RST pluviometro
#define Led1 12    //  P12  15 dht ... 14 ..........>>>>>>>>>>>>>
#define PIN_VLR 13 // Reset wifi 13
#define Conec 25   // Saida p pulso  5

// Declaração das funções
int ReadWord(int endereco);
void connectWifi();
int ReadByte(int endereco);
void WriteByte(int endereco, int dado);
void writeFile(fs::FS &fs, const char *path, const char *message);
void setupNTP();
void WriteWord(int endereco, int dado);
void appendFile(fs::FS &fs, const char *path, const char *message);
bool readBlock();
void Pluv();
void ProcessaCadaSegundo();
void ProcessaCadaMinuto();
void ProcessaCadaHora();
void ProcessaCadaDia();
void RTCset();
void windvelocity();
void TestCh();
void TimeStamp();
void LeTempUmiPress();
float DirWind();
void MedLuz();
void logSDCard();
void TestWf();


//---------Plataformas---------
// retiradas
const char *server = "api.thingspeak.com";

byte e2pBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
//-----------------------------------------------------------------------------------------------------
//                                      SETUP
//-----------------------------------------------------------------------------------------------------
void setup()
{

  Serial.begin(115200);
  while (!Serial)
    ;
  Wire.begin();          // P21 SDA  - P22 SCL
  Wire.setClock(100000); // Temporizacao entre dispositivos  4
  inicicializar_menssagens_ligar_estacao();
  // Habilita o watchdog configurando o timeout para 50 segundos
  esp_task_wdt_init(60, true);
  esp_task_wdt_add(NULL);

  pinMode(Anem, INPUT);     // input anem
  pinMode(RSTp, OUTPUT);    // reset pluviometro
  pinMode(PIN_VL, INPUT);   // entrada pluvio
  digitalWrite(RSTp, HIGH); // ativa pluviom
  pinMode(prog_pin, INPUT_PULLUP);
  pinMode(Led1, OUTPUT);          // LED 1
  digitalWrite(Led1, LOW);        // Apaga o Led 1
  pinMode(Conec, OUTPUT);         // Piloto wifi
  digitalWrite(Conec, LOW);       // Off
  pinMode(PIN_VLR, INPUT_PULLUP); // entrada chave rts wf
  // int Push_button_state = digitalRead(prog_pin);
  // if ( Push_button_state != HIGH) {bmp.begin();dht.begin();LeTempUmiPress();BT();}

  for (int i = 0; i < 5; i++)
  {
    digitalWrite(Led1, HIGH); // Acende o Led
    delay(200);
    digitalWrite(Led1, LOW); // Apaga o Led
    delay(200);
  }

  inicicializar_menssagens_DHT();

  dht.begin();
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Falha na leitura do Sensor DHT sensor!"));
    display.drawString(60, 0, "ERRO");
  }
  else
  {
    display.drawString(60, 0, "OK");
    Serial.println(F("OK"));
  }
  display.display();

  inicicializar_menssagens_RTC();
  if (!rtc.begin())
  {
    Serial.println(F("RTC não encontrado"));
    Serial.flush();
    display.clear();
    display.drawString(60, 12, "FALHA");
    display.display();
    //   abort();
  }
  else
  {
    display.drawString(60, 12, "OK");
    display.display();
    Serial.println(F("OK"));
  }

  Serial.print(F("Verificando alimentação RTC: "));
  display.drawString(0, 24, "RTC PW:");
  display.display();
  delay(1000);
  // rtc.adjust(DateTime(F(_DATE), F(TIME_)));

  if (rtc.lostPower())
  {
    Serial.println(F("Perda de alimentação"));
    display.drawString(60, 24, "ERRO");
    display.display();
  }
  else
  {
    display.drawString(60, 24, "OK");
    display.display();
    Serial.println(F("OK"));
  }

  Serial.print(F("Iniciando SD_CS Card: "));
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS))
  {
    Serial.println(F("Falha ao colocar Cartao !"));
    // return;
  }
  else
  {
    Serial.println(F("OK"));
  }

  Serial.print(F("Verificando presença SD_CS Card: "));
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println(F("SD card nâo encontrado"));
    // return;
  }
  else
  {
    Serial.println(F("OK"));
    LogOk = true;
  }

  Serial.print(F("Inicializando SD Card : "));
  if (!SD.begin(SD_CS))
  {
    Serial.println(F("ERROR - SD Card initialização falhou ! "));
    LogOk = false;
    // return;    // init failed
  }
  else
  {
    Serial.println(F("OK"));
  }
  File file = SD.open("/data.txt");
  if (!file)
  {
    Serial.println(F("Arquivo não existe ! "));
    Serial.println(F("Criando arquivo ..."));
    writeFile(SD, "/data.txt", "Date ; Time ; mm/h ; mm/dia ; Temp ; Umidade ; Pressao ; Vmed ; Vmax ; dir ; UV ; W/m2 ; \r\n");
  }
  else
  {
    Serial.println(F("O arquivo já existe "));
  }
  file.close();

  inicicializar_menssagens_BMP();
  unsigned status;
  status = bmp.begin();
  if (!status)
  {
    display.drawString(60, 36, "ERRO");
    display.display();
    Serial.println(F("ERRO"));
  }
  else
  {
    Serial.println(F("OK"));
    display.drawString(60, 36, "OK");
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

  // solar radiation
  lightMeter.begin();

  Serial.println(F("Adafruit SI1145 test"));
  if (!uv.begin())
  {
    Serial.println(F("Didn't find Si1145"));
    // digitalWrite(Led1, HIGH); // Acende o Led
    // while (1);
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
  if (ntpbol)
  {
    Serial.println(F("Nao pode acertar RTC "));
  }
  else
  {
    Serial.println(F("Acertar RTC "));
    RTCset(); // acertar RTC
  }
  //-----------------------
  // WriteByte(hrAnte2p,hr);
  // WriteByte(diaAnte2p,dia);

  hrAnt = ReadByte(hrAnte2p);
  diaAnt = ReadByte(diaAnte2p);
  REEDCOUNT = ReadWord(mmHe2p);
  REEDh = ReadWord(mmDe2p);
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
void loop()
{
  sTout = 5;

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
void ProcessaCadaSegundo()
{
  if (sg != sgAnt)
  {
    windvelocity(); // Le anemometro
    Pluv();         // le pluviometro
    // BT();           // Verifica BlueTooth
    TestCh();    // Verifica Reset Wifi
    TimeStamp(); // Atualiza display
  }
  sgAnt = sg;
}
//-----------------------------------------------------------------------------------------------------
void ProcessaCadaMinuto()
{
  if (minAnt != mn)
  {

    LeTempUmiPress();

    //----Pluviometro---------
    REEDCOUNT = ReadWord(mmHe2p);
    REEDh = ReadWord(mmDe2p);
    rain = REEDCOUNT * 0.1; // PL3 0.10 ou PL1/2  0.25 ---------------------- >  >  >  >  >  >  >  >  >  >
    rainh = REEDh * 0.1;    //.......................

    Serial.println(F(NL));

    vmd = vm / NL; // número de leituras do anemometro

    DirWind(); // verifica biruta

    MedLuz(); // Mede dados luz

    if (LogOk)
    {
      digitalWrite(Led1, HIGH); // Acende o Led
      logSDCard();              // Salva dados no SD Card
      delay(1500);
      digitalWrite(Led1, LOW); // Acende o Led
    }

    Serial.println(F("Testa WiFi .... "));

    TestWf(); // Verifica wifi

    TestWf1(); // Verifica roteador

    //----------------

    if (ntpbol)
    {
      setupNTP();
    }

    Wtempo();

    WriteByte(hrAnte2p, hr);
    WriteByte(diaAnte2p, dia);

    Pluv(); // le pluviometro

    vm = 0; // zera variaveis apos envio
    vmax = 0;
    NL = 0;
  }
  minAnt = mn;
}
//-----------------------------------------------------------------------------------------------------
void ProcessaCadaHora()
{
  if (hrAnt != hr)
  {
    REEDCOUNT = 0;
    WriteWord(mmHe2p, 0);
    WriteByte(hrAnte2p, hr);
    Serial.println(F("Resetando Contador milimetros por Hora"));
  }
  hrAnt = hr;
}
//-----------------------------------------------------------------------------------------------------
void ProcessaCadaDia()
{
  if (diaAnt != dia)
  {
    REEDCOUNT = 0;
    REEDh = 0;
    WriteWord(mmHe2p, 0);
    WriteWord(mmDe2p, 0);
    WriteByte(diaAnte2p, dia);
    Serial.println(F("Resetando Contador milimetros por Hora e dia "));

    //----------------
    if (ntpbol)
    {
      Serial.println(F("Nao pode acertar RTC "));
    }
    else
    {
      Serial.println(F("Acertar RTC "));
      RTCset(); // acertar RTC
    }
    //-----------------------
  }
  diaAnt = dia;
}
//-----------------------------------------------------------------------------------------------------
//                                            Sensores
//-----------------------------------------------------------------------------------------------------
void Pluv()
{

  if (digitalRead(PIN_VL) == HIGH)
  {

    REEDCOUNT = ReadWord(mmHe2p);
    REEDh = ReadWord(mmDe2p);
    REEDCOUNT++;
    REEDh++;
    WriteWord(mmHe2p, REEDCOUNT);
    WriteWord(mmDe2p, REEDh);

    Serial.println(F(""));
    Serial.print(F("Pulso p hora : "));
    Serial.print(F(REEDCOUNT));
    Serial.print(F(" Pulso p dia : "));
    Serial.println(F(REEDh));

    delay(100);
    digitalWrite(RSTp, LOW); // reseta pluviom
    Serial.println(F("Reseta hw externo "));
    delay(100);
    digitalWrite(RSTp, HIGH); // ativa pluviom e finaliza leitura
  }
  Serial.print(F("."));
}
//-----------------------------------------------------------------------------------------------------
//                                              LOG
//-----------------------------------------------------------------------------------------------------
// Write the sensor readings on the SD card
void logSDCard()
{
  tempo();
  String dataString2 = "";
  dataString2 = dataString2 + ";" + rain + ";" + rainh + ";" + t + ";" + h + ";" + Pr + ";" + vmd + ";" + vmax + ";" + winddir_eu + ";" + UVmax + ";" + solar + ";" + "\r\n";
  dataMessage = DataAtual + dataString2;
  Serial.println(F(""));
  Serial.print(F("Save data : "));
  Serial.println(dataMessage);
  appendFile(SD, "/data.txt", dataMessage.c_str());
}
//-----------------------------------------------------------------------------------------------------
// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println(F("Falha ao abrir o arquivo para gravação..."));
    return;
  }
  if (file.print(F(message)))
  {
    Serial.println(F("Arquivo escrito..."));
  }
  else
  {
    Serial.println(F("Falha na gravação..."));
  }
  file.close();
}
//-----------------------------------------------------------------------------------------------------
// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Anexando ao arquivo: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println(F("Falha ao abrir para anexar ... "));
    return;
  }
  if (file.print(F(message)))
  {
    Serial.println(F("Mensagem anexada..."));
  }
  else
  {
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

  if (client.connect(server, 80))
  {
    Serial.println(F(". . . WiFi Client Conectado ... "));

    dBm = WiFi.RSSI();
    Serial.print(F("WiFi nivel : "));
    Serial.println(F(dBm));

  } // end if
  else
  {
    client.stop();
    Serial.println(F(". . .  WiFi Client Nao Conectado ... "));
    connectWifi();
  }

  client.stop(); // rever posição
  esp_task_wdt_reset();
} // end send

void RTCset()
{
  rtc.adjust(DateTime(Wano, Wmes, Wdia, Whr, Wmn, Wsg));
}
//-----------------------------------------------------------------------------------------------------
//                                      SUB-ROTINAS
//-----------------------------------------------------------------------------------------------------
void TimeStamp()
{
  tempo();
  String dataString = DataAtual;
  display.clear();
  display.drawString(10, 0, dataString);
  for (int i = 0; i < 127; i++)
  {
    display.setPixel(i, 12);
  }
  display.drawString(0, 14, "Pluv/H.");
  display.drawString(55, 14, ":");
  display.drawString(60, 14, String(rain)); // REEDCOUNT*0.25
  display.drawString(0, 26, "Pluv/D.");
  display.drawString(55, 26, ":");
  display.drawString(60, 26, String(rainh)); // REEDh*0.25
  display.drawString(0, 38, "Temp./Umi.");
  display.drawString(55, 38, ":");
  display.drawString(60, 38, String(t) + " / " + String(h));
  display.drawString(0, 50, "Pressao");
  display.drawString(55, 50, ":");
  display.drawString(60, 50, String(Pr));
  display.display();
}
//-----------------------------------------------------------------------------------------------------
int ReadWord(int endereco)
{
  EEPROM.begin(512);
  int aux = EEPROM.read(endereco);
  aux = (aux * 256) + EEPROM.read(endereco + 1);
  EEPROM.end();
  return aux;
}
//-----------------------------------------------------------------------------------------------------
void WriteByte(int endereco, int dado)
{
  EEPROM.begin(512);
  EEPROM.write(endereco, dado);
  delay(10);
  EEPROM.commit();
  delay(10);
}
//-----------------------------------------------------------------------------------------------------
void WriteWord(int endereco, int dado)
{
  EEPROM.begin(512);
  EEPROM.write(endereco, dado / 256);
  delay(10);
  EEPROM.write(endereco + 1, dado % 256);
  delay(10);
  EEPROM.commit();
  delay(10);
}
//-----------------------------------------------------------------------------------------------------
int ReadByte(int endereco)
{
  EEPROM.begin(512);
  int aux = EEPROM.read(endereco);
  EEPROM.end();
  return aux;
}
//-----------------------------------------------------------------------------------------------------
bool writeBlock()
{
  e2pBuffer[1] = REEDCOUNT / 256;
  e2pBuffer[2] = REEDCOUNT % 256;
  e2pBuffer[3] = REEDh / 256;
  e2pBuffer[4] = REEDh % 256;
  e2pBuffer[5] = hrAnt;
  e2pBuffer[6] = diaAnt;
  byte crc = e2pBuffer[1] + e2pBuffer[1] + e2pBuffer[2] + e2pBuffer[3] + e2pBuffer[4] + e2pBuffer[5] + e2pBuffer[6];
  e2pBuffer[7] = crc;
  EEPROM.begin(512);
  for (int i = 1; i <= 7; i++)
  {
    EEPROM.write(i, e2pBuffer[i]);
    delay(10);
  }
  EEPROM.commit();
  if (readBlock())
    return true;
  else
    return false;
}
//-----------------------------------------------------------------------------------------------------
bool readBlock()
{
  EEPROM.begin(512);
  e2pBuffer[1] = EEPROM.read(1);
  e2pBuffer[2] = EEPROM.read(2);
  e2pBuffer[3] = EEPROM.read(3);
  e2pBuffer[4] = EEPROM.read(4);
  e2pBuffer[5] = EEPROM.read(5);
  e2pBuffer[6] = EEPROM.read(6);
  e2pBuffer[7] = EEPROM.read(7);
  byte crc = e2pBuffer[1] + e2pBuffer[1] + e2pBuffer[2] + e2pBuffer[3] + e2pBuffer[4] + e2pBuffer[5] + e2pBuffer[6];
  EEPROM.end();
  if (crc == e2pBuffer[7])
  {
    REEDCOUNT = (e2pBuffer[1] / 256) + e2pBuffer[2] % 256;
    REEDh = (e2pBuffer[3] / 256) + e2pBuffer[4] % 256;
    hrAnt = e2pBuffer[5];
    diaAnt = e2pBuffer[6];
    return true;
  }
  else
    return false;
}
//-----------------------------------------------------------------------------------------------------
void LeTempUmiPress()
{
  h = dht.readHumidity();
  t = dht.readTemperature();
  p = bmp.readPressure();
  ti = bmp.readTemperature();
  if ((t >= 60) || (t <= -10) || (p < 30000) || (isnan(h)) || (isnan(t)))
  {
    Serial.println(" ATENCAO : Erro no DHT22 . . . . . . ");
    t = ti * 0.8;
    h = 35;
  }
  if (h >= 99)
  {
    h = 98.5;
  }
  Serial.print("Temperatura Ext: ");
  Serial.print(t);
  Serial.print(" C :: Temp. Inter. : ");
  Serial.print(ti);
  Serial.print("  C :: Pressao : ");
  Serial.println(p);

  Pr = (((p) / pow((1 - ((float)(H)) / 44330), 5.255)) / 100.0);
  Serial.print(" Pressao Rel = ");
  Serial.print(Pr);
  Serial.println(" Pa ");
}

//-------------------------------------------------------------------------
void MedLuz()
{
  // solar radiation
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.println(lux);
  solar = (lux * 0.073); //(lux / 100) * 0.79  -- 0.0353
  Serial.print(" Radiacao Solar : ");
  Serial.print(solar);
  Serial.println("  W/mm2");
  // UV
  Serial.print("Vis: ");
  Serial.println(uv.readVisible());
  Serial.print("IR: ");
  Serial.println(uv.readIR());
  UVmax = uv.readUV();
  UVmax /= 100.0;
  UVmax = 0; // ATENÇÃO . . . . . .
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
  // wifiManager.autoConnect("Estacao Teste");
  wifiManager.setTimeout(sTout);

  if (!wifiManager.autoConnect("Conectar Estacao"))
  {
    Serial.println("failed to connect and hit timeout");
    digitalWrite(Conec, LOW); //  off
  }
  else
  {
    digitalWrite(Conec, HIGH); //  P27 ON
    delay(1500);
  }

  esp_task_wdt_reset();
}
/*
  esp_task_wdt_reset(); {
  */

//-----------Testa botao reset wf ---------
void TestCh()
{
  int vlr = digitalRead(PIN_VLR);
  // HIGH
  if (vlr != LOW)
  {
    return;
  }
  sTout = 50;
  Serial.println("resetando");
  digitalWrite(Led1, HIGH); // Acende o Led
  delay(1000);
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
  // End TestCh
}
//------------------------------------------------------------
float DirWind()
{
  for (int i = 0; i < 20; i++)
  {
    wd = analogRead(windirinp);
    Serial.println(wd);
    wds = wds + wd;
    delay(50);
  }
  winddirstate = wds / 20;
  if ((winddirstate >= 3150) && (winddirstate <= 5000))
  {
    winddir_eu = 0;
  }
  if ((winddirstate >= 0) && (winddirstate <= 99))
  {
    winddir_eu = 315; // N 45 I 315
  }
  if ((winddirstate >= 100) && (winddirstate <= 599))
  {
    winddir_eu = 270; // N 90 I 270
  }
  if ((winddirstate >= 600) && (winddirstate <= 1119))
  {
    winddir_eu = 225; // N 135 I 225
  }
  if ((winddirstate >= 1120) && (winddirstate <= 1649))
  {
    winddir_eu = 180;
  }
  if ((winddirstate >= 1650) && (winddirstate <= 2149))
  {
    winddir_eu = 135; // N 225  I 135
  }
  if ((winddirstate >= 2150) && (winddirstate <= 2599))
  {
    winddir_eu = 90; // N 270  I 90
  }
  if ((winddirstate >= 2600) && (winddirstate <= 3149))
  {
    winddir_eu = 45; // N 315  I 45
  }

  wd = 0;
  wds = 0;
  Serial.print(" -- Pin Status: ");
  Serial.println(winddirstate);
  //}
  return winddir_eu;
}
// Measure wind speed  -------------------------------------------------
void windvelocity()
{
  NL++;
  lastMillis = xTaskGetTickCount();
  while (xTaskGetTickCount() - lastMillis < 3000)
  {
    if (digitalRead(Anem) == HIGH)
      if (state == false)
      {
        delay(50);
        clicked++;
        state = true;
      }
    if (digitalRead(Anem) == LOW)
      if (state == true)
        state = false;
  }

  RPM = clicked * 20;
  speedwind = ((30.77 * RPM) / 1000) * 3.6;

  vm = vm + speedwind;

  if (speedwind > vmax)
  {
    vmax = speedwind;
  }

  Serial.print(" - Veloc : ");
  Serial.print(speedwind);
  Serial.print(" - Vmd : ");
  Serial.println(vm);
  clicked = 0;
  RPM = 0;
  // esp_task_wdt_reset();
  speedwind = 0;
}