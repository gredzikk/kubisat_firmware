// event_manager.cpp

#include "event_manager.h"
#include "storage.h"
#include <cstdio>
#include "ff.h"
#include "commands.h"
// External reference to PowerManager
extern PowerManager powerManager;

// Current power event state
static PowerEventState lastPowerState = PowerEventState::NORMAL;

// Voltage sampling parameters
static constexpr int VOLTAGE_SAMPLES = 10;
static float voltageHistory[VOLTAGE_SAMPLES] = {0};
static int historyIndex = 0;

// Detection parameters
static constexpr float FALL_RATE_THRESHOLD = -0.02f; // Voltage drop per sample (e.g., -0.05V per 10ms)
static constexpr int FALLING_TREND_REQUIRED = 3;      // Number of consecutive drops required
static constexpr float VOLTAGE_LOW_THRESHOLD = 4.7f;
// Debounce parameters
static constexpr int DEBOUNCE_TIME_MS = 5000; // 5 seconds
static constexpr float VOLTAGE_OVERCHARGE_THRESHOLD = 5.3f; // Voltage overcharge threshold
static uint32_t lastEventTime = 0;

// Consecutive falling trend counter
static int fallingTrendCount = 0;

// Calculates the moving average of the latest N samples (optional)
float calculateMovingAverage() {
    float sum = 0.0f;
    for(int i = 0; i < 5; ++i) { // Reduced window for faster response
        int idx = (historyIndex + VOLTAGE_SAMPLES - 5 + i) % VOLTAGE_SAMPLES;
        sum += voltageHistory[idx];
    }
    return sum / 5.0f;
}

// Checks power events, including downward trend detection
void checkPowerEvents(PowerManager& pm)
{
    float currentVoltage = pm.getVoltage5V(); // Using 5V measurement as it stabilizes longer

    // Store the current voltage in the ring buffer
    voltageHistory[historyIndex] = currentVoltage;
    historyIndex = (historyIndex + 1) % VOLTAGE_SAMPLES;

    // Optional: Calculate moving average if needed
    float movingAverage = calculateMovingAverage();

    // Compare current voltage with the previous sample
    static float previousVoltage = 0.0f;
    float delta = currentVoltage - previousVoltage;
    previousVoltage = currentVoltage;

    // Update falling trend counter
    if (delta < FALL_RATE_THRESHOLD) {
        fallingTrendCount++;
    } else {
        fallingTrendCount = 0;
    }

    // Get the current time
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());

    // Debounce: Ensure enough time has passed since the last event
    bool debounce = (currentTime - lastEventTime) > DEBOUNCE_TIME_MS;

    // Detect rapid voltage falling trend
    if (fallingTrendCount >= FALLING_TREND_REQUIRED)
    {
        logEvent("Battery losing power rapidly");
        sendMessage("ALERT: POWER FALLING | Voltage: " + std::to_string(currentVoltage) + " V");
        lastPowerState = PowerEventState::POWER_FALLING;
        lastEventTime = currentTime;
    }

    // Detect low battery
    if (movingAverage < VOLTAGE_LOW_THRESHOLD && lastPowerState != PowerEventState::LOW_BATTERY && debounce)
    {
        logEvent("Battery below threshold");
        lastPowerState = PowerEventState::LOW_BATTERY;
        sendMessage("ALERT: POWER LOW | Voltage: " + std::to_string(movingAverage) + " V");
        lastEventTime = currentTime;
    }
    // Detect overcharge
    else if (movingAverage > VOLTAGE_OVERCHARGE_THRESHOLD && lastPowerState != PowerEventState::OVERCHARGE && debounce)
    {
        logEvent("Battery overcharge level");
        lastPowerState = PowerEventState::OVERCHARGE;
        sendMessage("ALERT: OVERCHARGE | Voltage: " + std::to_string(movingAverage) + " V");
        lastEventTime = currentTime;
    }
    // Detect normal state
    else if (movingAverage >= VOLTAGE_LOW_THRESHOLD && movingAverage <= VOLTAGE_OVERCHARGE_THRESHOLD && lastPowerState != PowerEventState::NORMAL && debounce)
    {
        logEvent("Battery back to normal");
        lastPowerState = PowerEventState::NORMAL;
        sendMessage("INFO: Battery back to normal | Voltage: " + std::to_string(movingAverage) + " V");
        lastEventTime = currentTime;
    }
}

void logEvent(const char* message)
{
    std::cout << message << std::endl;
    // FIL fil;
    // if (f_open(&fil, "event_log.txt", FA_OPEN_APPEND | FA_WRITE) == FR_OK)
    // {
    //     uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    //     f_printf(&fil, "[%lu ms] %s\n", currentTime, message);
    //     f_close(&fil);
    // }
}