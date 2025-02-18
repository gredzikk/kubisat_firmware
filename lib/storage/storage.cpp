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
    uartPrint("fs_init littlefs on SD card\n");
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
        uartPrint(statusString);
        err = fs_format(fat, sd);
        if (err == -1) {
            statusString = "fs_format error: " + std::string(strerror(errno));
            uartPrint(statusString);
            return false;
        }
        err = fs_mount("/", fat, sd);
        if (err == -1) {
            statusString = "fs_mount error: " + std::string(strerror(errno));
            uartPrint(statusString);
            return false;
        }
    }

    return true;
}


/**
 * @brief Opens a file.
 * @param filename The name of the file to open.
 * @param mode The mode in which to open the file (e.g., "r", "w", "a").
 * @return A FileHandle structure containing the file descriptor and a flag indicating if the file is open.
 * @details Opens the specified file with the given mode. Converts the mode string to appropriate flags
 *          for the `open` system call.
 */
FileHandle fs_open_file(const char* filename, const char* mode) {
    FileHandle handle = {-1, false};
    std::string errorStr;

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
        errorStr = "Failed to open file " + std::string(filename) + ": " + strerror(errno);
        uartPrint(errorStr);
    }
    
    return handle;
}


/**
 * @brief Writes data to a file.
 * @param handle A reference to the FileHandle structure for the file.
 * @param buffer A pointer to the buffer containing the data to write.
 * @param size The number of bytes to write.
 * @return The number of bytes written, or -1 on error.
 * @details Writes data from the provided buffer to the file associated with the given FileHandle.
 */
ssize_t fs_write_file(FileHandle& handle, const void* buffer, size_t size) {
    if (!handle.is_open) {
        return -1;
    }

    std::string errorStr;

    ssize_t written = write(handle.fd, buffer, size);
    if (written < 0) {
        errorStr = "Write failed: " + std::string(strerror(errno));
        uartPrint(errorStr);
    }
    return written;
}


/**
 * @brief Reads data from a file.
 * @param handle A reference to the FileHandle structure for the file.
 * @param buffer A pointer to the buffer to store the read data.
 * @param size The number of bytes to read.
 * @return The number of bytes read, or -1 on error.
 * @details Reads data from the file associated with the given FileHandle into the provided buffer.
 */
ssize_t fs_read_file(FileHandle& handle, void* buffer, size_t size) {
    if (!handle.is_open) {
        return -1;
    }

    std::string errorStr;

    ssize_t bytes_read = read(handle.fd, buffer, size);
    if (bytes_read < 0) {
        errorStr = "Read failed " + std::string(strerror(errno));
        uartPrint(errorStr);
    }
    return bytes_read;
}


/**
 * @brief Closes a file.
 * @param handle A reference to the FileHandle structure for the file.
 * @return True if the file was closed successfully, false otherwise.
 * @details Closes the file associated with the given FileHandle.
 */
bool fs_close_file(FileHandle& handle) {
    if (!handle.is_open) {
        return false;
    }

    std::string errorStr;

    int result = close(handle.fd);
    if (result == 0) {
        handle.is_open = false;
        handle.fd = -1;
        return true;
    } else {
        errorStr = "Close failed " + std::string(strerror(errno));
        uartPrint(errorStr);
        return false;
    }
}


/**
 * @brief Checks if a file exists.
 * @param filename The name of the file to check.
 * @return True if the file exists, false otherwise.
 * @details Checks if a file with the given name exists in the file system.
 */
bool fs_file_exists(const char* filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}