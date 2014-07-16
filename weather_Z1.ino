/*
Author: Iuri Iakovlev <krotos139@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

  (Это свободная программа: вы можете перераспространять ее и/или изменять
   ее на условиях Стандартной общественной лицензии GNU в том виде, в каком
   она была опубликована Фондом свободного программного обеспечения; либо
   версии 3 лицензии, либо (по вашему выбору) любой более поздней версии.

   Эта программа распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <http://www.gnu.org/licenses/>.)
*/

// Interfaces
#define Z1_I_WEB
#define Z1_I_SERIAL
#define Z1_I_MODBUS

// Options
#undef Z1_O_DHCP

#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>
#include "DHT.h"
#include "BH1750.h"
#include "Adafruit_BMP085.h"
#include "LedControl.h"

#ifdef Z1_I_MODBUS
#include "Mudbus.h"
#endif

#ifdef Z1_I_WEB
#include "WebServer.h"
#endif

#define WEATHER_STATION_Z1 0x20

// ===============================================================
#define DHT_S1_PIN A0    // пин для датчика DHT22
// ===============================================================
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

#ifndef Z1_O_DHCP
IPAddress ip(192,168,0,20);
IPAddress gateway(192,168,0,1);	
IPAddress subnet(255, 255, 255, 0);
#endif
// ===============================================================
float humidity = 0, temp_dht = 0, temp_bmp = 0, temp = 0;
uint16_t light = 0;
int32_t pressure_pa = 0, pressure_mm = 0;
int mode = 0;
// ===============================================================

dht dht_s1;

BH1750 lightMeter;

Adafruit_BMP085 bmp;

/* This creates an instance of the webserver.  By specifying a prefix
 * of "", all pages will be at the root of the server. */
#define PREFIX ""
#ifdef Z1_I_WEB
WebServer webserver(PREFIX, 80);
#endif

#ifdef Z1_I_MODBUS
#define DEV_ID Mb.R[0]
#define TEMPERATURE Mb.R[1]
#define TEMPERATURE_DHT Mb.R[2]
#define TEMPERATURE_BMP Mb.R[3]
#define HUMIDITY Mb.R[4]
#define PRESSURE_MM Mb.R[5]
#define LIGHT Mb.R[6]
Mudbus Mb;
#endif

// pin A5 is connected to the DataIn 
// pin A6 is connected to the CLK 
// pin A7 is connected to LOAD 
LedControl lc=LedControl(A1,A2,A3,1);

// ======================== Web pages ==========================
#ifdef Z1_I_WEB
void web_index(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  server.httpSuccess("application/xml; charset=utf-8");

  if (type != WebServer::HEAD)
  {
    P(index_p1) = 
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<?xml-stylesheet type=\"text/xsl\" href=\"http://192.168.0.20/z1.xsl\"?>"
      "<response>"
      "	<temperature>"
      "		<celsius>";
    P(index_p2) = "</celsius>"
      "		<sensors>"
      "			<sensor name='BMP' unit='C'>";
    P(index_p3) = "</sensor>"
      "			<sensor name='DHT' unit='C'>";
    P(index_p4) = "</sensor>"
      "		</sensors>"
      "	</temperature>"
      "	<humidity>"
      "		<percentage>";
    P(index_p5) = "</percentage>"
      "	</humidity>"
      "	<pressure>"
      "		<pa>";
    P(index_p6) = "</pa>"
      "		<mmHg>";
    P(index_p7) = "</mmHg>"
      "	</pressure>"
      "	<illuminance>"
      "		<lx>";
    P(index_p8) = "</lx>"
      "	</illuminance>"
      "</response>";
    // this is a special form of print that outputs from PROGMEM 
    server.printP(index_p1);
    server.print(temp);
    server.printP(index_p2);
    server.print(temp_bmp);
    server.printP(index_p3);
    server.print(temp_dht);
    server.printP(index_p4);
    server.print(humidity);
    server.printP(index_p5);
    server.print(pressure_pa);
    server.printP(index_p6);
    server.print(pressure_mm);
    server.printP(index_p7);
    server.print(light);
    server.printP(index_p8);
  }
}
void web_z1_xsl(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  server.httpSuccess("text/xsl; charset=utf-8");
  if (type != WebServer::HEAD)
  {
    P(z1_xsl) = 
    "<?xml version='1.0' encoding='UTF-8'?>"
    "<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>"
    "<xsl:template match='/'>"
    "  <html>"
    "  <head>"
    "	  <title>Weather station</title>"
    "     <meta http-equiv='refresh' content='5'/>"
    "	  <style>"
    "	   .z1 {"
    "			font-family:Arial, Helvetica, sans-serif;"
    "			color:#666;"
    "			font-size:12px;"
    "			text-shadow: 1px 1px 0px #fff;"
    "			background:#eaebec;"
    "			margin:20px;"
    "			border:#ccc 1px solid;"
    "			border-collapse:separate; "
    "			border-radius:3px;"
    "			box-shadow: 0 1px 2px #d1d1d1;"
    "	   }"
    "	   .z1 th {"
    "			font-weight:bold;"
    "			padding:15px;"
    "			border-bottom:1px solid #e0e0e0;"
    "			background: #ededed;"
    "			background: linear-gradient(to top,  #ededed,  #ebebeb);"
    "	   }"
    "	   .z1 td {"
    "			padding:10px;"
    "			background: #f2f2f2;"
    "			background: linear-gradient(to top,  #f2f2f2,  #f0f0f0);  "
    "	   }"
    "		.z1 tr:hover td{"
    "			background: #aaaaaa;"
    "			background: linear-gradient(to top, #f2f2f2,  #e0e0e0);  "
    "		}"
    "	  </style>"
    "  </head>"
    "  <body>"
    "  <h2>Weather station</h2>"
    "    <table class='z1'>"
    "      <tr>"
    "        <th>Property</th>"
    "        <th>Value</th>"
    "      </tr>"
    "      <tr>"
    "        <td> Temperature </td>"
    "        <td><xsl:value-of select='response/temperature/celsius'/> C</td>"
    "      </tr>"
    "      <tr>"
    "        <td> Humidity </td>"
    "        <td><xsl:value-of select='response/humidity/percentage'/> %</td>"
    "      </tr>"
    "      <tr>"
    "        <td> Pressure </td>"
    "        <td><xsl:value-of select='response/pressure/mmHg'/> mm.Hg</td>"
    "      </tr>"
    "      <tr>"
    "        <td> Illuminance </td>"
    "        <td><xsl:value-of select='response/illuminance/lx'/> lx</td>"
    "      </tr>"
    "    </table>"
    "	<h2>Termosensor</h2>"
    "    <table class='z1'>"
    "      <tr>"
    "        <th>Sensor</th>"
    "        <th>Value</th>"
    "      </tr>"
    "	  <xsl:for-each select='response/temperature/sensors/sensor'>"
    "      <tr>"
    "        <td> <xsl:value-of select='@name'/> </td>"
    "        <td><xsl:value-of select='.'/> <xsl:value-of select='@unit'/></td>"
    "      </tr>"
    "	  </xsl:for-each>"
    "    </table>"
    "  </body>"
    "  </html>"
    "</xsl:template>"
    "</xsl:stylesheet>";

    // this is a special form of print that outputs from PROGMEM 
    server.printP(z1_xsl);
  }
}
#endif

// ========================СТАРТУЕМ=============================
void setup(){
  //enable serial data print
  Serial.begin(9600); 
  Serial.println("Weather Z1 v 0.2"); // Тестовые строки для отображения в мониторе порта
  
  // Init LED display
  lc.shutdown(0,false);
  lc.setIntensity(0,2);
  lc.clearDisplay(0);
  lc.setChar(0,7,'L',false);
  lc.setChar(0,6,'O',false);
  lc.setChar(0,5,'A',false);
  lc.setChar(0,4,'d',false);
  
  //запускаем Ethernet
  SPI.begin();
#ifdef Z1_O_DHCP
  Ethernet.begin(mac);
#else
  Ethernet.begin(mac, ip);
#endif

  // Init Light sensor
  lightMeter.begin();
  
  // Init pressure sensor
  if (!bmp.begin()) {
    Serial.println("ERROR: BMP085 sensor failed");
  }
  
#ifdef Z1_O_DHCP
  Serial.print("IP:");
  Serial.println(Ethernet.localIP());
#endif

#ifdef Z1_I_WEB
  webserver.setDefaultCommand(&web_index);
  webserver.addCommand("index.html", &web_index);
  webserver.addCommand("z1.xsl", &web_z1_xsl);
#endif  
//  webserver.begin();
}

void loop(){
  char buff[64];
  int len = 64;
  
  mode = (mode + 1) % 100;

  Z1_sensors_update();
  
#ifdef Z1_I_SERIAL
  Z1_SerialOutput();
#endif
  
  Z1_ledDisplay();
  
#ifdef Z1_I_MODBUS
  Z1_modbus_tcp_slave();
#endif

#ifdef Z1_I_WEB
  webserver.processConnection(buff, &len);
#endif
}

void Z1_sensors_update() {
 if (mode%30==0) {

  // BH1750 (light)
  light = lightMeter.readLightLevel();
  // BMP085 (Temp and Pressure)
  temp_bmp = bmp.readTemperature();
  pressure_pa = bmp.readPressure();
  pressure_mm = pressure_pa/133.3;
  // DHT22 (Temp)
  if (dht_s1.read22(DHT_S1_PIN) == DHTLIB_OK) {
    humidity = dht_s1.humidity;
    temp_dht = dht_s1.temperature;
    temp = temp_dht;
  } else {
    temp = temp_bmp;
  }
  
 }
}

void Z1_SerialOutput() {
  Serial.print("T1= "); 
  Serial.print(temp_dht);
  Serial.print(" *C \t");
  Serial.print("T2= "); 
  Serial.print(temp_bmp);
  Serial.print(" *C \t");
  Serial.print("Pressure= "); 
  Serial.print(pressure_mm);
  Serial.print(" mm \t");
  Serial.print("Humidity= "); 
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Light= "); 
  Serial.print(light);
  Serial.print(" lx \t");
  Serial.print("\n");

}

void Z1_ledDisplay() {
  int v;
  
  if (light<50) {
    lc.setIntensity(0,0);
  } else if (light>80 && light<200) {
    lc.setIntensity(0,2);
  } else if (light>250 && light<1000) {
    lc.setIntensity(0,5);
  } else if (light>1100) {
    lc.setIntensity(0,15);
  }
  
  if (mode<=25) {
  //  lc.clearDisplay(0);
    lc.setChar(0,7,'t',false);
    if (temp>=0) {
      lc.setChar(0,6,' ',false);
    } else {
      lc.setChar(0,6,'-',false);
    }
    v = (int)( temp / 10 ) % 10;
    lc.setDigit(0,5,(byte)v,false);
    v = (int)( temp  ) % 10;
    lc.setDigit(0,4,(byte)v,true);
    v = (int)( temp * 10 ) % 10;
    lc.setDigit(0,3,(byte)v,false);
    lc.setChar(0,2,' ',false);
    lc.setChar(0,1,'*',false);
    lc.setChar(0,0,'C',false);
    delay(1);
  } else if (mode<=50) {
  //  lc.clearDisplay(0);
    lc.setChar(0,7,'H',false);
    lc.setChar(0,6,' ',false);
    v = (int)( humidity / 10 ) % 10;
    lc.setDigit(0,5,(byte)v,false);
    v = (int)( humidity  ) % 10;
    lc.setDigit(0,4,(byte)v,true);
    v = (int)( humidity * 10 ) % 10;
    lc.setDigit(0,3,(byte)v,false);
    lc.setChar(0,2,' ',false);
    lc.setChar(0,1,'*',false);
    lc.setChar(0,0,'o',false);
    delay(1);
  } else if (mode<=75) {
  //  lc.clearDisplay(0);
    lc.setChar(0,7,'P',false);
    lc.setChar(0,6,' ',false);
    v = (int)( pressure_mm / 100 ) % 10;
    lc.setDigit(0,5,(byte)v,false);
    v = (int)( pressure_mm/10  ) % 10;
    lc.setDigit(0,4,(byte)v,false);
    v = (int)( pressure_mm ) % 10;
    lc.setDigit(0,3,(byte)v,false);
    lc.setChar(0,2,' ',false);
    lc.setChar(0,1,'n',false);
    lc.setChar(0,0,'n',false);
    delay(1);
  } else {
  //  lc.clearDisplay(0);
    lc.setChar(0,7,'L',false);
    lc.setChar(0,6,' ',false);
    v = (int)( light / 1000 ) % 10;
    lc.setDigit(0,5,(byte)v,false);
    v = (int)( light / 100  ) % 10;
    lc.setDigit(0,4,(byte)v,false);
    v = (int)( light / 10 ) % 10;
    lc.setDigit(0,3,(byte)v,false);
    v = (int)( light ) % 10;
    lc.setDigit(0,2,(byte)v,false);
    lc.setChar(0,1,' ',false);
    lc.setChar(0,0,' ',false);
    delay(1);
  }
}

#ifdef Z1_I_MODBUS
void Z1_modbus_tcp_slave() {
  Mb.Run();

  DEV_ID = WEATHER_STATION_Z1;
  TEMPERATURE = temp;
  TEMPERATURE_DHT = temp_dht;
  TEMPERATURE_BMP = temp_bmp;
  HUMIDITY = humidity;
  PRESSURE_MM = pressure_mm;
  LIGHT = light;
}
#endif

