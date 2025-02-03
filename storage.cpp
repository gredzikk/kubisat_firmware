#include "ff.h"
#include "sd_card.h"
#include "pin_config.h"
#include "stdio.h"
#include <pico/stdio.h>
#include "PowerManager.h"

extern PowerManager powerManager;

/**
 * @brief Waits for user input (enter to proceed or 's' to skip) with timeout.
 * @param TIMEOUT_MS Maximum time (in ms) to wait for user input.
 * @return True if proceed, false if skipping.
 */
static bool waitForUserInteraction(const uint64_t TIMEOUT_MS)
{
    uint64_t startTime = to_ms_since_boot(get_absolute_time());
    printf("\r\nSD card test. Press 'enter' to start or 's' to skip.\n");

    while (true)
    {
        int c = getchar_timeout_us(0);
        if (c == PICO_ERROR_TIMEOUT)
        {
            if (to_ms_since_boot(get_absolute_time()) - startTime > TIMEOUT_MS)
            {
                printf("No input within %llu ms. Skipping SD card test.\n", TIMEOUT_MS);
                return false;
            }
            continue;
        }
        if ((char)c == 's')
        {
            printf("Skipping SD card test.\n");
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
static bool initializeSDCard(char* buf)
{
    if (!sd_init_driver())
    {
        printf("ERROR: Could not initialize SD card. Press 's' to skip.\n");
        while (true)
        {
            buf[0] = getchar();
            if (buf[0] == 's')
            {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Mounts the drive on the filesystem.
 * @param fs FATFS object reference.
 * @param buf Buffer for user input.
 * @return True if mounted successfully, false if user skipped.
 */
static bool mountDrive(FATFS& fs, char* buf)
{
    FRESULT fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not mount filesystem (%d). Press 's' to skip.\n", fr);
        while (true)
        {
            buf[0] = getchar();
            if (buf[0] == 's')
            {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Opens a file on the SD card.
 * @param fil FIL object reference.
 * @param filename Name of the file to open.
 * @param mode File access mode.
 * @param buf Buffer for user input.
 * @return True if successful, false if user skipped or error occurred.
 */
static bool openFile(FIL& fil, const char* filename, BYTE mode, char* buf)
{
    FRESULT fr = f_open(&fil, filename, mode);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not open file (%d). Press 's' to skip.\n", fr);
        while (true)
        {
            buf[0] = getchar();
            if (buf[0] == 's')
            {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Writes data to an open file.
 * @param fil FIL object reference.
 * @param data Data string to write.
 * @param buf Buffer for user input.
 * @return True if successful, false if user skipped or error occurred.
 */
static bool writeToFile(FIL& fil, const char* data, char* buf)
{
    int ret = f_printf(&fil, data);
    if (ret < 0)
    {
        printf("ERROR: Could not write to file (%d). Press 's' to skip.\n", ret);
        f_close(&fil);
        while (true)
        {
            buf[0] = getchar();
            if (buf[0] == 's')
            {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Closes an open file.
 * @param fil FIL object reference.
 * @param buf Buffer for user input.
 * @return True if successful, false if user skipped or error occurred.
 */
static bool closeFile(FIL& fil, char* buf)
{
    FRESULT fr = f_close(&fil);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not close file (%d). Press 's' to skip.\n", fr);
        while (true)
        {
            buf[0] = getchar();
            if (buf[0] == 's')
            {
                printf("Skipping SD card test.\n");
                return false;
            }
        }
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
    FIL fil;
    char buf[1024];
    char filename[] = "test02.txt";

    // Wait for user's decision
    //if (!waitForUserInteraction(TIMEOUT_MS))
       // return false;
    
    uint64_t t_start = to_ms_since_boot(get_absolute_time());
    printf("Starting SD card test at %llu ms\n", t_start);

    if (!initializeSDCard(buf))
        return false;
    uint64_t now = to_ms_since_boot(get_absolute_time());
    printf("SD card driver initialized at %llu ms (elapsed %llu ms)\n", now, now - t_start);

    if (!mountDrive(fs, buf))
        return false;
    now = to_ms_since_boot(get_absolute_time());
    printf("Filesystem mounted at %llu ms (elapsed %llu ms)\n", now, now - t_start);

    if (!openFile(fil, filename, FA_WRITE | FA_CREATE_ALWAYS, buf))
        return false;
    now = to_ms_since_boot(get_absolute_time());
    printf("File opened for writing at %llu ms (elapsed %llu ms)\n", now, now - t_start);

    double voltageData = powerManager.getVoltageBattery();
    double currentData = powerManager.getCurrentChargeTotal();
    double drawData = powerManager.getCurrentDraw();

    std::string powerData = "Voltage: " + std::to_string(voltageData) + "V\n" +
                            "Current: " + std::to_string(currentData) + "mA\n" +
                            "Draw: " + std::to_string(drawData) + "mA\n";
    

    if (!writeToFile(fil, powerData.c_str(), buf))
        return false;
    now = to_ms_since_boot(get_absolute_time());

    printf("Write complete at %llu ms (elapsed %llu ms)\n", now, now - t_start);


    if (!closeFile(fil, buf))
        return false;
    now = to_ms_since_boot(get_absolute_time());
    printf("File closed after writing at %llu ms (elapsed %llu ms)\n", now, now - t_start);

    f_unmount("0:");
    uint64_t t_end = to_ms_since_boot(get_absolute_time());
    printf("SD card test completed successfully at %llu ms (total elapsed: %llu ms)\n",
           t_end, t_end - t_start);
    return true;
}