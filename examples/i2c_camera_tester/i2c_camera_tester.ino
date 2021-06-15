// This runs on a Metro Mini and pings OV7670-equipped Pico board
// running the cameratest_i2c_OV7670_Pico sketch.
// (Both boards run concurrently, talking over I2C)

/*
NOTES TO SELF:
Start librarifying these I2C transactions, because they're
gonna get awkward FAST. Prob 2 headers, one for cam I2C host &
peripheral (enumerates things like the connand bytes), and one
specifically for host (to encapsulate the ugly code below into
easy functions).

Consider changing the cam lib so PWM can be independently
initialized before starting up the whole camera capture deal.
This would allow registers to be read & written (camera needs
clock input via PWM to run its own I2C) and might allow things
like auto camera make & model detection later.
*/

#include <Wire.h>
#include "Adafruit_iCap_I2C_host.h"
#include "Adafruit_iCap_OV7670.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#if !defined(BUFFER_LENGTH)
#define BUFFER_LENGTH 256 // Max I2C transfer size
#endif

Adafruit_iCap_peripheral cam; // Remote camera on I2C

#define TFT_CS   4
#define TFT_DC   5
#define TFT_RST -1

Adafruit_ST7789 tft(&SPI, TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("HOST BOARD STARTED");

  tft.init(240, 240);
  tft.fillScreen(0);
  tft.println("Hello");

  cam.begin();

  // Start camera, check response
  int status = cam.cameraStart(ICAP_COLOR_RGB565, OV7670_SIZE_DIV4, 30.0);
  if(status != ICAP_STATUS_OK) {
    Serial.println("Camera failed to start");
    Serial.print("Status: ");
    Serial.println(cam.status(), HEX);
    for(;;);
  }

delay(5000); // Allow autoexposure to do things

  // Poll the PID and VER registers to see if camera's working

  Serial.print("PID: ");
  Serial.println(cam.readRegister(OV7670_REG_PID), HEX); // Expecting 0x76
  Serial.print("VER: ");
  Serial.println(cam.readRegister(OV7670_REG_VER), HEX); // Expecting 0x73
}

uint16_t pixelbuf[256];
uint8_t *pbuf8 = (uint8_t *)pixelbuf;
int      bytesinbuf = 0;

void loop() {
  int32_t bytes = cam.capture();
  Serial.print("Expecting ");
  Serial.print(bytes);
  Serial.print(" from camera, BUFFER_LENGTH is ");
  Serial.println(BUFFER_LENGTH);

  tft.startWrite();
  tft.setAddrWindow((tft.width() - cam.width()) / 2,
                    (tft.height() - cam.height()) / 2,
                    cam.width(), cam.height());

  while(bytes > 0) {
    uint8_t *data = cam.getData(BUFFER_LENGTH - 1);
    uint8_t len = data[0];
    // Add new bytes to pixelbuf
    memcpy(&pbuf8[bytesinbuf], &data[1], len);
    bytesinbuf += len;
    int pixelsThisPass = bytesinbuf / 2; // 16 bit pixels (any trailing byte is ignored)
    tft.writePixels(pixelbuf, pixelsThisPass, false, true);
    if (bytesinbuf & 1) {               // Trailing byte present?
      pbuf8[0] = pbuf8[bytesinbuf - 1]; // Move it to beginning
      bytesinbuf = 1;
    } else {
      bytesinbuf = 0;
    }

    bytes -= len;
  }
  tft.endWrite();   // Close out prior transfer

  cam.resume();

  delay(1000);
}
