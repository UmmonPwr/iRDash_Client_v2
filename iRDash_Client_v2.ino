#include "SPI.h"
#include "Arduino_GFX_Library.h"
#include "XPT2046_Touchscreen.h"

// pin definitions for the display controller
#define TFT_DC   32
#define TFT_CS   22
#define TFT_MOSI 33
#define TFT_CLK  25
#define TFT_RST  21
#define TFT_MISO 15
#define TFT_LED  26
#define TOUCH_CS  5
#define TOUCH_IRQ 4

// calibration values for the touch position
// may be different for a different display
#define TOUCH_X_MIN 160
#define TOUCH_X_MAX 3856
#define TOUCH_Y_MIN 266
#define TOUCH_Y_MAX 3895

// define car identification numbers
#define NUMOFCARS  8    // number of car profiles, maximum is 16
#define NUMOFGEARS 10   // maximum number of gears including reverse and neutral
#define DEFAULTCAR 5    // car profile to show at startup
#define ID_Skippy  0    // Skip Barber
#define ID_CTS_V   1    // Cadillac CTS-V
#define ID_MX5_NC  2    // Mazda MX5 NC
#define ID_MX5_ND  3    // Mazda MX5 ND
#define ID_FR20    4    // Formula Renault 2.0
#define ID_DF3     5    // Dallara Formula 3
#define ID_992_CUP 6    // Porsche 911 GT3 cup (992)
#define ID_GR86    7    // Toyota GR86

// define the drawing colors, rgb components in 8 bit range in comment
// color value in 16 bit; 5 bit red, 6 bit green, 5 bit blue format
#define dc 26604       // default color;     red: 100 (01100), green: 255 (111111), blue: 100 (01100)
#define mc 65408       // middle color;      red: 255 (11111), green: 240 (111100), blue: 0   (00000)
#define wc 64300       // warning color;     red: 255 (11111), green: 100 (011001), blue: 100 (01100)
#define bc 17832       // background color;  red: 70  (01000), green: 180 (101101), blue: 70  (01000)
#define icon_ok  32768 //                    red: 135 (10000), green: 0   (000000), blue: 0   (00000)
#define icon_nok 64300 //                    red: 255 (11111), green: 100  (011001), blue: 100  (01100)

// define the icon bitmaps
const unsigned char fuelpressure [128] = {
  // 'fuelpressure'
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xcc, 0x00, 0x00, 0xff, 0xce, 0x00, 0x00, 0xc0, 0x67, 0x00, 
  0x00, 0xc0, 0x63, 0x00, 0x00, 0xc0, 0x63, 0x80, 0x00, 0xc0, 0x63, 0x80, 0x00, 0xc0, 0x63, 0x00, 
  0x00, 0xff, 0xf3, 0x00, 0x00, 0xff, 0xf9, 0x00, 0x00, 0xff, 0xd9, 0x80, 0x00, 0xff, 0xd9, 0x80, 
  0x00, 0xff, 0xd9, 0x80, 0x00, 0xff, 0xd8, 0x80, 0x00, 0xff, 0xd8, 0xc0, 0x00, 0xff, 0xd8, 0xc0, 
  0x00, 0xff, 0xcc, 0xc0, 0x00, 0xff, 0xcf, 0x80, 0x00, 0xff, 0xc2, 0x00, 0x01, 0xff, 0xe0, 0x00, 
  0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 
  0x19, 0xb9, 0xcf, 0x78, 0x1f, 0xbb, 0x6c, 0x60, 0x1f, 0x37, 0xee, 0x70, 0x18, 0x37, 0x03, 0x18, 
  0x18, 0x33, 0xcf, 0x78, 0x18, 0x21, 0xce, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const unsigned char oilpressure [128] = {
  // 'oilpressure'
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x3c, 0x7c, 0x00, 0x00, 0x7f, 0x3c, 0x00, 0x00, 0x73, 0x18, 0x00, 0x3c, 
  0x7f, 0xff, 0x83, 0xfc, 0x1f, 0xff, 0xff, 0xc4, 0x03, 0x00, 0xff, 0x80, 0x03, 0x00, 0x43, 0x00, 
  0x03, 0x00, 0x06, 0x06, 0x03, 0x00, 0x0e, 0x07, 0x03, 0xff, 0xfc, 0x06, 0x03, 0xff, 0xf8, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 
  0x19, 0xb9, 0xcf, 0x78, 0x1f, 0x3b, 0x6c, 0x60, 0x1f, 0x37, 0xee, 0x70, 0x18, 0x33, 0x03, 0x18, 
  0x18, 0x33, 0xcf, 0x78, 0x18, 0x21, 0xce, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const unsigned char stall [128] = {
  // 'stall'
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0x80, 0x00, 
  0x00, 0x03, 0xc0, 0x00, 0x00, 0x06, 0xc0, 0x00, 0x00, 0x0e, 0x60, 0x00, 0x00, 0x0c, 0x60, 0x00, 
  0x00, 0x19, 0x30, 0x00, 0x00, 0x19, 0xb8, 0x00, 0x00, 0x33, 0x98, 0x00, 0x00, 0x77, 0xcc, 0x00, 
  0x00, 0x66, 0xcc, 0x00, 0x00, 0xce, 0x66, 0x00, 0x00, 0xcc, 0x66, 0x00, 0x01, 0x98, 0x33, 0x00, 
  0x01, 0x9f, 0xfb, 0x80, 0x03, 0x3f, 0xf9, 0x80, 0x07, 0x00, 0x00, 0xc0, 0x06, 0x00, 0x00, 0xc0, 
  0x0f, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const unsigned char water [128] = {
  // 'water'
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xfe, 0x00, 
  0x00, 0x01, 0xfe, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xfe, 0x00, 
  0x00, 0x01, 0xfe, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xfe, 0x00, 
  0x00, 0x01, 0xfe, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 
  0x00, 0x03, 0xe0, 0x00, 0x1f, 0x33, 0xee, 0x78, 0x1f, 0xf3, 0xef, 0xfc, 0x01, 0xe1, 0xc3, 0x80, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x1e, 0x3c, 0xf0, 0x0f, 0xff, 0xff, 0xf8, 0x01, 0xe3, 0xc7, 0x80, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

Arduino_DataBus *bus = new Arduino_SWSPI(TFT_DC, TFT_CS, TFT_CLK, TFT_MOSI, TFT_MISO);
Arduino_GFX *tft = new Arduino_ILI9341(bus, TFT_RST);

XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);  // the default SPI pins will be used for the communication

// bit fields of engine warnings for reference
/*enum irsdk_EngineWarnings 
{
  irsdk_waterTempWarning    = 0x01,
  irsdk_fuelPressureWarning = 0x02,
  irsdk_oilPressureWarning  = 0x04,
  irsdk_engineStalled     = 0x08,
  irsdk_pitSpeedLimiter   = 0x10,
  irsdk_revLimiterActive    = 0x20,
}*/

// structure of the incoming serial data block
// variables are aligned for 16/32 bit systems to prevent padding
struct SIncomingData
{
  char EngineWarnings;
  char Gear;
  bool IsInGarage;
  bool IsOnTrack;
  float Fuel;
  float RPM;
  float Speed;
  float WaterTemp;
};

// structure to store the actual and the previous screen data
struct SViewData
{
  char EngineWarnings;
  int Fuel;
  char Gear;
  int RPMgauge;
  int Speed;
  int SLI;
  int WaterTemp;
};

// structure to store the screen layout of the gauges
struct SScreenLayout
{
  int EngineWarningsPosX;
  int EngineWarningsPosY;
  bool ShowEngineWarnings;
  int FuelPosX;
  int FuelPosY;
  bool ShowFuel;
  int GearPosX;
  int GearPosY;
  bool ShowGear;
  int RPMPosY;
  bool ShowRPM;
  int SLIPosY;
  bool ShowSLI;
  int SpeedPosX;
  int SpeedPosY;
  bool ShowSpeed;
  int WaterTempPosX;
  int WaterTempPosY;
  bool ShowWaterTemp;
};

// limits where different drawing color have to be used
struct SCarProfile
{
  char CarName[10];
  int Fuel;                   // value which below warning color drawed (value in liter * 10)
  int RPM;                    // value where the redline starts in pixels
  float RPMscale;             // actual RPM value is divided by this number to scale it to display coordinate
  int WaterTemp;              // value in Celsius
  int SLI[NUMOFGEARS][8];     // RPM values for each SLI light
};

// structure to store the button layout
struct SButton
{
  int x;
  int y;
  int Width;
  int Height;
  int TextX;
  int TextY;
};

char inByte;                  // incoming byte from the serial port
int blockpos;                 // position of the actual incoming byte in the telemetry data block
SIncomingData* InData;        // pointer to access the telemetry data as a structure
char* pInData;                // pointer to access the telemetry data as a byte array
SViewData Screen[2];          // store the actual and the previous screen data
char ActiveCar;               // active car profile

// variables to manage screen layout
SScreenLayout ScreenLayout;            // store screen layout
SCarProfile CarProfile[NUMOFCARS];     // store warning limits for each car
SButton Button[NUMOFCARS];             // store car selection button layout

// clear our internal data block
void ResetInternalData()
{
  Screen[0].EngineWarnings = 0;
  Screen[0].Fuel = 0;
  Screen[0].Gear = -1;
  Screen[0].RPMgauge = 0;
  Screen[0].Speed = -1;
  Screen[0].SLI = 0;
  Screen[0].WaterTemp = 0;
  blockpos = 0;
}

// upload the screen layout and warning limits per car
// upload car selection button layout
void UploadProfiles()
{
  // upload gauge positions
  ScreenLayout.EngineWarningsPosX = 0;
  ScreenLayout.EngineWarningsPosY = 207;
  ScreenLayout.ShowEngineWarnings = true;
  ScreenLayout.FuelPosX = 0;
  ScreenLayout.FuelPosY = 150;
  ScreenLayout.ShowFuel = true;
  ScreenLayout.GearPosX = 282;
  ScreenLayout.GearPosY = 84;
  ScreenLayout.ShowGear = true;
  ScreenLayout.RPMPosY = 30;
  ScreenLayout.ShowRPM = true;
  ScreenLayout.SLIPosY = 0;
  ScreenLayout.ShowSLI = true;
  ScreenLayout.SpeedPosX = 0;
  ScreenLayout.SpeedPosY = 75;
  ScreenLayout.ShowSpeed = true;
  ScreenLayout.WaterTempPosX = 0;
  ScreenLayout.WaterTempPosY = 182;
  ScreenLayout.ShowWaterTemp = true;

  // Skip Barber
  CarProfile[ID_Skippy].CarName[0] = 'S';
  CarProfile[ID_Skippy].CarName[1] = 'k';
  CarProfile[ID_Skippy].CarName[2] = 'i';
  CarProfile[ID_Skippy].CarName[3] = 'p';
  CarProfile[ID_Skippy].CarName[4] = 'p';
  CarProfile[ID_Skippy].CarName[5] = 'y';
  CarProfile[ID_Skippy].CarName[6] = 0;

  CarProfile[ID_Skippy].Fuel = 25;
  CarProfile[ID_Skippy].RPM = 291;          // 6000 / RPMscale; where the redline starts on the gauge
  CarProfile[ID_Skippy].RPMscale = 20.625;  // 6600 / 320; max RPM divided by screen width
  CarProfile[ID_Skippy].WaterTemp = 90;

  for (int i = 0; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_Skippy].SLI[i][0] = 4700;
    CarProfile[ID_Skippy].SLI[i][1] = 4900;
    CarProfile[ID_Skippy].SLI[i][2] = 5100;
    CarProfile[ID_Skippy].SLI[i][3] = 5300;
    CarProfile[ID_Skippy].SLI[i][4] = 5500;
    CarProfile[ID_Skippy].SLI[i][5] = 5700;
    CarProfile[ID_Skippy].SLI[i][6] = 5900;
    CarProfile[ID_Skippy].SLI[i][7] = 6100;
  }
                               
  // Cadillac CTS-V
  CarProfile[ID_CTS_V].CarName[0] = 'C';
  CarProfile[ID_CTS_V].CarName[1] = 'T';
  CarProfile[ID_CTS_V].CarName[2] = 'S';
  CarProfile[ID_CTS_V].CarName[3] = '-';
  CarProfile[ID_CTS_V].CarName[4] = 'V';
  CarProfile[ID_CTS_V].CarName[5] = 0;

  CarProfile[ID_CTS_V].Fuel = 40;
  CarProfile[ID_CTS_V].RPM = 288;         // 7200 / RPMscale
  CarProfile[ID_CTS_V].RPMscale = 25;     // 8000 / 320
  CarProfile[ID_CTS_V].WaterTemp = 110;

  for (int i = 0; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_CTS_V].SLI[i][0] = 6600;
    CarProfile[ID_CTS_V].SLI[i][1] = 6700;
    CarProfile[ID_CTS_V].SLI[i][2] = 6800;
    CarProfile[ID_CTS_V].SLI[i][3] = 6900;
    CarProfile[ID_CTS_V].SLI[i][4] = 7000;
    CarProfile[ID_CTS_V].SLI[i][5] = 7100;
    CarProfile[ID_CTS_V].SLI[i][6] = 7200;
    CarProfile[ID_CTS_V].SLI[i][7] = 7300;
  }
  
  // Mazda MX-5 NC
  CarProfile[ID_MX5_NC].CarName[0] = 'M';
  CarProfile[ID_MX5_NC].CarName[1] = 'X';
  CarProfile[ID_MX5_NC].CarName[2] = '5';
  CarProfile[ID_MX5_NC].CarName[3] = ' ';
  CarProfile[ID_MX5_NC].CarName[4] = 'N';
  CarProfile[ID_MX5_NC].CarName[5] = 'C';
  CarProfile[ID_MX5_NC].CarName[6] = 0;

  CarProfile[ID_MX5_NC].Fuel = 40;
  CarProfile[ID_MX5_NC].RPM = 296;           // 6750 / RPMscale
  CarProfile[ID_MX5_NC].RPMscale = 22.8125;  // 7300 / 320
  CarProfile[ID_MX5_NC].WaterTemp = 100;

  for (int i = 0; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_MX5_NC].SLI[i][0] = 6000;
    CarProfile[ID_MX5_NC].SLI[i][1] = 6125;
    CarProfile[ID_MX5_NC].SLI[i][2] = 6250;
    CarProfile[ID_MX5_NC].SLI[i][3] = 6375;
    CarProfile[ID_MX5_NC].SLI[i][4] = 6500;
    CarProfile[ID_MX5_NC].SLI[i][5] = 6625;
    CarProfile[ID_MX5_NC].SLI[i][6] = 6750;
    CarProfile[ID_MX5_NC].SLI[i][7] = 6850;

  }
  // Mazda MX-5 ND
  CarProfile[ID_MX5_ND].CarName[0] = 'M';
  CarProfile[ID_MX5_ND].CarName[1] = 'X';
  CarProfile[ID_MX5_ND].CarName[2] = '5';
  CarProfile[ID_MX5_ND].CarName[3] = ' ';
  CarProfile[ID_MX5_ND].CarName[4] = 'N';
  CarProfile[ID_MX5_ND].CarName[5] = 'D';
  CarProfile[ID_MX5_ND].CarName[6] = 0;

  CarProfile[ID_MX5_ND].Fuel = 40;
  CarProfile[ID_MX5_ND].RPM = 294;           // 6900 / RPMscale
  CarProfile[ID_MX5_ND].RPMscale = 23.4375;  // 7500 / 320
  CarProfile[ID_MX5_ND].WaterTemp = 100;

  for (int i = 0; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_MX5_ND].SLI[i][0] = 4600;
    CarProfile[ID_MX5_ND].SLI[i][1] = 5000;
    CarProfile[ID_MX5_ND].SLI[i][2] = 5400;
    CarProfile[ID_MX5_ND].SLI[i][3] = 5750;
    CarProfile[ID_MX5_ND].SLI[i][4] = 6100;
    CarProfile[ID_MX5_ND].SLI[i][5] = 6800;
    CarProfile[ID_MX5_ND].SLI[i][6] = 7200;
    CarProfile[ID_MX5_ND].SLI[i][7] = 7200;
  }

  // Formula Renault 2.0
  CarProfile[ID_FR20].CarName[0] = 'F';
  CarProfile[ID_FR20].CarName[1] = 'R';
  CarProfile[ID_FR20].CarName[2] = ' ';
  CarProfile[ID_FR20].CarName[3] = '2';
  CarProfile[ID_FR20].CarName[4] = '.';
  CarProfile[ID_FR20].CarName[5] = '0';
  CarProfile[ID_FR20].CarName[6] = 0;

  CarProfile[ID_FR20].Fuel = 30;
  CarProfile[ID_FR20].RPM = 307;           // 7300 / RPMscale
  CarProfile[ID_FR20].RPMscale = 23.75;    // 7600 / 320
  CarProfile[ID_FR20].WaterTemp = 90;

  for (int i = 0; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_FR20].SLI[i][0] = 6600;
    CarProfile[ID_FR20].SLI[i][1] = 6700;
    CarProfile[ID_FR20].SLI[i][2] = 6800;
    CarProfile[ID_FR20].SLI[i][3] = 6900;
    CarProfile[ID_FR20].SLI[i][4] = 7000;
    CarProfile[ID_FR20].SLI[i][5] = 7100;
    CarProfile[ID_FR20].SLI[i][6] = 7200;
    CarProfile[ID_FR20].SLI[i][7] = 7300;
  }

  // Dallara Formula 3
  CarProfile[ID_DF3].CarName[0] = 'D';
  CarProfile[ID_DF3].CarName[1] = 'a';
  CarProfile[ID_DF3].CarName[2] = 'l';
  CarProfile[ID_DF3].CarName[3] = 'l';
  CarProfile[ID_DF3].CarName[4] = 'F';
  CarProfile[ID_DF3].CarName[5] = '3';
  CarProfile[ID_DF3].CarName[6] = 0;

  CarProfile[ID_DF3].Fuel = 10;
  CarProfile[ID_DF3].RPM = 305;           // 7050 / RPMscale
  CarProfile[ID_DF3].RPMscale = 23.125;   // 7400 / 320
  CarProfile[ID_DF3].WaterTemp = 110;

  for (int i = 0; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_DF3].SLI[i][0] = 6450;
    CarProfile[ID_DF3].SLI[i][1] = 6550;
    CarProfile[ID_DF3].SLI[i][2] = 6650;
    CarProfile[ID_DF3].SLI[i][3] = 6750;
    CarProfile[ID_DF3].SLI[i][4] = 6850;
    CarProfile[ID_DF3].SLI[i][5] = 6950;
    CarProfile[ID_DF3].SLI[i][6] = 7050;
    CarProfile[ID_DF3].SLI[i][7] = 7150;
  }

  // Porsche 992 GT3 cup
  CarProfile[ID_992_CUP].CarName[0] = '9';
  CarProfile[ID_992_CUP].CarName[1] = '9';
  CarProfile[ID_992_CUP].CarName[2] = '2';
  CarProfile[ID_992_CUP].CarName[3] = 'c';
  CarProfile[ID_992_CUP].CarName[4] = 'u';
  CarProfile[ID_992_CUP].CarName[5] = 'p';
  CarProfile[ID_992_CUP].CarName[6] = 0;

  CarProfile[ID_992_CUP].Fuel = 30;
  CarProfile[ID_992_CUP].RPM = 298;         // 8200 / RPMscale
  CarProfile[ID_992_CUP].RPMscale = 27.5;   // 8800 / 320
  CarProfile[ID_992_CUP].WaterTemp = 110;


  for (int i = 0; i<3; i++)
  // reverse, neutral, 1st gear
  {
    CarProfile[ID_992_CUP].SLI[i][0] = 1500;
    CarProfile[ID_992_CUP].SLI[i][1] = 1500;
    CarProfile[ID_992_CUP].SLI[i][2] = 1500;
    CarProfile[ID_992_CUP].SLI[i][3] = 3000;
    CarProfile[ID_992_CUP].SLI[i][4] = 4500;
    CarProfile[ID_992_CUP].SLI[i][5] = 5350;
    CarProfile[ID_992_CUP].SLI[i][6] = 6200;
    CarProfile[ID_992_CUP].SLI[i][7] = 7050;
  }

  // 2nd gear
  CarProfile[ID_992_CUP].SLI[3][0] = 3900;
  CarProfile[ID_992_CUP].SLI[3][1] = 4400;
  CarProfile[ID_992_CUP].SLI[3][2] = 4900;
  CarProfile[ID_992_CUP].SLI[3][3] = 5400;
  CarProfile[ID_992_CUP].SLI[3][4] = 5900;
  CarProfile[ID_992_CUP].SLI[3][5] = 6400;
  CarProfile[ID_992_CUP].SLI[3][6] = 6900;
  CarProfile[ID_992_CUP].SLI[3][7] = 7400;

  // 3rd gear
  CarProfile[ID_992_CUP].SLI[4][0] = 4825;
  CarProfile[ID_992_CUP].SLI[4][1] = 5225;
  CarProfile[ID_992_CUP].SLI[4][2] = 5625;
  CarProfile[ID_992_CUP].SLI[4][3] = 6025;
  CarProfile[ID_992_CUP].SLI[4][4] = 6425;
  CarProfile[ID_992_CUP].SLI[4][5] = 6825;
  CarProfile[ID_992_CUP].SLI[4][6] = 7225;
  CarProfile[ID_992_CUP].SLI[4][7] = 7625;

  // 4th gear
  CarProfile[ID_992_CUP].SLI[5][0] = 6425;
  CarProfile[ID_992_CUP].SLI[5][1] = 6625;
  CarProfile[ID_992_CUP].SLI[5][2] = 6825;
  CarProfile[ID_992_CUP].SLI[5][3] = 7025;
  CarProfile[ID_992_CUP].SLI[5][4] = 7225;
  CarProfile[ID_992_CUP].SLI[5][5] = 7425;
  CarProfile[ID_992_CUP].SLI[5][6] = 7725;
  CarProfile[ID_992_CUP].SLI[5][7] = 7925;

  // 5th gear
  for (int i = 6; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_992_CUP].SLI[i][0] = 7275;
    CarProfile[ID_992_CUP].SLI[i][1] = 7375;
    CarProfile[ID_992_CUP].SLI[i][2] = 7475;
    CarProfile[ID_992_CUP].SLI[i][3] = 7575;
    CarProfile[ID_992_CUP].SLI[i][4] = 7675;
    CarProfile[ID_992_CUP].SLI[i][5] = 7775;
    CarProfile[ID_992_CUP].SLI[i][6] = 7875;
    CarProfile[ID_992_CUP].SLI[i][7] = 7975;
  }

  // 6th gear and above
  // no display, move indicators above of real rpm range
  for (int i = 7; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_992_CUP].SLI[i][0] = 9200;
    CarProfile[ID_992_CUP].SLI[i][1] = 9300;
    CarProfile[ID_992_CUP].SLI[i][2] = 9400;
    CarProfile[ID_992_CUP].SLI[i][3] = 9500;
    CarProfile[ID_992_CUP].SLI[i][4] = 9600;
    CarProfile[ID_992_CUP].SLI[i][5] = 9700;
    CarProfile[ID_992_CUP].SLI[i][6] = 9800;
    CarProfile[ID_992_CUP].SLI[i][7] = 9900;
  }

  // Toyota GR86
  CarProfile[ID_GR86].CarName[0] = 'G';
  CarProfile[ID_GR86].CarName[1] = 'R';
  CarProfile[ID_GR86].CarName[2] = '8';
  CarProfile[ID_GR86].CarName[3] = '6';
  CarProfile[ID_GR86].CarName[4] = 0;
 
  CarProfile[ID_GR86].Fuel = 10;
  CarProfile[ID_GR86].RPM = 296;          // 6950 / RPMscale
  CarProfile[ID_GR86].RPMscale = 23.44;   // 7500 / 320
  CarProfile[ID_GR86].WaterTemp = 95;

  for (int i = 0; i<NUMOFGEARS; i++)
  {
    CarProfile[ID_GR86].SLI[i][0] = 6000; // 1st light
    CarProfile[ID_GR86].SLI[i][1] = 6225; // added middle
    CarProfile[ID_GR86].SLI[i][2] = 6450; // 2nd light
    CarProfile[ID_GR86].SLI[i][3] = 6575; // added middle
    CarProfile[ID_GR86].SLI[i][4] = 6700; // 3rd light
    CarProfile[ID_GR86].SLI[i][5] = 6825; // added middle
    CarProfile[ID_GR86].SLI[i][6] = 6950; // 4th light
    CarProfile[ID_GR86].SLI[i][7] = 7250; // all blue
  }

  // buttons for car selection menu
  // use the formula to determine button outline: (x*80)+10, (y*54)+30, 60, 36)
  // x and y is the position in the 4x4 matrix
  for (int i=0; i<NUMOFCARS; i++)
  {
    Button[i].x = ((i%4)*80)+10;
    Button[i].y = ((i/4)*54)+30;
    Button[i].Width = 60;
    Button[i].Height = 36;
    Button[i].TextX = ((i%4)*80)+22;
    Button[i].TextY = ((i/4)*54)+44;
  }
}

// draw the background and the active instruments for the car
void DrawBackground(char ID)
{
  tft->fillScreen(0);  // clear screen

  if(ScreenLayout.ShowEngineWarnings == true)
  {
    // draw the off state warning lights
    // must match to the positions used in the "DrawEngineWarnings" function
    tft->drawBitmap(ScreenLayout.EngineWarningsPosX +0, ScreenLayout.EngineWarningsPosY, fuelpressure, 32, 32, icon_ok);
    tft->drawBitmap(ScreenLayout.EngineWarningsPosX +40, ScreenLayout.EngineWarningsPosY, oilpressure, 32, 32, icon_ok);
    tft->drawBitmap(ScreenLayout.EngineWarningsPosX +80, ScreenLayout.EngineWarningsPosY, water, 32, 32, icon_ok);
    tft->drawBitmap(ScreenLayout.EngineWarningsPosX +120, ScreenLayout.EngineWarningsPosY, stall, 32, 32, icon_ok);
    
  }
  
  if(ScreenLayout.ShowFuel == true)
  {
    tft->setTextColor(dc, 0);
    tft->setTextSize(2);
    tft->setCursor(ScreenLayout.FuelPosX, ScreenLayout.FuelPosY);
    tft->println("Fuel:");
    tft->setCursor(ScreenLayout.FuelPosX+140, ScreenLayout.FuelPosY);
    tft->println("L");
  }

  if(ScreenLayout.ShowSpeed == true)
  {
    tft->setTextColor(dc, 0);
    tft->setTextSize(2);
    tft->setCursor(ScreenLayout.SpeedPosX, ScreenLayout.SpeedPosY);
    tft->println("Speed:");
    tft->setCursor(ScreenLayout.SpeedPosX+140, ScreenLayout.SpeedPosY);
    tft->println("km/h");
  }

  if(ScreenLayout.ShowWaterTemp == true)
  {
    tft->setTextColor(dc, 0);
    tft->setTextSize(2);
    tft->setCursor(ScreenLayout.WaterTempPosX, ScreenLayout.WaterTempPosY);
    tft->println("Water:");
    tft->setCursor(ScreenLayout.WaterTempPosX+140, ScreenLayout.WaterTempPosY);
    tft->println("C");
  }
  
  if(ScreenLayout.ShowRPM == true)
  {
    // use hard coded instructions to draw the RPM gauge layout per car
    switch (ID)
    {
      case ID_Skippy:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_Skippy].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(97, ScreenLayout.RPMPosY+28, 97, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(194, ScreenLayout.RPMPosY+28, 194, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(291, ScreenLayout.RPMPosY+28, 291, ScreenLayout.RPMPosY+34, wc); // 6000 rpm mark, red
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(43, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(140, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(237, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->drawLine(CarProfile[ID_Skippy].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;

      case ID_CTS_V:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_CTS_V].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(80, ScreenLayout.RPMPosY+28, 80, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(160, ScreenLayout.RPMPosY+28, 160, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(240, ScreenLayout.RPMPosY+28, 240, ScreenLayout.RPMPosY+34, bc); // 6000 rpm mark
        tft->drawLine(319, ScreenLayout.RPMPosY+28, 319, ScreenLayout.RPMPosY+34, wc); // 8000 rpm mark, red
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(35, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(115, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(195, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->setCursor(275, ScreenLayout.RPMPosY+24);  // 7000 rpm mark -5 pixel
        tft->println("70");
        tft->drawLine(CarProfile[ID_CTS_V].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;

      case ID_MX5_NC:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_MX5_NC].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(88, ScreenLayout.RPMPosY+28, 88, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(175, ScreenLayout.RPMPosY+28, 175, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(263, ScreenLayout.RPMPosY+28, 263, ScreenLayout.RPMPosY+34, bc); // 6000 rpm mark
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(39, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(127, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(214, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->setCursor(302, ScreenLayout.RPMPosY+24);  // 7000 rpm mark -5 pixel
        tft->setTextColor(wc, 0);
        tft->println("70");
        tft->drawLine(CarProfile[ID_MX5_NC].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;

      case ID_MX5_ND:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_MX5_ND].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(85, ScreenLayout.RPMPosY+28, 85, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(171, ScreenLayout.RPMPosY+28, 171, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(256, ScreenLayout.RPMPosY+28, 256, ScreenLayout.RPMPosY+34, bc); // 6000 rpm mark
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(38, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(123, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(208, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->setCursor(294, ScreenLayout.RPMPosY+24);  // 7000 rpm mark -5 pixel
        tft->setTextColor(wc, 0);
        tft->println("70");
        tft->drawLine(CarProfile[ID_MX5_ND].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;

      case ID_FR20:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_FR20].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(84, ScreenLayout.RPMPosY+28, 84, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(168, ScreenLayout.RPMPosY+28, 168, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(253, ScreenLayout.RPMPosY+28, 253, ScreenLayout.RPMPosY+34, bc); // 6000 rpm mark
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(37, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(121, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(205, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->setCursor(289, ScreenLayout.RPMPosY+24);  // 7000 rpm mark -5 pixel
        tft->println("70");
        tft->drawLine(CarProfile[ID_FR20].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;

      case ID_DF3:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_DF3].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(86, ScreenLayout.RPMPosY+28, 86, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(173, ScreenLayout.RPMPosY+28, 173, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(259, ScreenLayout.RPMPosY+28, 259, ScreenLayout.RPMPosY+34, bc); // 6000 rpm mark
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(38, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(125, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(211, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->setCursor(298, ScreenLayout.RPMPosY+24);  // 7000 rpm mark -5 pixel
        tft->println("70");
        tft->drawLine(CarProfile[ID_DF3].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;

      case ID_992_CUP:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_992_CUP].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(73, ScreenLayout.RPMPosY+28, 73, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(145, ScreenLayout.RPMPosY+28, 145, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(218, ScreenLayout.RPMPosY+28, 218, ScreenLayout.RPMPosY+34, bc); // 6000 rpm mark
        tft->drawLine(291, ScreenLayout.RPMPosY+28, 291, ScreenLayout.RPMPosY+34, bc); // 8000 rpm mark
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(31, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(104, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(177, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->setCursor(250, ScreenLayout.RPMPosY+24);  // 7000 rpm mark -5 pixel
        tft->println("70");
        tft->drawLine(CarProfile[ID_992_CUP].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;

      case ID_GR86:
        tft->drawLine(0, ScreenLayout.RPMPosY+35, CarProfile[ID_GR86].RPM-1, ScreenLayout.RPMPosY+35, bc); // horizontal green line
        tft->drawLine(0, ScreenLayout.RPMPosY+24, 0, ScreenLayout.RPMPosY+34, bc);     // 0 rmp mark
        tft->drawLine(85, ScreenLayout.RPMPosY+28, 85, ScreenLayout.RPMPosY+34, bc);   // 2000 rpm mark
        tft->drawLine(171, ScreenLayout.RPMPosY+28, 171, ScreenLayout.RPMPosY+34, bc); // 4000 rpm mark
        tft->drawLine(256, ScreenLayout.RPMPosY+28, 256, ScreenLayout.RPMPosY+34, bc); // 6000 rpm mark
        tft->setTextColor(bc, 0);
        tft->setTextSize(1);
        tft->setCursor(38, ScreenLayout.RPMPosY+24);   // 1000 rpm mark -5 pixel
        tft->println("10");
        tft->setCursor(123, ScreenLayout.RPMPosY+24);  // 3000 rpm mark -5 pixel
        tft->println("30");
        tft->setCursor(208, ScreenLayout.RPMPosY+24);  // 5000 rpm mark -5 pixel
        tft->println("50");
        tft->setCursor(294, ScreenLayout.RPMPosY+24);  // 7000 rpm mark -5 pixel
        tft->println("70");
        tft->drawLine(CarProfile[ID_GR86].RPM, ScreenLayout.RPMPosY+35, 319, ScreenLayout.RPMPosY+35, wc); // horizontal red line
        break;
    }
  }

  // draw background lines
  tft->drawLine(0, 143, 319, 143, bc);
  tft->drawLine(0, 203, 319, 203, bc);
  tft->drawRoundRect(270, 74, 49, 56, 5, bc);

  // draw car name
  tft->setCursor(271, 227);
  tft->setTextColor(dc, 0);
  tft->setTextSize(1);
  tft->println(CarProfile[ID].CarName);
}

// draw the engine warning icons
void DrawEngineWarnings(char ID, char Warning, char WarningPrev)
{
  char Filtered, FilteredPrev;

  // draw fuel pressure light
  // check only the bit which contains this warning light
  Filtered = Warning & 0x02;
  FilteredPrev = WarningPrev & 0x02;
  if (Filtered != FilteredPrev)
  {
    if (Filtered != 0) tft->drawBitmap(ScreenLayout.EngineWarningsPosX +0, ScreenLayout.EngineWarningsPosY, fuelpressure, 32, 32, icon_nok);
    else tft->drawBitmap(ScreenLayout.EngineWarningsPosX +0, ScreenLayout.EngineWarningsPosY, fuelpressure, 32, 32, icon_ok);
  }

  // draw oil pressure light
  // check only the bit which contains this warning light
  Filtered = Warning & 0x04;
  FilteredPrev = WarningPrev & 0x04;
  if (Filtered != FilteredPrev)
  {
    if (Filtered != 0) tft->drawBitmap(ScreenLayout.EngineWarningsPosX +40, ScreenLayout.EngineWarningsPosY, oilpressure, 32, 32, icon_nok);
    else tft->drawBitmap(ScreenLayout.EngineWarningsPosX +40, ScreenLayout.EngineWarningsPosY, oilpressure, 32, 32, icon_ok);
  }

  // draw water temp light
  // check only the bit which contains this warning light
  Filtered = Warning & 0x01;
  FilteredPrev = WarningPrev & 0x01;
  if (Filtered != FilteredPrev)
  {
    if (Filtered != 0) tft->drawBitmap(ScreenLayout.EngineWarningsPosX +80, ScreenLayout.EngineWarningsPosY, water, 32, 32, icon_nok);
    else tft->drawBitmap(ScreenLayout.EngineWarningsPosX +80, ScreenLayout.EngineWarningsPosY, water, 32, 32, icon_ok);
  }

  // draw stall sign light
  // check only the bit which contains this warning light
  Filtered = Warning & 0x08;
  FilteredPrev = WarningPrev & 0x08;
  if (Filtered != FilteredPrev)
  {
    if (Filtered != 0) tft->drawBitmap(ScreenLayout.EngineWarningsPosX +120, ScreenLayout.EngineWarningsPosY, stall, 32, 32, icon_nok);
    else tft->drawBitmap(ScreenLayout.EngineWarningsPosX +120, ScreenLayout.EngineWarningsPosY, stall, 32, 32, icon_ok);
  }
}

// draw fuel gauge, value is right aligned
void DrawFuel(char ID, int Fuel, int FuelPrev)
{
  tft->setTextSize(2);
  if (Fuel <= CarProfile[ID].Fuel) tft->setTextColor(wc, 0);
  else tft->setTextColor(dc, 0);
  
  tft->setCursor(ScreenLayout.FuelPosX+112, ScreenLayout.FuelPosY);
  tft->print(".");
  tft->setCursor(ScreenLayout.FuelPosX+124, ScreenLayout.FuelPosY);
  tft->print(Fuel % 10, DEC);
                  
  if (Fuel < 100)
  {
    tft->setCursor(ScreenLayout.FuelPosX+100, ScreenLayout.FuelPosY);
    tft->print(Fuel / 10, DEC);
    if (FuelPrev >= 100)  // previous value was greater than 9, so we have to clear the above 10 value from the screen
    {
      tft->fillRect(ScreenLayout.FuelPosX+76, ScreenLayout.FuelPosY, 24, 16, 0);
    }
  }
  else if (Fuel < 1000)
  {
    tft->setCursor(ScreenLayout.FuelPosX+88, ScreenLayout.FuelPosY);
    tft->print(Fuel / 10, DEC);
    if (FuelPrev >= 1000)  // previous value was greater than 99, so we have to clear the above 100 value from the screen
    {
      tft->fillRect(ScreenLayout.FuelPosX+76, ScreenLayout.FuelPosY, 12, 16, 0);
    }
  }
  else if (Fuel < 10000)
       {
        tft->setCursor(ScreenLayout.FuelPosX+76, ScreenLayout.FuelPosY);
        tft->print(Fuel / 10, DEC);
       }
}

// draw gear number
void DrawGear(char ID, signed char Gear)
{
  tft->setTextSize(5);
  if (Gear < 0)  // reverse gear
  {
    tft->setTextColor(wc, 0);
    tft->setCursor(ScreenLayout.GearPosX, ScreenLayout.GearPosY);
    tft->print("R");
  }
  else
  {
    tft->setTextColor(dc, 0);
    if (Gear<NUMOFGEARS)
    {
      tft->setCursor(ScreenLayout.GearPosX, ScreenLayout.GearPosY);
      tft->print(Gear, DEC);
    }
  }
}

// draw water temperature gauge, value is right aligned
void DrawWaterTemp(char ID, int WaterTemp, int WaterTempPrev)
{
  tft->setTextSize(2);
  if (WaterTemp >= CarProfile[ID].WaterTemp) tft->setTextColor(wc, 0);
  else tft->setTextColor(dc, 0);
                  
  if (WaterTemp < 10 && WaterTemp > 0)
  {
    tft->setCursor(ScreenLayout.WaterTempPosX+124, ScreenLayout.WaterTempPosY);
    tft->print(WaterTemp, DEC);
    if (WaterTempPrev >= 10)  // previous value was greater than 9, so we have to clear the above 10 value from the screen
    {
      tft->fillRect(ScreenLayout.WaterTempPosX+100, ScreenLayout.WaterTempPosY, 24, 16, 0);
    }
  }
  else if (WaterTemp < 100 && WaterTemp > 0)
       {
         tft->setCursor(ScreenLayout.WaterTempPosX+112, ScreenLayout.WaterTempPosY);
         tft->print(WaterTemp, DEC);
         if (WaterTempPrev >= 100)  // previous value was greater than 99, so we have to clear the above 100 value from the screen
         {
           tft->fillRect(ScreenLayout.WaterTempPosX+100, ScreenLayout.WaterTempPosY, 12, 16, 0);
         }                    
       }
       else if (WaterTemp<1000 && WaterTemp>0) 
            {
              tft->setCursor(ScreenLayout.WaterTempPosX+100, ScreenLayout.WaterTempPosY);
              tft->print(WaterTemp, DEC);
            }
}

// draw RPM gauge
void DrawRPM(char ID, int RPM, int RPMPrev, char Limiter, char LimiterPrev)
{
  if (Limiter == 0 && LimiterPrev != 0) //  RPM limiter was just switched off so we have to clear off the text
  {
    RPMPrev = 0;  //  redraw the complete RPM bar
    tft->fillRect(99, ScreenLayout.RPMPosY, 132, 20, 0); // clear the "PIT LIMITER" text from screen
  }

  if (Limiter != 0 && LimiterPrev == 0) //  RPM limiter was just switched on so we have to clear the RPM gauge and display the text
  {
    tft->fillRect(0, ScreenLayout.RPMPosY, RPMPrev, 20, 0);  // clear the rpm bar before displaying the text
    tft->setTextColor(mc, 0);
    tft->setTextSize(2);
    tft->setCursor(99, ScreenLayout.RPMPosY);
    tft->print("PIT LIMITER");
  }

  if (Limiter == 0) // we can draw the gauge because the limiter is off
  {
    // RPM is bigger than the previous
    if (RPMPrev < RPM)
    {
      if (RPM >= CarProfile[ID].RPM)  // RPM is bigger than the warning limit
      {
        if (RPMPrev < CarProfile[ID].RPM)
        {
          // we have to draw both color on the RPM gauge
          tft->fillRect(RPMPrev, ScreenLayout.RPMPosY, CarProfile[ID].RPM-1 -RPMPrev, 20, dc);
          tft->fillRect(CarProfile[ID].RPM, ScreenLayout.RPMPosY, RPM -CarProfile[ID].RPM, 20, wc);
        }
        // only the warning color have to be used
        else tft->fillRect(RPMPrev, ScreenLayout.RPMPosY, RPM -RPMPrev, 20, wc);
      }
      // RPM is not bigger than the warning limit, only the default color have to be used
      else tft->fillRect(RPMPrev, ScreenLayout.RPMPosY, RPM-RPMPrev, 20, dc);
    }
    // RPM is lower than the previous
    if (RPMPrev > RPM) tft->fillRect(RPM, ScreenLayout.RPMPosY, RPMPrev-RPM, 20, 0);
  }
}

// draw speed gauge, value is right aligned
void DrawSpeed(char ID, int Speed, int SpeedPrev)
{
  tft->setTextColor(dc, 0);
  tft->setTextSize(2);
  if (Speed<10)
  {
    tft->setCursor(ScreenLayout.SpeedPosX+124, ScreenLayout.SpeedPosY);
    tft->print(Speed, DEC);
    if (SpeedPrev>=10)  // clear the disappeared digit(s)
    {
      tft->fillRect(ScreenLayout.SpeedPosX+100, ScreenLayout.SpeedPosY, 24, 16, 0);
    }
  }
  else if (Speed<100)
       {
        tft->setCursor(ScreenLayout.SpeedPosX+112, ScreenLayout.SpeedPosY);
        tft->print(Speed, DEC);
         if (SpeedPrev>=100)  // clear the disappeared digit(s)
         {
           tft->fillRect(ScreenLayout.SpeedPosX+100, ScreenLayout.SpeedPosY, 12, 16, 0);
         }
       }
  else if (Speed<1000)
       {
        tft->setCursor(ScreenLayout.SpeedPosX+100, ScreenLayout.SpeedPosY);
        tft->print(Speed, DEC);
       }
}

// draw shift light indicator
void DrawSLI(char ID, int SLI, int SLIPrev)
{
  if (SLI < SLIPrev)
  {
    // clear only the disappeared indicators
    for (int i=SLI; i<=SLIPrev-1; i++)
    {
      switch (i)
      {
        case 0: tft->fillRect(10, ScreenLayout.SLIPosY, 25, 16, 0);
                break;
        case 1: tft->fillRect(50, ScreenLayout.SLIPosY, 25, 16, 0);
                break;
        case 2: tft->fillRect(90, ScreenLayout.SLIPosY, 25, 16, 0);
                break;
        case 3: tft->fillRect(130, ScreenLayout.SLIPosY, 25, 16, 0);
                break;
        case 4: tft->fillRect(170, ScreenLayout.SLIPosY, 25, 16, 0);
                break;
        case 5: tft->fillRect(210, ScreenLayout.SLIPosY, 25, 16, 0);
                break;
        case 6: tft->fillRect(250, ScreenLayout.SLIPosY, 25, 16, 0);
                break;
        case 7: tft->fillRect(290, ScreenLayout.SLIPosY, 25, 16, 0);
                break;                
      }
    }
  }
  else
  {
    // draw only the appeared indicators
    for (int i=SLIPrev+1; i<= SLI; i++)
    {
      switch (i)
      {
        case 1: tft->fillRect(10, ScreenLayout.SLIPosY, 25, 16, dc);
                break;
        case 2: tft->fillRect(50, ScreenLayout.SLIPosY, 25, 16, dc);
                break;
        case 3: tft->fillRect(90, ScreenLayout.SLIPosY, 25, 16, dc);
                break;
        case 4: tft->fillRect(130, ScreenLayout.SLIPosY, 25, 16, dc);
                break;
        case 5: tft->fillRect(170, ScreenLayout.SLIPosY, 25, 16, mc);
                break;
        case 6: tft->fillRect(210, ScreenLayout.SLIPosY, 25, 16, mc);
                break;
        case 7: tft->fillRect(250, ScreenLayout.SLIPosY, 25, 16, wc);
                break;
        case 8: tft->fillRect(290, ScreenLayout.SLIPosY, 25, 16, wc);
                break;
     }
   }
  }
}

void DrawCarSelectionMenu()
{
  tft->fillScreen(0);  
  tft->setTextSize(2);
  tft->setCursor(52, 0);
  tft->setTextColor(dc, 0);
  tft->print("Car Selection Menu");
  tft->setTextSize(1);

  for (int i=0; i<NUMOFCARS; i++)
  {
    tft->setTextColor(bc, 0);
    tft->drawRoundRect(Button[i].x, Button[i].y, Button[i].Width, Button[i].Height, 5, bc);
    tft->setCursor(Button[i].TextX, Button[i].TextY);
    tft->setTextColor(dc, 0);
    tft->print(CarProfile[i].CarName);
  }
}

char WhichButtonWasPressed(TS_Point p)
{
  char button;
  int x, y;
  long c;

  // convert the touch coordinates to screen coordinates
  c = long(long(p.x - TOUCH_X_MIN) * (319 /*disp_x_size*/)) / long(TOUCH_X_MAX - TOUCH_X_MIN);
  if (c<0) c = 0;
  if (c>319 /*disp_x_size*/) c = 319; /*disp_x_size*/
  x = (int)c;

  c = long(long(p.y - TOUCH_Y_MIN) * (239 /*disp_y_size*/)) / long(TOUCH_Y_MAX - TOUCH_Y_MIN);
  if (c<0) c = 0;
  if (c>239 /*disp_y_size*/) c = 239 /*disp_y_size*/;
  y = (int)c;

  button = 255; // if no button was selected then return this value

  for (int i=0; i<NUMOFCARS; i++)
    if ((x>Button[i].x) && (y>Button[i].y) && (x<Button[i].x+Button[i].Width) && (y<Button[i].y+Button[i].Height)) button = i;

  return button;
}

void setup()
{
  randomSeed(analogRead(0));
  
  // setup the LCD and switch on the backlight
  tft->begin(80000000);  // set SPI bus to 80 MHz
  tft->setRotation(1); // set to landscape mode
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);  // switch on the backlight

  // setup the touch sensing
  ts.begin();
  ts.setRotation(3);  // not necessarily same as the screen rotation
  
  // start serial port at speed of 115200 bps
  Serial.begin(115200, SERIAL_8N1);
 
  // initialize internal variables
  InData = new SIncomingData;  // allocate the data structure of the telemetry data
  pInData = (char*)InData;     // set the byte array pointer to the telemetry data
  ResetInternalData();

  UploadProfiles();
  DrawBackground(DEFAULTCAR);  // draw the default screen layout
  ActiveCar = DEFAULTCAR;
}

void loop()
{
  int rpm_int, pressed_button;
  byte Limiter[2];
  TS_Point p; // screen touch coordinates
  char PressedButton, gear;

  // read serial port
  if (Serial.available() > 0)
  {
    inByte = Serial.read();

    // check the incoming byte, store the byte in the correct telemetry data block position
    switch (blockpos)
    {
      // first identify if the three byte long header is received in the correct order
      case 0: if (inByte == 3) blockpos = 1;
              break;
      case 1: if (inByte == 12) blockpos = 2;
              else blockpos = 0;
              break;
      case 2: if (inByte == 48) blockpos = 3;
              else blockpos = 0;
              break;
      default:
              if (blockpos >=3) // the three byte long identification header was found, now we store the next incoming bytes in our telemetry data block.
              {
                *(pInData +blockpos-3) = inByte;  // we don't store the identification header
                blockpos++;
              }
              
              if (blockpos == sizeof(SIncomingData)+4)  // last byte of the incoming telemetry data was received, now the screen can be drawn
              {
                blockpos = 0; // reset block position

                // draw Engine Warning lights
                Screen[1].EngineWarnings = InData->EngineWarnings;
                if (Screen[0].EngineWarnings != Screen[1].EngineWarnings && ScreenLayout.ShowEngineWarnings == true) DrawEngineWarnings(ActiveCar, Screen[1].EngineWarnings, Screen[0].EngineWarnings);

                // draw RPM gauge
                Screen[1].RPMgauge = (int)(InData->RPM / CarProfile[ActiveCar].RPMscale);
                Limiter[0] = Screen[0].EngineWarnings & 0x10;
                Limiter[1] = Screen[1].EngineWarnings & 0x10;
                if (Screen[1].RPMgauge > 319) Screen[1].RPMgauge = 319;  // limit RPM gauge to maximum display width
                if ((Screen[0].RPMgauge != Screen[1].RPMgauge || Limiter[1] != Limiter[0]) && ScreenLayout.ShowRPM == true) DrawRPM(ActiveCar, Screen[1].RPMgauge, Screen[0].RPMgauge, Limiter[1], Limiter[0]);

                // draw gear number
                Screen[1].Gear = InData->Gear;
                if (Screen[0].Gear != Screen[1].Gear && ScreenLayout.ShowGear == true) DrawGear(ActiveCar, Screen[1].Gear);

                // draw fuel level gauge
                Screen[1].Fuel = (int)(InData->Fuel*10); // convert float data to int and keep the first digit of the fractional part also
                if (Screen[0].Fuel != Screen[1].Fuel && ScreenLayout.ShowFuel == true) DrawFuel(ActiveCar, Screen[1].Fuel, Screen[0].Fuel);

                // draw speed gauge
                Screen[1].Speed = (int)(InData->Speed*3.6);  // convert m/s to km/h
                if (Screen[0].Speed != Screen[1].Speed && ScreenLayout.ShowSpeed == true) DrawSpeed(ActiveCar, Screen[1].Speed, Screen[0].Speed);

                // draw water temperature gauge
                Screen[1].WaterTemp = (int)InData->WaterTemp;
                if (Screen[0].WaterTemp != Screen[1].WaterTemp && ScreenLayout.ShowWaterTemp == true) DrawWaterTemp(ActiveCar, Screen[1].WaterTemp, Screen[0].WaterTemp);
                
                // draw shift light indicator
                rpm_int = (int)InData->RPM;
                gear = InData->Gear+1;  // "-1" corresponds to reverse but index should start with "0"
                if (rpm_int <= CarProfile[ActiveCar].SLI[gear][0]) Screen[1].SLI = 0; // determine how many light to be activated for the current gear
                else
                {
                  if (rpm_int > CarProfile[ActiveCar].SLI[gear][7]) Screen[1].SLI = 8;
                  else if (rpm_int > CarProfile[ActiveCar].SLI[gear][6]) Screen[1].SLI = 7;
                       else if (rpm_int > CarProfile[ActiveCar].SLI[gear][5]) Screen[1].SLI = 6;
                            else if (rpm_int > CarProfile[ActiveCar].SLI[gear][4]) Screen[1].SLI = 5;
                                 else if (rpm_int > CarProfile[ActiveCar].SLI[gear][3]) Screen[1].SLI = 4;
                                      else if (rpm_int > CarProfile[ActiveCar].SLI[gear][2]) Screen[1].SLI = 3;
                                           else if (rpm_int > CarProfile[ActiveCar].SLI[gear][1]) Screen[1].SLI = 2;
                                                else if (rpm_int > CarProfile[ActiveCar].SLI[gear][0]) Screen[1].SLI = 1;
                }
                if (Screen[0].SLI != Screen[1].SLI && ScreenLayout.ShowSLI == true) DrawSLI(ActiveCar, Screen[1].SLI, Screen[0].SLI);

                // update old screen data
                Screen[0].EngineWarnings = Screen[1].EngineWarnings;
                Screen[0].Fuel           = Screen[1].Fuel;
                Screen[0].Gear           = Screen[1].Gear;
                Screen[0].RPMgauge       = Screen[1].RPMgauge;
                Screen[0].Speed          = Screen[1].Speed;
                Screen[0].WaterTemp      = Screen[1].WaterTemp;
                Screen[0].SLI            = Screen[1].SLI;
  
                // clear the incoming serial buffer which was filled up during the drawing, if it is not done then data misalignment can happen when we read the next telemetry block
                inByte = Serial.available();
                for (int i=0; i<inByte; i++) Serial.read();
              }
              break;
    }
  }

  // check if the screen was touched, if yes then draw the car selection menu
  if (ts.touched())
  {
    p = ts.getPoint();

    // check if the screen was pressed in the middle 1/3rd area and draw the car selection menu only if it was
    // touch coordinates are in the 0..4096 range
    if (p.x>1365 && p.x<2731 && p.y>1365 && p.y<2731)
    {
      while (ts.touched()); // wait until the screen is not touched

      DrawCarSelectionMenu();

      // wait until the screen is touched
      while (!ts.touched());

      // set the active car profile based on the pressed button
      PressedButton = WhichButtonWasPressed(ts.getPoint());
      if (PressedButton!=255) ActiveCar = PressedButton;
    
      DrawBackground(ActiveCar);
      ResetInternalData(); // clear the old screen data
    
      // clear the incoming serial buffer which was filled up during the drawing, if it is not done then data misalignment can happen when we read the next telemetry block
      inByte = Serial.available();
      for (int i=0; i<inByte; i++) Serial.read();
    }
  }
}
