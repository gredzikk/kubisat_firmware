#include "ff.h"
#include "sd_card.h"
#include "pin_config.h"
#include "stdio.h"
#include <pico/stdio.h>

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
    char buf[100];
    char filename[] = "test02.txt";

    // Wait for user's decision
    if (!waitForUserInteraction(TIMEOUT_MS))
        return false;

    // Initialize SD card driver
    if (!initializeSDCard(buf))
        return false;

    // Mount filesystem
    if (!mountDrive(fs, buf))
        return false;

    // Write test
    if (!openFile(fil, filename, FA_WRITE | FA_CREATE_ALWAYS, buf))
        return false;
    if (!writeToFile(fil, "This is another test\n", buf))
        return false;
    if (!writeToFile(fil, "of writing to an SD card.\n", buf))
        return false;
    if (!closeFile(fil, buf))
        return false;

    // Read test
    if (!openFile(fil, filename, FA_READ, buf))
        return false;

    printf("Reading from file '%s':\n", filename);
    printf("---\n");
    while (f_gets(buf, sizeof(buf), &fil))
    {
        printf("%s", buf);
    }
    printf("\n---\n");

    // Close and unmount
    if (!closeFile(fil, buf))
        return false;

    f_unmount("0:");
    printf("SD card test completed successfully.\n");
    return true;
}