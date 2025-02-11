#include "ff.h"
#include "sd_card.h"

bool waitForUserInteraction(const uint64_t TIMEOUT_MS);
bool initializeSDCard(char* buf);
bool mountDrive(FATFS& fs, char* buf);
bool openFile(FIL& fil, const char* filename, BYTE mode, char* buf);
bool writeToFile(FIL& fil, const char* data, char* buf);
bool closeFile(FIL& fil, char* buf);

bool testSDCard();



