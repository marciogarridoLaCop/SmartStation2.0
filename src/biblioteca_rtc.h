//---------RTC-------------
#include "RTClib.h"
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