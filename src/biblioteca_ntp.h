//Objeto responsável por recuperar dados sobre horário
#include <NTPClient.h> //Biblioteca NTPClient modificada
#include <WiFiUdp.h> //Socket UDP
boolean ntpbol=false;  
//Socket UDP que a lib utiliza para recuperar dados sobre o horário
WiFiUDP udp;
NTPClient ntpClient(
    udp,                    //socket udp
    "0.br.pool.ntp.org",    //URL do servwer NTP
    timeZone*3600,          //Deslocamento do horário em relacão ao GMT 0
    60000);                 //Intervalo entre verificações online
 
//Nomes dos dias da semana
const char* dayOfWeekNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setupNTP()
{
  // Inicializa o client NTP
  ntpClient.begin();
  // Espera pelo primeiro update online
  Serial.println(F("Aguardando atualização"));
  if (!ntpClient.update())
  {
    Serial.print(F(" Forcar Update . . .  "));
    ntpClient.forceUpdate();
    delay(500);
  }

  if (!ntpClient.update())
  {
    ntpbol = true;
  }
  else
  {
    ntpbol = false;

    // Serial.println(F());
    Serial.println(F("Primeira atualização OK"));
  }
}

Date getDate()
{
  // Recupera os dados de data e horário usando o client NTP
  char *strDate = (char *)ntpClient.getFormattedTime().c_str();

  // Passa os dados da string para a struct
  Date date;
  sscanf(strDate, "%d-%d-%dT%d:%d:%dZ",
         &date.year,
         &date.month,
         &date.day,
         &date.hours,
         &date.minutes,
         &date.seconds);

  // Dia da semana de 0 a 6, sendo 0 o domingo
  date.dayOfWeek = ntpClient.getDay();
  return date;
}

void Wtempo()
{
  // Recupera os dados sobre a data e horário
  Serial.println(F("  Data / Hora  web . . .  "));

  Date date = getDate();

  Wano = date.year;
  Wmes = date.month;
  Wdia = date.day;
  Whr = date.hours;
  Wmn = date.minutes;
  Wsg = date.seconds;

  // Serial.println(F());
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
  // Serial.println(F());

  delay(250);
}

void tempo()
{
  DateTime now = rtc.now();
  ano = now.year();
  mes = now.month();
  dia = now.day();
  hr = now.hour();
  mn = now.minute();
  sg = now.second();
  String dataString = "";
  if (ano < 10)
  {
    dataString += "0";
  }
  dataString += String(ano) + "/";
  if (mes < 10)
  {
    dataString += "0";
  }
  dataString += String(mes) + "/";
  if (dia < 10)
  {
    dataString += "0";
  }
  dataString += String(dia) + " ";
  if (hr < 10)
  {
    dataString += "0";
  }
  dataString += String(hr) + ":";
  if (mn < 10)
  {
    dataString += "0";
  }
  dataString += String(mn) + ":";
  if (sg < 10)
  {
    dataString += "0";
  }
  dataString += String(sg) + " ";
  DataAtual = dataString;
}
