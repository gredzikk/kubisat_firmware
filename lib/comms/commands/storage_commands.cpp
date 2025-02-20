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
#define DOWNLOAD_GROUP 6
#define START_COMMAND 1
#define DATA_COMMAND 2
#define END_COMMAND 3
#define LIST_FILES_COMMAND 0

/**
 * @brief Handles the file download command.
 *
 * This function reads a file from the SD card and sends it to the ground station
 * in blocks over LoRa. The ground station must acknowledge each block before
 * the next block is sent. A checksum is calculated and sent at the end of the
 * transmission to verify data integrity.
 *
 * @param param The filename to download.
 * @param operationType The operation type (must be SET).
 * @return A Frame indicating the result of the operation.
 */
Frame handle_file_download(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::SET) {
        return frame_build(ExecutionResult::ERROR, DOWNLOAD_GROUP, START_COMMAND, "Invalid operation type");
    }

    const char* filename = param.c_str();
    FILE* file = fopen(filename, "rb");

    if (!file) {
        return frame_build(ExecutionResult::ERROR, DOWNLOAD_GROUP, START_COMMAND, "File not found");
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send file size to ground station
    Frame sizeFrame = frame_build(ExecutionResult::INFO, DOWNLOAD_GROUP, START_COMMAND, std::to_string(fileSize));
    send_frame(sizeFrame);

    size_t block_size = MAX_BLOCK_SIZE;
    size_t block_count = (fileSize + block_size - 1) / block_size;

    // Send block size and count
    Frame blockSizeFrame = frame_build(ExecutionResult::INFO, DOWNLOAD_GROUP, START_COMMAND, std::to_string(block_size));
    send_frame(blockSizeFrame);
    Frame blockCountFrame = frame_build(ExecutionResult::INFO, DOWNLOAD_GROUP, START_COMMAND, std::to_string(block_count));
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
            return frame_build(ExecutionResult::ERROR, DOWNLOAD_GROUP, DATA_COMMAND, "ACK timeout");
        }
        blockIndex++;
    }

    fclose(file);

    // Send end frame with checksum
    std::stringstream ss;
    ss << std::hex << totalChecksum;
    Frame endFrame = frame_build(ExecutionResult::SUCCESS, DOWNLOAD_GROUP, END_COMMAND, ss.str());
    send_frame(endFrame);

    return frame_build(ExecutionResult::SUCCESS, DOWNLOAD_GROUP, END_COMMAND, "File download complete");
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
 */
Frame handle_list_files(const std::string& param, OperationType operationType) {
    if (operationType != OperationType::GET) {
        return frame_build(ExecutionResult::ERROR, DOWNLOAD_GROUP, LIST_FILES_COMMAND, "Invalid operation type");
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
            Frame fileFrame = frame_build(ExecutionResult::INFO, DOWNLOAD_GROUP, LIST_FILES_COMMAND, fileInfo);
            send_frame(fileFrame);
        }
        closedir(dir);
        return frame_build(ExecutionResult::SUCCESS, DOWNLOAD_GROUP, LIST_FILES_COMMAND, "File listing complete");
    } else {
        return frame_build(ExecutionResult::ERROR, DOWNLOAD_GROUP, LIST_FILES_COMMAND, "Could not open directory");
    }
}