#include <rpcWiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

/* Display */
#include "Free_Fonts.h"
#include "TFT_eSPI.h"

/* Sensor */
#include <DHT20.h>
#include <Seeed_HM330X.h>
#include <Air_Quality_Sensor.h>

/* Button */
#include <Bounce2.h>

// INSTANTIATE 3 Button OBJECT
Bounce2::Button btnA = Bounce2::Button();
Bounce2::Button btnL = Bounce2::Button();
Bounce2::Button btnR = Bounce2::Button();

TFT_eSPI         tft;
DHT20            DHT;
AirQualitySensor air_sensor(A0);
HM330X           PM_sensor;

//const char* ssid = "x.factory";
//const char* password =  "make0314";
//const char* URL_BASE = "https://restapi.amap.com/v3/weather/weatherInfo?city=cityCode&key=yourKey";
//const char* URL_ALL  = "https://restapi.amap.com/v3/weather/weatherInfo?city=cityCode&key=yourKey&extensions=all";

const char* ssid = "FCTC_89";
const char* password =  "Lu15899962740";
// 440100
const char* URL_BASE = "https://restapi.amap.com/v3/weather/weatherInfo?city=441802&key=ac901c195798b1f2767987a55ee74156";
const char* URL_ALL  = "https://restapi.amap.com/v3/weather/weatherInfo?city=441802&key=ac901c195798b1f2767987a55ee74156&extensions=all";


WiFiClientSecure client;

#define STR_SIZE_MAX   16

typedef struct lives {
    char province[16];
    char city[16];
    char adcode[16];
    char weather[16];
    char temperature[16];
    char humidity[16];
    char winddirection[16];
    char windpower[16];
    char reporttime[16];
} lives_t;

lives_t lives_data;

typedef struct forecasts {
    char date[16];
    char week[16];
    char dayweather[16];
    char nightweather[16];
    char daytemp[16];
    char nighttemp[16];
    char daywind[16];
    char nightwind[16];
    char daypower[16];
    char nightpower[16];
} forecasts_t;

#define FORECASTS_SIZE  4
forecasts_t forecasts_data[FORECASTS_SIZE];

enum {
    PAGE_1 = 1,
    PAGE_2 = 2,
    PAGE_3 = 3,
    PAGE_4 = 4,
    PAGE_5 = 5,
    PAGE_MAX = PAGE_5
};

int currentPage = PAGE_1;
boolean pageChanged = false;
boolean update_flag = true;

void setup()
{
    btnA.attach( WIO_KEY_C ,  INPUT_PULLUP );
    btnL.attach( WIO_5S_LEFT ,  INPUT_PULLUP );
    btnR.attach( WIO_5S_RIGHT ,  INPUT_PULLUP );

    btnA.interval(5);
    btnL.interval(5);
    btnR.interval(5);

    btnA.setPressedState(LOW);
    btnL.setPressedState(LOW);
    btnR.setPressedState(LOW);

    Serial.begin(115200);
    //while (!Serial);

    // init LCD
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(tft.color565(24,15,60));
    tft.fillScreen(TFT_NAVY);
    tft.setFreeFont(FMB12);
    tft.setCursor((320 - tft.textWidth("Funpack Weather Box"))/2, 100);
    tft.print("Funpack Weather Box");

    delay(5);

    if (! DHT.begin()) {
        Serial.println("Could not find AHT Sensor? Check wiring");
        while (1) delay(10);
    }
    Serial.println("DHT20 init OK!");

    if (! air_sensor.init()) {
        Serial.println("Could not find Air Sensor? Check wiring");
        while (1) delay(10);
    }
    Serial.println("Air sensor found");
    
    if (PM_sensor.init()) {
        Serial.println("Could not find HM330X Sensor? Check wiring");
        while (1) delay(10);
    }
    Serial.println("HM330X sensor found");

    tft.setFreeFont(FM9);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor((320 - tft.textWidth("Connecting WiFi..."))/2, 180);
    tft.print("Connecting WiFi...");
    
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) { //Check for the connection
        delay(500);
        Serial.println("Connecting WiFi...");
    }
    Serial.print("Connected to the WiFi network with IP: ");
    Serial.println(WiFi.localIP());
    //client.setCACert(test_root_ca);

    if(&client) {   
        getWeatherLives();
        getWeatherForecasts();
    }
    drawWeatherLivePage(lives_data);
}

void loop()
{
    btnA.update();
    btnL.update();
    btnR.update();

    if ( btnA.pressed() ) {
        pageChanged = true;
        getWeatherLives();
        getWeatherForecasts();
        currentPage = PAGE_1;
        Serial.print("updated!");
    }

    if ( btnL.pressed() ) {
        pageChanged = true;
        currentPage--;
        if (currentPage < PAGE_1) {
            currentPage = PAGE_MAX;
        }
        Serial.print("prev page: ");Serial.println(currentPage);
    }

    if ( btnR.pressed() ) {
        pageChanged = true;
        currentPage++;
        if (currentPage > PAGE_MAX) {
            currentPage = PAGE_1;
        }
        Serial.print("next page: ");Serial.println(currentPage);
    }
    
    switch (currentPage) {
      case PAGE_1:
      {
        static int cnt = 0;
        if (pageChanged) {
          drawWeatherLivePage(lives_data);
          updateSensorData();
        }
        if (cnt++ % 1000000 == 0) {
          updateSensorData();
        }
        
        pageChanged = false;
      } break;
      case PAGE_2:
      {
        if (pageChanged) {
          drawWeatherForecastPage(&forecasts_data[0]);
        }
        pageChanged = false;
      } break;
      case PAGE_3:
      {
        if (pageChanged) {
          drawWeatherForecastPage(&forecasts_data[1]);
        }
        pageChanged = false;
      } break;
      case PAGE_4:
      {
        if (pageChanged) {
          drawWeatherForecastPage(&forecasts_data[2]);
        }
        pageChanged = false;
      } break;
      case PAGE_5:
      {
        if (pageChanged) {
          drawWeatherForecastPage(&forecasts_data[3]);
        }
        pageChanged = false;
      } break;
      default: break;
    }
}
                    
HM330XErrorCode parse_result_PM2(uint8_t *data, int *PM_value) {
    uint16_t value = 0;
    if (NULL == data) {
        return ERROR_PARAM;
    }
    
    value = (uint16_t) data[3 * 2] << 8 | data[3 * 2 + 1];
    
    *PM_value = value;
    
    return NO_ERROR;
}

void updateSensorData()
{
    // Temperature and Humidity
    int status = DHT.read();
    switch (status)
    {
    case DHT20_OK:
      Serial.print("OK,\t");
      break;
    case DHT20_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHT20_ERROR_CONNECT:
      Serial.print("Connect error,\t");
      break;
    case DHT20_MISSING_BYTES:
      Serial.print("Missing bytes,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
    }

    // Air Quality
    int air_quality = air_sensor.slope();
    int aqi = air_sensor.getValue();

    Serial.print("AQI: "); Serial.println(aqi);

    // PM2.5
    uint8_t buf[30];
    int PM_value;
    if (PM_sensor.read_sensor_value(buf, 29)) {
        Serial.println("HM330X read result failed!!!");
    }
    Serial.println(">>>>>>>>>>>>>>>>>>");
    parse_result_PM2(buf, &PM_value);
    Serial.print("PM2.5: ");
    Serial.println(PM_value);

#if 0
    // This is used to print out on Serial Plotter, check Serial Plotter!
    Serial.print(temp.temperature);
    Serial.print(",");
    Serial.print(humi.relative_humidity);
    Serial.print(",");
    Serial.print(aqi);
    Serial.print(",");
    Serial.println(PM_value);
#endif

    drawTempValue(DHT.getTemperature());
    drawHumiValue(DHT.getHumidity());
    drawAqiValue(aqi);
    drawPm25Value(PM_value);
}

void drawAqiValue(const int aqi) {
   tft.setFreeFont(FM9);

   if (aqi < 50) {
      tft.setTextColor(TFT_GREEN, tft.color565(24,15,60));
   } else if (aqi < 100) {
      tft.setTextColor(TFT_YELLOW, tft.color565(24,15,60));
   } else if (aqi < 150) {
      tft.setTextColor(TFT_ORANGE, tft.color565(24,15,60));
   } else if (aqi < 200) {
      tft.setTextColor(TFT_RED, tft.color565(24,15,60));
   } else if (aqi < 300) {
      tft.setTextColor(TFT_PURPLE, tft.color565(24,15,60));
   } else {
      tft.setTextColor(TFT_MAROON, tft.color565(24,15,60));
   }
   
   tft.drawString("AQI: "+String(aqi)+" ", 20, 210);
}

void drawPm25Value(const int pm) {
   tft.setFreeFont(FM9);

   if (pm < 15) {
      tft.setTextColor(TFT_GREEN, tft.color565(24,15,60));
   } else if (pm < 35) {
      tft.setTextColor(TFT_YELLOW, tft.color565(24,15,60));
   } else if (pm < 55) {
      tft.setTextColor(TFT_ORANGE, tft.color565(24,15,60));
   } else if (pm < 150) {
      tft.setTextColor(TFT_RED, tft.color565(24,15,60));
   } else if (pm < 250) {
      tft.setTextColor(TFT_PURPLE, tft.color565(24,15,60));
   } else if (pm < 300) {
      tft.setTextColor(TFT_MAROON, tft.color565(24,15,60));
   }
   
   tft.drawString("PM2.5: "+String(pm)+" ", 170, 210);
}

void drawTempValue(const float temp) {
   tft.setFreeFont(FMB24);
   tft.setTextColor(TFT_RED, tft.color565(40,40,86));
   tft.drawString(String(temp, 1)+" ", 30, 140);
}

void drawHumiValue(const float humi) {
   tft.setFreeFont(FMB24);
   tft.setTextColor(TFT_GREEN, tft.color565(40,40,86));
   tft.drawString(String(humi, 1)+" ", 180, 140);
}

void drawWeatherLivePage(lives_t &lives_data)
{
    // -----------------LCD---------------------

    tft.fillScreen(tft.color565(24,15,60));
    tft.setFreeFont(FF17);
    tft.setTextColor(tft.color565(224,225,232));
    tft.drawString("Funpack Weather Box", 10, 10);

    tft.setFreeFont(FMB9);
    if (0 == strcmp(lives_data.weather, "晴")) {
      tft.setTextColor(TFT_ORANGE);
      tft.drawString("sunny", 240, 10);
    } else if (0 == strcmp(lives_data.weather, "多云")) {
      tft.setTextColor(TFT_WHITE);
      tft.drawString("cloudy", 240, 10);
    } else if (0 == strcmp(lives_data.weather, "阴")) {
      tft.setTextColor(TFT_LIGHTGREY);
      tft.drawString("cloudy", 240, 10);
    } else if (strstr(forecasts_data->dayweather, "雨")) {
      tft.setTextColor(TFT_DARKCYAN);
      tft.drawString("rainy", 240, 10);
    }
 
    tft.fillRoundRect(10,  45, 145, 150, 5, tft.color565(40,40,86));
    tft.fillRoundRect(165, 45, 145, 150, 5, tft.color565(40,40,86));

    tft.setFreeFont(FM9);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Temperature", 25, 60);
    tft.drawString("Humidity",   195, 60);
    
    tft.setTextColor(TFT_DARKGREY);
    tft.drawString("  'C",  90, 110);
    tft.drawString("% rH", 250, 110);

    tft.setFreeFont(FMB24);
    tft.setTextColor(TFT_RED, tft.color565(40,40,86));
    tft.drawString(lives_data.temperature, 30, 100);
    Serial.println(lives_data.temperature);

    tft.setFreeFont(FMB24);
    tft.setTextColor(TFT_GREEN, tft.color565(40,40,86));
    tft.drawString(lives_data.humidity, 180, 100);
    Serial.println(lives_data.humidity);
}

void drawWeatherForecastPage(forecasts_t *forecasts_data)
{
    // -----------------LCD---------------------

    tft.fillScreen(tft.color565(24,15,60));
    tft.setFreeFont(FF17);
    tft.setTextColor(tft.color565(224,225,232));
    tft.drawString(forecasts_data->date, 120, 10);

    tft.fillRoundRect(10,  45, 300, 180, 5, tft.color565(40,40,86));

    tft.setFreeFont(FM9);
    tft.drawString("    Weather", 25, 60);
    tft.drawString("  Day  temp", 25, 90);
    tft.drawString("Night  temp", 25, 120);
    tft.drawString("  Day power", 25, 150);
    tft.drawString("Night power", 25, 180);

    tft.setFreeFont(FS9);
    tft.setTextColor(TFT_DARKGREY);
    tft.drawString("'C", 250, 90);
    tft.drawString("'C", 250, 120);
    tft.drawString("level", 250, 150);
    tft.drawString("level", 250, 180);

    tft.drawFastVLine(160, 60, 140, TFT_DARKGREY);

    tft.setFreeFont(FMB9);
    tft.setTextColor(TFT_RED, tft.color565(40,40,86));
    tft.drawString(forecasts_data->daytemp, 190, 90);
    
    tft.setTextColor(TFT_GREEN, tft.color565(40,40,86));
    tft.drawString(forecasts_data->nighttemp, 190, 120);
    
    tft.setTextColor(TFT_MAGENTA, tft.color565(40,40,86));
    tft.drawString(forecasts_data->daypower, 190, 150);
    
    tft.setTextColor(TFT_PURPLE, tft.color565(40,40,86));
    tft.drawString(forecasts_data->nightpower, 190, 180);

    tft.setFreeFont(FMB9);
    if (0 == strcmp(forecasts_data->dayweather, "晴")) {
      tft.setTextColor(TFT_ORANGE);
      tft.drawString("sunny", 190, 60);
    } else if (0 == strcmp(forecasts_data->dayweather, "多云")) {
      tft.setTextColor(TFT_WHITE);
      tft.drawString("cloudy", 190, 60);
    } else if (0 == strcmp(forecasts_data->dayweather, "阴")) {
      tft.setTextColor(TFT_LIGHTGREY);
      tft.drawString("cloudy", 190, 60);
    } else if (strstr(forecasts_data->dayweather, "雨")) {
      tft.setTextColor(TFT_DARKCYAN);
      tft.drawString("rainy", 190, 60);
    }
}

void getWeatherLives()
{
    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    HTTPClient https;
    
    Serial.print("[HTTPS] begin... weather lives\n");
    
    if (https.begin(client, URL_BASE))
    {
        Serial.print("[HTTPS] GET...\n");
        
        int httpCode = https.GET();
        
        if (httpCode > 0)
        {
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
            {
                String payload = https.getString();
                Serial.println(payload);
                //payload.toCharArray(lives_payload, payload.length());
                
                const size_t capacity = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(9) + 250;
                DynamicJsonDocument doc(capacity);
                deserializeJson(doc, payload);

                strncpy(lives_data.province, doc["lives"][0]["province"], STR_SIZE_MAX);
                strncpy(lives_data.city, doc["lives"][0]["city"], STR_SIZE_MAX);
                strncpy(lives_data.weather, doc["lives"][0]["weather"], STR_SIZE_MAX);
                strncpy(lives_data.temperature, doc["lives"][0]["temperature"], STR_SIZE_MAX);
                strncpy(lives_data.humidity, doc["lives"][0]["humidity"], STR_SIZE_MAX);
                strncpy(lives_data.winddirection, doc["lives"][0]["winddirection"], STR_SIZE_MAX);
                strncpy(lives_data.windpower, doc["lives"][0]["windpower"], STR_SIZE_MAX);
                strncpy(lives_data.reporttime, doc["lives"][0]["reporttime"], STR_SIZE_MAX);
                
                Serial.print(lives_data.province);Serial.println(lives_data.city);
                Serial.println(lives_data.weather);
                Serial.println(lives_data.temperature);
                Serial.println(lives_data.humidity);
                Serial.println(lives_data.winddirection);
                Serial.println(lives_data.windpower);
                Serial.println(lives_data.reporttime);
            }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        
        https.end();
        
    } else {
        Serial.printf("[HTTPS] Unable to connect\n");
    }
    // End extra scoping block
}

void getWeatherForecasts()
{
    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    HTTPClient https;
    
    Serial.print("[HTTPS] begin... weather lives\n");
    
    if (https.begin(client, URL_ALL))
    {
        Serial.print("[HTTPS] GET...\n");
        
        int httpCode = https.GET();
        
        if (httpCode > 0) 
        {
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
            {
                String payload = https.getString();
                Serial.println(payload);
                
                const size_t capacity = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(5) + 3 * JSON_OBJECT_SIZE(10) + 1250;
                DynamicJsonDocument doc(capacity);
                deserializeJson(doc, payload);

                for (int i=0; i<FORECASTS_SIZE; i++)
                {
                    strncpy(forecasts_data[i].date, doc["forecasts"][0]["casts"][i]["date"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].week, doc["forecasts"][0]["casts"][i]["week"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].dayweather, doc["forecasts"][0]["casts"][i]["dayweather"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].nightweather, doc["forecasts"][0]["casts"][i]["nightweather"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].daytemp, doc["forecasts"][0]["casts"][i]["daytemp"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].nighttemp, doc["forecasts"][0]["casts"][i]["nighttemp"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].daywind, doc["forecasts"][0]["casts"][i]["daywind"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].nightwind, doc["forecasts"][0]["casts"][i]["nightwind"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].daypower, doc["forecasts"][0]["casts"][i]["daypower"], STR_SIZE_MAX);
                    strncpy(forecasts_data[i].nightpower, doc["forecasts"][0]["casts"][i]["nightpower"], STR_SIZE_MAX);
                }

                for (int i=0; i<FORECASTS_SIZE; i++)
                {
                    Serial.print("-------- ");Serial.println(forecasts_data[i].date);
                    Serial.print(forecasts_data[i].dayweather);Serial.print("-");Serial.println(forecasts_data[i].nightweather);
                    Serial.print(forecasts_data[i].daytemp);Serial.print("-");Serial.println(forecasts_data[i].nighttemp);
                    Serial.print(forecasts_data[i].daywind);Serial.print("-");Serial.println(forecasts_data[i].nightwind);
                    Serial.print(forecasts_data[i].daypower);Serial.print("-");Serial.println(forecasts_data[i].nightpower);
                }
            }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        
        https.end();
        
    } else {
        Serial.printf("[HTTPS] Unable to connect\n");
    }
      // End extra scoping block
}
