#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define ENABLE_DEEPSLEEP
//#define ENABLE_SERIAL_PRINT

Adafruit_BME280 bme; // I2C

// WiFi
const char *wifi_ssid = "<wifi name>";
const char *wifi_password = "<wifi password>";
WiFiClient wifiClient;

void setup_wifi()
{
    delay(10);
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("WiFi connected at IP address: ");
    Serial.println(WiFi.localIP());
}

void setup_bme280()
{
    if (!bme.begin(0x76))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }

    Serial.println("-- Weather Station Scenario --");
    Serial.println("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling,");
    Serial.println("filter off");
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
}

void setup()
{
    Serial.begin(115200);
    Serial.println(F("BME280"));

    setup_bme280();
    setup_wifi();
}

void loop()
{
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;

    // Build post data
    String postData = "rooms,room=studio temperature=" + String(temperature);
    postData += "\nrooms,room=studio humidity=" + String(humidity);
    postData += "\nrooms,room=studio pressure=" + String(pressure);

    HTTPClient http;
    http.begin(F("http://192.168.1.30:8086/write?db=home"));
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(postData);
    if (httpCode == 204)
    {
        Serial.println("InfluxDB Updated");
    }
    else
    {
        Serial.print("InfluxDB Error code: ");
        Serial.println(httpCode);
        Serial.println(http.getString());
    }
    http.end();
    
#ifdef ENABLE_SERIAL_PRINT
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");

    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" hPa");

    // Serial.print("Approx. Altitude = ");
    // Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    // Serial.println(" m");

    Serial.println();
#endif

#ifdef ENABLE_DEEPSLEEP
    Serial.println("Force deepsleep 10min!");
    ESP.deepSleep(10 * 60 * 1000000);
    delay(100);
#endif
}
