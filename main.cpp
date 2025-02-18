#include "includes.h"

#define LOG_FILENAME "log.txt"

PowerManager powerManager(MAIN_I2C_PORT);
DS3231 systemClock(MAIN_I2C_PORT);

char buffer[BUFFER_SIZE];
int bufferIndex = 0;

void core1_entry() {
    while (true) {
        collectGPSData();
        checkPowerEvents(powerManager);
    }
}

bool initSystems() {
    stdio_init_all();

    uart_init(DEBUG_UART_PORT, DEBUG_UART_BAUD_RATE);
    gpio_set_function(DEBUG_UART_TX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_TX_PIN));
    gpio_set_function(DEBUG_UART_RX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_RX_PIN));

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    i2c_init(MAIN_I2C_PORT, 400 * 1000);
    gpio_set_function(MAIN_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAIN_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MAIN_I2C_SCL_PIN);
    gpio_pull_up(MAIN_I2C_SDA_PIN);

    uart_init(GPS_UART_PORT, GPS_UART_BAUD_RATE);
    gpio_set_function(GPS_UART_TX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_TX_PIN));
    gpio_set_function(GPS_UART_RX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_RX_PIN));

    if (true)
    {
        gpio_init(GPS_POWER_ENABLE_PIN);
        gpio_set_dir(GPS_POWER_ENABLE_PIN, GPIO_OUT);
        gpio_put(GPS_POWER_ENABLE_PIN, 1); 
    }
    
    bool sdInitDone = fs_init();
    uartPrint("SD card init: " + std::to_string(sdInitDone));
    std::string bootString = "System init completed @ " + std::to_string(to_ms_since_boot(get_absolute_time())) + " ms";
    uartPrint(bootString);

    Frame boot = buildFrame(ExecutionResult::INFO, 0, 0, "HELLO");
    sendFrame(boot);

    return true;
}

void loggingRoutine();

int main()
{
    initSystems();
    multicore_launch_core1(core1_entry);

    systemClock.setTime(0, 41, 20, 4, 14, 11, 2024);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    if (powerManager.initialize())
    {
        std::map<std::string, std::string> powerConfig = {
            {"operating_mode", "continuous"},
            {"averaging_mode", "16"},
        };
        powerManager.configure(powerConfig);
    }
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    bool radioInitSuccess = false;

    radioInitSuccess = initializeRadio();
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    if (radioInitSuccess)
    {
        sendMessage("System initialized successfully!");    
    }
    
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    uartPrint("This message will only be printed to UART.");
    // uartPrint("This message will be printed to UART and logged to Core 1.", true);

    for (int i = 5; i > 0; --i)
    {
        std::string intro = "Main loop starts in " + std::to_string(i) + " seconds...";
        uartPrint(intro);
        gpio_put(PICO_DEFAULT_LED_PIN, (i%2==0));
        sleep_ms(1000);
    }
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    Frame boot = buildFrame(ExecutionResult::INFO, 0, 0, "START");
    sendFrame(boot);

    while (true)
    {
        int packetSize = LoRa.parsePacket();
        if (packetSize)
        {
            onReceive(packetSize);
        }

        //acquireLock();
        //float voltage = sharedData.voltage5V;
        //releaseLock();

        //std::string voltageReading = "Core 0: voltage from common data structure: " + std::to_string(voltage);
        //uartPrint(voltageReading.c_str());

        //collectGPSData();
        handleUartInput();
    }

    return 0;
}

// BH1750    0X23
// INA3221   0X40
// BME280    0X76
// DS3231    0X86