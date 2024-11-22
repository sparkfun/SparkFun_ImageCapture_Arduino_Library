////////////////////////////////////////////////////////////////////////////////
// Below is a modification of the OV2640 camera driver to work with the OV5640.
// This is largely an Arduino adaptation of the following:
// https://github.com/adafruit/Adafruit_CircuitPython_OV5640
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Adafruit_iCap_parallel.h>

/** Supported sizes for OV5640_set_size() */
typedef enum {
  OV5640_SIZE_96X96 = 0,  // 96x96
  OV5640_SIZE_QQVGA = 1,  // 160x120
  OV5640_SIZE_QCIF = 2,  // 176x144
  OV5640_SIZE_HQVGA = 3,  // 240x176
  OV5640_SIZE_240X240 = 4,  // 240x240
  OV5640_SIZE_QVGA = 5,  // 320x240
  OV5640_SIZE_CIF = 6,  // 400x296
  OV5640_SIZE_HVGA = 7,  // 480x320
  OV5640_SIZE_VGA = 8,  // 640x480
  OV5640_SIZE_SVGA = 9,  // 800x600
  OV5640_SIZE_XGA = 10,  // 1024x768
  OV5640_SIZE_HD = 11,  // 1280x720
  OV5640_SIZE_SXGA = 12,  // 1280x1024
  OV5640_SIZE_UXGA = 13,  // 1600x1200
  OV5640_SIZE_QHDA = 14,  // 2560x1440
  OV5640_SIZE_WQXGA = 15,  // 2560x1600
  OV5640_SIZE_PFHD = 16,  // 1088x1920
  OV5640_SIZE_QSXGA = 17,  // 2560x1920
} OV5640_size;

/** Supported effects for OV5640_set_special_effect() */
typedef enum{
  OV5640_SPECIAL_EFFECT_NONE = 0,
  OV5640_SPECIAL_EFFECT_NEGATIVE = 1,
  OV5640_SPECIAL_EFFECT_GRAYSCALE = 2,
  OV5640_SPECIAL_EFFECT_RED_TINT = 3,
  OV5640_SPECIAL_EFFECT_GREEN_TINT = 4,
  OV5640_SPECIAL_EFFECT_BLUE_TINT = 5,
  OV5640_SPECIAL_EFFECT_SEPIA = 6,
} OV5640_special_effect;

typedef enum {
  OV5640_WHITE_BALANCE_AUTO = 0,
  OV5640_WHITE_BALANCE_SUNNY = 1,
  OV5640_WHITE_BALANCE_FLUORESCENT = 2,
  OV5640_WHITE_BALANCE_CLOUDY = 3,
  OV5640_WHITE_BALANCE_INCANDESCENT = 4
} OV5640_white_balance;


#if defined(ICAP_FULL_SUPPORT)

typedef iCap_parallel_pins OV5640_pins;

#define OV5640_ADDR 0x3C //< Default I2C address if unspecified

/*!
    @brief  Class encapsulating OmniVision OV5640 functionality.
*/
class SparkFun_iCap_OV5640 : public Adafruit_iCap_parallel {
public:
  /*!
    @brief  Constructor for OV5640 camera class.
    @param  pins      OV5640_pins structure, describing physical connection
                      to the camera.
    @param  arch      Pointer to structure containing architecture-specific
                      settings. For example, on SAMD51, this structure
                      includes a pointer to a timer peripheral's base
                      address, used to generate the xclk signal. The
                      structure is always of type iCap_arch, but the
                      specific elements within will vary with each supported
                       architecture.
    @param  twi       TwoWire instance (e.g. Wire or Wire1), used for I2C
                      communication with camera.
    @param  pbuf      Preallocated buffer for captured pixel data, or NULL
                      for library to allocate as needed when a camera
                      resolution is selected.
    @param  pbufsize  Size of passed-in buffer (or 0 if NULL).
    @param  addr      I2C address of camera.
    @param  speed     I2C communication speed to camera.
    @param  delay_us  Delay in microseconds between register writes.
  */
  SparkFun_iCap_OV5640(iCap_parallel_pins &pins, iCap_arch *arch = NULL,
                       TwoWire &twi = Wire, uint16_t *pbuf = NULL,
                       uint32_t pbufsize = 0, uint8_t addr = OV5640_ADDR,
                       uint32_t speed = 100000, uint32_t delay_us = 1000);
  ~SparkFun_iCap_OV5640(); // Destructor

  /*!
    @brief   Initialize peripherals behind an SparkFun_iCap_OV5640 instance,
             but do not actually start capture; must follow with a config()
             call for that.
    @return  Status code. ICAP_STATUS_OK on successful init.
  */
  iCap_status begin(void);

  /*!
    @brief   Initialize peripherals and allocate resources behind an
             SparkFun_iCap_OV5640 instance, start capturing data in
             background. Really just a one-step wrapper around begin(void)
             and config(...).
    @param   size   Frame size as a OV5640_size enum value.
    @param   space  ICAP_COLOR_RGB or ICAP_COLOR_YUV.
    @param   fps    Desired capture framerate, in frames per second, as a
                    float up to 30.0. Actual device frame rate may differ
                    from this, depending on a host's available PWM timing.
    @param   nbuf   Number of full-image buffers, 1-3. For now, always use
                    1, multi-buffering isn't handled yet.
    @return  Status code. ICAP_STATUS_OK on successful init.
  */
  iCap_status begin(OV5640_size size, iCap_colorspace space = ICAP_COLOR_RGB565,
                    float fps = 30.0, uint8_t nbuf = 1);

  /*!
    @brief   Change frame configuration on an already-running camera.
    @param   size  One of the OV5640_size values (TBD).
    @param   space  ICAP_COLOR_RGB or ICAP_COLOR_YUV.
    @param   fps    Desired capture framerate, in frames per second, as a
                    float up to 30.0. Actual device frame rate may differ
                    from this, depending on a host's available PWM timing.
    @param   nbuf   Number of full-image buffers, 1-3. For now, always use
                    1, multi-buffering isn't handled yet.
    @param   allo   (Re-)allocation behavior. This value is IGNORED if a
                    static pixel buffer was passed to the constructor; it
                    only applies to dynamic allocation. ICAP_REALLOC_NONE
                    keeps the existing buffer (if new dimensions still fit),
                    ICAP_REALLOC_CHANGE will reallocate if the new dimensions
                    are smaller or larger than before. ICAP_REALLOC_LARGER
                    reallocates only if the new image specs won't fit in the
                    existing buffer (but ignoring reductions, some RAM will
                    go unused but avoids fragmentation).
    @return  Status code. ICAP_STATUS_OK on successful update, may return
             ICAP_STATUS_ERR_MALLOC if using dynamic allocation and the
             buffer resize fails.
    @note    Reallocating the camera buffer is fraught with peril and should
             only be done if you're prepared to handle any resulting error.
             In most cases, code should call the constructor with a static
             buffer suited to the size of LARGEST image it anticipates
             needing (including any double buffering, etc.). Some RAM will
             go untilized at times, but it's favorable to entirely losing
             the camera mid-run.
  */
  iCap_status config(OV5640_size size,
                     iCap_colorspace space = ICAP_COLOR_RGB565,
                     float fps = 30.0, uint8_t nbuf = 1,
                     iCap_realloc allo = ICAP_REALLOC_CHANGE);

  /*!
    @brief  Configure camera colorspace.
    @param  space  ICAP_COLOR_RGB565 or ICAP_COLOR_YUV.
  */
  void setColorspace(iCap_colorspace space = ICAP_COLOR_RGB565);

  /*!
    @brief  Flip Camera output on horizontal and/or vertical axes.
    @param  flip_x True to flip horizontally, false for normal.
    @param  flip_y True to flip vertically, false for normal.
  */
  void flip(bool flip_x, bool flip_y);

  /*!
    @brief  Enable or disable the night mode setting of the sensor
    @param  enable_night True to enable night mode, false to disable.
  */
  void setNight(bool enable_night);

  /*!
    @brief  Set the camera to output a test pattern.
    @param enable True to enable test pattern, false to disable.
  */
  void testPattern(bool enable);

  /*!
    @brief  Set the saturation level of the camera.
    @param  sat_level Saturation level from -4 to 4 inclusive
  */
  void setSaturation(int sat_level);

  /*!
    @brief  Sensor contrast adjustment
    @param  size Contrast level, from -3 to 3 inclusive
  */
  void setContrast(int contrast_level);

  /*!
    @brief  Set the effect of the camera.
    @param  value Effect value. See OV5640_special_effect enum.
  */
  void setSpecialEffect(OV5640_special_effect value);

  /*!
    @brief  Set the exposure level of the camera.
    @param  exposure_level Exposure level from -3 to 3 inclusive
  */
  void setExposure(int exposure_level);

  /*!
    @brief  Set the brightness level of the camera.
    @param  brightness_level Brightness level from -4 to 4 inclusive
  */
  void setBrightness(int brightness_level);

  /*!
    @brief  Set the white balance of the camera.
    @param  white_balance White balance value. See OV5640_white_balance enum.
  */
  void setWhiteBalance(OV5640_white_balance white_balance);

  //////////////////////////////////////////////////////////////////////////////
  // The OV5640 uses 16-bit register addresses, requiring the functions below.
  //////////////////////////////////////////////////////////////////////////////

  int readRegister16(uint16_t reg);
  void writeRegister16(uint16_t reg, uint8_t value);
  void writeList16(const uint16_t cfg[][2], uint16_t len);
  void _write_addr_reg(uint16_t reg, uint16_t x_value, uint16_t y_value);
  void _write_reg_bits(uint16_t reg, uint16_t mask, bool enable);
  void _write_group_3_settings(const uint16_t cfg[][2], uint16_t len);
  void _set_size_and_colorspace(OV5640_size size, iCap_colorspace colorspace);
  void _set_image_options();
  void _set_colorspace(iCap_colorspace colorspace);
  void _set_pll(bool bypass, int multiplier, int sys_div, int pre_div, bool root_2x, int pclk_root_div, bool pclk_manual, int pclk_div);

  bool _binning = false;
  bool _scale = false;
  bool _flip_x = false;
  bool _flip_y = true;
  bool _test_pattern = false;
  int _saturation = 0; // -4 to 4
  int _contrast = 0; // -3 to 3
  OV5640_special_effect _effect = OV5640_SPECIAL_EFFECT_NONE;
  int _exposure = 0; // -3 to 3
  iCap_colorspace _colorspace = ICAP_COLOR_RGB565;
  int _brightness = 0; // -4 to 4

};

#endif // end ICAP_FULL_SUPPORT
