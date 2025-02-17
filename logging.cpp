#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "utils.h"

#define LOG_FILENAME "log.txt"
#define EVENT_LOG_FILENAME "event_log.txt" // Define event log filename
#define LOG_BUFFER_SIZE 512

// Define the power falling signal (must match event_manager.cpp)
#define POWER_FALLING_SIGNAL 0x01

// Define a signal to indicate an event log message
#define EVENT_LOG_SIGNAL 0x02 // Choose a different signal value

void loggingRoutine() {
    // // Initialize SD card driver
    // if (!initializeSDCard(buf)) {
    //     uartPrint("SD card driver initialization failed!\n");
    //     return;
    // }
    // sd_initialized = true;
    // uartPrint("SD card driver initialized.");

    // // Mount the file system
    // if (!mountDrive(fs, buf)) {
    //     uartPrint("SD card mount failed!\n");
    //     return;
    // }
    // sd_mounted = true;
    // uartPrint("SD card mounted.");

    uartPrint("Logging routine started.");

    while (true) {
        // Read from the FIFO
        char c = multicore_fifo_pop_blocking();

        if (c == POWER_FALLING_SIGNAL) {
            // Power falling signal received!
            uartPrint("Power falling signal received. Closing log file and unmounting SD card.\n");

            break; // Exit the logging routine
        } else if (c == EVENT_LOG_SIGNAL) {
            uartPrint("Event log signal received.");
            // Event log signal received
            // Read the length of the message from the FIFO
        }
    }
}