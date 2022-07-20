#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "bsec.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
 
#define SEALEVELPRESSURE_HPA (1013.25)
 
Bsec iaqSensor;
String output;
void checkIaqSensorStatus(void);
void errLeds(void);

float temperature;
float humidity;
float pressure;
float IAQ;
float carbon;
float VOC;
const char* IAQsts;
 
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
// Replace with your network credentials
const char* ssid = "oneplus";  // Enter SSID here
const char* password = "oneplus1";  //Enter Password here

ESP8266WebServer server(80);
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Wire.begin();
 
  Serial.println(F("Starting..."));
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
 
  Serial.println("OLED begin");
 
  display.display();
  delay(100);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(0);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  

  //Connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  
  Serial.println("HTTP server started");
   
  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
 
  Serial.println(output);
  checkIaqSensorStatus();
  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };
 
  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
  }
 
void loop() {
  // put your main code here, to run repeatedly
  server.handleClient();
  display.setCursor(0,0);
  display.clearDisplay();
 
  unsigned long time_trigger = millis();
  if (iaqSensor.run()) { // If new data is available
    output = String(time_trigger);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    Serial.println(output);
  } else {
    checkIaqSensorStatus();
  }
 
  Serial.print("Temperature = "); 
  Serial.print(iaqSensor.temperature); 
  Serial.println(" *C");
  display.print("Temperature: "); 
  display.print(iaqSensor.temperature); 
  display.println(" *C");
 
  Serial.print("Pressure = "); 
  Serial.print(iaqSensor.pressure / 100.0); 
  Serial.println(" hPa");
  display.print("Pressure: "); 
  display.print(iaqSensor.pressure / 100); 
  display.println(" hPa");
 
  Serial.print("Humidity = "); 
  Serial.print(iaqSensor.humidity); 
  Serial.println(" %");
  display.print("Humidity: "); 
  display.print(iaqSensor.humidity); 
  display.println(" %");
 
  Serial.print("IAQ = "); 
  Serial.print(iaqSensor.staticIaq); 
  Serial.println(" PPM");
  display.print("IAQ: "); 
  display.print(iaqSensor.staticIaq); 
  display.println(" PPM");
 
  Serial.print("CO2 equiv = "); 
  Serial.print(iaqSensor.co2Equivalent); 
  Serial.println(" PPM");
  display.print("CO2eq: "); 
  display.print(iaqSensor.co2Equivalent); 
  display.println(" PPM");
 
  Serial.print("Breath VOC = "); 
  Serial.print(iaqSensor.breathVocEquivalent); 
  Serial.println(" PPM");
  display.print("VOC: "); 
  display.print(iaqSensor.breathVocEquivalent); 
  display.println(" PPM");

  if ((iaqSensor.staticIaq > 0)  && (iaqSensor.staticIaq  <= 50)) {
    IAQsts = "Good";
    Serial.print("IAQ: Good");
    display.print("IAQ: Good"); 
  }
  if ((iaqSensor.staticIaq > 51)  && (iaqSensor.staticIaq  <= 100)) {
    IAQsts = "Average";
    Serial.print("IAQ: Average");
    display.print("IAQ: Average");
  }
  if ((iaqSensor.staticIaq > 101)  && (iaqSensor.staticIaq  <= 150)) {
    IAQsts = "Little Bad";
    Serial.print("IAQ: Little Bad");
    display.print("IAQ: Little Bad");
  }
  if ((iaqSensor.staticIaq > 151)  && (iaqSensor.staticIaq  <= 200)) {
    IAQsts = "Bad";
    Serial.print("IAQ: Bad");
    display.print("IAQ: Bad");
  }
  if ((iaqSensor.staticIaq > 201)  && (iaqSensor.staticIaq  <= 300)) {
    IAQsts = "Worse";
    Serial.print("IAQ: Worse");
    display.print("IAQ: Worse");
  }
  if ((iaqSensor.staticIaq > 301)  && (iaqSensor.staticIaq  <= 500)) {
    IAQsts = "Very Bad";
    Serial.print("IAQ: Very Bad");
    display.print("IAQ: Very Bad");
  }
  if ((iaqSensor.staticIaq > 500)){
  IAQsts = "Very Very Bad";
  Serial.print("IAQ: Very Very Bad");
  display.print("IAQ: Very Very Bad");
  }
  Serial.println();
  display.display();
  delay(2000);
}
 
// Helper function definitions
void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }
 
  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}

void handle_OnConnect() {

  temperature = iaqSensor.temperature;
  humidity = iaqSensor.humidity;
  pressure = iaqSensor.pressure / 100.0;
  IAQ = iaqSensor.staticIaq;
  carbon = iaqSensor.co2Equivalent;
  VOC = iaqSensor.breathVocEquivalent;
  
  server.send(200, "text/html", SendHTML(temperature, humidity, pressure, IAQ, carbon, VOC, IAQsts));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature, float humidity, float pressure, float IAQ, float carbon, float VOC, const char* IAQsts) {
String html = "<!DOCTYPE html>";
html += "<html>";
html += "<head>";
html += "<title>BME680 Webserver</title>";
html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css'>";
html += "<link rel='stylesheet' type='text/css' href='styles.css'>";
html += "<style>";
html += "body { background-color: #fff; font-family: sans-serif; color: #333333; font: 12px Helvetica, sans-serif box-sizing: border-box;}";
html += "#page { margin: 18px; background-color: #fff;}";
html += ".container { height: inherit; padding-bottom: 18px;}";
html += ".header { padding: 18px;}";
html += ".header h1 { padding-bottom: 0.3em; color: #f4a201; font-size: 25px; font-weight: bold; font-family: Garmond, 'sans-serif'; text-align: center;}";
html += "h2 { padding-bottom: 0.2em; border-bottom: 1px solid #eee; margin: 2px; text-align: center;}";
html += ".box-full { padding: 18px; border 1px solid #ddd; border-radius: 1em 1em 1em 1em; box-shadow: 1px 7px 7px 1px rgba(0,0,0,0.4); background: #fff; margin: 18px; width: 300px;}";
html += "@media (max-width: 494px) { #page { width: inherit; margin: 5px auto; } #content { padding: 1px;} .box-full { margin: 8px 8px 12px 8px; padding: 10px; width: inherit;; float: none; } }";
html += "@media (min-width: 494px) and (max-width: 980px) { #page { width: 465px; margin 0 auto; } .box-full { width: 380px; } }";
html += "@media (min-width: 980px) { #page { width: 930px; margin: auto; } }";
html += ".sensor { margin: 10px 0px; font-size: 2.5rem;}";
html += ".sensor-labels { font-size: 1rem; vertical-align: middle; padding-bottom: 15px;}";
html += ".units { font-size: 1.2rem;}";
html += "hr { height: 1px; color: #eee; background-color: #eee; border: none;}";
html += "</style>";

//Ajax Code Start
  html += "<script>\n";
  html += "setInterval(loadDoc,1000);\n";
  html += "function loadDoc() {\n";
  html += "var xhttp = new XMLHttpRequest();\n";
  html += "xhttp.onreadystatechange = function() {\n";
  html += "if (this.readyState == 4 && this.status == 200) {\n";
  html += "document.body.innerHTML =this.responseText}\n";
  html += "};\n";
  html += "xhttp.open(\"GET\", \"/\", true);\n";
  html += "xhttp.send();\n";
  html += "}\n";
  html += "</script>\n";
  //Ajax Code END
  
html += "</head>";
html += "<body>";
html += "<div id='page'>";
html += "<div class='header'>";
html += "<h1>BME680 IAQ Monitoring System</h1>";
html += "</div>";
html += "<div id='content' align='center'>";
html += "<div class='box-full' align='left'>";
html += "<h2>";
html += "IAQ Status: ";
html += IAQsts;
html += "</h2>";
html += "<div class='sensors-container'>";

//For Temperature
html += "<div class='sensors'>";
html += "<p class='sensor'>";
html += "<i class='fas fa-thermometer-half' style='color:#0275d8'></i>";
html += "<span class='sensor-labels'> Temperature </span>";
html += temperature;
html += "<sup class='units'>°C</sup>";
html += "</p>";
html += "<hr>";
html += "</div>";

//For Humidity
html += "<p class='sensor'>";
html += "<i class='fas fa-tint' style='color:#0275d8'></i>";
html += "<span class='sensor-labels'> Humidity </span>";
html += humidity;
html += "<sup class='units'>%</sup>";
html += "</p>";
html += "<hr>";

//For Pressure
html += "<p class='sensor'>";
html += "<i class='fas fa-tachometer-alt' style='color:#ff0040'></i>";
html += "<span class='sensor-labels'> Pressure </span>";
html += pressure;
html += "<sup class='units'>hPa</sup>";
html += "</p>";
html += "<hr>";

//For VOC IAQ
html += "<div class='sensors'>";
html += "<p class='sensor'>";
html += "<i class='fab fa-cloudversify' style='color:#483d8b'></i>";
html += "<span class='sensor-labels'> IAQ </span>";
html += IAQ;
html += "<sup class='units'>PPM</sup>";
html += "</p>";
html += "<hr>";

//For C02 Equivalent
html += "<p class='sensor'>";
html += "<i class='fas fa-smog' style='color:#35b22d'></i>";
html += "<span class='sensor-labels'> Co2 Eq. </span>";
html += carbon;
html += "<sup class='units'>PPM</sup>";
html += "</p>";
html += "<hr>";

//For Breath VOC
html += "<p class='sensor'>";
html += "<i class='fas fa-wind' style='color:#0275d8'></i>";
html += "<span class='sensor-labels'> Breath VOC </span>";
html += VOC;
html += "<sup class='units'>PPM</sup>";
html += "</p>";


html += "</div>";
html += "</div>";
html += "</div>";
html += "</div>";
html += "</div>";
html += "</body>";
html += "</html>";
return html;
}
 
void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}
