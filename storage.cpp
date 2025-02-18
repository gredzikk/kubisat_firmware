/*
 * Copyright 2024, Hiroyuki OYAMA. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "storage.h"
#include "errno.h"

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

FileHandle fs_open_file(const char* filename, const char* mode) {
    FileHandle handle = {-1, false};
    
    // Convert mode string to flags
    int flags = 0;
    if (strchr(mode, 'r')) flags |= O_RDONLY;
    if (strchr(mode, 'w')) flags |= O_WRONLY | O_CREAT | O_TRUNC;
    if (strchr(mode, 'a')) flags |= O_WRONLY | O_CREAT | O_APPEND;
    if (strchr(mode, '+')) flags = O_RDWR | (flags & ~(O_RDONLY | O_WRONLY));

    // Open the file
    handle.fd = open(filename, flags, 0666);
    if (handle.fd >= 0) {
        handle.is_open = true;
    } else {
        printf("Failed to open file %s: %s\n", filename, strerror(errno));
    }
    
    return handle;
}

ssize_t fs_write_file(FileHandle& handle, const void* buffer, size_t size) {
    if (!handle.is_open) {
        return -1;
    }

    ssize_t written = write(handle.fd, buffer, size);
    if (written < 0) {
        printf("Write failed: %s\n", strerror(errno));
    }
    return written;
}

ssize_t fs_read_file(FileHandle& handle, void* buffer, size_t size) {
    if (!handle.is_open) {
        return -1;
    }

    ssize_t bytes_read = read(handle.fd, buffer, size);
    if (bytes_read < 0) {
        printf("Read failed: %s\n", strerror(errno));
    }
    return bytes_read;
}

bool fs_close_file(FileHandle& handle) {
    if (!handle.is_open) {
        return false;
    }

    int result = close(handle.fd);
    if (result == 0) {
        handle.is_open = false;
        handle.fd = -1;
        return true;
    } else {
        printf("Close failed: %s\n", strerror(errno));
        return false;
    }
}

// Helper function to check if file exists
bool fs_file_exists(const char* filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}