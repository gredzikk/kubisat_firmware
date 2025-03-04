/**
 * @file system_state_manager.cpp
 * @brief Implementation of the SystemStateManager singleton class
 * @details Provides thread-safe implementation of system state management functions
 */

#include "system_state_manager.h"

// Initialize static members
SystemStateManager* SystemStateManager::instance = nullptr;
mutex_t SystemStateManager::instance_mutex;

SystemStateManager& SystemStateManager::get_instance() {
    // Initialize mutex once
    static bool mutex_initialized = false;
    if (!mutex_initialized) {
        mutex_init(&instance_mutex);
        mutex_initialized = true;
    }

    // Thread-safe singleton access
    mutex_enter_blocking(&instance_mutex);
    if (instance == nullptr) {
        instance = new SystemStateManager();
    }
    mutex_exit(&instance_mutex);
    return *instance;
}

bool SystemStateManager::is_bootloader_reset_pending() const {
    return pending_bootloader_reset;
}

void SystemStateManager::set_bootloader_reset_pending(bool pending) {
    pending_bootloader_reset = pending;
}

bool SystemStateManager::is_gps_collection_paused() const {
    return gps_collection_paused;
}

void SystemStateManager::set_gps_collection_paused(bool paused) {
    gps_collection_paused = paused;
}

bool SystemStateManager::is_sd_card_mounted() const {
    return sd_card_mounted;
}

void SystemStateManager::set_sd_card_mounted(bool mounted) {
    sd_card_mounted = mounted;
}

VerbosityLevel SystemStateManager::get_uart_verbosity() const {
    return uart_verbosity;
}

void SystemStateManager::set_uart_verbosity(VerbosityLevel level) {
    uart_verbosity = level;
}