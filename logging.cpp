#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "sd_card.h"
#include "ff.h"
#include "utils.h"
#include "storage.h" // Include storage.h to access the initialization functions

#define LOG_FILENAME "log.txt"
#define EVENT_LOG_FILENAME "event_log.txt" // Define event log filename
#define LOG_BUFFER_SIZE 512

// Define the power falling signal (must match event_manager.cpp)
#define POWER_FALLING_SIGNAL 0x01

// Define a signal to indicate an event log message
#define EVENT_LOG_SIGNAL 0x02 // Choose a different signal value

// Global variables for event logging
FIL eventFil;
bool eventFile_opened = false;

void logEventToFile(const char* message) {
    FRESULT fr;
    char buf[1024]; // Local buffer for file operations
    if (eventFile_opened) {
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        char timestampedMessage[256]; // Adjust size as needed
        snprintf(timestampedMessage, sizeof(timestampedMessage), "[%lu ms] %s\n", currentTime, message);

        // Use storage.cpp's writeToFile function
        if (!writeToFile(eventFil, timestampedMessage, buf)) {
            printf("Core 1: Failed to write to event log file.\n");
        }
    } else {
        printf("Core 1: Event log file not opened!\n");
    }
}

void loggingRoutine() {
    char logBuffer[LOG_BUFFER_SIZE];
    int logIndex = 0;
    FRESULT fr;
    FIL fil;
    FATFS fs;
    char buf[1024]; // Buffer for SD card functions
    bool sd_initialized = false;
    bool sd_mounted = false;
    bool file_opened = false;

    // Initialize SD card driver
    if (!initializeSDCard(buf)) {
        printf("Core 1: SD card driver initialization failed!\n");
        return;
    }
    sd_initialized = true;

    // Mount the file system
    if (!mountDrive(fs, buf)) {
        printf("Core 1: SD card mount failed!\n");
        return;
    }
    sd_mounted = true;

    // Open the log file (create if it doesn't exist)
    if (!openFile(fil, LOG_FILENAME, FA_WRITE | FA_CREATE_ALWAYS, buf)) {
        printf("Core 1: Failed to open log file.\n");
        return;
    }
    file_opened = true;

    // Open the event log file
    if (!openFile(eventFil, EVENT_LOG_FILENAME, FA_WRITE | FA_CREATE_ALWAYS, buf)) {
        printf("Core 1: Failed to open event log file.\n");
    } else {
        eventFile_opened = true;
    }

    printf("Core 1: Logging routine started.\n");

    while (true) {
        // Read from the FIFO
        char c = multicore_fifo_pop_blocking();

        if (c == POWER_FALLING_SIGNAL) {
            // Power falling signal received!
            printf("Core 1: Power falling signal received. Closing log file and unmounting SD card.\n");

            // Close the main log file
            if (file_opened) {
                if (!closeFile(fil, buf)) {
                    printf("Core 1: Failed to close log file.\n");
                }
                file_opened = false;
            }

            // Close the event log file
            if (eventFile_opened) {
                if (!closeFile(eventFil, buf)) {
                    printf("Core 1: Failed to close event log file.\n");
                }
                eventFile_opened = false;
            }

            // Unmount the SD card
            if (sd_mounted) {
                fr = f_unmount("0:");
                if (fr != FR_OK) {
                    printf("Core 1: Failed to unmount SD card: %d\n", fr);
                }
                sd_mounted = false;
            }

            // Potentially add code here to shut down other peripherals or enter a low-power state

            break; // Exit the logging routine
        } else if (c == EVENT_LOG_SIGNAL) {
            // Event log signal received
            // Read the length of the message from the FIFO
            int messageLength = multicore_fifo_pop_blocking();
            if (messageLength > 0 && messageLength < 256) {
                char eventMessage[256];
                for (int i = 0; i < messageLength; ++i) {
                    eventMessage[i] = multicore_fifo_pop_blocking();
                }
                eventMessage[messageLength] = '\0'; // Null-terminate the message
                logEventToFile(eventMessage);
            }
        } else if (c == 0) { // Null terminator indicates end of message
            // Null-terminate the log buffer
            logBuffer[logIndex] = '\0';

            // Use storage.cpp's writeToFile function
            if (!writeToFile(fil, logBuffer, buf)) {
                printf("Core 1: Failed to write to log file.\n");
            }

            // Reset the index
            logIndex = 0;
        } else {
            // Add the character to the log buffer
            logBuffer[logIndex++] = c;

            // Check for buffer overflow
            if (logIndex >= LOG_BUFFER_SIZE - 1) {
                printf("Core 1: Log buffer overflow!\n");
                logIndex = 0; // Reset to avoid further overflows
            }
        }
    }

    // Close the files (this might not be reached in this example)
    if (file_opened) {
        closeFile(fil, buf);
    }
    if (eventFile_opened) {
        closeFile(eventFil, buf);
    }
    if (sd_mounted) {
        f_unmount("0:"); // Unmount the drive before exiting
    }
}