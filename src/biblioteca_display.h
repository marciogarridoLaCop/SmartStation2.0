//---------Display---------
#include "SSD1306.h"
SSD1306 display(0x3c, 21, 22);

void inicicializar_menssagens_ligar_estacao()
{
    Serial.println(F(""));
    Serial.println(F("Inicializando WX WF SD 5 ..."));
    Serial.println(F(""));
    display.init();
    display.flipScreenVertically();
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Iniciando...");
    display.display();
    display.setFont(ArialMT_Plain_10);
}

void inicicializar_menssagens_DHT()
{
    Serial.print(F("Inicializando DHT: "));
    display.clear();
    display.drawString(0, 0, "BME:");
    display.display();
    delay(1000);
}

void inicicializar_menssagens_RTC()
{
    Serial.print(F("Inicializando RTC: "));
    display.drawString(0, 12, "RTC:");
    display.display();
    delay(1000);
}

void inicicializar_menssagens_BMP()
{
    Serial.print(F("Inicializando BMP: "));
    display.drawString(0, 36, "BMP:");
    display.display();
    delay(1000);
}