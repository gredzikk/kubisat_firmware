#include "ff.h"
#include "sd_card.h"

bool waitForUserInteraction(const uint64_t TIMEOUT_MS);
bool initializeSDCard(char* buf);
bool mountDrive(FATFS& fs, char* buf);
bool writeToFile(const char* filename, const char* data);

bool testSDCard();