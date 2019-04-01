/*
 SDL_Weather_80422.cpp - Library for SwitchDoc Labs WeatherRack.
 SparkFun Weather Station Meters
 Argent Data Systems
 Created by SwitchDoc Labs July 27, 2014.
 Released into the public domain.
 Version 1.1 - updated constants to suppport 3.3V
 Version 1.6 - Support for ADS1015 in WeatherPiArduino Board February 7, 2015
 */

#include "Arduino.h"
#include <time.h>
#include "SDL_Weather_80422.h"
#include "OWMAdafruit_ADS1015.h"

extern Adafruit_ADS1015 ads1015;

#define ESP8266
// Factor dervied from empherical analysis.  The ESP8266 screws with interrupt timing.
#define ESP8266_Factor 0.298

// to indicate that port c will not be used for pin change interrupts
//#define NO_PORTC_PINCHANGES
// to indicate that port d will not be used for pin change interrupts
//#define NO_PORTD_PINCHANGES

//#include "PinChangeInt.h"

// Private functions.

boolean fuzzyCompare(float compareValue, float value) {
#define VARYVALUE 0.05
  if ((value > (compareValue * (1.0 - VARYVALUE)))
      && (value < (compareValue * (1.0 + VARYVALUE)))) {
    return true;
  }

  return false;
}

float voltageToDegrees(float value, float defaultWindDirection) {
  // The original documentation for the wind vane says 16 positions.
  // Typically only recieve 8 positions.  And 315 degrees was wrong.

  // For 5V, use 1.0.  For 3.3V use 0.66
#define ADJUST3OR5 0.66
#define PowerVoltage 3.3

  if (fuzzyCompare(3.84 * ADJUST3OR5, value))
    return 0.0;

  if (fuzzyCompare(1.98 * ADJUST3OR5, value))
    return 22.5;

  if (fuzzyCompare(2.25 * ADJUST3OR5, value))
    return 45;

  if (fuzzyCompare(0.41 * ADJUST3OR5, value))
    return 67.5;

  if (fuzzyCompare(0.45 * ADJUST3OR5, value))
    return 90.0;

  if (fuzzyCompare(0.32 * ADJUST3OR5, value))
    return 112.5;

  if (fuzzyCompare(0.90 * ADJUST3OR5, value))
    return 135.0;

  if (fuzzyCompare(0.62 * ADJUST3OR5, value))
    return 157.5;

  if (fuzzyCompare(1.40 * ADJUST3OR5, value))
    return 180;

  if (fuzzyCompare(1.19 * ADJUST3OR5, value))
    return 202.5;

  if (fuzzyCompare(3.08 * ADJUST3OR5, value))
    return 225;

  if (fuzzyCompare(2.93 * ADJUST3OR5, value))
    return 247.5;

  if (fuzzyCompare(4.62 * ADJUST3OR5, value))
    return 270.0;

  if (fuzzyCompare(4.04 * ADJUST3OR5, value))
    return 292.5;

  // Chart in documentation wrong.
  if (fuzzyCompare(4.34 * ADJUST3OR5, value))
    return 315.0;

  if (fuzzyCompare(3.43 * ADJUST3OR5, value))
    return 337.5;

//  Serial.print(" FAIL WIND DIRECTION");

  // Return previous value if not found.
  return defaultWindDirection;
}

// Interrupt routines.
// For mega only Port B works.
// - Interrupt 4 - Pin 19
// - Interrupt 5 - Pin 18

unsigned long lastWindTime;

#ifdef ESP8266

void ICACHE_RAM_ATTR serviceInterruptAnem() {
  noInterrupts();
  unsigned long currentTime = (unsigned long) (micros() - lastWindTime);

  lastWindTime = micros();
//  lastWindTime = millis();

//  if (currentTime > 25000)  // debounce
  if (currentTime > 10000) {
    SDL_Weather_80422::_totalWindTime = SDL_Weather_80422::_totalWindTime
        + currentTime;
    SDL_Weather_80422::_currentWindCount++;

    if (currentTime < SDL_Weather_80422::_shortestWindTime) {
      SDL_Weather_80422::_shortestWindTime5 =
          SDL_Weather_80422::_shortestWindTime4;
      SDL_Weather_80422::_shortestWindTime4 =
          SDL_Weather_80422::_shortestWindTime3;
      SDL_Weather_80422::_shortestWindTime3 =
          SDL_Weather_80422::_shortestWindTime2;
      SDL_Weather_80422::_shortestWindTime2 =
          SDL_Weather_80422::_shortestWindTime;
      SDL_Weather_80422::_shortestWindTime = currentTime;
      Serial.println(currentTime);
    }
  }
  interrupts();
}

#else
void serviceInterruptAnem() {
  unsigned long currentTime = (unsigned long)(micros() - lastWindTime);

  lastWindTime = micros();
  if (currentTime > 20000) {
    SDL_Weather_80422::_currentWindCount++;
    if (currentTime < SDL_Weather_80422::_shortestWindTime) {
      SDL_Weather_80422::_shortestWindTime = currentTime;
    }
  }
}
#endif

unsigned long currentRainMin;
unsigned long lastRainTime;

#ifdef ESP8266
void ICACHE_RAM_ATTR serviceInterruptRain() {
  unsigned long currentTime = (unsigned long) (micros() - lastRainTime);

  lastRainTime = micros();
  if (currentTime > 500) {
    SDL_Weather_80422::_currentRainCount++;
//    interrupt_count[19]++;
    if (currentTime < currentRainMin) {
      currentRainMin = currentTime;
    }
  }
//  interrupt_count[18]++;
}
#else
void serviceInterruptRain() {
  unsigned long currentTime = (unsigned long) (micros() - lastRainTime);

  lastRainTime = micros();
  if (currentTime > 500) {
    SDL_Weather_80422::_currentRainCount++;
//    interrupt_count[19]++;
    if (currentTime < currentRainMin) {
      currentRainMin = currentTime;
    }
  }
//  interrupt_count[18]++;
}
#endif

long SDL_Weather_80422::_currentWindCount = 0;
long SDL_Weather_80422::_currentRainCount = 0;
long SDL_Weather_80422::_shortestWindTime = 1000000;
long SDL_Weather_80422::_shortestWindTime2 = 1000000;
long SDL_Weather_80422::_shortestWindTime3 = 1000000;
long SDL_Weather_80422::_shortestWindTime4 = 1000000;
long SDL_Weather_80422::_shortestWindTime5 = 1000000;
long SDL_Weather_80422::_totalWindTime;

SDL_Weather_80422::SDL_Weather_80422(int pinAnem, int pinRain, int intAnem,
                                     int intRain, int ADChannel, int ADMode) {
//  pinMode(pinAnem, INPUT);
//  pinMode(pinRain, INPUT);
  _pinAnem = pinAnem;
  _pinRain = pinRain;
  _intAnem = intAnem;
  _intRain = intRain;
  _ADChannel = ADChannel;
  _ADMode = ADMode;

  _currentRainCount = 0;
  _currentWindCount = 0;
  _currentWindSpeed = 0.0;
  _currentWindDirection = 0.0;

  _totalWindTime = 0;

  lastWindTime = 0;
  _shortestWindTime = 0xffffffff;
  _shortestWindTime2 = 0xffffffff;
  _shortestWindTime3 = 0xffffffff;
  _shortestWindTime4 = 0xffffffff;
  _shortestWindTime5 = 0xffffffff;

  _sampleTime = 5.0;
  _selectedMode = SDL_MODE_SAMPLE;

  _startSampleTime = micros();

  // Set up interrupts.
#ifdef ESP8266
  // pinAnem is input to which a switch is connected.
  pinMode(pinAnem, INPUT);
  // Configure internal pull-up resistor.
  // digitalWrite(pinAnem, HIGH);
  // pinRain is input to which a switch is connected
  pinMode(pinRain, INPUT);
  // Configure internal pull-up resistor
  // digitalWrite(pinRain, HIGH);
  attachInterrupt(_pinAnem, serviceInterruptAnem, RISING);
  attachInterrupt(_pinRain, serviceInterruptRain, RISING);
#else
  // pinAnem is input to which a switch is connected.
  pinMode(pinAnem, INPUT);
  // Configure internal pull-up resistor.
  digitalWrite(pinAnem, HIGH);
  // pinRain is input to which a switch is connected
  pinMode(pinRain, INPUT);
  // Configure internal pull-up resistor.
  digitalWrite(pinRain, HIGH);
  attachInterrupt(_intAnem, serviceInterruptAnem, RISING);
  attachInterrupt(_intRain, serviceInterruptRain, RISING);
#endif

  if (_ADMode == SDL_MODE_I2C_ADS1015)
    ads1015.begin();
}

float SDL_Weather_80422::get_current_rain_total() {
  // mm of rain - we get two interrupts per bucket.
  float rain_amount = 0.2794 * _currentRainCount / 2;
  _currentRainCount = 0;
  return rain_amount;
}

#define WIND_FACTOR 2.400

float SDL_Weather_80422::current_wind_speed() {
  // In milliseconds.
  if (_selectedMode == SDL_MODE_SAMPLE) {
    _currentWindSpeed = get_current_wind_speed_when_sampling();
  } else {
    // km/h * 1000 msec.
    _currentWindCount = 0;
    delay(_sampleTime * 1000);
    _currentWindSpeed = ((float) _currentWindCount / _sampleTime) * WIND_FACTOR;
  }
  return _currentWindSpeed;
}

float SDL_Weather_80422::get_wind_gust() {
  unsigned long latestTime;

  if (_shortestWindTime2 = 0xffffffff) {
    _shortestWindTime2 = _shortestWindTime;
  }
  if (_shortestWindTime3 = 0xffffffff) {
    _shortestWindTime3 = _shortestWindTime2;
  }
  if (_shortestWindTime4 = 0xffffffff) {
    _shortestWindTime4 = _shortestWindTime3;
  }
  if (_shortestWindTime5 = 0xffffffff) {
    _shortestWindTime5 = _shortestWindTime4;
  }

  latestTime = (_shortestWindTime + _shortestWindTime2 + _shortestWindTime3
      + _shortestWindTime4 + _shortestWindTime5) / 5;
//  Serial.println("shwt=");
//  Serial.println(_shortestWindTime);
  _shortestWindTime = 0xffffffff;
  _shortestWindTime2 = 0xffffffff;
  _shortestWindTime3 = 0xffffffff;
  _shortestWindTime4 = 0xffffffff;
  _shortestWindTime5 = 0xffffffff;

  double time = latestTime / 1000000.0; // in microseconds

  float returnTime;

#ifdef ESP8266
  returnTime = ((1 / (time)) * WIND_FACTOR) * ESP8266_Factor;
#else
  returnTime = (1 / (time)) * WIND_FACTOR;
#endif

  if (returnTime < _currentWindSpeed) {
    returnTime = _currentWindSpeed;
  }
  return returnTime;
}

float SDL_Weather_80422::current_wind_direction() {
  float voltageValue;
  float Vcc = PowerVoltage;

  if (_ADMode == SDL_MODE_I2C_ADS1015) {
    // AIN1 wired to wind vane on WeatherPiArduino.
    int value = ads1015.readADC_SingleEnded(1);
    voltageValue = value * 0.003f;
  } else {
    // Use internal A/D converter.
    voltageValue = (analogRead(_ADChannel) / 1023.0) * Vcc;
  }

  float direction = voltageToDegrees(voltageValue, _currentWindDirection);
  return direction;
}

float SDL_Weather_80422::current_wind_direction_voltage() {
  float voltageValue;
  float Vcc = PowerVoltage;

  if (_ADMode == SDL_MODE_I2C_ADS1015) {
    // AIN1 wired to wind vane on WeatherPiArduino.
    int value = ads1015.readADC_SingleEnded(1);
    voltageValue = value * 0.003f;
  } else {
    // Use internal A/D converter.
    voltageValue = (analogRead(_ADChannel) / 1023.0) * Vcc;
  }

  return voltageValue;
}

void SDL_Weather_80422::reset_rain_total() {
  _currentRainCount = 0;
}

float SDL_Weather_80422::accessInternalCurrentWindDirection() {
  return _currentWindDirection;
}

void SDL_Weather_80422::reset_wind_gust() {
  _shortestWindTime = 0xffffffff;
}

// Ongoing samples in wind.
void SDL_Weather_80422::startWindSample(float sampleTime) {
  _startSampleTime = micros();
  _sampleTime = sampleTime;
}

// Factor dervied from empherical analysis.  The ESP8266 screws with interrupt timing.
#define ESP8266_Factor 0.298

float SDL_Weather_80422::get_current_wind_speed_when_sampling() {
  unsigned long compareValue;
  compareValue = _sampleTime * 1000000;

  if (micros() - _startSampleTime >= compareValue) {
    // Sample time exceeded, calculate currentWindSpeed.
    float _timeSpan;
//    _timeSpan = (unsigned long)(micros() - _startSampleTime);
    _timeSpan = (micros() - _startSampleTime);

#ifdef ESP8266
    _currentWindSpeed = (((float) _currentWindCount / (_timeSpan)) * WIND_FACTOR
        * 1000000) * ESP8266_Factor;
#else
    _currentWindSpeed = ((float)_currentWindCount / (_timeSpan)) * WIND_FACTOR * 1000000;
#endif

    float _averageWindTime;
    _averageWindTime = (float) _totalWindTime / (float) _currentWindCount;

//   Serial.print("TotalWindTime=");
//   Serial.print(_totalWindTime);
//   Serial.print(" _currentWindCount=");
//   Serial.print(_currentWindCount);
//   Serial.print(" _averageWindTime=");
//   Serial.println(_averageWindTime);

    _currentWindCount = 0;
    _totalWindTime = 0;
    _startSampleTime = micros();
  } else {
//    Serial.println("NOT Triggered");
//    Serial.print("time = ");
//    Serial.println(micros() - _startSampleTime);
    // If not, then return last wind speed.
  }

  return _currentWindSpeed;
}

void SDL_Weather_80422::setWindMode(int selectedMode, float sampleTime /* time in seconds */) {
  _sampleTime = sampleTime;
  _selectedMode = selectedMode;

  if (_selectedMode == SDL_MODE_SAMPLE) {
    startWindSample(_sampleTime);
  }
}

