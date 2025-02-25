#include "includes.h"

#define LOG_FILENAME "/log.txt"

PowerManager powerManager(MAIN_I2C_PORT);
DS3231 systemClock(MAIN_I2C_PORT);
volatile bool g_pending_bootloader_reset = false;

char buffer[BUFFER_SIZE];
int bufferIndex = 0;


void process_pending_actions() {    
    if (g_pending_bootloader_reset) {
        sleep_ms(100);
        uart_print("Entering BOOTSEL mode...", VerbosityLevel::WARNING);
        reset_usb_boot(0, 0);
    }
}

void core1_entry() {
    uart_print("Starting core 1", VerbosityLevel::DEBUG);
    while (true) {
        collect_gps_data();
        check_power_events(powerManager); 
        process_pending_actions();
        sleep_ms(10);
    }
}

bool init_systems() {
    stdio_init_all();

    uart_init(DEBUG_UART_PORT, DEBUG_UART_BAUD_RATE);
    gpio_set_function(DEBUG_UART_TX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_TX_PIN));
    gpio_set_function(DEBUG_UART_RX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_RX_PIN));

    uart_init(GPS_UART_PORT, GPS_UART_BAUD_RATE);
    gpio_set_function(GPS_UART_TX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_TX_PIN));
    gpio_set_function(GPS_UART_RX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_RX_PIN));

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    i2c_init(MAIN_I2C_PORT, 400 * 1000);
    gpio_set_function(MAIN_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAIN_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MAIN_I2C_SCL_PIN);
    gpio_pull_up(MAIN_I2C_SDA_PIN);

    if (true)
    {
        gpio_init(GPS_POWER_ENABLE_PIN);
        gpio_set_dir(GPS_POWER_ENABLE_PIN, GPIO_OUT);
        gpio_put(GPS_POWER_ENABLE_PIN, 1); 
    }
    system("color");

    bool radioInitSuccess = false;
    radioInitSuccess = initialize_radio();
    
    bool sdInitDone = fs_init();
    if (sdInitDone) {
        FILE *fp = fopen(LOG_FILENAME, "w");
        if (fp) {
            uart_print("Log file opened.", VerbosityLevel::DEBUG);
            int bytesWritten = fprintf(fp, "System init started.\n");
            uart_print("Written " + std::to_string(bytesWritten) + " bytes.", VerbosityLevel::DEBUG);
            int closeStatus = fclose(fp);
            uart_print("Close file status: " + std::to_string(closeStatus), VerbosityLevel::DEBUG);

            struct stat file_stat;
            if (stat(LOG_FILENAME, &file_stat) == 0) {
                size_t fileSize = file_stat.st_size;
                uart_print("File size: " + std::to_string(fileSize) + " bytes", VerbosityLevel::DEBUG);
            } else {
                uart_print("Failed to get file size", VerbosityLevel::ERROR);
            }

            uart_print("File path: /" + std::string(LOG_FILENAME), VerbosityLevel::DEBUG);
        } else {
            uart_print("Failed to open log file for writing.", VerbosityLevel::ERROR);
        }
    }

    if (sdInitDone) {
        uart_print("SD card init: OK", VerbosityLevel::DEBUG);
    } else {
        uart_print("SD card init: FAILED", VerbosityLevel::ERROR);
    }

    if (radioInitSuccess) {
        uart_print("Radio init: OK", VerbosityLevel::DEBUG);
    } else {
        uart_print("Radio init: FAILED", VerbosityLevel::ERROR);
    }

    Frame boot = frame_build(OperationType::RES, 0, 0, "HELLO");
    send_frame(boot);

    return radioInitSuccess;
}


int main()
{
    init_systems();
    multicore_launch_core1(core1_entry);

    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    bool powerManagerInitStatus = powerManager.initialize();
    if (powerManagerInitStatus)
    {
        std::map<std::string, std::string> powerConfig = {
            {"operating_mode", "continuous"},
            {"averaging_mode", "16"},
        };
        powerManager.configure(powerConfig);
    } else {
        uart_print("Power manager init error", VerbosityLevel::ERROR);
    }
    
    Frame boot = frame_build(OperationType::RES, 0, 0, "START");
    send_frame(boot);
    
    std::string bootString = "System init completed @ " + std::to_string(to_ms_since_boot(get_absolute_time())) + " ms";
    uart_print(bootString, VerbosityLevel::WARNING);

    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    while (true)
    {
        int packetSize = LoRa.parse_packet();
        if (packetSize)
        {
            on_receive(packetSize);
        }

        handle_uart_input();
    }

    return 0;
}