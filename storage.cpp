#include "ff.h"
#include "sd_card.h"
#include "pin_config.h"
#include "stdio.h"
#include <pico/stdio.h>
#include "PowerManager.h"
#include "storage.h"
#include "utils.h"

extern PowerManager powerManager;

// Helper function to convert FRESULT enum to string
const std::string fresult_str(FRESULT fr) {
    switch (fr) {
        case FR_OK: return "FR_OK";
        case FR_DISK_ERR: return "FR_DISK_ERR";
        case FR_INT_ERR: return "FR_INT_ERR";
        case FR_NOT_READY: return "FR_NOT_READY";
        case FR_NO_FILE: return "FR_NO_FILE";
        case FR_NO_PATH: return "FR_NO_PATH";
        case FR_INVALID_NAME: return "FR_INVALID_NAME";
        case FR_DENIED: return "FR_DENIED";
        case FR_EXIST: return "FR_EXIST";
        case FR_INVALID_OBJECT: return "FR_INVALID_OBJECT";
        case FR_WRITE_PROTECTED: return "FR_WRITE_PROTECTED";
        case FR_INVALID_DRIVE: return "FR_INVALID_DRIVE";
        case FR_NOT_ENABLED: return "FR_NOT_ENABLED";
        case FR_NO_FILESYSTEM: return "FR_NO_FILESYSTEM";
        case FR_MKFS_ABORTED: return "FR_MKFS_ABORTED";
        case FR_TIMEOUT: return "FR_TIMEOUT";
        case FR_LOCKED: return "FR_LOCKED";
        case FR_NOT_ENOUGH_CORE: return "FR_NOT_ENOUGH_CORE";
        case FR_TOO_MANY_OPEN_FILES: return "FR_TOO_MANY_OPEN_FILES";
        case FR_INVALID_PARAMETER: return "FR_INVALID_PARAMETER";
        default: return "UNKNOWN_FRESULT";
    }
}

/**
 * @brief Writes data to a file, handling open, write, and close operations.
 * @param filename Name of the file to write to.
 * @param data Data string to write.
 * @param buf Buffer for user input.
 * @return True if successful, false if an error occurred.
 */
bool writeToFile(const char* filename, const char* data) {
    uint32_t status = save_and_disable_interrupts();
    FIL fil;
    FRESULT fr;
    uartPrint("Writing data to file " + std::string(filename));
    // Open the file
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        uartPrint("ERROR: Could not open file " + fresult_str(fr));
        return false;
    }
    uartPrint(std::string("Opened file ") + filename);

    // Write to the file
    int ret = f_printf(&fil, data);
    if (ret < 0) {
        uartPrint("ERROR: Could not write to file " + std::to_string(ret));
        f_close(&fil);
        return false;
    }
    uartPrint("Wrote " + std::to_string(ret) + " bytes to file " + filename);

    // Close the file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        uartPrint("ERROR: Could not close file " + fresult_str(fr));
        return false;
    }
    uartPrint("File closed, exiting");
    restore_interrupts(status);

    return true;
}

/**
 * @brief Waits for user input (enter to proceed or 's' to skip) with timeout.
 * @param TIMEOUT_MS Maximum time (in ms) to wait for user input.
 * @return True if proceed, false if skipping.
 */
bool waitForUserInteraction(const uint64_t TIMEOUT_MS)
{
    uint64_t startTime = to_ms_since_boot(get_absolute_time());
    uartPrint("SD card test. Press 'enter' to start or 's' to skip.");

    while (true)
    {
        int c = getchar_timeout_us(0);
        if (c == PICO_ERROR_TIMEOUT)
        {
            if (to_ms_since_boot(get_absolute_time()) - startTime > TIMEOUT_MS)
            {
                uartPrint("User input timeout");
                return false;
            }
            continue;
        }
        if ((char)c == 's')
        {
            uartPrint("Skipping SD card test.\n");
            return false;
        }
        if ((char)c == '\r' || (char)c == '\n')
        {
            break;
        }
    }
    return true;
}

/**
 * @brief Initializes the SD card driver.
 * @param buf Buffer for user input.
 * @return True if initialized successfully, false if user skipped.
 */
bool initializeSDCard(char* buf)
{
    if (!sd_init_driver())
    {
        uartPrint("ERROR: Could not initialize SD card.");
        return false;
    }
    return true;
}

/**
 * @brief Mounts the drive on the filesystem.
 * @param fs FATFS object reference.
 * @param buf Buffer for user input.
 * @return True if mounted successfully, false if user skipped.
 */
// storage.cpp
bool mountDrive(FATFS& fs, char* buf)
{
    uint64_t t_start = to_ms_since_boot(get_absolute_time());
    FRESULT fr = f_mount(&fs, "SD", 1);
    uint64_t t_end = to_ms_since_boot(get_absolute_time());
    std::string message = "f_mount took " + std::to_string(t_end - t_start) + " ms";
    uartPrint(message);
    if (fr != FR_OK)
    {
        uartPrint("ERROR: Could not mount filesystem " + fresult_str(fr));
        return false;
    }
    return true;
}

/**
 * @brief Main function to test SD card operations: initialization, mount, write, read, and close.
 * @return True if the test completes successfully, otherwise false.
 */
bool testSDCard()
{
    const uint64_t TIMEOUT_MS = 5000;  // 5-second timeout
    FATFS fs;
    char buf[1024];
    char filename[] = "test02.txt";

    //Wait for user's decision
    if (!waitForUserInteraction(TIMEOUT_MS))
       return false;
    
    uint64_t t_start = to_ms_since_boot(get_absolute_time());
    std::string message = "Starting SD card test @ " + std::to_string(t_start);
    uartPrint(message);

    if (!initializeSDCard(buf))
        return false;
    uint64_t now = to_ms_since_boot(get_absolute_time());
    message = "SD card driver initialized at " + std::to_string(now) + " ms (elapsed " + std::to_string(now - t_start) + " ms)";
    uartPrint(message);

    if (!mountDrive(fs, buf))
        return false;
    now = to_ms_since_boot(get_absolute_time());
    message = "Filesystem mounted at " + std::to_string(now) + " ms (elapsed " + std::to_string(now - t_start) + " ms)";
    uartPrint(message);

    double voltageData = powerManager.getVoltageBattery();
    double currentData = powerManager.getCurrentChargeTotal();
    double drawData = powerManager.getCurrentDraw();

    std::string powerData = "Voltage: " + std::to_string(voltageData) + "V\n" +
                            "Current: " + std::to_string(currentData) + "mA\n" +
                            "Draw: " + std::to_string(drawData) + "mA\n";
    

    if (!writeToFile(filename, powerData.c_str()))
        return false;
    now = to_ms_since_boot(get_absolute_time());

    message = "Data written to file at " + std::to_string(now) + " ms (elapsed " + std::to_string(now - t_start) + " ms)";
    uartPrint(message);

    f_unmount("SD");
    uint64_t t_end = to_ms_since_boot(get_absolute_time());
    message = "SD card test completed at " + std::to_string(t_end) + " ms (total elapsed " + std::to_string(t_end - t_start) + " ms)";
    uartPrint(message);
    return true;
}