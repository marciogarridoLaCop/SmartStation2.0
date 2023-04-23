
int H = 1035 ;  // altitude do local de instalacao em metros

boolean state = false;

boolean tx_thing=false;
boolean tx_wunderg=false;
boolean LogOk=false;
boolean tx_appwrf=false;
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