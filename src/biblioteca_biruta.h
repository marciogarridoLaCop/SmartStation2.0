//---------DHT-----------
#include "DHT.h"
#define DHTPIN  15 // Pin 33 interfere no biruta 
#define DHTTYPE DHT22   // AM2301 
DHT dht(DHTPIN, DHTTYPE,20);
//---------DHT-----------