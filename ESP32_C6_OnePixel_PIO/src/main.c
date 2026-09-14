/*
 * ESP-IDF + ESP32-C6: один адресный светодиод (WS2812B/NeoPixel) на GPIO7
 *
 * Подключение:
 * - DIN -> GPIO7 (желательно резистор 330-470 Ом)
 * - Питание 5V (или 3.3V, если ваш светодиод уверенно работает от 3.3V)
 * - Общий GND с ESP32-C6 обязателен
 */

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"

static const char *TAG = "onepixel";

typedef struct {
  rmt_encoder_t base;
  rmt_encoder_t *bytes_encoder;
  rmt_encoder_t *copy_encoder;
  rmt_symbol_word_t reset_code;
  int state;
} ws2812_encoder_t;

static size_t ws2812_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel,
                            const void *primary_data, size_t data_size,
                            rmt_encode_state_t *ret_state) {
  ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
  size_t encoded_symbols = 0;
  rmt_encode_state_t state = RMT_ENCODING_RESET;

  if (ws->state == 0) {
    encoded_symbols += ws->bytes_encoder->encode(ws->bytes_encoder, channel,
                                                primary_data, data_size, &state);
    if (state & RMT_ENCODING_COMPLETE) ws->state = 1;
    if (state & RMT_ENCODING_MEM_FULL) {
      *ret_state = RMT_ENCODING_MEM_FULL;
      return encoded_symbols;
    }
  }

  if (ws->state == 1) {
    encoded_symbols += ws->copy_encoder->encode(ws->copy_encoder, channel,
                                               &ws->reset_code, sizeof(ws->reset_code), &state);
    if (state & RMT_ENCODING_COMPLETE) {
      ws->state = 0;
      *ret_state = RMT_ENCODING_COMPLETE;
    } else if (state & RMT_ENCODING_MEM_FULL) {
      *ret_state = RMT_ENCODING_MEM_FULL;
    }
    return encoded_symbols;
  }

  *ret_state = RMT_ENCODING_MEM_FULL;
  return encoded_symbols;
}

static esp_err_t ws2812_del(rmt_encoder_t *encoder) {
  ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
  if (ws->bytes_encoder) ws->bytes_encoder->del(ws->bytes_encoder);
  if (ws->copy_encoder) ws->copy_encoder->del(ws->copy_encoder);
  free(ws);
  return ESP_OK;
}

static esp_err_t ws2812_reset(rmt_encoder_t *encoder) {
  ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
  ws->state = 0;
  if (ws->bytes_encoder) ws->bytes_encoder->reset(ws->bytes_encoder);
  if (ws->copy_encoder) ws->copy_encoder->reset(ws->copy_encoder);
  return ESP_OK;
}

static esp_err_t ws2812_new_encoder(rmt_encoder_handle_t *ret_encoder) {
  // 10MHz resolution => 0.1us per tick
  // WS2812 timing (approx):
  // 0: T0H=0.4us, T0L=0.85us
  // 1: T1H=0.8us, T1L=0.45us
  ws2812_encoder_t *ws = calloc(1, sizeof(ws2812_encoder_t));
  if (!ws) return ESP_ERR_NO_MEM;

  rmt_bytes_encoder_config_t bytes_cfg = {
      .bit0 = {
          .level0 = 1, .duration0 = 4,
          .level1 = 0, .duration1 = 9,
      },
      .bit1 = {
          .level0 = 1, .duration0 = 8,
          .level1 = 0, .duration1 = 5,
      },
      .flags.msb_first = 1,
  };
  esp_err_t err = rmt_new_bytes_encoder(&bytes_cfg, &ws->bytes_encoder);
  if (err != ESP_OK) {
    free(ws);
    return err;
  }

  rmt_copy_encoder_config_t copy_cfg = {};
  err = rmt_new_copy_encoder(&copy_cfg, &ws->copy_encoder);
  if (err != ESP_OK) {
    ws->bytes_encoder->del(ws->bytes_encoder);
    free(ws);
    return err;
  }

  // Reset code: hold low for >50us. At 10MHz, 50us = 500 ticks.
  ws->reset_code = (rmt_symbol_word_t){
      .level0 = 0, .duration0 = 500,
      .level1 = 0, .duration1 = 0,
  };

  ws->base.encode = ws2812_encode;
  ws->base.del = ws2812_del;
  ws->base.reset = ws2812_reset;
  ws->state = 0;

  *ret_encoder = &ws->base;
  return ESP_OK;
}

static uint32_t hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v) {
  // h,s,v: 0..255. returns 0xRRGGBB
  const uint8_t region = h / 43;          // 0..5
  const uint8_t remainder = (h - region * 43) * 6; // 0..258

  const uint8_t p = (uint8_t)((v * (255 - s)) >> 8);
  const uint8_t q = (uint8_t)((v * (255 - ((s * remainder) >> 8))) >> 8);
  const uint8_t t = (uint8_t)((v * (255 - ((s * (255 - remainder)) >> 8))) >> 8);

  uint8_t r = 0, g = 0, b = 0;
  switch (region) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void app_main(void) {
  ESP_LOGI(TAG, "Started. WS2812 on GPIO7, 1 LED (RMT).");

  const gpio_num_t led_gpio = GPIO_NUM_7;

  rmt_tx_channel_config_t tx_cfg = {
      .gpio_num = led_gpio,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 10 * 1000 * 1000, // 10MHz => 0.1us
      .mem_block_symbols = 64,
      .trans_queue_depth = 4,
      .flags.invert_out = 0,
      .flags.with_dma = 0,
  };

  rmt_channel_handle_t tx_chan = NULL;
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_cfg, &tx_chan));

  rmt_encoder_handle_t encoder = NULL;
  ESP_ERROR_CHECK(ws2812_new_encoder(&encoder));

  ESP_ERROR_CHECK(rmt_enable(tx_chan));

  uint8_t hue = 0;
  while (1) {
    uint32_t rgb = hsv_to_rgb(hue++, 255, 64); // V=64 чтобы не слепило
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;

    // WS2812 expects GRB order
    uint8_t grb[3] = {g, r, b};
    rmt_transmit_config_t tx_trans_cfg = {
        .loop_count = 0,
    };
    ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, grb, sizeof(grb), &tx_trans_cfg));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_chan, portMAX_DELAY));

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

