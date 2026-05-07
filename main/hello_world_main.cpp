/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#define LGFX_WT32_SC01

#include "LGFX_ESP32S3_WT32_SC01_Plus.hpp"
#include <LovyanGFX.hpp>

static LGFX lcd;
static LGFX_Sprite sprite[10];

static std::uint32_t count = 0;
static float zoom = 0;


#include "partyparrot.h"

void setup()
{
  lcd.init();
  lcd.setRotation(0);
  if (lcd.width() < lcd.height()) { lcd.setRotation(lcd.getRotation() ^ 1); }

  zoom = (float)lcd.width() / 128;
  float ztmp = (float)lcd.height() / 96;
  if (zoom > ztmp) { zoom = ztmp; }

  // lcd.setPivot(lcd.width() >> 1, lcd.height() >> 1);
  lcd.setPivot(0, 0);
  lcd.fillScreen(0xFFFFFFU);

  sprite[0].createFromBmp(parrot00);
  sprite[1].createFromBmp(parrot01);
  sprite[2].createFromBmp(parrot02);
  sprite[3].createFromBmp(parrot03);
  sprite[4].createFromBmp(parrot04);
  sprite[5].createFromBmp(parrot05);
  sprite[6].createFromBmp(parrot06);
  sprite[7].createFromBmp(parrot07);
  sprite[8].createFromBmp(parrot08);
  sprite[9].createFromBmp(parrot09);
}

// Arduino関数のエミュレーション(エラーチェックしていない)
int random(int s, int e)
{
	int r = s - e + 1;
	return rand() % r;
}

void loop() {
  static uint32_t ri[4];

  std::int32_t tx, ty, tc;
  tc = lcd.getTouch(&tx, &ty);

  if (++count == 10) count = 0;
  if (tc)
  {
    count = 0;
    ri[0] = random(0, 9);
    ri[1] = random(0, 9);
    ri[2] = random(0, 9);
    ri[3] = random(0, 9);
  }

  float z = zoom / (2.0);

  int32_t xbase = lcd.width() >> (2);
  int32_t ybase = lcd.height() >> (2);
  int32_t dox = lcd.width() >> (1);
  int32_t doy = lcd.height() >> (1);

  sprite[(count + ri[0]) % 10].pushRotateZoom(&lcd, xbase, ybase, 0, z, z);
  sprite[(count + ri[1]) % 10].pushRotateZoom(&lcd, xbase + dox, ybase, 0, z, z);
  sprite[(count + ri[2]) % 10].pushRotateZoom(&lcd, xbase, ybase + doy, 0, z, z);
  sprite[(count + ri[3]) % 10].pushRotateZoom(&lcd, xbase + dox, ybase + doy, 0, z, z);
}

extern "C" void app_main(void)
{
    printf("Hello world!\n");

    setup();

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 5; i >= 0; i--) {
        printf("starting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    // esp_restart();

	while(1)
	{
		loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);	/* 1tick以上ないとWDT警告が出る? */
	}

}
