#include <stdio.h>
#include "mupeSdCard.h"
#include <esp_log.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "mupeWeb.h"

#include "mupeDataBase.h"
#include "mupeSdCardWeb.h"
#include "mupeSdCardNvs.h"

#define PIN_NUM_MISO  19
#define PIN_NUM_MOSI  23
#define PIN_NUM_CLK   18
#define PIN_NUM_CS    5

#define FORMAT 0

static const char *TAG = "mupeSdCard";

char* pathReplace(char *string) {
	char find[8] = { '<', '>', ':', '\"',  '\\', '|', '?', '*' };

	for (int i = 0; i < 8; ++i) {
		for (int x = 0; x < strlen(string); ++x) {
			if (string[x]==find[i]){
				string[x]='_';
			}
		}
	}
	return string;
}

void mupeSdCardInit(void) {
	mupeSdCardWebInit();
	esp_err_t ret;
	esp_vfs_fat_sdmmc_mount_config_t mount_config = { .format_if_mount_failed =
	FORMAT, .max_files = 5, .allocation_unit_size = 16 * 1024 };
	sdmmc_card_t *card;
	const char mount_point[] = MOUNT_POINT;
	ESP_LOGI(TAG, "Initializing SD card");

// Use settings defined above to initialize SD card and mount FAT filesystem.
// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
// Please check its source code and implement error recovery when developing
// production applications.
	ESP_LOGI(TAG, "Using SPI peripheral");

// By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
// For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
// Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();

	uint32_t conf;
	conf=sDCardGet();


	spi_bus_config_t bus_cfg = { .mosi_io_num = (conf>>16)&0xFF , .miso_io_num =
			(conf>>24)&0xFF , .sclk_io_num = (conf>>8)&0xFF, .quadwp_io_num = -1,
			.quadhd_io_num = -1, .max_transfer_sz = 4000, };
	ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize bus.");
		return;
	}

// This initializes the slot without card detect (CD) and write protect (WP) signals.
// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = conf&0xFF ;
	slot_config.host_id = host.slot;

	ESP_LOGI(TAG, "Mounting filesystem");
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config,
			&mount_config, &card);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG,
					"Failed to mount filesystem. " "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE(TAG,
					"Failed to initialize the card (%s). " "Make sure SD card lines have pull-up resistors in place.",
					esp_err_to_name(ret));
		}
		return;
	}
	ESP_LOGI(TAG, "Filesystem mounted");

// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	mupeDataBaseInit();

//	ESP_LOGI(TAG, "Read from file: '%s'", line);

// All done, unmount partition and disable SPI peripheral
//	esp_vfs_fat_sdcard_unmount(mount_point, card);
//	ESP_LOGI(TAG, "Card unmounted");

//deinitialize the bus after all devices are removed
//	spi_bus_free(host.slot);
}

