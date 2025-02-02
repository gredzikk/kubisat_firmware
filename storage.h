#include "ff.h"
#include "sd_card.h"

static bool waitForUserInteraction(const uint64_t TIMEOUT_MS);
static bool initializeSDCard(char* buf);
static bool mountDrive(FATFS& fs, char* buf);
static bool openFile(FIL& fil, const char* filename, BYTE mode, char* buf);
static bool writeToFile(FIL& fil, const char* data, char* buf);
static bool closeFile(FIL& fil, char* buf);

bool testSDCard();



