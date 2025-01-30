#include "PowerManager.h"

enum class PowerEventState
{
    NORMAL,
    LOW_BATTERY,
    OVERCHARGE,
    POWER_FALLING
};

void checkPowerEvents(PowerManager& pm);
void logEvent(const char* message);
