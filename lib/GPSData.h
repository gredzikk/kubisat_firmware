// GPSData.h
#include <string.h>
#include <iostream>

class GPSData {
private:
    static const int MAX_SENTENCE_LENGTH = 256;
    static const int MAX_SENTENCES = 10;
    char buffers[MAX_SENTENCES][MAX_SENTENCE_LENGTH];
    size_t bufferPos = 0;
    int sentenceCount = 0;
    bool messageSetComplete = false;

public:
    void addChar(char c) {
        if (bufferPos < MAX_SENTENCE_LENGTH - 1) {
            buffers[sentenceCount][bufferPos++] = c;
            
            if (c == '\n') {
                buffers[sentenceCount][bufferPos] = '\0';
                if (strncmp(buffers[sentenceCount], "$GPGGA", 6) == 0) {
                    sentenceCount = 0; // Start new set
                    messageSetComplete = true;
                } else {
                    sentenceCount = (sentenceCount + 1) % MAX_SENTENCES;
                }
                bufferPos = 0;
            }
        }
    }

    bool isMessageSetComplete() const { 
        return messageSetComplete; 
    }
    
    void clearBuffer() {
        bufferPos = 0;
        sentenceCount = 0;
        messageSetComplete = false;
    }

    void printMessages() const {
        std::cout << "\nGPS Messages:" << std::endl;
        for (int i = 0; i < sentenceCount; i++) {
            std::cout << buffers[i];
        }
    }
};