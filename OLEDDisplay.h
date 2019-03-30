/*********************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 This example is for a 128x64 size display using I2C to communicate

 Written by Limor Fried/Ladyada  for Adafruit Industries.
 BSD license, check license.txt for more information

 Heavily modified by SDL
 *********************************************************************/
#include "vector";

// Cannot use ticker for OLED updates as yield()
// in display.display() causes panic.
// #include <Ticker.h>;
// Ticker tickerQueueUpdate;

// Forward declarations.
void displayQueueDelay(int milliseconds);

/**
 * Define OLED display 'Pages'. Each 'Page' option must define a switch
 * option in displayPageRegister() and displayPageRender() for the OLED to be
 * updated.
 */

// Removed following as they are converted to Console output.
// DISPLAY_POWERUP
// DISPLAY_ACCESSPOINT
// DISPLAY_TRYING_AP
// DISPLAY_FAILING_AP
// DISPLAY_FAILED_RECONNECT
// DISPLAY_IPDISPLAY
// DISPLAY_UPDATING
// DISPLAY_NO_UPDATE_FAILED
// DISPLAY_UPDATE_FINISHED
// DISPLAY_NO_UPDATE_AVAILABLE

/**
 * Define Display screens that are shown using timed intervals.
 */
typedef enum {
  kScreenSmallWeather, // DISPLAY_WEATHER_SMALL
  kScreenMediumAM2315, // DISPLAY_WEATHER_MEDIUM
  kScreenMediumBMP180, // Separated from DISPLAY_WEATHER_MEDIUM
  kScreenMediumRainfall, // Separated from DISPLAY_WEATHER_MEDIUM
  kScreenMediumWind, // Separated from DISPLAY_WEATHER_MEDIUM
  kScreenMediumDateTime, // Separated from DISPLAY_WEATHER_MEDIUM
  kScreenMediumAirQuality, // Separated from DISPLAY_WEATHER_MEDIUM
  kScreenLargeAM2315Temp, // DISPLAY_WEATHER_LARGE
  kScreenLargeAM2315Hum, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeBMP180Temp, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeBMP180Pres, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeRainTotal, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeWindSpeed, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeWindGust, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeWindDir, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeDateTime, // Separated from DISPLAY_WEATHER_LARGE
  kScreenLargeAirQual, // Separated from DISPLAY_WEATHER_LARGE
  kScreenDemoTemp, // DISPLAY_WEATHER_DEMO
  kScreenDemoHum,
  kScreenDemoWindSpeed,
  kScreenDemoWindDir,
  kScreenDemoRainTotal,
  kScreenDemoDateTime,
  kScreenSunAirPlus, // DISPLAY_SUNAIRPLUS
  kScreenLightning, // DISPLAY_LIGHTNING_STATUS
  kScreenWxLink, // DISPLAY_WXLINK
} displayScreenTimed;

// Store timed display queue pointers in vector.
std::vector<displayScreenTimed> displayQueue;
// Next display index that will be rendered. Auto increments.
int display_queue_index = 0;
// Default time bto display a screen. Changed later per display mode.
int display_queue_timeout = 1800;
// Whether the display queue has started.
bool display_queue_started = false;

/**
 * Render max 6 text lines using smallest text size.
 */
void displayRenderSmall(std::vector<String> lines) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  int vector_size = lines.size();
  if (vector_size > 6) {
    vector_size = 6;
  }

  short int cursor_y = 0;
  for (int i = 0; i < vector_size; i++) {
    display.setCursor(0, cursor_y);
    display.println(String(lines[i]));
    cursor_y += 10;
  }

  display.display();
}

/**
 * Render max 3 text lines using medium text size.
 */
void displayRenderMedium(std::vector<String> lines) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  int vector_size = lines.size();
  if (vector_size > 3) {
    vector_size = 3;
  }

  int cursor_y = 0;
  for (int i = 0; i < vector_size; i++) {
    display.setCursor(0, cursor_y);
    display.println(lines[i]);
    cursor_y += 20;
  }

  display.display();
}

/**
 * Render 2 text lines using medium and large text size.
 */
void displayRenderLarge(std::vector<String> lines) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(lines[0]);
  display.setTextSize(3);
  display.setCursor(0, 20);
  display.println(lines[1]);
  display.display();
}

// Store text lines to be rendered on screen as console.
std::vector<String> console_buffer;
// Max number of line the screen will display.
int console_lines_max = 6;

/**
 * Render console text from buffer using small text size.
 */
void displayRenderConsole() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  short int cursor_y = 0;
  for (int i = 0; i < console_buffer.size(); i++) {
    display.setCursor(0, cursor_y);
    display.println(console_buffer[i]);
    cursor_y += 10;
  }

  display.display();
}

/**
 * Instant output of text to screen like a traditional console.
 *
 * New lines will be added up to max line count and then
 * then the screen will scroll down erasing the top lines.
 *
 * Use render FALSE to add multiple lines and set to TRUE on last
 * line added to update the display.
 */
void displayConsolePrint(String text, bool render) {
  console_buffer.push_back(text);
  if (console_buffer.size() > console_lines_max) {
    console_buffer.erase(console_buffer.begin());
  }
  if (render) {
    displayRenderConsole();
  }
}

void displayConsolePrint(String text) {
  displayConsolePrint(text, true);
}

/**
 * Add multiple lines to console display and render.
 */
void displayConsolePrint(std::vector<String> lines) {
  for (int i = 0; i < console_buffer.size(); i++) {
    console_buffer.push_back(lines[i]);
    if (console_buffer.size() > console_lines_max) {
      console_buffer.erase(console_buffer.begin());
    }
  }
  displayRenderConsole();
}

/**
 * Clear the display console.
 */
void displayConsoleClear() {
  console_buffer.clear();
}

/**
 * Generate list of screens for each display format.
 */
void displayQueueScreens(int format) {
  displayQueue.clear();
  display_queue_index = 0;
  switch (format) {
    case DISPLAY_WEATHER_SMALL:
      displayQueue.push_back(kScreenSmallWeather);

      if (AirQuality_Present) {
        displayQueue.push_back(kScreenMediumAirQuality);
      }
      if (WXLink_Present) {
        displayQueue.push_back(kScreenWxLink);
      }
      if (SunAirPlus_Present) {
        displayQueue.push_back(kScreenSunAirPlus);
      }
      if (AS3935_Present) {
        displayQueue.push_back(kScreenLightning);
      }

      display_queue_timeout = 5000;
      break;

    case DISPLAY_WEATHER_MEDIUM:
      displayQueue.push_back(kScreenMediumAM2315);
      displayQueue.push_back(kScreenMediumBMP180);
      displayQueue.push_back(kScreenMediumRainfall);
      displayQueue.push_back(kScreenMediumWind);
      displayQueue.push_back(kScreenMediumDateTime);

      if (AirQuality_Present) {
        displayQueue.push_back(kScreenMediumAirQuality);
      }
      if (WXLink_Present) {
        displayQueue.push_back(kScreenWxLink);
      }
      if (SunAirPlus_Present) {
        displayQueue.push_back(kScreenSunAirPlus);
      }
      if (AS3935_Present) {
        displayQueue.push_back(kScreenLightning);
      }

      display_queue_timeout = 1800;
      break;

    case DISPLAY_WEATHER_LARGE:
      displayQueue.push_back(kScreenLargeAM2315Temp);
      displayQueue.push_back(kScreenLargeAM2315Hum);
      displayQueue.push_back(kScreenLargeBMP180Temp);
      displayQueue.push_back(kScreenLargeBMP180Pres);
      displayQueue.push_back(kScreenLargeRainTotal);
      displayQueue.push_back(kScreenLargeWindSpeed);
      displayQueue.push_back(kScreenLargeWindGust);
      displayQueue.push_back(kScreenLargeWindDir);
      displayQueue.push_back(kScreenLargeDateTime);

      if (AirQuality_Present) {
        displayQueue.push_back(kScreenLargeAirQual);
      }
      if (WXLink_Present) {
        displayQueue.push_back(kScreenWxLink);
      }
      if (SunAirPlus_Present) {
        displayQueue.push_back(kScreenSunAirPlus);
      }
      if (AS3935_Present) {
        displayQueue.push_back(kScreenLightning);
      }

      display_queue_timeout = 1800;
      break;

    case DISPLAY_WEATHER_DEMO:
      displayQueue.push_back(kScreenDemoTemp);
      displayQueue.push_back(kScreenDemoHum);
      displayQueue.push_back(kScreenDemoWindSpeed);
      displayQueue.push_back(kScreenDemoWindDir);
      displayQueue.push_back(kScreenDemoRainTotal);
      displayQueue.push_back(kScreenDemoDateTime);

      display_queue_timeout = 900;
      break;
  }
}

/**
 * Generate text to be displayed on OLED and store in vector.
 */
void displayQueueRenderScreen(displayScreenTimed displayScreen) {
  std::vector<String> lines;
  String temp_string;
  String windDirection;

  switch (displayScreen) {
    case kScreenSunAirPlus: {
      lines.push_back("Solar Readings");
      lines.push_back("----------------");
      lines.push_back("Battery:" + String(BatteryVoltage, 2) + "V/" + String(BatteryCurrent, 1) + "mA");
      lines.push_back("Solar:" + String(SolarPanelVoltage, 2) + "V/" + String(SolarPanelCurrent, 1) + "mA");
      lines.push_back("Load:" + String(LoadVoltage, 2) + "V/" + String(LoadCurrent, 1) + "mA");
      displayRenderSmall(lines);
    }
    break;

    case kScreenLightning: {
      lines.push_back("Lightning Status");
      lines.push_back("----------------");
      lines.push_back("Last:" + as3935_LastLightning);
      lines.push_back("Date:" + getValue(as3935_LastLightningTimeStamp, ' ', 0));
      lines.push_back("Time:" + getValue(as3935_LastLightningTimeStamp, ' ', 1));
      displayRenderSmall(lines);
    }
    break;

// TODO: This should probably be a Console display rather than timed queue display.
//    case DISPLAY_LIGHTNING_DISPLAY: {
//      String stringSolar;
//      stringSolar = as3935_LastLightning + " away";
//
//      setDisplayLine(0, "LIGHTNING!");
//      setDisplayLine(1, "----------------");
//      setDisplayLine(2, const_cast<char*>(stringSolar.c_str()) );
//      setDisplayLine(3, "----------------");
//      setDisplayLine(4, "LIGHTNING!");
//    }
//    break;

    case kScreenWxLink: {
      lines.push_back("WXLink Readings");
      lines.push_back("----------------");
      lines.push_back("Battery:" + String(WXBatteryVoltage, 2) + "V/" + String(WXBatteryCurrent, 1) + "mA");
      lines.push_back("Solar:" + String(WXSolarPanelVoltage, 2) + "V/" + String(WXSolarPanelCurrent, 1) + "mA");
      lines.push_back("Load:" + String(WXLoadCurrent, 1) + "mA");
      lines.push_back("----------------");
      lines.push_back("MessageID: " + String(WXMessageID));
      displayRenderSmall(lines);
    }
    break;

    case kScreenSmallWeather: {
      // AM2315 Temperature and Humidity.
      lines.push_back("OT:" + formatTemperatureString(AM2315_Temperature, 1, true) +
                     " OH:" + formatHumidityString(AM2315_Humidity, 1, true));
      // BMP180 Temperature and Pressure.
      lines.push_back("IT:" + formatTemperatureString(BMP180_Temperature, 1, true) +
                     " BP:" + formatPressureString(BMP180_Pressure, 1, true));
      // Rain Total.
      lines.push_back("Rain Total:" + formatRainfallString(rainTotal, 1, true));
      // Current Wind Speed.
      lines.push_back("Wind Speed:" + formatWindspeedString(currentWindSpeed, 1, true));
      // Current Wind Gust.
      lines.push_back("Wind Gust:" + formatWindspeedString(currentWindGust, 1, true));
      // Current Wind Direction.
      lines.push_back("Wind Dir:" + returnDirectionFromDegrees(int(currentWindDirection)) +
                       "-" + String(currentWindDirection, 0) + "d");
      // Air Quality
      if (AirQuality_Present) {
        lines.push_back("Air Qual:" + reportAirQuality(currentAirQuality));
      }
      displayRenderSmall(lines);
    }
    break;

    case kScreenMediumAM2315: {
      // AM2315 Temperature.
      lines.push_back("OT:" + formatTemperatureString(AM2315_Temperature, 1, true));
      // AM2315 Humidity.
      lines.push_back("OH:" + formatHumidityString(AM2315_Humidity, 1, true));
      displayRenderMedium(lines);
    }
    break;

    case kScreenMediumBMP180: {
      // BMP180 Temperature.
      lines.push_back("IT:" + formatTemperatureString(BMP180_Temperature, 1, true));
      // BMP180 Pressure.
      lines.push_back("BP:" + formatPressureString(BMP180_Pressure, 1, true));
      displayRenderMedium(lines);
    }
    break;

    case kScreenMediumRainfall: {
      // Rain fall.
      lines.push_back("RH:" + formatRainfallString(rain60Minutes, 1, true));
      lines.push_back("RD:" + formatRainfallString(rainCalendarDay, 1, true));
      lines.push_back("RT:" + formatRainfallString(rainTotal, 1, true));

      displayRenderMedium(lines);
    }
    break;

    case kScreenMediumWind: {
      // Current Wind Speed.
      lines.push_back("WS:" + formatWindspeedString(currentWindSpeed, 1, true));
      // Current Wind Gust.
      lines.push_back("WG:" + formatWindspeedString(currentWindGust, 1, true));
      // Current Wind Direction.
      lines.push_back("WD:" + returnDirectionFromDegrees(int(currentWindDirection)) +
                      "-" + String(currentWindDirection, 0) + "d");
      displayRenderMedium(lines);
    }
    break;

    case kScreenMediumDateTime: {
      // Display date and time.
      RtcDateTime now = Rtc.GetDateTime();
      lines.push_back(formatDate(now));
      lines.push_back(rtcTime(now));
      displayRenderMedium(lines);
    }
    break;

    case kScreenMediumAirQuality: {
      lines.push_back("Air Qual");
      lines.push_back(reportAirQuality(currentAirQuality));
      displayRenderMedium(lines);
    }
    break;

    case kScreenLargeAM2315Temp: {
      // AM2315 Temperature.
      lines.push_back("Out Temp");
      lines.push_back(formatTemperatureString(AM2315_Temperature, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeAM2315Hum: {
      // AM2315 Humidity.
      lines.push_back("Out Humd");
      lines.push_back(formatHumidityString(AM2315_Humidity, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeBMP180Temp: {
      // BMP180 Temperature.
      lines.push_back("BMP Temp");
      lines.push_back(formatTemperatureString(BMP180_Temperature, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeBMP180Pres: {
      // BMP180 Pressure.
      lines.push_back("BMP Pres");
      lines.push_back(formatPressureString(BMP180_Pressure, 2, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeRainTotal: {
      // Rain Total.
      lines.push_back("Rain Total");
      lines.push_back(formatRainfallString(rainTotal, 2, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeWindSpeed: {
      // Current Wind Speed.
      lines.push_back("Wind Speed");
      lines.push_back(formatWindspeedString(currentWindSpeed, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeWindGust: {
      // Current Wind Gust.
      lines.push_back("Wind Gust");
      lines.push_back(formatWindspeedString(currentWindGust, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeWindDir: {
      // Current Wind direction.
      lines.push_back("Wind Dir");
      lines.push_back(returnDirectionFromDegrees(int(currentWindDirection)) +
        "-" + String(currentWindDirection, 0) + 'd');
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeDateTime: {
      // Date/Time
      RtcDateTime now = Rtc.GetDateTime();
      lines.push_back(formatDate(now));
      lines.push_back(rtcTime(now));
      displayRenderLarge(lines);
    }
    break;

    case kScreenLargeAirQual: {
      // Air quality.
      if (AirQuality_Present) {
        lines.push_back("Air Qual");
        lines.push_back(reportAirQuality(currentAirQuality));
        displayRenderLarge(lines);
      }
    }
    break;

    case kScreenDemoTemp: {
      // AM2315 Temperature.
      lines.push_back("Out Temp");
      lines.push_back(formatTemperatureString(AM2315_Temperature, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenDemoHum: {
      // AM2315 Humidity.
      lines.push_back("Out Humid");
      lines.push_back(formatHumidityString(AM2315_Humidity, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenDemoWindSpeed: {
      // Current Wind Speed.
      lines.push_back("Wind Speed");
      lines.push_back(formatWindspeedString(currentWindSpeed, 1, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenDemoWindDir: {
      // Current Wind direction.
      lines.push_back("Wind Dir");
      lines.push_back(returnDirectionFromDegrees(int(currentWindDirection)) +
                       "-" + String(currentWindDirection, 0) + 'd');
      displayRenderLarge(lines);
    }
    break;

    case kScreenDemoRainTotal: {
      // Rain Total.
      lines.push_back("Rain Total");
      lines.push_back(formatRainfallString(rainTotal, 2, true));
      displayRenderLarge(lines);
    }
    break;

    case kScreenDemoDateTime: {
      // display date and time
      RtcDateTime now = Rtc.GetDateTime();
      lines.push_back(formatDate(now));
      lines.push_back(rtcTime(now));
      displayRenderLarge(lines);
    }
    break;

    default:
      Serial.println("ERROR: Default Screen reached with value: " + String(displayScreen));
      break;
  }
}

/**
 * Display the next queue screen on the OLED.
 */
void displayQueueNext() {
  if (sensor_readings_present) {
    // Render current screen.
    displayQueueRenderScreen(displayQueue[display_queue_index]);
    // Increment next screen.
    display_queue_index++;
    // Reset screen increment to beginning when at end.
    if (display_queue_index == displayQueue.size()) {
      display_queue_index = 0;
    }
  }
}

void displayQueueStart() {
  display_queue_started = true;
  //tickerQueueUpdate.attach_ms(display_queue_timeout, displayQueueNext);
}

void setupDisplayQueue(int format) {
  Serial.println('OLED: Queue Setup');
  displayQueueScreens(format);
  displayQueueStart();
  //displayQueueDelay(5500);
}

void displayQueueDelay(int milliseconds) {
  if (display_queue_started) {
    Serial.println('OLED: Queue Delayed');
    //tickerQueueUpdate.detach();
    //tickerQueueDelay.attach_ms(milliseconds, displayQueueStart);
  }
}

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

//static const unsigned char PROGMEM logo16_glcd_bmp[] =
static const unsigned char logo16_glcd_bmp[] = { B00000000, B11000000,
    B00000001, B11000000, B00000001, B11000000, B00000011, B11100000, B11110011,
    B11100000, B11111110, B11111000, B01111110, B11111111, B00110011, B10011111,
    B00011111, B11111100, B00001101, B01110000, B00011011, B10100000, B00111111,
    B11100000, B00111111, B11110000, B01111100, B11110000, B01110000, B01110000,
    B00000000, B00110000 };

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];

  // initialize
  for (uint8_t f = 0; f < NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(5) + 1;

    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1) {
    // draw each icon
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h,
          WHITE);
    }
    display.display();
    delay(200);

    // then erase it + move it
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h,
          BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = random(5) + 1;
      }
    }
  }
}

void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') {
      continue;
    }
    display.write(i);
    if ((i > 0) && (i % 21 == 0)) {
      display.println();
    }
  }
  display.display();
}

void testdrawcircle(void) {
  for (int16_t i = 0; i < display.height(); i += 2) {
    display.drawCircle(display.width() / 2, display.height() / 2, i, WHITE);
    display.display();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i = 0; i < display.height() / 2; i += 3) {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, color % 2);
    display.display();
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5) {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
        display.width() / 2 - i, display.height() / 2 + i,
        display.width() / 2 + i, display.height() / 2 + i, WHITE);
    display.display();
  }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5) {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
        display.width() / 2 - i, display.height() / 2 + i,
        display.width() / 2 + i, display.height() / 2 + i, WHITE);
    if (color == WHITE) {
      color = BLACK;
    } else {
      color = WHITE;
    }
    display.display();
  }
}

void testdrawroundrect(void) {
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.drawRoundRect(i, i, display.width() - 2 * i,
        display.height() - 2 * i, display.height() / 4, WHITE);
    display.display();
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.fillRoundRect(i, i, display.width() - 2 * i,
        display.height() - 2 * i, display.height() / 4, color);
    if (color == WHITE) {
      color = BLACK;
    } else {
      color = WHITE;
    }
    display.display();
  }
}

void testdrawrect(void) {
  for (int16_t i = 0; i < display.height() / 2; i += 2) {
    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, WHITE);
    display.display();
  }
}

void testdrawline() {
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, 0, i, display.height() - 1, WHITE);
    display.display();
  }
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(0, 0, display.width() - 1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, display.height() - 1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = display.width() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(display.width() - 1, 0, 0, i, WHITE);
    display.display();
  }
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, WHITE);
    display.display();
  }
  delay(250);
}

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.clearDisplay();
  display.println("scroll");
  display.display();

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}

#if (SSD1306_LCDHEIGHT != 64)
// #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void oledDisplaySetup() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false); // initialize with the I2C addr 0x3C (for the 128x64)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
  /*
   // draw a single pixel
   display.drawPixel(10, 10, WHITE);
   // Show the display buffer on the hardware.
   // NOTE: You _must_ call display after making any drawing commands
   // to make them visible on the display hardware!
   display.display();
   delay(2000);
   display.clearDisplay();


   // draw many lines
   testdrawline();
   display.display();
   delay(2000);
   display.clearDisplay();

   // draw rectangles
   testdrawrect();
   display.display();
   delay(2000);
   display.clearDisplay();

   // draw multiple rectangles
   testfillrect();
   display.display();
   delay(2000);
   display.clearDisplay();

   // draw mulitple circles
   testdrawcircle();
   display.display();
   delay(2000);
   display.clearDisplay();

   // draw a white circle, 10 pixel radius
   display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
   display.display();
   delay(2000);
   display.clearDisplay();

   testdrawroundrect();
   delay(2000);
   display.clearDisplay();

   testfillroundrect();
   delay(2000);
   display.clearDisplay();

   testdrawtriangle();
   delay(2000);
   display.clearDisplay();

   testfilltriangle();
   delay(2000);
   display.clearDisplay();

   // draw the first ~12 characters in the font
   testdrawchar();
   display.display();
   delay(2000);
   display.clearDisplay();

   // draw scrolling text
   testscrolltext();
   delay(2000);
   display.clearDisplay();

   // text display tests
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0,0);
   display.println("Hello, world!");
   display.setTextColor(BLACK, WHITE); // 'inverted' text
   display.println(3.141592);
   display.setTextSize(2);
   display.setTextColor(WHITE);
   display.print("0x"); display.println(0xDEADBEEF, HEX);
   display.display();
   delay(2000);

   // miniature bitmap display
   display.clearDisplay();
   display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
   display.display();

   // invert the display
   display.invertDisplay(true);
   delay(1000);
   display.invertDisplay(false);
   delay(1000);

   // draw a bitmap icon and 'animate' movement
   testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
   */
}
