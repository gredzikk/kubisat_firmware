/*
 * Copyright 2024, Hiroyuki OYAMA. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "storage.h"
#include "errno.h"
#include "utils.h"

/**
 * @file storage.cpp
 * @brief Implements file system operations for the Kubisat firmware.
 * @details This file contains functions for initializing the file system, opening, writing,
 *          reading, and closing files.
 */

/**
 * @brief Initializes the file system on the SD card.
 * @return True if initialization was successful, false otherwise.
 * @details Mounts the littlefs file system on the SD card. If mounting fails, it formats the SD card
 *          with littlefs and then attempts to mount again.
 */
bool fs_init(void) {
    uart_print("fs_init littlefs on SD card\n");
    blockdevice_t *sd = blockdevice_sd_create(SD_SPI_PORT,
                                              SD_MOSI_PIN,
                                              SD_MISO_PIN,
                                              SD_SCK_PIN,
                                              SD_CS_PIN,
                                              24 * MHZ,
                                              false);
    filesystem_t *fat = filesystem_fat_create();

    std::string statusString;
    int err = fs_mount("/", fat, sd);
    if (err == -1) {
        statusString = "Formatting / with FAT";
        uart_print(statusString);
        err = fs_format(fat, sd);
        if (err == -1) {
            statusString = "fs_format error: " + std::string(strerror(errno));
            uart_print(statusString);
            return false;
        }
        err = fs_mount("/", fat, sd);
        if (err == -1) {
            statusString = "fs_mount error: " + std::string(strerror(errno));
            uart_print(statusString);
            return false;
        }
    }

    return true;
}
