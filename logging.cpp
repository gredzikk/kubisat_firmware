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

void logEventToFile(const char* event) {
    //writeToFile(EVENT_LOG_FILENAME, event, nullptr);
}

void loggingRoutine() {
    char logBuffer[LOG_BUFFER_SIZE];
    int logIndex = 0;
    FRESULT fr;
    FATFS fs;
    char buf[1024]; // Buffer for SD card functions
    bool sd_initialized = false;
    bool sd_mounted = false;

    // Initialize SD card driver
    if (!initializeSDCard(buf)) {
        uartPrint("SD card driver initialization failed!\n");
        return;
    }
    sd_initialized = true;
    uartPrint("SD card driver initialized.");

    // Mount the file system
    if (!mountDrive(fs, buf)) {
        uartPrint("SD card mount failed!\n");
        return;
    }
    sd_mounted = true;
    uartPrint("SD card mounted.");

    uartPrint("Logging routine started.");

    while (true) {
        // Read from the FIFO
        char c = multicore_fifo_pop_blocking();

        if (c == POWER_FALLING_SIGNAL) {
            // Power falling signal received!
            uartPrint("Power falling signal received. Closing log file and unmounting SD card.\n");

            // Unmount the SD card
            if (sd_mounted) {
                fr = f_unmount("SD");
                if (fr != FR_OK) {
                    uartPrint("Failed to unmount SD card: " + std::to_string(fr));
                }
                sd_mounted = false;
            }

            // Potentially add code here to shut down other peripherals or enter a low-power state

            break; // Exit the logging routine
        } else if (c == EVENT_LOG_SIGNAL) {
            uartPrint("Event log signal received.");
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
        } else if (c == 0) { 
            uartPrint("End of message signal received. Writing to log file.");
            // Null-terminate the log buffer
            logBuffer[logIndex] = '\0';

            // Use storage.cpp's writeToFile function
            //writeToFile(LOG_FILENAME, logBuffer, buf);

            // Reset the index
            logIndex = 0;
        } else {
            // Add the character to the log buffer
            logBuffer[logIndex++] = c;

            // Check for buffer overflow
            if (logIndex >= LOG_BUFFER_SIZE - 1) {
                uartPrint("Log buffer overflow!\n");
                logIndex = 0; // Reset to avoid further overflows
            }
        }
    }

    if (sd_mounted) {
        f_unmount("SD"); // Unmount the drive before exiting
    }
}