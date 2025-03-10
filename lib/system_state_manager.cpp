#include "system_state_manager.h"
#include <mutex>
#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"

// No need for static initialization here