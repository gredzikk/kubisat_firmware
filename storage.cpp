/*
 * Copyright 2024, Hiroyuki OYAMA. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "storage.h"

bool fs_init(void) {
    printf("fs_init littlefs on SD card\n");
    blockdevice_t *sd = blockdevice_sd_create(SD_SPI_PORT,
                                              SD_MOSI_PIN,
                                              SD_MISO_PIN,
                                              SD_SCK_PIN,
                                              SD_CS_PIN,
                                              24 * MHZ,
                                              false);
    filesystem_t *littlefs = filesystem_littlefs_create(500, 16);
    int err = fs_mount("/", littlefs, sd);
    if (err == -1) {
        printf("format / with littlefs\n");
        err = fs_format(littlefs, sd);
        if (err == -1) {
            printf("fs_format error: %s", strerror(errno));
            return false;
        }
        err = fs_mount("/", littlefs, sd);
        if (err == -1) {
            printf("fs_mount error: %s", strerror(errno));
            return false;
        }
    }

    return true;
}