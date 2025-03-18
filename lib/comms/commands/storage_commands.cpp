#include "commands.h"
#include "communication.h"
#include "storage.h"
#include "filesystem/vfs.h"
#include "filesystem/littlefs.h"
#include <sys/stat.h>
#include <errno.h>
#include "dirent.h"

static constexpr uint8_t storage_commands_group_id = 6;
static constexpr uint8_t list_files_command_id = 0;
static constexpr uint8_t mount_command_id = 4;

/**
 * @defgroup StorageCommands Storage Commands
 * @brief Commands for interacting with the SD card storage.
 * @{
 */

/**
 * @brief Handles the list files command.
 *
 * This function lists the files in the root directory of the SD card and sends
 * the filename and size of each file to the ground station.
 *
 * @param param Unused.
 * @param operationType The operation type (must be GET).
 * @return A vector of Frames indicating the result of the operation.
 *         - Success: Frame with "File listing complete" message.
 *         - Error: Frame with error message (e.g., "Could not open directory").
 *
 * @note <b>KBST;0;GET;6;0;;TSBK</b>
 * @note This command lists the files and their sizes in the root directory of the SD card.
 * @ingroup StorageCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 6.0
 */
std::vector<Frame> handle_list_files([[maybe_unused]] const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET) {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, list_files_command_id, error_msg));
        return frames;
    }

    DIR* dir;
    struct dirent* ent;
    int file_count = 0; 
    if ((dir = opendir("/")) != nullptr) {
        // First, count the number of files
        while ((ent = readdir(dir)) != nullptr) {
            const char* filename = ent->d_name;
            if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
                continue;
            }
            file_count++;
        }
        closedir(dir);

        // Send the number of files
        frames.push_back(frame_build(OperationType::VAL, storage_commands_group_id, list_files_command_id, std::to_string(file_count)));

        // Open the directory again to read file information
        dir = opendir("/");
        if (dir != nullptr) {
            while ((ent = readdir(dir)) != nullptr) {
                const char* filename = ent->d_name;

                // Skip "." and ".." directories
                if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
                    continue;
                }

                // Get file size
                std::array<char, 256> filepath;
                int written = snprintf(filepath.data(), filepath.size(), "/%s", filename);
                if (written < 0 || written >= static_cast<int>(sizeof(filepath))) {
                    continue; // Skip this file if path is too long
                }

                FILE* file = fopen(filepath.data(), "rb");
                size_t file_size = 0;

                if (file != nullptr)
                {
                    fseek(file, 0, SEEK_END);
                    file_size = ftell(file);
                    fclose(file);
                }

                // Create and send frame with filename and size
                std::array<char, 512> file_info;
                snprintf(file_info.data(), file_info.size(), "%s:%zu", filename, file_size);
                frames.push_back(frame_build(OperationType::SEQ, storage_commands_group_id, list_files_command_id, file_info.data()));
            }
            closedir(dir);
            frames.push_back(frame_build(OperationType::VAL, storage_commands_group_id, list_files_command_id, "SEQ_DONE"));
            return frames;
        } else {
            error_msg = error_code_to_string(ErrorCode::INTERNAL_FAIL_TO_READ);
            frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, list_files_command_id, error_msg));
            return frames;
        }
    } else {
        frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, list_files_command_id, error_msg));
        return frames;
    }
}

/**
 * @brief Handles the SD card mount/unmount command.
 *
 * This function mounts or unmounts the SD card.
 *
 * @param param "0" to unmount, "1" to mount.
 * @param operationType The operation type (must be SET).
 * @return A vector of Frames indicating the result of the operation.
 *         - Success: Frame with "SD card mounted" or "SD card unmounted" message.
 *         - Error: Frame with error message (e.g., "Invalid parameter", "Mount failed", "Unmount failed").
 *
 * @note <b>KBST;0;SET;6;4;[0|1];TSBK</b>
 * @note Example: <b>KBST;0;SET;6;4;1;TSBK</b> - Mounts the SD card.
 * @ingroup StorageCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 6.4
 */
std::vector<Frame> handle_mount(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType == OperationType::GET) { 
        //get always allowed
        bool state = SystemStateManager::get_instance().is_sd_card_mounted();

        frames.push_back(frame_build(OperationType::VAL, storage_commands_group_id, mount_command_id, std::to_string(state)));
        return frames;
    } else if (operationType == OperationType::SET) { 
        //set allowed only in ground mode
        SystemOperatingMode mode = SystemStateManager::get_instance().get_operating_mode();
        if (mode == SystemOperatingMode::BATTERY_POWERED) {
            error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
            frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, mount_command_id, error_msg));
            return frames;
        }

        if (param == "1") {
            if (fs_init()) {
                frames.push_back(frame_build(OperationType::RES, storage_commands_group_id, mount_command_id, "SD_MOUNT_OK"));
                return frames;
            } else {
                error_msg = error_code_to_string(ErrorCode::FAIL_TO_SET);
                frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, mount_command_id, error_msg));
                return frames;
            }
        } else if (param == "0") {
            if (fs_unmount("/") == 0) {
                if (SystemStateManager::get_instance().is_sd_card_mounted()) {
                    frames.push_back(frame_build(OperationType::RES, storage_commands_group_id, mount_command_id, "SD_UNMOUNT_OK"));
                }
                return frames;
            } else {
                error_msg = error_code_to_string(ErrorCode::FAIL_TO_SET);
                frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, mount_command_id, error_msg));
                return frames;
            }
        } else {
            error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
            frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, mount_command_id, error_msg));
            return frames;
        }
    } else {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, storage_commands_group_id, mount_command_id, error_msg));
        return frames;
    }
}
/** @} */ // StorageCommands