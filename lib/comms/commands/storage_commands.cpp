#include "commands.h"
#include "communication.h"
#include "storage.h"
#include "filesystem/vfs.h"
#include "filesystem/littlefs.h"
#include <sys/stat.h>
#include <errno.h>
#include "storage_commands_utils.h"
#include "dirent.h"

#define MAX_BLOCK_SIZE 250
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
 * @return A vector of Frames indicating the result of the operation.
 *         - Success: Frame with "File download complete" message.
 *         - Error: Frame with error message (e.g., "File not found", "ACK timeout").
 *
 * @note <b>KBST;0;GET;6;1;[filename];TSBK</b>
 * @note Example: <b>KBST;0;GET;6;1;test.txt;TSBK</b> - Downloads the file "test.txt".
 * @ingroup StorageCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 6.1
 */
std::vector<Frame> handle_file_download(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    if (operationType != OperationType::GET) {
        frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, START_COMMAND, "Invalid operation type"));
        return frames;
    }

    const char* filename = param.c_str();
    FILE* file = fopen(filename, "rb");

    if (!file) {
        frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, START_COMMAND, "File not found"));
        return frames;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send file size to ground station
    frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, START_COMMAND, std::to_string(fileSize)));
    send_frame(frames.back());

    size_t block_size = MAX_BLOCK_SIZE;
    size_t block_count = (fileSize + block_size - 1) / block_size;

    // Send block size and count
    frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, START_COMMAND, std::to_string(block_size)));
    send_frame(frames.back());
    frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, START_COMMAND, std::to_string(block_count)));
    send_frame(frames.back());

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
            frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, DATA_COMMAND, "ACK timeout"));
            return frames;
        }
        blockIndex++;
    }

    fclose(file);

    // Send end frame with checksum
    std::stringstream ss;
    ss << std::hex << totalChecksum;
    frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, END_COMMAND, ss.str()));
    send_frame(frames.back());

    frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, END_COMMAND, "File download complete"));
    return frames;
}

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
std::vector<Frame> handle_list_files(const std::string& param, OperationType operationType) {
    std::vector<Frame> frames;
    if (operationType != OperationType::GET) {
        frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, LIST_FILES_COMMAND, "Invalid operation type"));
        return frames;
    }

    DIR* dir;
    struct dirent* ent;
    int fileCount = 0; // Counter for the number of files
    if ((dir = opendir("/")) != NULL) {
        // First, count the number of files
        while ((ent = readdir(dir)) != NULL) {
            const char* filename = ent->d_name;
            if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
                continue;
            }
            fileCount++;
        }
        closedir(dir);

        // Send the number of files
        frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, LIST_FILES_COMMAND, std::to_string(fileCount)));

        // Open the directory again to read file information
        dir = opendir("/");
        if (dir != NULL) {
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
                frames.push_back(frame_build(OperationType::SEQ, STORAGE_GROUP, LIST_FILES_COMMAND, fileInfo));
            }
            closedir(dir);
            frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, LIST_FILES_COMMAND, "FILE_LIST_DONE"));
            return frames;
        } else {
            frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, LIST_FILES_COMMAND, "Could not open directory for file info"));
            return frames;
        }
    } else {
        frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, LIST_FILES_COMMAND, "Could not open directory"));
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
    if (operationType == OperationType::GET) {
        frames.push_back(frame_build(OperationType::VAL, STORAGE_GROUP, MOUNT_COMMAND, std::to_string(sd_card_mounted)));
        return frames;
    } else if (operationType == OperationType::SET) {
        if (param == "1") {
            if (fs_init()) {
                frames.push_back(frame_build(OperationType::RES, STORAGE_GROUP, MOUNT_COMMAND, "SD card mounted"));
                return frames;
            } else {
                frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, MOUNT_COMMAND, "Mount failed"));
                return frames;
            }
        } else if (param == "0") {
            if (fs_unmount("/") == 0) {
                sd_card_mounted = false;
                frames.push_back(frame_build(OperationType::RES, STORAGE_GROUP, MOUNT_COMMAND, "SD card unmounted"));
                return frames;
            } else {
                frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, MOUNT_COMMAND, "Unmount failed"));
                return frames;
            }
        } else {
            frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, MOUNT_COMMAND, "Invalid parameter"));
            return frames;
        }
    } else {
        frames.push_back(frame_build(OperationType::ERR, STORAGE_GROUP, MOUNT_COMMAND, "Invalid operation type"));
        return frames;
    }
}
/** @} */ // StorageCommands