#include "communication.h"
#include "lib/location/gps_collector.h"
#include <sstream> // Include for stringstream

/**
 * @defgroup GPSCommands GPS Commands
 * @brief Commands for controlling and monitoring the GPS module
 * @{
 */

/**
 * @brief Handler for controlling GPS module power state
 * @param param For SET: "0" to power off, "1" to power on. For GET: empty
 * @param operationType GET to read current state, SET to change state
 * @return Vector of Frames containing:
 *         - Success: Current power state (0/1)
 *          or
 *         - Error: Error reason
 * @note <b>KBST;0;GET;7;1;;TSBK</b>
 * @note Return current GPS module power state: ON/OFF
 * @note <b>KBST;0;SET;7;1;POWER;TSBK</b>
 * @note POWER - 0 - OFF, 1 - ON
 * @ingroup GPSCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 7.1
 */
std::vector<Frame> handle_gps_power_status(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;

    if (!(operationType == OperationType::GET || operationType == OperationType::SET)) {
        frames.push_back(frame_build(OperationType::ERR, 7, 1, "INVALID OPERATION"));
        return frames;
    }

    if (operationType == OperationType::SET) {
        if (param.empty()) {
            frames.push_back(frame_build(OperationType::ERR, 7, 1, "PARAM REQUIRED"));
            return frames;
        }

        try {
            int powerStatus = std::stoi(param);
            if (powerStatus != 0 && powerStatus != 1) {
                frames.push_back(frame_build(OperationType::ERR, 7, 1, "INVALID VALUE. USE 0 OR 1"));
                return frames;
            }
            gpio_put(GPS_POWER_ENABLE_PIN, powerStatus);
            EventEmitter::emit(EventGroup::GPS, powerStatus ? GPSEvent::POWER_ON : GPSEvent::POWER_OFF);
            frames.push_back(frame_build(OperationType::RES, 7, 1, std::to_string(powerStatus)));
            return frames;
        } catch (...) {
            frames.push_back(frame_build(OperationType::ERR, 7, 1, "INVALID PARAMETER FORMAT"));
            return frames;
        }
    }

    // GET operation
    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 7, 1, "PARAM UNNECESSARY"));
        return frames;
    }

    bool powerStatus = gpio_get(GPS_POWER_ENABLE_PIN);
    frames.push_back(frame_build(OperationType::VAL, 7, 1, std::to_string(powerStatus)));
    return frames;
}


/**
 * @brief Handler for enabling GPS transparent mode (UART pass-through)
 * @param param TIMEOUT in seconds (optional, defaults to 60)
 * @param operationType SET
 * @return Vector of Frames containing:
 *         - Success: Exit message + reason
 *          or
 *         - Error: Error reason
 * @note <b>KBST;0;SET;7;2;TIMEOUT;TSBK</b>
 * @note TIMEOUT - 1-600s, default 60s
 * @note Enters a pass-through mode where UART communication is bridged directly to GPS
 * @note Send "##EXIT##" to exit mode before TIMEOUT
 * @ingroup GPSCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 7.2
 */
std::vector<Frame> handle_enable_gps_uart_passthrough(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    // Validate operation type
    if (!(operationType == OperationType::SET)) {
        frames.push_back(frame_build(OperationType::ERR, 7, 2, "NOT ALLOWED"));
        return frames;
    }

    // Parse and validate timeout parameter
    uint32_t timeoutMs;
    try {
        timeoutMs = param.empty() ? 60000u : std::stoul(param) * 1000;
    } catch (...) {
        frames.push_back(frame_build(OperationType::ERR, 7, 2, "INVALID TIMEOUT FORMAT"));
        return frames;
    }

    // Setup UART parameters and exit sequence
    const std::string EXIT_SEQUENCE = "##EXIT##";
    std::string inputBuffer;
    bool exitRequested = false;
    uint32_t originalBaudRate = DEBUG_UART_BAUD_RATE;
    uint32_t gpsBaudRate = GPS_UART_BAUD_RATE;
    uint32_t startTime = to_ms_since_boot(get_absolute_time());

    // Log start of transparent mode
    EventEmitter::emit(EventGroup::GPS, GPSEvent::PASS_THROUGH_START);

    // Print startup message
    std::string message = "Entering GPS Serial Pass-Through Mode @" + 
                         std::to_string(gpsBaudRate) + " for " + 
                         std::to_string(timeoutMs/1000) + "s\r\n" +
                         "Send " + EXIT_SEQUENCE + " to exit";
    uart_print(message, VerbosityLevel::INFO);
    // Allow time for message to be sent before baudrate change
    sleep_ms(10);
    
    // Switch to GPS baudrate
    uart_set_baudrate(DEBUG_UART_PORT, gpsBaudRate);

    // Main transparent mode loop
    while (!exitRequested) {
        while (uart_is_readable(DEBUG_UART_PORT)) {
            char ch = uart_getc(DEBUG_UART_PORT);
            
            inputBuffer += ch;
            if (inputBuffer.length() > EXIT_SEQUENCE.length()) {
                inputBuffer = inputBuffer.substr(1);
            }
            
            if (inputBuffer == EXIT_SEQUENCE) {
                exitRequested = true;
                break;
            }
            
            if (inputBuffer != EXIT_SEQUENCE.substr(0, inputBuffer.length())) {
                uart_write_blocking(GPS_UART_PORT, 
                    reinterpret_cast<const uint8_t*>(&ch), 1);
            }
        }

        while (uart_is_readable(GPS_UART_PORT)) {
            char gpsByte = uart_getc(GPS_UART_PORT);
            uart_write_blocking(DEBUG_UART_PORT, 
                reinterpret_cast<const uint8_t*>(&gpsByte), 1);
        }

        if (to_ms_since_boot(get_absolute_time()) - startTime >= timeoutMs) {
            break;
        }
    }

    uart_set_baudrate(DEBUG_UART_PORT, originalBaudRate);
    
    sleep_ms(10);

    EventEmitter::emit(EventGroup::GPS, GPSEvent::PASS_THROUGH_END);
    
    std::string exitReason = exitRequested ? "USER_EXIT" : "TIMEOUT";
    std::string response = "GPS UART BRIDGE EXIT: " + exitReason;
    uart_print(response, VerbosityLevel::INFO);
    
    frames.push_back(frame_build(OperationType::RES, 7, 2, response));
    return frames;
}


/**
 * @brief Handler for retrieving GPS RMC (Recommended Minimum Navigation) data
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: Comma-separated RMC tokens
 *          or
 *         - Error: Error message
 * @note <b>KBST;0;GET;7;3;;TSBK</b>
 * @ingroup GPSCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 7.3
 */
std::vector<Frame> handle_get_rmc_data(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    if (operationType != OperationType::GET) {
        frames.push_back(frame_build(OperationType::ERR, 7, 3, "INVALID OPERATION"));
        return frames;
    }

    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 7, 3, "PARAM UNNECESSARY"));
        return frames;
    }

    std::vector<std::string> tokens = nmea_data.get_rmc_tokens();
    if (tokens.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 7, 3, "NO RMC DATA"));
        return frames;
    }

    // Join tokens with commas to create the response
    std::stringstream ss;
    for (size_t i = 0; i < tokens.size(); ++i) {
        ss << tokens[i];
        if (i < tokens.size() - 1) {
            ss << ",";
        }
    }

    frames.push_back(frame_build(OperationType::VAL, 7, 3, ss.str()));
    return frames;
}


/**
 * @brief Handler for retrieving GPS GGA (Global Positioning System Fix Data) data
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of Frames containing:
 *         - Success: Comma-separated GGA tokens
 *          or
 *         - Error: Error message
 * @note <b>KBST;0;GET;7;4;;TSBK</b>
 * @ingroup GPSCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 7.4
 */
std::vector<Frame> handle_get_gga_data(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    if (operationType != OperationType::GET) {
        frames.push_back(frame_build(OperationType::ERR, 7, 4, "INVALID OPERATION"));
        return frames;
    }

    if (!param.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 7, 4, "PARAM UNNECESSARY"));
        return frames;
    }

    std::vector<std::string> tokens = nmea_data.get_gga_tokens();
    if (tokens.empty()) {
        frames.push_back(frame_build(OperationType::ERR, 7, 4, "NO GGA DATA"));
        return frames;
    }

    // Join tokens with commas to create the response
    std::stringstream ss;
    for (size_t i = 0; i < tokens.size(); ++i) {
        ss << tokens[i];
        if (i < tokens.size() - 1) {
            ss << ",";
        }
    }

    frames.push_back(frame_build(OperationType::VAL, 7, 4, ss.str()));
    return frames;
}
/** @} */ // end of GPSCommands group