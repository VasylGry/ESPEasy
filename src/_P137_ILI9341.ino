#ifdef USES_P137
//#######################################################################################################
//#################################### Plugin 137: PCF8574 ##############################################
//#######################################################################################################

//#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_137
#define PLUGIN_ID_137         137
#define PLUGIN_NAME_137       "ILI9341 LCD"
#define PLUGIN_VALUENAME1_137 "State"
#define PLUGIN_137_PCF_ADDR   39
#define PLUGIN_137_LCD_DC     0
#define PLUGIN_137_LCD_CS     2
#define PLUGIN_137_TS_CS      15
#define PLUGIN_137_TS_IRQ     16
#define PLUGIN_137_DC_LOW 1
#define PLUGIN_137_DC_HIGH 2
#define PLUGIN_137_DC_BOTH 3
#define PLUGIN_137_LONGPRESS_DISABLED 0
#define PLUGIN_137_LONGPRESS_LOW 1
#define PLUGIN_137_LONGPRESS_HIGH 2
#define PLUGIN_137_LONGPRESS_BOTH 3

#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include "ds3231.h"

int testCount, curX, curY;
Adafruit_ILI9341 tft = Adafruit_ILI9341(PLUGIN_137_LCD_CS, PLUGIN_137_LCD_DC);

/**************************************************\
Button structure
\**************************************************/
struct Button
{
  int8_t w;
  int8_t h;
  int8_t x,y;
  int16_t color;
  int8_t state;
  String& name;
  Button(int8_t aX, int8_t aY, String aName) : x(aX), y(aY), name(aName)
  {
    w = 80;
    h = 40;
    state = 0;
    color = ILI9341_DARKGREY;
  }
};

boolean Plugin_137(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  //static int8_t switchstate[TASKS_MAX];

  switch (function)
  {


    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_137;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 8;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_137);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_137));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //@giig1967g: set current task value for taking actions after changes
        const uint32_t key = createKey(PLUGIN_ID_137,CONFIG_PORT);
        if (existPortStatus(key)) {
          globalMapPortStatus[key].previousTask = event->TaskIndex;
        }

        //Configure PCF8574
        addFormSubHeader(F("PCF8574 config"));
        addFormNumericBox(F("I2C address"), F("p137_pcf8574_addr"), round(PCONFIG_FLOAT(0)), PLUGIN_137_PCF_ADDR, 255);
        //Configure LCD
        addFormSubHeader(F("ILI9341 config"));
        addFormNumericBox(F("LCD DC pin"), F("p137_lcd_dc"), round(PCONFIG_FLOAT(1)), PLUGIN_137_LCD_DC, 255);
        addFormNumericBox(F("LCD CS pin"), F("p137_lcd_cs"), round(PCONFIG_FLOAT(2)), PLUGIN_137_LCD_CS, 255);

        //Configure touch
        addFormSubHeader(F("Touch config"));
        addFormNumericBox(F("Touch CS pin"), F("p137_ts_cs"), round(PCONFIG_FLOAT(3)), PLUGIN_137_TS_CS, 255);
        addFormNumericBox(F("Touch IRQ pin"), F("p137_ts_irq"), round(PCONFIG_FLOAT(4)), PLUGIN_137_TS_IRQ, 255);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p137_pcf8574_addr"));
        PCONFIG(1) = getFormItemInt(F("p137_lcd_dc"));
        PCONFIG(2) = getFormItemInt(F("p137_lcd_cs"));
        PCONFIG(3) = getFormItemInt(F("p137_ts_cs"));
        PCONFIG(4) = getFormItemInt(F("p137_ts_irq"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        tft.begin();
        tft.fillScreen(ILI9341_BLACK);
        success = true;
        break;
      }
    case PLUGIN_TEN_PER_SECOND:
      {
        testCount++;
        if(testCount > 10){
          tft.fillRect(220, 0, 230, 10, ILI9341_GREEN);
          Plugin_137_TestText(testCount);
          testCount = 0;
          curX = 0;
        }else{
          curX += 5;
          tft.fillRect(220, 0, 230, 10, ILI9341_BLACK);
        }
        success = true;
        break;
      }
      //giig1967g: Added EXIT function
    case PLUGIN_EXIT:
      {
        // removeTaskFromPort(createKey(PLUGIN_ID_137,CONFIG_PORT));
        break;
      }
  }// switch
  return success;
} // function

//********************************************************************************
// Test text
//********************************************************************************
void Plugin_137_TestText(int count)
{
  int8_t taskIndex = getTaskIndexByName("Temperature");
  int8_t BaseVarIndex = taskIndex * VARS_PER_TASK;
  float value = UserVar[BaseVarIndex];
  String tmpStr;
  char tmpBuf[20] = {0};
  
//  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(3);
  tft.println("GREEN HOUSE");
    tft.setTextSize(2);
  tft.println();
  tft.setTextColor(ILI9341_YELLOW);
  if (systemTimePresent())
    {
      tmpStr = getValue(LabelType::LOCAL_TIME);
    }
    else {
      tmpStr = F("No sys time");
    } 
  tft.fillRect(0, 30, 240, 40, ILI9341_BLACK);
  tft.println(tmpStr.c_str());
  tft.setTextColor(ILI9341_RED);    
  tft.println();
  sprintf_P(tmpBuf, PSTR("Temperature: %4.2f"), value);
  tft.println(tmpBuf);
  tft.println();
  taskIndex = getTaskIndexByName("Pump");
  sprintf_P(tmpBuf, PSTR("Program: %d"),Settings.TaskDevicePluginConfig[taskIndex][0]);
  tft.setTextColor(ILI9341_WHITE);
  tft.println(tmpBuf);
/*  tft.println();
  tft.println("Pump   ON 08:00");
  tft.println("Pump   ON 12:00");
  tft.println("Pump   ON 16:00");
  tft.println("Pump   ON 20:00");
  tft.println("Light  ON 20:00");
  tft.println("Light  OFF 08:00");
  tft.println();
  tft.println("Fan    ON/OFF 27/25");
  tft.println("Heater ON/OFF 17/20");
*/
  tft.println();
  tft.setTextSize(1);
  tmpStr = getValue(LabelType::IP_ADDRESS);
  tft.println(tmpStr.c_str());
}
#endif // USES_P137
