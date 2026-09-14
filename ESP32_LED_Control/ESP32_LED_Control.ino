/*
 * ESP32-C3 Управление адресной светодиодной лентой
 * ESP32-C3 Addressable LED Strip Control
 *
 * Лента: 144 светодиода (WS2812B/NeoPixel)
 * Strip: 144 LEDs (WS2812B/NeoPixel)
 *
 * Требуемые библиотеки / Required libraries:
 * - FastLED (установить через менеджер библиотек Arduino IDE /
 *            install via Arduino IDE Library Manager)
 *
 * Подключение / Wiring:
 * - Лента к пину GPIO8 (можно изменить в коде)
 *   Strip to GPIO8 pin (can be changed in code)
 * - Питание ленты: 5V (рекомендуется внешний источник питания)
 *   Strip power: 5V (external power supply recommended)
 * - GND общий с ESP32-C3
 *   Common GND with ESP32-C3
 *
 * Серийные команды / Serial commands:
 * - "next"              — следующий эффект / next effect
 * - "effect <0-5>"      — выбрать эффект / select effect
 * - "brightness <0-255>" — установить яркость / set brightness
 */

#include <FastLED.h>

// === Настройки ленты / Strip configuration ===
#define LED_PIN     8        // Пин подключения ленты (можно изменить) / Data pin (can be changed)
#define NUM_LEDS    20      // Количество светодиодов / Number of LEDs
#define LED_TYPE    WS2812B  // Тип светодиодов / LED type
#define COLOR_ORDER GRB      // Порядок цветов (может быть GRB или RGB) / Color order (GRB or RGB)
#define BRIGHTNESS  50      // Яркость (0-255) / Brightness (0-255)

// Создаем массив светодиодов / Create LED array
CRGB leds[NUM_LEDS];

// === Переменные для эффектов / Effect variables ===
unsigned long previousMillis = 0;
int currentEffect = 0;  // Текущий эффект / Current effect
int hue = 0;            // Текущий цвет (тон) / Current hue
int position = 0;       // Текущая позиция / Current position
bool direction = true;   // Направление движения / Movement direction

// Количество эффектов / Number of effects
#define NUM_EFFECTS 6

// ============================================================
// Инициализация / Setup
// ============================================================
void setup() {
  // Инициализация Serial для отладки / Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-C3 LED Strip Control");

  // Инициализация FastLED / Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Очистка ленты (все LED выключены) / Clear strip (all LEDs off)
  FastLED.clear();
  FastLED.show();

  Serial.println("LED Strip initialized");
  Serial.println("Commands: next | effect <0-5> | brightness <0-255>");
}

// ============================================================
// Основной цикл / Main loop
// ============================================================
void loop() {
  // Проверка команд через Serial / Check Serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Команда "next" — переключить на следующий эффект
    // Command "next" — switch to next effect
    if (command == "next") {
      currentEffect = (currentEffect + 1) % NUM_EFFECTS;
      Serial.print("Effect: ");
      Serial.println(currentEffect);
      FastLED.clear();
    }
    // Команда "brightness <значение>" — установить яркость
    // Command "brightness <value>" — set brightness
    else if (command.startsWith("brightness ")) {
      int newBrightness = command.substring(11).toInt();
      if (newBrightness >= 0 && newBrightness <= 255) {
        FastLED.setBrightness(newBrightness);
        Serial.print("Brightness: ");
        Serial.println(newBrightness);
      }
    }
    // Команда "effect <номер>" — выбрать конкретный эффект
    // Command "effect <number>" — select specific effect
    else if (command.startsWith("effect ")) {
      int effectNum = command.substring(7).toInt();
      if (effectNum >= 0 && effectNum < NUM_EFFECTS) {
        currentEffect = effectNum;
        Serial.print("Effect: ");
        Serial.println(currentEffect);
        FastLED.clear();
      }
    }
  }

  // Выполнение текущего эффекта / Run current effect
  switch (currentEffect) {
    case 0: effectRainbow();      break;  // Радуга / Rainbow
    case 1: effectRainbowCycle(); break;  // Радужный цикл / Rainbow cycle
    case 2: effectColorWipe();    break;  // Цветная волна / Color wipe
    case 3: effectTheaterChase(); break;  // Театральная погоня / Theater chase
    case 4: effectFire();         break;  // Огонь / Fire
    case 5: effectStarryNight();  break;  // Звездная ночь / Starry night
  }

  FastLED.show();
  delay(20);  // Задержка для плавности / Delay for smooth animation
}

// ============================================================
// Эффект 0: Радуга / Effect 0: Rainbow
// Плавное переливание цветов по всей ленте
// Smooth color transition across the entire strip
// ============================================================
void effectRainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((hue + i * 2) % 256, 255, 255);
  }
  hue = (hue + 2) % 256;
}

// ============================================================
// Эффект 1: Радужный цикл / Effect 1: Rainbow Cycle
// Полный спектр радуги распределен по ленте
// Full rainbow spectrum distributed across the strip
// ============================================================
void effectRainbowCycle() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((hue + i * 256 / NUM_LEDS) % 256, 255, 255);
  }
  hue = (hue + 1) % 256;
}

// ============================================================
// Эффект 2: Цветная волна / Effect 2: Color Wipe
// Две противоположные точки движутся с затуханием
// Two opposing points move with fading trail
// ============================================================
void effectColorWipe() {
  static int pos = 0;
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 50) {
    leds[pos] = CHSV(hue, 255, 255);
    leds[(pos + NUM_LEDS / 2) % NUM_LEDS] = CHSV((hue + 128) % 256, 255, 255);

    // Затемнение предыдущих пикселей / Fade previous pixels
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].fadeToBlackBy(10);
    }

    pos = (pos + 1) % NUM_LEDS;
    hue = (hue + 1) % 256;
    lastUpdate = millis();
  }
}

// ============================================================
// Эффект 3: Театральная погоня / Effect 3: Theater Chase
// Классический эффект бегущих огней
// Classic running lights effect
// ============================================================
void effectTheaterChase() {
  static int step = 0;
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 100) {
    FastLED.clear();

    for (int i = step; i < NUM_LEDS; i += 3) {
      leds[i] = CHSV(hue, 255, 255);
    }

    step = (step + 1) % 3;
    hue = (hue + 5) % 256;
    lastUpdate = millis();
  }
}

// ============================================================
// Эффект 4: Огонь / Effect 4: Fire
// Имитация пламени с движением вверх
// Flame simulation with upward movement
// ============================================================
void effectFire() {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 50) {
    // Сдвигаем пиксели вверх / Shift pixels upward
    for (int i = NUM_LEDS - 1; i > 0; i--) {
      leds[i] = leds[i - 1];
      leds[i].fadeToBlackBy(30);
    }

    // Добавляем новый огонь внизу / Add new fire at the bottom
    int heat = random(160, 255);
    leds[0] = HeatColor(heat);

    lastUpdate = millis();
  }
}

// ============================================================
// Эффект 5: Звездная ночь / Effect 5: Starry Night
// Случайные вспышки с плавным затуханием
// Random sparkles with smooth fading
// ============================================================
void effectStarryNight() {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 100) {
    // Затемняем все пиксели / Fade all pixels
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].fadeToBlackBy(20);
    }

    // Добавляем случайные звезды / Add random stars
    if (random(10) < 3) {
      int starPos = random(NUM_LEDS);
      leds[starPos] = CHSV(random(256), random(100, 256), 255);
    }

    lastUpdate = millis();
  }
}
