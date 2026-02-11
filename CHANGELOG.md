# Журнал изменений / Changelog

Формат основан на [Keep a Changelog](https://keepachangelog.com/ru/1.1.0/),
проект придерживается [Семантического версионирования](https://semver.org/lang/ru/).

This format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

---

## [1.0.0] — 2026-02-11

### Добавлено / Added

- Начальная версия проекта управления адресной LED-лентой на ESP32-C3.
  Initial release of ESP32-C3 addressable LED strip controller.
- Поддержка 144 светодиодов WS2812B (NeoPixel) через библиотеку FastLED.
  Support for 144 WS2812B (NeoPixel) LEDs via FastLED library.
- 6 световых эффектов / 6 light effects:
  - **Радуга / Rainbow** — плавное переливание цветов / smooth color transition.
  - **Радужный цикл / Rainbow Cycle** — полный спектр по длине ленты / full spectrum across strip.
  - **Цветная волна / Color Wipe** — две точки с затуханием / two opposing points with fade.
  - **Театральная погоня / Theater Chase** — бегущие огни / running lights.
  - **Огонь / Fire** — имитация пламени / flame simulation.
  - **Звёздная ночь / Starry Night** — случайные вспышки / random sparkles.
- Управление через Serial-порт / Serial port control:
  - `next` — следующий эффект / next effect.
  - `effect <0-5>` — выбрать эффект / select effect.
  - `brightness <0-255>` — установить яркость / set brightness.
