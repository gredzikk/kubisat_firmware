#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <string.h>
#include <hardware/clocks.h>
#include <hardware/flash.h>
#include "blockdevice/flash.h"
#include "blockdevice/sd.h"
#include "filesystem/littlefs.h"
#include "filesystem/vfs.h"
#include "pin_config.h"
#include "lfs.h" 
#include "filesystem/fat.h"
struct FileHandle {
    int fd;
    bool is_open;
};

bool fs_init(void);
FileHandle fs_open_file(const char* filename, const char* mode);
ssize_t fs_write_file(FileHandle& handle, const void* buffer, size_t size);
ssize_t fs_read_file(FileHandle& handle, void* buffer, size_t size);
bool fs_close_file(FileHandle& handle);
bool fs_file_exists(const char* filename);

#endif

// void example_file_operations() {
//     // Open a file for writing
//     FileHandle log_file = fs_open_file("/log.txt", "w");
//     if (!log_file.is_open) {
//         uartPrint("Failed to open log file");
//         return;
//     }

//     // Write some data
//     const char* message = "Hello, World!\n";
//     ssize_t written = fs_write_file(log_file, message, strlen(message));
//     if (written < 0) {
//         uartPrint("Failed to write to log file");
//     }

//     // Close the file
//     fs_close_file(log_file);

//     // Open file for reading
//     log_file = fs_open_file("/log.txt", "r");
//     if (!log_file.is_open) {
//         uartPrint("Failed to open log file for reading");
//         return;
//     }

//     // Read the data
//     char buffer[128];
//     ssize_t bytes_read = fs_read_file(log_file, buffer, sizeof(buffer) - 1);
//     if (bytes_read > 0) {
//         buffer[bytes_read] = '\0';  // Null terminate the string
//         uartPrint(buffer);
//     }

//     // Close the file
//     fs_close_file(log_file);
// }