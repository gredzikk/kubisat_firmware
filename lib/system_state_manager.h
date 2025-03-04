/**
 * @file system_state_manager.h
 * @brief Declaration of the SystemStateManager singleton class for managing global system states
 * @details The SystemStateManager provides a thread-safe way to manage system-wide states
 *          such as bootloader reset flags, GPS collection status, SD card mount status,
 *          and UART verbosity levels. It uses a singleton pattern to ensure a single
 *          instance across the system with controlled access through mutex protection.
 */

 #ifndef SYSTEM_STATE_MANAGER_H
 #define SYSTEM_STATE_MANAGER_H
 
 #include <mutex>
 #include "utils.h"
 #include "pico/multicore.h"
 #include "pico/sync.h"
 
 /**
  * @class SystemStateManager
  * @brief Singleton class for managing global system states
  * @details This class provides a centralized, thread-safe way to manage system-wide
  *          state variables instead of using global variables. It ensures proper
  *          initialization and thread-safe access to system states across both cores.
  */
 class SystemStateManager {
 private:
     /** @brief Pointer to the singleton instance */
     static SystemStateManager* instance;
     
     /** @brief Mutex for thread-safe access to the singleton instance */
     static mutex_t instance_mutex;
     
     // Private states
     bool pending_bootloader_reset;  /**< Flag indicating if a bootloader reset is pending */
     bool gps_collection_paused;     /**< Flag indicating if GPS data collection is paused */
     bool sd_card_mounted;           /**< Flag indicating if the SD card is currently mounted */
     VerbosityLevel uart_verbosity;  /**< Current verbosity level for UART logging */
     
     /**
      * @brief Private constructor for singleton pattern
      * @details Initializes all state variables to their default values
      */
     SystemStateManager() : 
         pending_bootloader_reset(false),
         gps_collection_paused(false),
         sd_card_mounted(false),
         uart_verbosity(VerbosityLevel::DEBUG) {}
 
 public:
     /**
      * @brief Get the singleton instance of SystemStateManager
      * @return Reference to the singleton instance
      * @details This method ensures thread-safe access to the singleton instance using a mutex.
      *          It initializes the mutex on first call and creates the instance if it doesn't exist.
      */
     static SystemStateManager& get_instance();
     
     /**
      * @brief Check if a bootloader reset is pending
      * @return True if bootloader reset is pending, false otherwise
      */
     bool is_bootloader_reset_pending() const;
     
     /**
      * @brief Set the bootloader reset pending state
      * @param pending The new state to set (true = reset pending)
      */
     void set_bootloader_reset_pending(bool pending);
     
     /**
      * @brief Check if GPS data collection is paused
      * @return True if GPS collection is paused, false otherwise
      */
     bool is_gps_collection_paused() const;
     
     /**
      * @brief Set the GPS collection paused state
      * @param paused The new state to set (true = collection paused)
      */
     void set_gps_collection_paused(bool paused);
     
     /**
      * @brief Check if the SD card is currently mounted
      * @return True if the SD card is mounted, false otherwise
      */
     bool is_sd_card_mounted() const;
     
     /**
      * @brief Set the SD card mounted state
      * @param mounted The new state to set (true = SD card mounted)
      */
     void set_sd_card_mounted(bool mounted);
 
     /**
      * @brief Get the current UART verbosity level
      * @return The current verbosity level
      */
     VerbosityLevel get_uart_verbosity() const;
     
     /**
      * @brief Set the UART verbosity level
      * @param level The new verbosity level to set
      */
     void set_uart_verbosity(VerbosityLevel level);
     
     // Delete copy constructor and assignment operator to enforce singleton pattern
     SystemStateManager(const SystemStateManager&) = delete;
     SystemStateManager& operator=(const SystemStateManager&) = delete;
 };
 
 #endif // SYSTEM_STATE_MANAGER_H