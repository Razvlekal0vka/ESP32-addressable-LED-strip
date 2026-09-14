/*
 * ESP32-C3 Белый свет + пульсация оранжевым
 * ESP32-C3 White light + orange pulse
 *
 * Лента: WS2812B (количество LED настраивается ниже)
 * Strip: WS2812B (LED count configurable below)
 *
 * Все светодиоды горят одинаково:
 *   — постоянный белый фон заданной яркости
 *   — поверх него периодическая вспышка оранжевым
 *     с настраиваемым временем нарастания и затухания
 *
 * All LEDs are identical:
 *   — constant white background at set brightness
 *   — periodic orange flash on top
 *     with configurable rise and fade times
 *
 * Серийные команды / Serial commands:
 *   white <0-255>      — яркость белого / white brightness
 *   white_dim <0-255>  — яркость белого при вспышке / white brightness during flash
 *   orange_br <0-255>  — макс. яркость оранжевого / max orange brightness
 *   hue <0-255>        — тон оранжевого (HSV hue) / orange hue
 *   rise <мс>          — время нарастания вспышки / flash rise time (ms)
 *   hold <мс>          — время горения на макс. яркости / hold at max brightness (ms)
 *   fade <мс>          — время затухания вспышки / flash fade time (ms)
 *   pause <мс>         — пауза между вспышками / pause between flashes (ms)
 *   status             — показать текущие настройки / show current settings
 *
 * Подключение / Wiring:
 *   Лента → GPIO8, 5V (внешний БП), общий GND
 *   Strip → GPIO8, 5V (external PSU), common GND
 */

#include <FastLED.h>

// === Настройки ленты / Strip configuration ===
#define LED_PIN     8
#define NUM_LEDS    60       // Количество светодиодов / Number of LEDs
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// === Настраиваемые параметры / Adjustable parameters ===
uint8_t whiteBrightness  = 100;  // Яркость белого в покое (0-255) / White brightness at rest
uint8_t whiteDimBrightness = 5; // Яркость белого при вспышке (0-255) / White brightness during flash
uint8_t orangeBrightness = 200;  // Макс. яркость оранжевого (0-255) / Max orange brightness
uint8_t orangeHue        = 30;   // Тон оранжевого в HSV (0-255, ~24 = оранжевый) / Orange hue (HSV)
uint16_t riseTime        = 250;  // Время нарастания вспышки, мс / Flash rise time, ms
uint16_t holdTime        = 250;  // Время горения на макс. яркости, мс / Hold at max brightness, ms
uint16_t fadeTime        = 250;  // Время затухания вспышки, мс / Flash fade time, ms
uint16_t pauseTime       = 500;   // Пауза между вспышками, мс / Pause between flashes, ms

// === Внутренние переменные / Internal variables ===
unsigned long cycleStart = 0;    // Начало текущего цикла / Current cycle start

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-C3 White + Orange Pulse");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);  // Управляем яркостью вручную / We control brightness manually

  FastLED.clear();
  FastLED.show();

  cycleStart = millis();
  printStatus();
}

void loop() {
  handleSerial();

  // --- Вычисляем фазу в текущем цикле / Calculate phase in current cycle ---
  // Цикл: rise → hold → fade → pause
  unsigned long now = millis();
  unsigned long elapsed = now - cycleStart;
  unsigned long totalCycle = (unsigned long)riseTime + holdTime + fadeTime + pauseTime;

  if (elapsed >= totalCycle) {
    cycleStart = now;
    elapsed = 0;
  }

  // --- Определяем интенсивность оранжевого (0.0 – 1.0) / Orange intensity ---
  float orangeLevel = 0.0;

  if (elapsed < riseTime) {
    // Фаза нарастания / Rise phase
    orangeLevel = (float)elapsed / (float)riseTime;
  } else if (elapsed < (unsigned long)riseTime + holdTime) {
    // Фаза горения на макс. / Hold at max phase
    orangeLevel = 1.0;
  } else if (elapsed < (unsigned long)riseTime + holdTime + fadeTime) {
    // Фаза затухания / Fade phase
    orangeLevel = 1.0 - (float)(elapsed - riseTime - holdTime) / (float)fadeTime;
  } else {
    // Пауза / Pause
    orangeLevel = 0.0;
  }

  // --- Смешиваем белый фон + оранжевую вспышку / Blend white + orange flash ---
  // Белый приглушается пропорционально вспышке / White dims proportionally to flash
  uint8_t curWhite = whiteBrightness - (uint8_t)(orangeLevel * (float)(whiteBrightness - whiteDimBrightness));
  CRGB white = CRGB(curWhite, curWhite, curWhite);

  // Оранжевая вспышка (HSV → RGB, затем масштабируем по уровню)
  // Orange flash (HSV → RGB, then scale by level)
  uint8_t curOrangeBr = (uint8_t)(orangeLevel * orangeBrightness);
  CRGB orange = CHSV(orangeHue, 255, curOrangeBr);

  // Складываем цвета с ограничением / Add colors with clamping
  CRGB result = white;
  result += orange;

  // --- Заполняем все светодиоды одинаковым цветом / Fill all LEDs with same color ---
  fill_solid(leds, NUM_LEDS, result);

  FastLED.show();
  delay(10);  // ~100 FPS
}

// ============================================================
// Обработка Serial-команд / Handle Serial commands
// ============================================================
void handleSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("white_dim ")) {
    int val = cmd.substring(10).toInt();
    whiteDimBrightness = constrain(val, 0, 255);
    Serial.print("White dim brightness: ");
    Serial.println(whiteDimBrightness);
  }
  else if (cmd.startsWith("white ")) {
    int val = cmd.substring(6).toInt();
    whiteBrightness = constrain(val, 0, 255);
    Serial.print("White brightness: ");
    Serial.println(whiteBrightness);
  }
  else if (cmd.startsWith("orange_br ")) {
    int val = cmd.substring(10).toInt();
    orangeBrightness = constrain(val, 0, 255);
    Serial.print("Orange brightness: ");
    Serial.println(orangeBrightness);
  }
  else if (cmd.startsWith("hue ")) {
    int val = cmd.substring(4).toInt();
    orangeHue = constrain(val, 0, 255);
    Serial.print("Orange hue: ");
    Serial.println(orangeHue);
  }
  else if (cmd.startsWith("rise ")) {
    int val = cmd.substring(5).toInt();
    if (val > 0) {
      riseTime = val;
      Serial.print("Rise time: ");
      Serial.print(riseTime);
      Serial.println(" ms");
    }
  }
  else if (cmd.startsWith("hold ")) {
    int val = cmd.substring(5).toInt();
    if (val >= 0) {
      holdTime = val;
      Serial.print("Hold time: ");
      Serial.print(holdTime);
      Serial.println(" ms");
    }
  }
  else if (cmd.startsWith("fade ")) {
    int val = cmd.substring(5).toInt();
    if (val > 0) {
      fadeTime = val;
      Serial.print("Fade time: ");
      Serial.print(fadeTime);
      Serial.println(" ms");
    }
  }
  else if (cmd.startsWith("pause ")) {
    int val = cmd.substring(6).toInt();
    if (val >= 0) {
      pauseTime = val;
      Serial.print("Pause time: ");
      Serial.print(pauseTime);
      Serial.println(" ms");
    }
  }
  else if (cmd == "status") {
    printStatus();
  }
  else {
    Serial.println("Unknown command. Available:");
    Serial.println("  white <0-255>");
    Serial.println("  white_dim <0-255>");
    Serial.println("  orange_br <0-255>");
    Serial.println("  hue <0-255>");
    Serial.println("  rise <ms>");
    Serial.println("  hold <ms>");
    Serial.println("  fade <ms>");
    Serial.println("  pause <ms>");
    Serial.println("  status");
  }
}

// ============================================================
// Вывод текущих настроек / Print current settings
// ============================================================
void printStatus() {
  Serial.println("=== Current Settings ===");
  Serial.print("  White brightness : "); Serial.println(whiteBrightness);
  Serial.print("  White dim (flash): "); Serial.println(whiteDimBrightness);
  Serial.print("  Orange brightness: "); Serial.println(orangeBrightness);
  Serial.print("  Orange hue       : "); Serial.println(orangeHue);
  Serial.print("  Rise time        : "); Serial.print(riseTime);  Serial.println(" ms");
  Serial.print("  Hold time        : "); Serial.print(holdTime);  Serial.println(" ms");
  Serial.print("  Fade time        : "); Serial.print(fadeTime);  Serial.println(" ms");
  Serial.print("  Pause time       : "); Serial.print(pauseTime); Serial.println(" ms");
  Serial.print("  Total cycle      : "); Serial.print((unsigned long)riseTime + holdTime + fadeTime + pauseTime); Serial.println(" ms");
  Serial.println("========================");
}
