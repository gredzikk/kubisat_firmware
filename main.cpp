#include "includes.h"

#define LOG_FILENAME "/log.txt"

PowerManager powerManager(MAIN_I2C_PORT);
DS3231 systemClock(MAIN_I2C_PORT);

char buffer[BUFFER_SIZE] = {0};
int buffer_index = 0;

void core1_entry() {
    uart_print("Starting core 1", VerbosityLevel::DEBUG);
    EventEmitter::emit(EventGroup::SYSTEM, SystemEvent::CORE1_START);
    
    uint32_t last_clock_check_time = 0;
    uint32_t last_telemetry_time = 0;
    uint32_t telemetry_collection_counter = 0;

    telemetry_init();

    while (true) {
        collect_gps_data();
        
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        
        uint32_t check_interval_ms = systemClock.get_clock_sync_interval() * 60000;
        if (currentTime - last_clock_check_time >= check_interval_ms) {
            last_clock_check_time = currentTime;
            
            if (systemClock.is_sync_needed()) {
                uart_print("Clock sync interval reached, attempting sync", VerbosityLevel::INFO);
                systemClock.sync_clock_with_gps();
            }
        }
        
        if (is_telemetry_collection_time(currentTime, last_telemetry_time)) {
            collect_telemetry();
            telemetry_collection_counter++;
            
            if (is_telemetry_flush_time(telemetry_collection_counter)) {
                flush_telemetry();
                flush_sensor_data();
            }
        }

        if (SystemStateManager::get_instance().is_bootloader_reset_pending()) {
            sleep_ms(100);
            uart_print("Entering BOOTSEL mode...", VerbosityLevel::WARNING);
            reset_usb_boot(0, 0);
        }
        
        sleep_ms(10);
    }
}

bool init_pico_hw() {
    stdio_init_all();

    uart_init(DEBUG_UART_PORT, DEBUG_UART_BAUD_RATE);
    gpio_set_function(DEBUG_UART_TX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_TX_PIN));
    gpio_set_function(DEBUG_UART_RX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_RX_PIN));

    uart_init(GPS_UART_PORT, GPS_UART_BAUD_RATE);
    gpio_set_function(GPS_UART_TX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_TX_PIN));
    gpio_set_function(GPS_UART_RX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_RX_PIN));

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    
    i2c_init(MAIN_I2C_PORT, 400 * 1000);
    gpio_set_function(MAIN_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAIN_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MAIN_I2C_SCL_PIN);
    gpio_pull_up(MAIN_I2C_SDA_PIN);

    gpio_init(GPS_POWER_ENABLE_PIN);
    gpio_set_dir(GPS_POWER_ENABLE_PIN, GPIO_OUT);
    gpio_put(GPS_POWER_ENABLE_PIN, 1); 

    SystemStateManager::get_instance();

    EventEmitter::emit(EventGroup::GPS, GPSEvent::POWER_ON);
    
    system("color");

    return true;
}

bool init_modules(){
    bool radio_init_status = false;
    radio_init_status = initialize_radio();
    
    bool sd_init_status = fs_init();
    if (sd_init_status) {
        FILE *fp = fopen(LOG_FILENAME, "w");
        if (fp) {
            uart_print("Log file opened.", VerbosityLevel::DEBUG);
            int bytes_written = fprintf(fp, "System init started.\n");
            uart_print("Written " + std::to_string(bytes_written) + " bytes.", VerbosityLevel::DEBUG);
            int close_status = fclose(fp);
            uart_print("Close file status: " + std::to_string(close_status), VerbosityLevel::DEBUG);

            struct stat file_stat;
            if (stat(LOG_FILENAME, &file_stat) == 0) {
                size_t file_size = file_stat.st_size;
                uart_print("File size: " + std::to_string(file_size) + " bytes", VerbosityLevel::DEBUG);
            } else {
                uart_print("Failed to get file size", VerbosityLevel::ERROR);
            }

            uart_print("File path: /" + std::string(LOG_FILENAME), VerbosityLevel::DEBUG);
        } else {
            uart_print("Failed to open log file for writing.", VerbosityLevel::ERROR);
        }
    }

    if (sd_init_status) {
        uart_print("SD card init: OK", VerbosityLevel::DEBUG);
    } else {
        uart_print("SD card init: FAILED", VerbosityLevel::ERROR);
    }

    if (radio_init_status) {
        uart_print("Radio init: OK", VerbosityLevel::DEBUG);
    } else {
        uart_print("Radio init: FAILED", VerbosityLevel::ERROR);
    }

    Frame boot = frame_build(OperationType::RES, 0, 0, "HELLO");
    send_frame_lora(boot);

    // uart_print("Initializing sensors...", VerbosityLevel::DEBUG);
    // SensorWrapper& sensor_wrapper = SensorWrapper::get_instance();
    // bool light_sensor_init = sensor_wrapper.sensor_init(SensorType::LIGHT, MAIN_I2C_PORT);
    // bool env_sensor_init = sensor_wrapper.sensor_init(SensorType::ENVIRONMENT, MAIN_I2C_PORT);
    // bool mag_sensor_init = sensor_wrapper.sensor_init(SensorType::MAGNETOMETER, MAIN_I2C_PORT);

    // if (!light_sensor_init || !env_sensor_init || !mag_sensor_init) {
    //     uart_print("One or more sensors failed to initialize", VerbosityLevel::WARNING);
    // }

    return sd_init_status && radio_init_status;
}

int main()
{
    init_pico_hw();
    sleep_ms(100);
    init_modules();
    EventEmitter::emit(EventGroup::SYSTEM, SystemEvent::BOOT);
    sleep_ms(100);
    multicore_launch_core1(core1_entry);

    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    bool power_manager_init_status = powerManager.initialize();
    if (power_manager_init_status)
    {
        std::map<std::string, std::string> power_config = {
            {"operating_mode", "continuous"},
            {"averaging_mode", "16"},
        };
        powerManager.configure(power_config);
    } else {
        uart_print("Power manager init error", VerbosityLevel::ERROR);
    }
    
    Frame boot = frame_build(OperationType::RES, 0, 0, "START");
    send_frame_lora(boot);
    
    std::string boot_string = "System init completed @ " + std::to_string(to_ms_since_boot(get_absolute_time())) + " ms";
    uart_print(boot_string, VerbosityLevel::WARNING);

    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    while (true)
    {
        int packet_size = LoRa.parse_packet();
        if (packet_size)
        {
            on_receive(packet_size);
        }

        handle_uart_input();
    }

    return 0;
}