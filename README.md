# ESP32 Addressable LED Strip / Адресная LED-лента на ESP32

> **[English](#english)** | **[Русский](#русский)**

---

<a id="english"></a>

## English

### Description

ESP32-C3 controller for WS2812B (NeoPixel) addressable LED strip with 6 built-in light effects and Serial control.

### Features

- **6 light effects:**
  - Rainbow — smooth color transition across the strip
  - Rainbow Cycle — full spectrum distributed along the strip
  - Color Wipe — two opposing color points with fading trail
  - Theater Chase — classic running lights
  - Fire — realistic flame simulation
  - Starry Night — random sparkles with smooth fading
- **Serial control:** switch effects, adjust brightness in real time
- **Configurable:** LED count, pin, brightness — easily adjustable via `#define`

### Hardware Requirements

| Component | Specification |
|-----------|---------------|
| Microcontroller | ESP32-C3 (or any ESP32) |
| LED strip | WS2812B / NeoPixel, 144 LEDs |
| Power supply | 5V, ≥ 8A recommended for 144 LEDs |
| Data pin | GPIO8 (configurable) |

### Wiring Diagram

```
ESP32-C3          LED Strip
─────────         ─────────
GPIO8  ──────────  DIN (Data In)
GND    ──────────  GND
               5V ── VCC (external PSU)
               GND ── GND (external PSU)
```

> **Important:** For 144 LEDs, use an external 5V power supply. Do NOT power the strip directly from the ESP32.

### Installation

1. Install [Arduino IDE](https://www.arduino.cc/en/software) (v2.0+) or [PlatformIO](https://platformio.org/)
2. Add ESP32 board support:
   - In Arduino IDE: File → Preferences → Additional Board URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
3. Install the **FastLED** library (v3.6.0+):
   - Arduino IDE: Sketch → Include Library → Manage Libraries → search "FastLED"
4. Open `ESP32_LED_Control/ESP32_LED_Control.ino`
5. Select board: **ESP32C3 Dev Module**
6. Upload to your ESP32-C3

### Serial Commands

Open Serial Monitor at **115200 baud** and send:

| Command | Description |
|---------|-------------|
| `next` | Switch to next effect |
| `effect <0-5>` | Select specific effect by number |
| `brightness <0-255>` | Set brightness level |

### Configuration

Edit the `#define` values at the top of the `.ino` file:

```cpp
#define LED_PIN     8        // Data pin
#define NUM_LEDS    144      // Number of LEDs
#define LED_TYPE    WS2812B  // LED type
#define COLOR_ORDER GRB      // Color order
#define BRIGHTNESS  100      // Default brightness (0-255)
```

### License

This project is licensed under the [MIT License](LICENSE).

---

<a id="русский"></a>

## Русский

### Описание

Контроллер адресной светодиодной ленты WS2812B (NeoPixel) на ESP32-C3 с 6 встроенными световыми эффектами и управлением через Serial-порт.

### Возможности

- **6 световых эффектов:**
  - Радуга — плавное переливание цветов по ленте
  - Радужный цикл — полный спектр распределён по длине ленты
  - Цветная волна — две противоположные цветовые точки с затуханием
  - Театральная погоня — классические бегущие огни
  - Огонь — реалистичная имитация пламени
  - Звёздная ночь — случайные вспышки с плавным затуханием
- **Управление через Serial:** переключение эффектов, настройка яркости в реальном времени
- **Настраиваемость:** количество LED, пин, яркость — легко меняются через `#define`

### Аппаратные требования

| Компонент | Характеристика |
|-----------|----------------|
| Микроконтроллер | ESP32-C3 (или любой ESP32) |
| LED-лента | WS2812B / NeoPixel, 144 светодиода |
| Блок питания | 5V, рекомендуется ≥ 8A для 144 LED |
| Пин данных | GPIO8 (настраиваемый) |

### Схема подключения

```
ESP32-C3          LED-лента
─────────         ─────────
GPIO8  ──────────  DIN (вход данных)
GND    ──────────  GND
               5V ── VCC (внешний БП)
               GND ── GND (внешний БП)
```

> **Важно:** Для 144 светодиодов используйте внешний блок питания 5V. НЕ питайте ленту напрямую от ESP32.

### Установка

1. Установите [Arduino IDE](https://www.arduino.cc/en/software) (v2.0+) или [PlatformIO](https://platformio.org/)
2. Добавьте поддержку плат ESP32:
   - В Arduino IDE: Файл → Настройки → Дополнительные URL для менеджера плат:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
3. Установите библиотеку **FastLED** (v3.6.0+):
   - Arduino IDE: Скетч → Подключить библиотеку → Управлять библиотеками → поиск "FastLED"
4. Откройте `ESP32_LED_Control/ESP32_LED_Control.ino`
5. Выберите плату: **ESP32C3 Dev Module**
6. Загрузите на ESP32-C3

### Команды Serial

Откройте Serial Monitor на **115200 бод** и отправляйте:

| Команда | Описание |
|---------|----------|
| `next` | Переключить на следующий эффект |
| `effect <0-5>` | Выбрать эффект по номеру |
| `brightness <0-255>` | Установить яркость |

### Настройка

Измените значения `#define` в начале `.ino` файла:

```cpp
#define LED_PIN     8        // Пин данных
#define NUM_LEDS    144      // Количество светодиодов
#define LED_TYPE    WS2812B  // Тип светодиодов
#define COLOR_ORDER GRB      // Порядок цветов
#define BRIGHTNESS  100      // Яркость по умолчанию (0-255)
```

### Лицензия

Проект распространяется под лицензией [MIT](LICENSE).
