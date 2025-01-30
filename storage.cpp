#include "ff.h"
#include "sd_card.h"
#include "pin_config.h"
#include "stdio.h"
#include <pico/stdio.h>

bool testSDCard()
{
    const uint64_t TIMEOUT_MS = 5000; // 5-second timeout
    uint64_t startTime;
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char filename[] = "test02.txt";

    printf("\r\nSD card test. Press 'enter' to start or 's' to skip.\n");
    startTime = to_ms_since_boot(get_absolute_time());
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

    // Initialize SD card
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

    // Mount drive
    fr = f_mount(&fs, "0:", 1);
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

    // Open file for writing
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
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

    // Write something to file
    ret = f_printf(&fil, "This is another test\n");
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
    ret = f_printf(&fil, "of writing to an SD card.\n");
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

    // Close file
    fr = f_close(&fil);
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

    // Open file for reading
    fr = f_open(&fil, filename, FA_READ);
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

    // Print every line in file over serial
    printf("Reading from file '%s':\n", filename);
    printf("---\n");
    while (f_gets(buf, sizeof(buf), &fil))
    {
        printf("%s", buf);
    }
    printf("\n---\n");

    // Close file
    fr = f_close(&fil);
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

    // Unmount drive
    f_unmount("0:");
    printf("SD card test completed successfully.\n");
    return true;
}