/*
 * ESP32-C6: один адресный светодиод (WS2812B/NeoPixel) на GPIO7
 *
 * Библиотека: FastLED (установить через Arduino IDE Library Manager)
 *
 * Подключение (важно):
 * - DIN светодиода -> GPIO7 (лучше через резистор 330-470 Ом)
 * - Питание светодиода 5V (или 3.3V, если ваш LED уверенно работает от 3.3V)
 * - GND светодиода и ESP32-C6 общий
 * - Если питание 5V: общий GND обязателен; при необходимости используйте уровневый преобразователь
 */

#include <FastLED.h>

// ===== Настройки =====
#define LED_PIN     7
#define NUM_LEDS    1
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// Яркость ограничиваем по умолчанию (чтобы не слепило и не грелось)
static uint8_t g_brightness = 40; // 0-255

// ===== Простой "демо-режим": плавная радуга + редкий белый "пульс" =====
static uint8_t g_hue = 0;
static uint32_t g_lastPulseMs = 0;

static void showSolid(const CRGB& c) {
  leds[0] = c;
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("ESP32-C6 OnePixel (WS2812B) on GPIO7");
  Serial.println("Commands: br <0-255> | off | white | red | green | blue");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(g_brightness);
  FastLED.clear(true);
}

static void handleSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("br ")) {
    int v = cmd.substring(3).toInt();
    g_brightness = (uint8_t)constrain(v, 0, 255);
    FastLED.setBrightness(g_brightness);
    Serial.print("Brightness: ");
    Serial.println(g_brightness);
    return;
  }

  if (cmd == "off")   { showSolid(CRGB::Black); return; }
  if (cmd == "white") { showSolid(CRGB::White); return; }
  if (cmd == "red")   { showSolid(CRGB::Red); return; }
  if (cmd == "green") { showSolid(CRGB::Green); return; }
  if (cmd == "blue")  { showSolid(CRGB::Blue); return; }

  Serial.println("Unknown command. Use: br <0-255> | off | white | red | green | blue");
}

void loop() {
  handleSerial();

  // Радуга (плавно)
  leds[0] = CHSV(g_hue++, 255, 255);

  // Редкий короткий белый пульс поверх (пример "вспышки")
  const uint32_t now = millis();
  if (now - g_lastPulseMs > 2500) {
    g_lastPulseMs = now;
  }
  const uint32_t t = now - g_lastPulseMs;
  if (t < 120) {
    // quick ease-out
    uint8_t k = (uint8_t)(255 - (t * 255UL / 120UL));
    leds[0] += CRGB(k, k, k);
  }

  FastLED.show();
  delay(15);
}

