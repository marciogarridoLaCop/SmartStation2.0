#include <WiFi.h>

void TestWf1()
{

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(F(" --- Conectado ao roteador --- "));
  }
  else
  {
    Serial.println(F(" --- Nao conectado ao roteador !!!"));
    
  }
}