////////////////////////////////////////////////////////////////////////////////
// Below is a modification of the OV2640 camera driver to work with the OV5640.
// This is largely an Arduino adaptation of the following:
// https://github.com/adafruit/Adafruit_CircuitPython_OV5640
////////////////////////////////////////////////////////////////////////////////

#include <SparkFun_iCap_OV5640.h>
#include <Arduino.h>

#if defined(ICAP_FULL_SUPPORT)

const uint16_t _SYSTEM_RESET00 = 0x3000; // Reset for Individual Block
// (0: enable block; 1: reset block)
// Bit[7]: Reset BIST
// Bit[6]: Reset MCU program memory
// Bit[5]: Reset MCU
// Bit[4]: Reset OTP
// Bit[3]: Reset STB
// Bit[2]: Reset d5060
// Bit[1]: Reset timing control
// Bit[0]: Reset array control

const uint16_t _SYSTEM_RESET02 = 0x3002; // Reset for Individual Block
// (0: enable block; 1: reset block)
// Bit[7]: Reset VFIFO
// Bit[5]: Reset format
// Bit[4]: Reset JFIFO
// Bit[3]: Reset SFIFO
// Bit[2]: Reset JPG
// Bit[1]: Reset format MUX
// Bit[0]: Reset average

const uint16_t _CLOCK_ENABLE02 = 0x3006; // Clock Enable Control
// (0: disable clock; 1: enable clock)
// Bit[7]: Enable PSRAM clock
// Bit[6]: Enable FMT clock
// Bit[5]: Enable JPEG 2x clock
// Bit[3]: Enable JPEG clock
// Bit[1]: Enable format MUX clock
// Bit[0]: Enable average clock

const uint16_t _SYSTEM_CTROL0 = 0x3008;
// Bit[7]: Software reset
// Bit[6]: Software power down
// Bit[5]: Reserved
// Bit[4]: SRB clock SYNC enable
// Bit[3]: Isolation suspend select
// Bit[2:0]: Not used

const uint16_t _CHIP_ID_HIGH = 0x300A;

const uint16_t _DRIVE_CAPABILITY = 0x302C;
// Bit[7:6]:
//          00: 1x
//          01: 2x
//          10: 3x
//          11: 4x

const uint16_t _SC_PLLS_CTRL0 = 0x303A;
// Bit[7]: PLLS bypass
const uint16_t _SC_PLLS_CTRL1 = 0x303B;
// Bit[4:0]: PLLS multiplier
const uint16_t _SC_PLLS_CTRL2 = 0x303C;
// Bit[6:4]: PLLS charge pump control
// Bit[3:0]: PLLS system divider
const uint16_t _SC_PLLS_CTRL3 = 0x303D;
// Bit[5:4]: PLLS pre-divider
//          00: 1
//          01: 1.5
//          10: 2
//          11: 3
// Bit[2]: PLLS root-divider - 1
// Bit[1:0]: PLLS seld5
//          00: 1
//          01: 1
//          10: 2
//          11: 2.5

// AEC/AGC control functions
const uint16_t _AEC_PK_MANUAL = 0x3503;
// AEC Manual Mode Control
// Bit[7:6]: Reserved
// Bit[5]: Gain delay option
//         Valid when 0x3503[4]=1’b0
//         0: Delay one frame latch
//         1: One frame latch
// Bit[4:2]: Reserved
// Bit[1]: AGC manual
//         0: Auto enable
//         1: Manual enable
// Bit[0]: AEC manual
//         0: Auto enable
//         1: Manual enable

// gain = {0x350A[1:0], 0x350B[7:0]} / 16

// AEC/AGC power domain control
const uint16_t _AEC_POWER_DOMAIN = 0x3A00;
// Bit[7]: Debug mode
// Bit[6]: Less one line enable
// Bit[5]: Band function enable
// Bit[4]: Less 1 band enable
// Bit[3]: Start selection
// Bit[2]: Night mode
// Bit[1]: New balance function
// Bit[0]: Freeze
const uint16_t _AEC_POWER_DOMAIN_NIGHT_MASK = 0x04;

const uint16_t _X_ADDR_ST_H = 0x3800;
const uint16_t _X_ADDR_ST_L = 0x3801;
const uint16_t _Y_ADDR_ST_H = 0x3802;
const uint16_t _Y_ADDR_ST_L = 0x3803;
const uint16_t _X_ADDR_END_H = 0x3804;
const uint16_t _X_ADDR_END_L = 0x3805;
const uint16_t _Y_ADDR_END_H = 0x3806;
const uint16_t _Y_ADDR_END_L = 0x3807;
const uint16_t _X_OUTPUT_SIZE_H = 0x3808;
const uint16_t _X_OUTPUT_SIZE_L = 0x3809;
const uint16_t _Y_OUTPUT_SIZE_H = 0x380A;
const uint16_t _Y_OUTPUT_SIZE_L = 0x380B;
const uint16_t _X_TOTAL_SIZE_H = 0x380C;
const uint16_t _X_TOTAL_SIZE_L = 0x380D;
const uint16_t _Y_TOTAL_SIZE_H = 0x380E;
const uint16_t _Y_TOTAL_SIZE_L = 0x380F;
const uint16_t _X_OFFSET_H = 0x3810;
const uint16_t _X_OFFSET_L = 0x3811;
const uint16_t _Y_OFFSET_H = 0x3812;
const uint16_t _Y_OFFSET_L = 0x3813;
const uint16_t _X_INCREMENT = 0x3814;
const uint16_t _Y_INCREMENT = 0x3815;

const uint16_t _TIMING_TC_REG20 = 0x3820;
// Timing Control Register
// Bit[2:1]: Vertical flip enable
//         00: Normal
//         11: Vertical flip
// Bit[0]: Vertical binning enable

const uint16_t _TIMING_TC_REG21 = 0x3821;
// Timing Control Register
// Bit[5]: Compression Enable
// Bit[2:1]: Horizontal mirror enable
//         00: Normal
//         11: Horizontal mirror
// Bit[0]: Horizontal binning enable

const uint16_t _PCLK_RATIO = 0x3824;
// Bit[4:0]: PCLK ratio manual

// frame control registers
const uint16_t _FRAME_CTRL01 = 0x4201;
// Control Passed Frame Number When both ON and OFF number set to 0x00,frame
// control is in bypass mode
// Bit[7:4]: Not used
// Bit[3:0]: Frame ON number

const uint16_t _FRAME_CTRL02 = 0x4202;
// Control Masked Frame Number When both ON and OFF number set to 0x00,frame
// control is in bypass mode
// Bit[7:4]: Not used
// BIT[3:0]: Frame OFF number

// format control registers
const uint16_t _FORMAT_CTRL00 = 0x4300;

const uint16_t _CLOCK_POL_CONTROL = 0x4740;
// Bit[5]: PCLK polarity 0: active low
//          1: active high
// Bit[3]: Gate PCLK under VSYNC
// Bit[2]: Gate PCLK under HREF
// Bit[1]: HREF polarity
//          0: active low
//          1: active high
// Bit[0] VSYNC polarity
//          0: active low
//          1: active high

const uint16_t _ISP_CONTROL_01 = 0x5001;
// Bit[5]: Scale enable
//          0: Disable
//          1: Enable

// output format control registers
const uint16_t _FORMAT_CTRL = 0x501F;
// Format select
// Bit[2:0]:
//  000: YUV422
//  001: RGB
//  010: Dither
//  011: RAW after DPC
//  101: RAW after CIP

// ISP top control registers
const uint16_t _PRE_ISP_TEST_SETTING_1 = 0x503D;
// Bit[7]: Test enable
//         0: Test disable
//         1: Color bar enable
// Bit[6]: Rolling
// Bit[5]: Transparent
// Bit[4]: Square black and white
// Bit[3:2]: Color bar style
//         00: Standard 8 color bar
//         01: Gradual change at vertical mode 1
//         10: Gradual change at horizontal
//         11: Gradual change at vertical mode 2
// Bit[1:0]: Test select
//         00: Color bar
//         01: Random data
//         10: Square data
//         11: Black image

// exposure = {0x3500[3:0], 0x3501[7:0], 0x3502[7:0]} / 16 × tROW

const uint16_t _SCALE_CTRL_1 = 0x5601;
// Bit[6:4]: HDIV RW
//          DCW scale times
//          000: DCW 1 time
//          001: DCW 2 times
//          010: DCW 4 times
//          100: DCW 8 times
//          101: DCW 16 times
//          Others: DCW 16 times
// Bit[2:0]: VDIV RW
//          DCW scale times
//          000: DCW 1 time
//          001: DCW 2 times
//          010: DCW 4 times
//          100: DCW 8 times
//          101: DCW 16 times
//          Others: DCW 16 times

const uint16_t _SCALE_CTRL_2 = 0x5602;
// X_SCALE High Bits
const uint16_t _SCALE_CTRL_3 = 0x5603;
// X_SCALE Low Bits
const uint16_t _SCALE_CTRL_4 = 0x5604;
// Y_SCALE High Bits
const uint16_t _SCALE_CTRL_5 = 0x5605;
// Y_SCALE Low Bits
const uint16_t _SCALE_CTRL_6 = 0x5606;
// Bit[3:0]: V Offset

const uint16_t _VFIFO_CTRL0C = 0x460C;
// Bit[1]: PCLK manual enable
//          0: Auto
//          1: Manual by PCLK_RATIO

const uint16_t _VFIFO_X_SIZE_H = 0x4602;
const uint16_t _VFIFO_X_SIZE_L = 0x4603;
const uint16_t _VFIFO_Y_SIZE_H = 0x4604;
const uint16_t _VFIFO_Y_SIZE_L = 0x4605;

const uint16_t _COMPRESSION_CTRL00 = 0x4400;
const uint16_t _COMPRESSION_CTRL01 = 0x4401;
const uint16_t _COMPRESSION_CTRL02 = 0x4402;
const uint16_t _COMPRESSION_CTRL03 = 0x4403;
const uint16_t _COMPRESSION_CTRL04 = 0x4404;
const uint16_t _COMPRESSION_CTRL05 = 0x4405;
const uint16_t _COMPRESSION_CTRL06 = 0x4406;
const uint16_t _COMPRESSION_CTRL07 = 0x4407;
// Bit[5:0]: QS
const uint16_t _COMPRESSION_ISI_CTRL = 0x4408;
const uint16_t _COMPRESSION_CTRL09 = 0x4409;
const uint16_t _COMPRESSION_CTRL0A = 0x440A;
const uint16_t _COMPRESSION_CTRL0B = 0x440B;
const uint16_t _COMPRESSION_CTRL0C = 0x440C;
const uint16_t _COMPRESSION_CTRL0D = 0x440D;
const uint16_t _COMPRESSION_CTRL0E = 0x440E;

const uint8_t _TEST_COLOR_BAR = 0xC0;
// Enable Color Bar roling Test

const uint8_t _AEC_PK_MANUAL_AGC_MANUALEN = 0x02;
// Enable AGC Manual enable
const uint8_t _AEC_PK_MANUAL_AEC_MANUALEN = 0x01;
// Enable AEC Manual enable

const uint8_t _TIMING_TC_REG20_VFLIP = 0x06;
// Vertical flip enable
const uint8_t _TIMING_TC_REG21_HMIRROR = 0x06;
// Horizontal mirror enable

const int _ASPECT_RATIO_4X3 = 0;
const int _ASPECT_RATIO_3X2 = 1;
const int _ASPECT_RATIO_16X10 = 2;
const int _ASPECT_RATIO_5X3 = 3;
const int _ASPECT_RATIO_16X9 = 4;
const int _ASPECT_RATIO_21X9 = 5;
const int _ASPECT_RATIO_5X4 = 6;
const int _ASPECT_RATIO_1X1 = 7;
const int _ASPECT_RATIO_9X16 = 8;

const int _resolution_info[][3] = {
  {96, 96, _ASPECT_RATIO_1X1},  // 96x96
  {160, 120, _ASPECT_RATIO_4X3},  // QQVGA
  {176, 144, _ASPECT_RATIO_5X4},  // QCIF
  {240, 176, _ASPECT_RATIO_4X3},  // HQVGA
  {240, 240, _ASPECT_RATIO_1X1},  // 240x240
  {320, 240, _ASPECT_RATIO_4X3},  // QVGA
  {400, 296, _ASPECT_RATIO_4X3},  // CIF
  {480, 320, _ASPECT_RATIO_3X2},  // HVGA
  {640, 480, _ASPECT_RATIO_4X3},  // VGA
  {800, 600, _ASPECT_RATIO_4X3},  // SVGA
  {1024, 768, _ASPECT_RATIO_4X3},  // XGA
  {1280, 720, _ASPECT_RATIO_16X9},  // HD
  {1280, 1024, _ASPECT_RATIO_5X4},  // SXGA
  {1600, 1200, _ASPECT_RATIO_4X3},  // UXGA
  {2560, 1440, _ASPECT_RATIO_16X9}, // QHD
  {2560, 1600, _ASPECT_RATIO_16X10}, // WQXGA
  {1088, 1920, _ASPECT_RATIO_9X16}, // Portrait FHD
  {2560, 1920, _ASPECT_RATIO_4X3}, // QSXGA
};

const int _ratio_table[][10] = {
  //  mw,   mh,  sx,  sy,   ex,   ey, ox, oy,   tx,   ty
  {2560, 1920, 0, 0, 2623, 1951, 32, 16, 2844, 1968},  // 4x3
  {2560, 1704, 0, 110, 2623, 1843, 32, 16, 2844, 1752},  // 3x2
  {2560, 1600, 0, 160, 2623, 1791, 32, 16, 2844, 1648},  // 16x10
  {2560, 1536, 0, 192, 2623, 1759, 32, 16, 2844, 1584},  // 5x3
  {2560, 1440, 0, 240, 2623, 1711, 32, 16, 2844, 1488},  // 16x9
  {2560, 1080, 0, 420, 2623, 1531, 32, 16, 2844, 1128},  // 21x9
  {2400, 1920, 80, 0, 2543, 1951, 32, 16, 2684, 1968},  // 5x4
  {1920, 1920, 320, 0, 2543, 1951, 32, 16, 2684, 1968},  // 1x1
  {1088, 1920, 736, 0, 1887, 1951, 32, 16, 1884, 1968},  // 9x16
};

const float _pll_pre_div2x_factors[] = {1, 1, 2, 3, 4, 1.5, 6, 2.5, 8};
const int _pll_pclk_root_div_factors[] = {1, 2, 4, 8};

const int _REG_DLY = 0xFFFF;
const int _REGLIST_TAIL = 0x0000;

const int _OV5640_STAT_FIRMWAREBAD = 0x7F;
const int _OV5640_STAT_STARTUP = 0x7E;
const int _OV5640_STAT_IDLE = 0x70;
const int _OV5640_STAT_FOCUSING = 0x00;
const int _OV5640_STAT_FOCUSED = 0x10;

const int _OV5640_CMD_TRIGGER_AUTOFOCUS = 0x03;
const int _OV5640_CMD_AUTO_AUTOFOCUS = 0x04;
const int _OV5640_CMD_RELEASE_FOCUS = 0x08;
const int _OV5640_CMD_AF_SET_VCM_STEP = 0x1A;
const int _OV5640_CMD_AF_GET_VCM_STEP = 0x1B;

const int _OV5640_CMD_MAIN = 0x3022;
const int _OV5640_CMD_ACK = 0x3023;
const int _OV5640_CMD_PARA0 = 0x3024;
const int _OV5640_CMD_PARA1 = 0x3025;
const int _OV5640_CMD_PARA2 = 0x3026;
const int _OV5640_CMD_PARA3 = 0x3027;
const int _OV5640_CMD_PARA4 = 0x3028;
const int _OV5640_CMD_FW_STATUS = 0x3029;

const uint16_t _sensor_default_regs[][2] = {
  {_SYSTEM_CTROL0, 0x82},  // software reset
  {_REG_DLY, 10},  // delay 10ms
  {_SYSTEM_CTROL0, 0x42},  // power down
  // enable pll
  {0x3103, 0x13},
  // io direction
  {0x3017, 0xFF},
  {0x3018, 0xFF},
  {_DRIVE_CAPABILITY, 0xC3},
  {_CLOCK_POL_CONTROL, 0x21},
  {0x4713, 0x02},  // jpg mode select
  {_ISP_CONTROL_01, 0x83},  // turn color matrix, awb and SDE
  // sys reset
  {_SYSTEM_RESET00, 0x00}, // enable all blocks
  {_SYSTEM_RESET02, 0x1C}, // reset jfifo, sfifo, jpg, fmux, avg
  // clock enable
  {0x3004, 0xFF},
  {_CLOCK_ENABLE02, 0xC3},
  // isp control
  {0x5000, 0xA7},
  {_ISP_CONTROL_01, 0xA3},  // +scaling?
  {0x5003, 0x08},  // special_effect
  // unknown
  {0x370C, 0x02},  //!!IMPORTANT
  {0x3634, 0x40},  //!!IMPORTANT
  // AEC/AGC
  {0x3A02, 0x03},
  {0x3A03, 0xD8},
  {0x3A08, 0x01},
  {0x3A09, 0x27},
  {0x3A0A, 0x00},
  {0x3A0B, 0xF6},
  {0x3A0D, 0x04},
  {0x3A0E, 0x03},
  {0x3A0F, 0x30},  // ae_level
  {0x3A10, 0x28},  // ae_level
  {0x3A11, 0x60},  // ae_level
  {0x3A13, 0x43},
  {0x3A14, 0x03},
  {0x3A15, 0xD8},
  {0x3A18, 0x00},  // gainceiling
  {0x3A19, 0xF8},  // gainceiling
  {0x3A1B, 0x30},  // ae_level
  {0x3A1E, 0x26},  // ae_level
  {0x3A1F, 0x14},  // ae_level
  // vcm debug
  {0x3600, 0x08},
  {0x3601, 0x33},
  // 50/60Hz
  {0x3C01, 0xA4},
  {0x3C04, 0x28},
  {0x3C05, 0x98},
  {0x3C06, 0x00},
  {0x3C07, 0x08},
  {0x3C08, 0x00},
  {0x3C09, 0x1C},
  {0x3C0A, 0x9C},
  {0x3C0B, 0x40},
  {0x460C, 0x22},  // disable jpeg footer
  // BLC
  {0x4001, 0x02},
  {0x4004, 0x02},
  // AWB
  {0x5180, 0xFF},
  {0x5181, 0xF2},
  {0x5182, 0x00},
  {0x5183, 0x14},
  {0x5184, 0x25},
  {0x5185, 0x24},
  {0x5186, 0x09},
  {0x5187, 0x09},
  {0x5188, 0x09},
  {0x5189, 0x75},
  {0x518A, 0x54},
  {0x518B, 0xE0},
  {0x518C, 0xB2},
  {0x518D, 0x42},
  {0x518E, 0x3D},
  {0x518F, 0x56},
  {0x5190, 0x46},
  {0x5191, 0xF8},
  {0x5192, 0x04},
  {0x5193, 0x70},
  {0x5194, 0xF0},
  {0x5195, 0xF0},
  {0x5196, 0x03},
  {0x5197, 0x01},
  {0x5198, 0x04},
  {0x5199, 0x12},
  {0x519A, 0x04},
  {0x519B, 0x00},
  {0x519C, 0x06},
  {0x519D, 0x82},
  {0x519E, 0x38},
  // color matrix (Saturation)
  {0x5381, 0x1E},
  {0x5382, 0x5B},
  {0x5383, 0x08},
  {0x5384, 0x0A},
  {0x5385, 0x7E},
  {0x5386, 0x88},
  {0x5387, 0x7C},
  {0x5388, 0x6C},
  {0x5389, 0x10},
  {0x538A, 0x01},
  {0x538B, 0x98},
  // CIP control (Sharpness)
  {0x5300, 0x10},  // sharpness
  {0x5301, 0x10},  // sharpness
  {0x5302, 0x18},  // sharpness
  {0x5303, 0x19},  // sharpness
  {0x5304, 0x10},
  {0x5305, 0x10},
  {0x5306, 0x08},  // denoise
  {0x5307, 0x16},
  {0x5308, 0x40},
  {0x5309, 0x10},  // sharpness
  {0x530A, 0x10},  // sharpness
  {0x530B, 0x04},  // sharpness
  {0x530C, 0x06},  // sharpness
  // GAMMA
  {0x5480, 0x01},
  {0x5481, 0x00},
  {0x5482, 0x1E},
  {0x5483, 0x3B},
  {0x5484, 0x58},
  {0x5485, 0x66},
  {0x5486, 0x71},
  {0x5487, 0x7D},
  {0x5488, 0x83},
  {0x5489, 0x8F},
  {0x548A, 0x98},
  {0x548B, 0xA6},
  {0x548C, 0xB8},
  {0x548D, 0xCA},
  {0x548E, 0xD7},
  {0x548F, 0xE3},
  {0x5490, 0x1D},
  // Special Digital Effects (SDE) (UV adjust)
  {0x5580, 0x06},  // enable brightness and contrast
  {0x5583, 0x40},  // special_effect
  {0x5584, 0x10},  // special_effect
  {0x5586, 0x20},  // contrast
  {0x5587, 0x00},  // brightness
  {0x5588, 0x00},  // brightness
  {0x5589, 0x10},
  {0x558A, 0x00},
  {0x558B, 0xF8},
  {0x501D, 0x40},  // enable manual offset of contrast
  // power on
  {0x3008, 0x02},
  // 50Hz
  {0x3C00, 0x04},
  //_REG_DLY, 300,
};

const uint16_t _reset_awb[][2] = {
  {_ISP_CONTROL_01, 0x83},  // turn color matrix, awb and SDE
  // sys reset
  {_SYSTEM_RESET00, 0x00}, // enable all blocks
  {_SYSTEM_RESET02, 0x1C}, // reset jfifo, sfifo, jpg, fmux, avg
  // clock enable
  //0x3004, 0xFF,
  //_CLOCK_ENABLE02, 0xC3,
  // isp control
  {0x5000, 0xA7},
  {_ISP_CONTROL_01, 0xA3},  // +scaling?
  {0x5003, 0x08},  // special_effect
  // unknown
  {0x370C, 0x02},  //!!IMPORTANT
  {0x3634, 0x40},  //!!IMPORTANT
  // AEC/AGC
  {0x3A02, 0x03},
  {0x3A03, 0xD8},
  {0x3A08, 0x01},
  {0x3A09, 0x27},
  {0x3A0A, 0x00},
  {0x3A0B, 0xF6},
  {0x3A0D, 0x04},
  {0x3A0E, 0x03},
  {0x3A0F, 0x30},  // ae_level
  {0x3A10, 0x28},  // ae_level
  {0x3A11, 0x60},  // ae_level
  {0x3A13, 0x43},
  {0x3A14, 0x03},
  {0x3A15, 0xD8},
  {0x3A18, 0x00},  // gainceiling
  {0x3A19, 0xF8},  // gainceiling
  {0x3A1B, 0x30},  // ae_level
  {0x3A1E, 0x26},  // ae_level
  {0x3A1F, 0x14},  // ae_level
  // vcm debug
  {0x3600, 0x08},
  {0x3601, 0x33},
  // 50/60Hz
  {0x3C01, 0xA4},
  {0x3C04, 0x28},
  {0x3C05, 0x98},
  {0x3C06, 0x00},
  {0x3C07, 0x08},
  {0x3C08, 0x00},
  {0x3C09, 0x1C},
  {0x3C0A, 0x9C},
  {0x3C0B, 0x40},
  {0x460C, 0x22},  // disable jpeg footer
  // BLC
  {0x4001, 0x02},
  {0x4004, 0x02},
  // AWB
  {0x5180, 0xFF},
  {0x5181, 0xF2},
  {0x5182, 0x00},
  {0x5183, 0x14},
  {0x5184, 0x25},
  {0x5185, 0x24},
  {0x5186, 0x09},
  {0x5187, 0x09},
  {0x5188, 0x09},
  {0x5189, 0x75},
  {0x518A, 0x54},
  {0x518B, 0xE0},
  {0x518C, 0xB2},
  {0x518D, 0x42},
  {0x518E, 0x3D},
  {0x518F, 0x56},
  {0x5190, 0x46},
  {0x5191, 0xF8},
  {0x5192, 0x04},
  {0x5193, 0x70},
  {0x5194, 0xF0},
  {0x5195, 0xF0},
  {0x5196, 0x03},
  {0x5197, 0x01},
  {0x5198, 0x04},
  {0x5199, 0x12},
  {0x519A, 0x04},
  {0x519B, 0x00},
  {0x519C, 0x06},
  {0x519D, 0x82},
  {0x519E, 0x38},
};

const uint16_t _sensor_format_jpeg[][2] = {
  {_FORMAT_CTRL, 0x00},  // YUV422
  {_FORMAT_CTRL00, 0x30},  // YUYV
  {_SYSTEM_RESET02, 0x00},  // enable everything
  {_CLOCK_ENABLE02, 0xFF},  // enable all clocks
  {0x471C, 0x50},  // 0xd0 to 0x50 !!!
};

const uint16_t _sensor_format_raw[][2] = {
  {_FORMAT_CTRL, 0x03},  // RAW (DPC)
  {_FORMAT_CTRL00, 0x00},  // RAW
};

const uint16_t _sensor_format_grayscale[][2] = {
  {_FORMAT_CTRL, 0x00},  // YUV422
  {_FORMAT_CTRL00, 0x10},  // Y8
};

const uint16_t _sensor_format_yuv422[][2] = {
  {_FORMAT_CTRL, 0x00},  // YUV422
  {_FORMAT_CTRL00, 0x30},  // YUYV
};

const uint16_t _sensor_format_rgb565[][2] = {
  {_FORMAT_CTRL, 0x01},  // RGB
  {_FORMAT_CTRL00, 0x61},  // RGB565 (BGR)
  {_SYSTEM_RESET02, 0x1C}, // reset jfifo, sfifo, jpg, fmux, avg
  {_CLOCK_ENABLE02, 0xC3}, // reset to how it was before (no jpg clock)
};

#define NUM_SENSOR_CONTRAST_LEVELS (7)
const uint16_t _contrast_settings[NUM_SENSOR_CONTRAST_LEVELS][2] = {
  {0x20, 0x00}, //  0
  {0x24, 0x10}, // +1
  {0x28, 0x18}, // +2
  {0x2c, 0x1c}, // +3
  {0x14, 0x14}, // -3
  {0x18, 0x18}, // -2
  {0x1c, 0x1c}, // -1
};

#define NUM_SENSOR_SATURATION_LEVELS (9)
#define NUM_SAT_VALUES_PER_LEVEL (11)
const uint16_t _sensor_saturation_levels[NUM_SENSOR_SATURATION_LEVELS][NUM_SAT_VALUES_PER_LEVEL] = {
  {0x1D, 0x60, 0x03, 0x0C, 0x78, 0x84, 0x7D, 0x6B, 0x12, 0x01, 0x98},  // 0
  {0x1D, 0x60, 0x03, 0x0D, 0x84, 0x91, 0x8A, 0x76, 0x14, 0x01, 0x98},  // +1
  {0x1D, 0x60, 0x03, 0x0E, 0x90, 0x9E, 0x96, 0x80, 0x16, 0x01, 0x98},  // +2
  {0x1D, 0x60, 0x03, 0x10, 0x9C, 0xAC, 0xA2, 0x8B, 0x17, 0x01, 0x98},  // +3
  {0x1D, 0x60, 0x03, 0x11, 0xA8, 0xB9, 0xAF, 0x96, 0x19, 0x01, 0x98},  // +4
  {0x1D, 0x60, 0x03, 0x07, 0x48, 0x4F, 0x4B, 0x40, 0x0B, 0x01, 0x98},  // -4
  {0x1D, 0x60, 0x03, 0x08, 0x54, 0x5C, 0x58, 0x4B, 0x0D, 0x01, 0x98},  // -3
  {0x1D, 0x60, 0x03, 0x0A, 0x60, 0x6A, 0x64, 0x56, 0x0E, 0x01, 0x98},  // -2
  {0x1D, 0x60, 0x03, 0x0B, 0x6C, 0x77, 0x70, 0x60, 0x10, 0x01, 0x98},  // -1
};

#define NUM_SENSOR_EV_LEVELS (7)
#define NUM_EV_VALUES_PER_LEVEL (6)
const uint16_t _sensor_ev_levels[NUM_SENSOR_EV_LEVELS][NUM_EV_VALUES_PER_LEVEL] = {
  {0x38, 0x30, 0x61, 0x38, 0x30, 0x10}, //  0
  {0x40, 0x38, 0x71, 0x40, 0x38, 0x10}, // +1
  {0x50, 0x48, 0x90, 0x50, 0x48, 0x20}, // +2
  {0x60, 0x58, 0xa0, 0x60, 0x58, 0x20}, // +3
  {0x10, 0x08, 0x10, 0x08, 0x20, 0x10}, // -3
  {0x20, 0x18, 0x41, 0x20, 0x18, 0x10}, // -2
  {0x30, 0x28, 0x61, 0x30, 0x28, 0x10}, // -1
};

#define NUM_WHITE_BALANCE_LEVELS (5)
#define NUM_WHITE_BALANCE_PER_LEVEL (7)
const uint16_t _light_registers[NUM_WHITE_BALANCE_PER_LEVEL] = {0x3406, 0x3400, 0x3401, 0x3402, 0x3403, 0x3404, 0x3405};
const uint16_t _light_modes[NUM_WHITE_BALANCE_LEVELS][NUM_WHITE_BALANCE_PER_LEVEL] = {
  {0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00}, // auto
  {0x01, 0x06, 0x1c, 0x04, 0x00, 0x04, 0xf3}, // sunny
  {0x01, 0x05, 0x48, 0x04, 0x00, 0x07, 0xcf}, // office / fluorescent
  {0x01, 0x06, 0x48, 0x04, 0x00, 0x04, 0xd3}, // cloudy
  {0x01, 0x04, 0x10, 0x04, 0x00, 0x08, 0x40}, // home / incandescent
};

const uint16_t _sensor_special_effects[][4] = {
  {0x06, 0x40, 0x10, 0x08},  // Normal
  {0x46, 0x40, 0x28, 0x08},  // Negative
  {0x1E, 0x80, 0x80, 0x08},  // Grayscale
  {0x1E, 0x80, 0xC0, 0x08},  // Red Tint
  {0x1E, 0x60, 0x60, 0x08},  // Green Tint
  {0x1E, 0xA0, 0x40, 0x08},  // Blue Tint
  {0x1E, 0x40, 0xA0, 0x08},  // Sepia
};

const uint16_t _sensor_regs_gamma0[][2] = {
  {0x5480, 0x01},
  {0x5481, 0x08},
  {0x5482, 0x14},
  {0x5483, 0x28},
  {0x5484, 0x51},
  {0x5485, 0x65},
  {0x5486, 0x71},
  {0x5487, 0x7D},
  {0x5488, 0x87},
  {0x5489, 0x91},
  {0x548A, 0x9A},
  {0x548B, 0xAA},
  {0x548C, 0xB8},
  {0x548D, 0xCD},
  {0x548E, 0xDD},
  {0x548F, 0xEA},
  {0x5490, 0x1D},
};

const uint16_t _sensor_regs_gamma1[][2] = {
  {0x5480, 0x1},
  {0x5481, 0x0},
  {0x5482, 0x1E},
  {0x5483, 0x3B},
  {0x5484, 0x58},
  {0x5485, 0x66},
  {0x5486, 0x71},
  {0x5487, 0x7D},
  {0x5488, 0x83},
  {0x5489, 0x8F},
  {0x548A, 0x98},
  {0x548B, 0xA6},
  {0x548C, 0xB8},
  {0x548D, 0xCA},
  {0x548E, 0xD7},
  {0x548F, 0xE3},
  {0x5490, 0x1D},
};

const uint16_t _sensor_regs_awb0[][2] = {
  {0x5180, 0xFF},
  {0x5181, 0xF2},
  {0x5182, 0x00},
  {0x5183, 0x14},
  {0x5184, 0x25},
  {0x5185, 0x24},
  {0x5186, 0x09},
  {0x5187, 0x09},
  {0x5188, 0x09},
  {0x5189, 0x75},
  {0x518A, 0x54},
  {0x518B, 0xE0},
  {0x518C, 0xB2},
  {0x518D, 0x42},
  {0x518E, 0x3D},
  {0x518F, 0x56},
  {0x5190, 0x46},
  {0x5191, 0xF8},
  {0x5192, 0x04},
  {0x5193, 0x70},
  {0x5194, 0xF0},
  {0x5195, 0xF0},
  {0x5196, 0x03},
  {0x5197, 0x01},
  {0x5198, 0x04},
  {0x5199, 0x12},
  {0x519A, 0x04},
  {0x519B, 0x00},
  {0x519C, 0x06},
  {0x519D, 0x82},
  {0x519E, 0x38},
};

SparkFun_iCap_OV5640::SparkFun_iCap_OV5640(OV5640_pins &pins, iCap_arch *arch,
                                           TwoWire &twi, uint16_t *pbuf,
                                           uint32_t pbufsize, uint8_t addr,
                                           uint32_t speed, uint32_t delay_us)
    : Adafruit_iCap_parallel((iCap_parallel_pins *)&pins, arch, pbuf, pbufsize,
                             (TwoWire *)&twi, addr, speed, delay_us) {}

SparkFun_iCap_OV5640::~SparkFun_iCap_OV5640() {}

// CAMERA STARTUP ----------------------------------------------------------

iCap_status SparkFun_iCap_OV5640::begin(void) {
  iCap_status status;

  // Initialize peripherals for parallel+I2C camera:
  status = Adafruit_iCap_parallel::begin();
  if (status != ICAP_STATUS_OK) {
    return status;
  }

  // ENABLE AND/OR RESET CAMERA --------------------------------------------

  if (pins.enable >= 0) { // Enable pin defined?
    pinMode(pins.enable, OUTPUT);
    digitalWrite(pins.enable, 0); // PWDN low (enable)
  }

  // Unsure of camera startup time from beginning of input clock.
  // Let's guess it's similar to tS:REG (300 ms) from datasheet.
  // delayMicroseconds(300000);

  // Read manufacturer and product IDs -- these are Bank 1 registers
  uint16_t chip_id = (readRegister16(_CHIP_ID_HIGH) << 8) |
                      readRegister16(_CHIP_ID_HIGH+1);
  if(chip_id != 0x5640) {
    return ICAP_STATUS_ERR_CAMERA_ID;
  }

  // if (pins.reset >= 0) { // Hard reset pin defined?
  //   pinMode(pins.reset, OUTPUT);
  //   digitalWrite(pins.reset, LOW);
  //   delayMicroseconds(1000);
  //   digitalWrite(pins.reset, HIGH);
  // } else {                                                    // Soft reset
    // writeRegister16(OV5640_REG_RA_DLMT, OV5640_RA_DLMT_SENSOR); // Bank select 1
    // writeRegister16(OV5640_REG1_COM7, OV5640_COM7_SRST);        // System reset
  // }
  // delay(1); // Datasheet: tS:RESET = 1 ms

  // Init main camera settings
  writeList16(_sensor_default_regs, sizeof _sensor_default_regs / sizeof _sensor_default_regs[0]);

  // Further initialization for specific colorspaces, frame sizes, timing,
  // etc. are done in other functions.

  return ICAP_STATUS_OK;
}

iCap_status SparkFun_iCap_OV5640::begin(OV5640_size size, iCap_colorspace space,
                                        float fps, uint8_t nbuf) {
  iCap_status status = begin();
  if (status == ICAP_STATUS_OK) {
    status = config(size, space, fps, nbuf);
    if (status == ICAP_STATUS_OK) {
      resume();
    }
  }

  return status;
}

void SparkFun_iCap_OV5640::_set_image_options() {
  uint8_t reg20 = 0;
  uint8_t reg21 = 0;
  uint8_t reg4514 = 0;
  uint8_t reg4514_test = 0;

  // if (colorspace == OV5640_COLOR_JPEG) {
  //   reg21 |= 0x20;
  // }

  if (_binning) {
    reg20 |= 1;
    reg21 |= 1;
    reg4514_test |= 4;
  } else {
    reg20 |= 0x40;
  }

  if (_flip_y) {
    reg20 |= _TIMING_TC_REG20_VFLIP;
    reg4514_test |= 1;
  }

  if (_flip_x) {
    reg21 |= _TIMING_TC_REG21_HMIRROR;
    reg4514_test |= 2;
  }

  if (reg4514_test == 0) {
    reg4514 = 0x88;
  } else if (reg4514_test == 1) {
    reg4514 = 0x00;
  } else if (reg4514_test == 2) {
    reg4514 = 0xBB;
  } else if (reg4514_test == 3) {
    reg4514 = 0x00;
  } else if (reg4514_test == 4) {
    reg4514 = 0xAA;
  } else if (reg4514_test == 5) {
    reg4514 = 0xBB;
  } else if (reg4514_test == 6) {
    reg4514 = 0xBB;
  } else if (reg4514_test == 7) {
    reg4514 = 0xAA;
  }

  writeRegister16(_TIMING_TC_REG20, reg20);
  writeRegister16(_TIMING_TC_REG21, reg21);
  writeRegister16(0x4514, reg4514);

  if (_binning) {
    writeRegister16(0x4520, 0x0B);
    writeRegister16(_X_INCREMENT, 0x31);
    writeRegister16(_Y_INCREMENT, 0x31);
  } else {
    writeRegister16(0x4520, 0x10);
    writeRegister16(_X_INCREMENT, 0x11);
    writeRegister16(_Y_INCREMENT, 0x11);
  }
}

void SparkFun_iCap_OV5640::flip(bool flip_x, bool flip_y){
  _flip_x = flip_x;
  _flip_y = flip_y;

  // Alternatively could read reg20, reg21, and reg4514 and only modify the bits we need if these are ever modified external to that function...
  _set_image_options(); 
}

void SparkFun_iCap_OV5640::setNight(bool enable_night){
  _write_reg_bits(_AEC_POWER_DOMAIN, _AEC_POWER_DOMAIN_NIGHT_MASK, enable_night);
}

void SparkFun_iCap_OV5640::_set_colorspace(iCap_colorspace colorspace) {
  _colorspace = colorspace;

  switch(colorspace){
    case ICAP_COLOR_RGB565:
      writeList16(_sensor_format_rgb565, sizeof _sensor_format_rgb565 / sizeof _sensor_format_rgb565[0]);
      break;
    case ICAP_COLOR_YUV:
      writeList16(_sensor_format_yuv422, sizeof _sensor_format_yuv422 / sizeof _sensor_format_yuv422[0]);
      break;
  }
}

void SparkFun_iCap_OV5640::testPattern(bool enable){
  _test_pattern = enable;
  writeRegister16(_PRE_ISP_TEST_SETTING_1, enable ? 1 << 7 : 0);
}

void SparkFun_iCap_OV5640::setSaturation(int sat_level){
  // Confirm sat_level is within bounds
  if ( (sat_level > 4) || (sat_level < -4) ) {
    return;
  }

  _saturation = sat_level;

  // In a bit of a weird form as original library used python indexing:   
  if (sat_level < 0)
    sat_level += NUM_SENSOR_SATURATION_LEVELS; //  converts sat level (-4 to -1) to index into _sensor_saturation_levels(5 to 8)
  
  for (int offset = 0 ; offset < NUM_SAT_VALUES_PER_LEVEL; offset++){
    writeRegister16(0x5381 + offset, _sensor_saturation_levels[sat_level][offset]);
  }
}

void SparkFun_iCap_OV5640::setContrast(int contrast_level){
  // Confirm contrast_level is within bounds
  if ( (contrast_level > 3) || (contrast_level < -3) ) {
    return;
  }

    _contrast = contrast_level;

  // In a bit of a weird form as original library used python indexing:
  if (contrast_level < 0)
    contrast_level += NUM_SENSOR_CONTRAST_LEVELS; //  converts contrast level (-3 to -1) to index into _contrast_settings(4 to 6)

  const uint16_t contrast_settings_to_write[][2] = {
    {0x5586, _contrast_settings[contrast_level][0]},
    {0x5585, _contrast_settings[contrast_level][1]},
  };

  _write_group_3_settings(contrast_settings_to_write, 2);  
}

void SparkFun_iCap_OV5640::setSpecialEffect(OV5640_special_effect value){
  _effect = value;
  
  uint16_t special_effect_regs[] = {0x5580, 0x5583, 0x5584, 0x5003};

  for (int i = 0 ; i < 4; i++){
    writeRegister16(special_effect_regs[i], _sensor_special_effects[value][i]);
  }
}

void SparkFun_iCap_OV5640::setExposure(int exposure_level){
  // Confirm exposure_level is within bounds
  if ( (exposure_level > 3) || (exposure_level < -3) ) {
    return;
  }

  _exposure = exposure_level;

  // In a bit of a weird form as original library used python indexing:
  if (exposure_level < 0)
    exposure_level += NUM_SENSOR_EV_LEVELS; //  converts exposure level (-3 to -1) to index into _sensor_ev_levels(4 to 6)

  for (int offset = 0 ; offset < NUM_EV_VALUES_PER_LEVEL; offset++){
    writeRegister16(0x5381 + offset, _sensor_ev_levels[exposure_level][offset]);
  }
}

void SparkFun_iCap_OV5640::setBrightness(int brightness_level){
  // Confirm brightness_level is within bounds
  if ( (brightness_level > 4) || (brightness_level < -4) ) {
    return;
  }

  _brightness = brightness_level;
  
  // Abs value of brightness_level
  if (brightness_level < 0)
    brightness_level *= -1; 

  const uint16_t brightness_to_write = (uint16_t)brightness_level << 4;
  const uint16_t sign_to_write = _brightness < 0 ? 0x9 : 0x01;

  const uint16_t brightness_settings_to_write[][2] = {
    {0x5587, brightness_to_write},
    {0x5588, sign_to_write}
  };

  _write_group_3_settings(brightness_settings_to_write, 2);
}

void SparkFun_iCap_OV5640::setWhiteBalance(OV5640_white_balance white_balance){
  writeRegister16(0x3212, 0x03); // Start group 3
  for (int i = 0; i < NUM_WHITE_BALANCE_PER_LEVEL; i++){
    writeRegister16(_light_registers[i], _light_modes[white_balance][i]);
  }

  writeRegister16(0x3212, 0x13); // End group 3
  writeRegister16(0x3212, 0xA3); //launch group 3
}

void SparkFun_iCap_OV5640::_set_pll(bool bypass, int multiplier, int sys_div, int pre_div, bool root_2x, int pclk_root_div, bool pclk_manual, int pclk_div)
{
  if (
    multiplier > 252 ||
    multiplier < 4 ||
    sys_div > 15 ||
    pre_div > 8 ||
    pclk_div > 31 ||
    pclk_root_div > 3
  ) {
    return;
  }

  writeRegister16(0x3039, bypass ? 0x80 : 0);
  writeRegister16(0x3034, 0x1A);
  writeRegister16(0x3035, 1 | ((sys_div & 0xF) << 4));
  writeRegister16(0x3036, multiplier & 0xFF);
  writeRegister16(0x3037, (pre_div & 0xF) | (root_2x ? 0x10 : 0));
  writeRegister16(0x3108, (pclk_root_div & 3) << 4 | 0x06);
  writeRegister16(0x3824, pclk_div & 0x1F);
  writeRegister16(0x460C, pclk_manual ? 0x22 : 0x22); //TODO: this is how the adafruit lib does it, should we instead write 0x20 if pclk_manual is false?
  writeRegister16(0x3103, 0x13);
}

void SparkFun_iCap_OV5640::_set_size_and_colorspace(OV5640_size size, iCap_colorspace colorspace)
{
  // size = _size;
  uint16_t width, height, ratio;
  width = _resolution_info[size][0];
  height = _resolution_info[size][1];
  ratio = _resolution_info[size][2];
  // _w = width;
  // _h = height;
  uint16_t max_width, max_height, start_x, start_y, end_x, end_y, offset_x, offset_y, total_x, total_y;
  // std::tie(max_width, max_height, start_x, start_y, end_x, end_y, offset_x, offset_y, total_x, total_y) = _ratio_table[ratio];
  max_width = _ratio_table[ratio][0];
  max_height = _ratio_table[ratio][1];
  start_x = _ratio_table[ratio][2];
  start_y = _ratio_table[ratio][3];
  end_x = _ratio_table[ratio][4];
  end_y = _ratio_table[ratio][5];
  offset_x = _ratio_table[ratio][6];
  offset_y = _ratio_table[ratio][7];
  total_x = _ratio_table[ratio][8];
  total_y = _ratio_table[ratio][9];

  _binning = (width <= max_width / 2) && (height <= max_height / 2);
  _scale = !((width == max_width && height == max_height) || (width == max_width / 2 && height == max_height / 2));

  _write_addr_reg(_X_ADDR_ST_H, start_x, start_y);
  _write_addr_reg(_X_ADDR_END_H, end_x, end_y);
  _write_addr_reg(_X_OUTPUT_SIZE_H, width, height);

  if (!_binning) {
    _write_addr_reg(_X_TOTAL_SIZE_H, total_x, total_y);
    _write_addr_reg(_X_OFFSET_H, offset_x, offset_y);
  }
  else {
    if (width > 920) {
      _write_addr_reg(_X_TOTAL_SIZE_H, total_x - 200, total_y / 2);
    }
    else {
      _write_addr_reg(_X_TOTAL_SIZE_H, 2060, total_y / 2);
    }
    _write_addr_reg(_X_OFFSET_H, offset_x / 2, offset_y / 2);
  }

  _write_reg_bits(_ISP_CONTROL_01, 0x20, _scale);

  _set_image_options();

  // if (colorspace == OV5640_COLOR_JPEG) {
  //   uint16_t sys_mul = 200;
  //   if (size < OV5640_SIZE_QVGA) {
  //     sys_mul = 160;
  //   }
  //   if (size < OV5640_SIZE_XGA) {
  //     sys_mul = 180;
  //   }
  //   _set_pll(false, sys_mul, 4, 2, false, 2, true, 4);
  // }
  // else {
    _set_pll(false, 40, 1, 1, false, 1, true, 4);
  // }

  _set_colorspace(colorspace);
}

void SparkFun_iCap_OV5640::setColorspace(iCap_colorspace space) {
  // self._colorspace = colorspace
  // self._set_size_and_colorspace()
}

// void SparkFun_iCap_OV5640::setSize(OV5640_size size) {
//   // self._size = size
//   // self._set_size_and_colorspace()
// }

iCap_status SparkFun_iCap_OV5640::config(OV5640_size size,
                                         iCap_colorspace space, float fps,
                                         uint8_t nbuf, iCap_realloc allo) {
  // RIGGED FOR QQVGA FOR NOW, 30 fps
  uint16_t width = 320;
  uint16_t height = 240;
  iCap_status status = bufferConfig(width, height, space, nbuf, allo);
  if (status == ICAP_STATUS_OK) {
    // writeList(OV5640_qqvga, sizeof OV5640_qqvga / sizeof OV5640_qqvga[0]);
    
    // setColorspace(OV5640_COLOR_RGB);
    // _flip_x = False
    // _flip_y = False
    // _w = None
    // _h = None
    // _size = None
    // _test_pattern = False
    // _binning = False
    // _scale = False
    // _ev = 0
    // _white_balance = 0
    // setSize(size);
    _set_size_and_colorspace(OV5640_SIZE_QVGA, space);
    // _set_pll(false, 32, 1, 1, false, 1, true, 4);
    // _set_pll(false, 32, 1, 1, false, 1, true, 4);
    writeRegister16(_PRE_ISP_TEST_SETTING_1, 0 << 7);//Test pattern

    if (fps > 0.0) {
      delayMicroseconds((int)(10000000.0 / fps)); // 10 frame settling time
    }
    dma_change(pixbuf[0], _width * _height);
    resume(); // Start DMA cycle
  }

  return status;
}

int SparkFun_iCap_OV5640::readRegister16(uint16_t reg)
{
  wire->beginTransmission(i2c_address);
  wire->write((reg) >> 8) & 0xff;
  wire->write((reg) >> 0) & 0xff;
  wire->endTransmission();
  wire->requestFrom(i2c_address, (uint8_t)1);
  return wire->read();
}

void SparkFun_iCap_OV5640::writeRegister16(uint16_t reg, uint8_t value)
{
  wire->beginTransmission(i2c_address);
  wire->write((reg) >> 8) & 0xff;
  wire->write((reg) >> 0) & 0xff;
  wire->write(value);
  wire->endTransmission();
}

void SparkFun_iCap_OV5640::writeList16(const uint16_t cfg[][2], uint16_t len)
{
  for (int i = 0; i < len; i++) {
    
    if(cfg[i][0] == _REG_DLY) {
      delay(cfg[i][1]);
    }
    else{
      writeRegister16(cfg[i][0], cfg[i][1]);
      delayMicroseconds(i2c_delay_us); // Some cams require, else lockup on init
    }
  }
}

void SparkFun_iCap_OV5640::_write_addr_reg(uint16_t reg, uint16_t x_value, uint16_t y_value) {
  writeRegister16(reg + 0, (x_value >> 8) & 0xff);
  writeRegister16(reg + 1, (x_value >> 0) & 0xff);
  writeRegister16(reg + 2, (y_value >> 8) & 0xff);
  writeRegister16(reg + 3, (y_value >> 0) & 0xff);
}

void SparkFun_iCap_OV5640::_write_reg_bits(uint16_t reg, uint16_t mask, bool enable) {
  uint16_t val = readRegister16(reg);
  if (enable) {
    val |= mask;
  } else {
    val &= ~mask;
  }
  writeRegister16(reg, val);
}

void SparkFun_iCap_OV5640::_write_group_3_settings(const uint16_t cfg[][2], uint16_t len) {
  writeRegister16(0x3212, 0x03); // Start group 3
  writeList16(cfg, len);
  writeRegister16(0x3212, 0x13); // End group 3
  writeRegister16(0x3212, 0xA3); //launch group 3
} 

#endif // end ICAP_FULL_SUPPORT
