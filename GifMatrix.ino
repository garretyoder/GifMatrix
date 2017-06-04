#include <SmartMatrix3.h>
#include <SPI.h>
#include <SD.h>
#include "GIFDecoder.h"

#define DISPLAY_TIME_SECONDS 5

#define ENABLE_SCROLLING  1

// range 0-255
const int defaultBrightness = 255;

const rgb24 COLOR_BLACK = {
  0, 0, 0 
};
const rgb24 COLOR_WHITE = {
  255,255,255
};
const rgb24 COLOR_RED = {
  255,0,0
};
const rgb24 COLOR_GREEN = {
  0,255,0
};
const rgb24 COLOR_BLUE = {
  0,0,255
};
const rgb24 COLOR_YELLOW = {
  244, 241, 66
};


/* SmartMatrix configuration and memory allocation */
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 2;       // known working: 2-4
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN; // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);    // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
#if ENABLE_SCROLLING == 1
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
#endif

// Chip select for SD card on the SmartMatrix Shield
#define SD_CS 15

#define GIF_DIRECTORY "/gifs/"

int num_files;

void screenClearCallback(void) {
  backgroundLayer.fillScreen({0,0,0});
}

void updateScreenCallback(void) {
  backgroundLayer.swapBuffers();
}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  backgroundLayer.drawPixel(x, y, {red, green, blue});
}

// Setup method runs once, when the sketch starts
void setup() {
    setScreenClearCallback(screenClearCallback);
    setUpdateScreenCallback(updateScreenCallback);
    setDrawPixelCallback(drawPixelCallback);

    // Seed the random number generator
    randomSeed(analogRead(14));

    Serial.begin(115200);

    // Initialize matrix
    matrix.addLayer(&backgroundLayer); 
#if ENABLE_SCROLLING == 1
    matrix.addLayer(&scrollingLayer); 
#endif

    matrix.begin();

    //matrix.setRefreshRate(90);
    // for large panels, set the refresh rate lower to leave more CPU time to decoding GIFs (needed if GIFs are playing back slowly)

    // Clear screen
    backgroundLayer.fillScreen(COLOR_BLACK);
    backgroundLayer.swapBuffers();

    // initialize the SD card at full speed
    pinMode(SD_CS, OUTPUT);
    if (!SD.begin(SD_CS)) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("No SD card", -1);
#endif
        Serial.println("No SD card");
        while(1);
    }

    // Determine how many animated GIF files exist
    num_files = enumerateGIFFiles(GIF_DIRECTORY, false);

    if(num_files < 0) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("No gifs directory", -1);
#endif
        Serial.println("No gifs directory");
        while(1);
    }

    if(!num_files) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("Empty gifs directory", -1);
#endif
        Serial.println("Empty gifs directory");
        while(1);
    }
    scrollingLayer.setMode(wrapForward);
    scrollingLayer.setFont(font8x13);
    scrollingLayer.setSpeed(20);
}

void loop() {
    unsigned long futureTime;
    char pathname[30];

    int index = random(num_files);

    // Do forever
    while (true) {
        if (random(0,2)==0) {
          //PLAY GIF
          getGIFFilenameByIndex(GIF_DIRECTORY, index++, pathname);
          if (index >= num_files) {
            index = 0;
          }
          // Calculate time in the future to terminate animation
          futureTime = millis() + (DISPLAY_TIME_SECONDS * 1000);
          while (futureTime > millis()) {
            processGIFFile(pathname);
          }
        } else {
          //PLAY TEXT
          backgroundLayer.fillScreen(COLOR_BLACK);
          backgroundLayer.swapBuffers();
          int choice = random(13);
          switch (choice) {
            case 0:
            {
              scrollingLayer.start("Congratulations Class of 2017",1);
            }
            break;
            case 1:
            {
              scrollingLayer.start("Commencement Whoop Whoop!",1);
            }
            break;
            case 2:
            {
              scrollingLayer.start("THIS WAS HARD TO MAKE",1);
            }
            break;
            case 3:
            {
              scrollingLayer.setMode(bounceForward);
              scrollingLayer.start("Class of 2017",3);
              while (scrollingLayer.getStatus()>0) {};
              scrollingLayer.setMode(wrapForward);
            }
            break;
            case 4:
            {
              backgroundLayer.fillScreen(COLOR_WHITE);
              backgroundLayer.swapBuffers();
              scrollingLayer.setColor(COLOR_BLACK);
              scrollingLayer.start("Anime was a mistake",1);
              while (scrollingLayer.getStatus()>0) {};
              backgroundLayer.fillScreen(COLOR_BLACK);
              backgroundLayer.swapBuffers();
              scrollingLayer.setColor(COLOR_WHITE);
            }
            break;
            case 5:
            {
              scrollingLayer.start("FIDGET SPINNERS ARE STUPID",1);
            }
            break;
            case 6:
            {
              scrollingLayer.setColor(COLOR_GREEN);
              scrollingLayer.start("It's a grand mold time",1);
              while (scrollingLayer.getStatus()>0) {};
              scrollingLayer.setColor(COLOR_WHITE);
            }
            break;
            case 7:
            {
              scrollingLayer.setColor(COLOR_RED);
              scrollingLayer.start("Bloomsburg here I come",1);
              while (scrollingLayer.getStatus()>0) {};
              scrollingLayer.setColor(COLOR_WHITE);
            }
            break;
            case 8:
            {
              scrollingLayer.start("Show me what you got",1);
            }
            break;
            case 9:
            {
              //scrollingLayer.setColor(COLOR_BLACK);
              scrollingLayer.start("PRETTY COLORS",5);
              while (scrollingLayer.getStatus()>0) {
                scrollingLayer.setColor({random(256), random(256), random(256)});
                backgroundLayer.fillScreen({random(256), random(256), random(256)});
                backgroundLayer.swapBuffers();
                delay(500);
              }
              backgroundLayer.fillScreen(COLOR_BLACK);
              backgroundLayer.swapBuffers();
              scrollingLayer.setColor(COLOR_WHITE);
            }
            break;
            case 10:
            {
              scrollingLayer.start("Wubba lubba dub dub!",1);
            }
            break;
            case 11:
            {
              scrollingLayer.setColor(COLOR_YELLOW);
              backgroundLayer.fillScreen({0,0,180});
              backgroundLayer.swapBuffers();
              scrollingLayer.start("Cedar Cliff Colts",1);
              while (scrollingLayer.getStatus()>0) {};
              backgroundLayer.fillScreen(COLOR_BLACK);
              backgroundLayer.swapBuffers();
              scrollingLayer.setColor(COLOR_WHITE);
            }
            break;
            case 12:
            {
              scrollingLayer.start("Major: Biochemistry",1);
            }
            break;
          }
          while (scrollingLayer.getStatus()>0) {};
        }
    }
}
