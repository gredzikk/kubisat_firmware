#include "commands.h"
#include "communication.h"
#include "storage.h"
#include "filesystem/vfs.h"
#include "filesystem/littlefs.h"
#include <sys/stat.h> 
#include <errno.h>
#include "storage_commands_utils.h"
#include "dirent.h"

#define MAX_BLOCK_SIZE  250
#define STORAGE_GROUP 6
#define START_COMMAND 1
#define DATA_COMMAND 2
#define END_COMMAND 3
#define LIST_FILES_COMMAND 0
#define MOUNT_COMMAND 4 
/**
 * @defgroup StorageCommands Storage Commands
 * @brief Commands for interacting with the SD card storage.
 * @{
 */

/**
 * @brief Handles the file download command.
 *
 * This function reads a file from the SD card and sends it to the ground station
 * in blocks over LoRa. The ground station must acknowledge each block before
 * the next block is sent. A checksum is calculated and sent at the end of the
 * transmission to verify data integrity.
 *
 * @param param The filename to download.
 * @param operationType The operation type (must be GET).
 * @return A Frame indicating the result of the operation.
 *         - Success: Frame with "File download complete" message.
 *         - Error: Frame with error message (e.g., "File not found", "ACK timeout").
 *
 * @note <b>KBST;0;GET;6;1;[filename];TSBK</b>
 * @note Example: <b>KBST;0;GET;6;1;test.txt;TSBK</b> - Downloads the file "test.txt".
 * @ingroup StorageCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 6.1
 */
Frame handle_file_download(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET) {
        return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, START_COMMAND, "Invalid operation type");
    }

    const char* filename = param.c_str();
    FILE* file = fopen(filename, "rb");

    if (!file) {
        return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, START_COMMAND, "File not found");
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send file size to ground station
    Frame sizeFrame = frame_build(ExecutionResult::INFO, STORAGE_GROUP, START_COMMAND, std::to_string(fileSize));
    send_frame(sizeFrame);

    size_t block_size = MAX_BLOCK_SIZE;
    size_t block_count = (fileSize + block_size - 1) / block_size;

    // Send block size and count
    Frame blockSizeFrame = frame_build(ExecutionResult::INFO, STORAGE_GROUP, START_COMMAND, std::to_string(block_size));
    send_frame(blockSizeFrame);
    Frame blockCountFrame = frame_build(ExecutionResult::INFO, STORAGE_GROUP, START_COMMAND, std::to_string(block_count));
    send_frame(blockCountFrame);

    uint8_t buffer[MAX_BLOCK_SIZE];
    size_t bytesRead;
    uint32_t totalChecksum = 0;
    size_t blockIndex = 0;

    while ((bytesRead = fread(buffer, 1, MAX_BLOCK_SIZE, file)) > 0) {
        // Send data block
        send_data_block(buffer, bytesRead);
        totalChecksum = calculate_checksum(buffer, bytesRead);

        // Wait for ACK
        if (!receive_ack()) {
            fclose(file);
            return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, DATA_COMMAND, "ACK timeout");
        }
        blockIndex++;
    }

    fclose(file);

    // Send end frame with checksum
    std::stringstream ss;
    ss << std::hex << totalChecksum;
    Frame endFrame = frame_build(ExecutionResult::SUCCESS, STORAGE_GROUP, END_COMMAND, ss.str());
    send_frame(endFrame);

    return frame_build(ExecutionResult::SUCCESS, STORAGE_GROUP, END_COMMAND, "File download complete");
}


/**
 * @brief Handles the list files command.
 *
 * This function lists the files in the root directory of the SD card and sends
 * the filename and size of each file to the ground station in separate frames.
 *
 * @param param Unused.
 * @param operationType The operation type (must be GET).
 * @return A Frame indicating the result of the operation.
 *         - Success: Frame with "File listing complete" message.
 *         - Error: Frame with error message (e.g., "Could not open directory").
 *
 * @note <b>KBST;0;GET;6;0;;TSBK</b>
 * @note This command lists the files and their sizes in the root directory of the SD card.
 * @ingroup StorageCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 6.0
 */
Frame handle_list_files(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET) {
        return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, LIST_FILES_COMMAND, "Invalid operation type");
    }

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("/")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            const char* filename = ent->d_name;

            // Skip "." and ".." directories
            if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
                continue;
            }

            // Get file size
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "/%s", filename);
            
            FILE* file = fopen(filepath, "rb");
            size_t fileSize = 0;
            
            if (file != NULL) {
                fseek(file, 0, SEEK_END);
                fileSize = ftell(file);
                fclose(file);
            }

            // Create and send frame with filename and size
            char fileInfo[512];
            snprintf(fileInfo, sizeof(fileInfo), "%s:%zu", filename, fileSize);
            Frame fileFrame = frame_build(ExecutionResult::INFO, STORAGE_GROUP, LIST_FILES_COMMAND, fileInfo);
            uart_print(fileInfo, VerbosityLevel::INFO);
            send_frame(fileFrame);
        }
        closedir(dir);
        return frame_build(ExecutionResult::SUCCESS, STORAGE_GROUP, LIST_FILES_COMMAND, "File listing complete");
    } else {
        return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, LIST_FILES_COMMAND, "Could not open directory");
    }
}


/**
 * @brief Handles the SD card mount/unmount command.
 *
 * This function mounts or unmounts the SD card.
 *
 * @param param "0" to unmount, "1" to mount.
 * @param operationType The operation type (must be SET).
 * @return A Frame indicating the result of the operation.
 *         - Success: Frame with "SD card mounted" or "SD card unmounted" message.
 *         - Error: Frame with error message (e.g., "Invalid parameter", "Mount failed", "Unmount failed").
 *
 * @note <b>KBST;0;SET;6;4;[0|1];TSBK</b>
 * @note Example: <b>KBST;0;SET;6;4;1;TSBK</b> - Mounts the SD card.
 * @ingroup StorageCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 6.4
 */
Frame handle_mount(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::SET) {
        return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, MOUNT_COMMAND, "Invalid operation type");
    }

    if (param == "1") {
        if (fs_init()) {
            return frame_build(ExecutionResult::SUCCESS, STORAGE_GROUP, MOUNT_COMMAND, "SD card mounted");
        } else {
            return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, MOUNT_COMMAND, "Mount failed");
        }
    } else if (param == "0") {
        if (fs_unmount("/") == 0) { 
            sd_card_mounted = false;
            return frame_build(ExecutionResult::SUCCESS, STORAGE_GROUP, MOUNT_COMMAND, "SD card unmounted");
        } else {
            return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, MOUNT_COMMAND, "Unmount failed");
        }
        return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, MOUNT_COMMAND, "Unmount not implemented");
    } else {
        return frame_build(ExecutionResult::ERROR, STORAGE_GROUP, MOUNT_COMMAND, "Invalid parameter");
    }
}
/** @} */ // StorageCommands