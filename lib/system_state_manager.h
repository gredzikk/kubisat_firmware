#ifndef SYSTEM_STATE_MANAGER_H
#define SYSTEM_STATE_MANAGER_H

#include <mutex>
#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"

class SystemStateManager {
    private:
        // Private states
        bool pending_bootloader_reset;
        bool gps_collection_paused;
        bool sd_card_mounted;
        VerbosityLevel uart_verbosity;
        recursive_mutex_t mutex_;
    
        SystemStateManager() :
            pending_bootloader_reset(false),
            gps_collection_paused(false),
            sd_card_mounted(false),
            uart_verbosity(VerbosityLevel::DEBUG)
        {
            recursive_mutex_init(&mutex_);
        }
    
        SystemStateManager(const SystemStateManager&) = delete;
        SystemStateManager& operator=(const SystemStateManager&) = delete;
    
    public:
        static SystemStateManager& get_instance() {
            static SystemStateManager instance;
            return instance;
        }
    
        bool is_bootloader_reset_pending() const {
            recursive_mutex_enter_blocking(const_cast<recursive_mutex_t*>(&mutex_));
            bool result = pending_bootloader_reset;
            recursive_mutex_exit(const_cast<recursive_mutex_t*>(&mutex_));
            return result;
        }
    
        void set_bootloader_reset_pending(bool pending) {
            recursive_mutex_enter_blocking(&mutex_);
            pending_bootloader_reset = pending;
            recursive_mutex_exit(&mutex_);
        }
    
        bool is_gps_collection_paused() const {
            recursive_mutex_enter_blocking(const_cast<recursive_mutex_t*>(&mutex_));
            bool result = gps_collection_paused;
            recursive_mutex_exit(const_cast<recursive_mutex_t*>(&mutex_));
            return result;
        }
    
        void set_gps_collection_paused(bool paused) {
            recursive_mutex_enter_blocking(&mutex_);
            gps_collection_paused = paused;
            recursive_mutex_exit(&mutex_);
        }
    
        bool is_sd_card_mounted() const {
            recursive_mutex_enter_blocking(const_cast<recursive_mutex_t*>(&mutex_));
            bool result = sd_card_mounted;
            recursive_mutex_exit(const_cast<recursive_mutex_t*>(&mutex_));
            return result;
        }
    
        void set_sd_card_mounted(bool mounted) {
            recursive_mutex_enter_blocking(&mutex_);
            sd_card_mounted = mounted;
            recursive_mutex_exit(&mutex_);
        }
    
        VerbosityLevel get_uart_verbosity() const {
            recursive_mutex_enter_blocking(const_cast<recursive_mutex_t*>(&mutex_));
            VerbosityLevel result = uart_verbosity;
            recursive_mutex_exit(const_cast<recursive_mutex_t*>(&mutex_));
            return result;
        }
    
        void set_uart_verbosity(VerbosityLevel level) {
            recursive_mutex_enter_blocking(&mutex_);
            uart_verbosity = level;
            recursive_mutex_exit(&mutex_);
        }
    };
    
#endif // SYSTEM_STATE_MANAGER_H