#ifdef USES_P137
//#######################################################################################################
//#################################### Plugin 137: PCF8574 ##############################################
//#######################################################################################################

/**************************************************\
CONFIG
TaskDevicePluginConfig settings:
0: send boot state (true,false)
1:
2:
3:
4: use doubleclick (0,1,2,3)
5: use longpress (0,1,2,3)
6: LP fired (true,false)
7: doubleclick counter (=0,1,2,3)

TaskDevicePluginConfigFloat settings:
0: debounce interval ms
1: doubleclick interval ms
2: longpress interval ms
3: use safebutton (=0,1)

TaskDevicePluginConfigLong settings:
0: clickTime debounce ms
1: clickTime doubleclick ms
2: clickTime longpress ms
3: safebutton counter (=0,1)
\**************************************************/

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
        tft.drawRect(10 + curX, 300, 100 + curX, 350, ILI9341_BLACK);
        if(testCount > 20){
          Plugin_137_TestText(testCount);
          testCount = 0;
          curX = 0;
        }else{
          curX += 5;
          tft.drawRect(10 + curX, 300, 40 + curX, 320, ILI9341_BLUE);
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
// Test FastLines
//********************************************************************************
void Plugin_137_FastLines(uint16_t color1, uint16_t color2)
{

  int           x, y, w = tft.width(), h = tft.height();

  tft.fillScreen(ILI9341_BLACK);

  for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);

}

//********************************************************************************
// Test text
//********************************************************************************
void Plugin_137_TestText(int count)
{
  int8_t taskIndex = getTaskIndexByName("Temperature");
  int8_t BaseVarIndex = taskIndex * VARS_PER_TASK;
  float value = UserVar[BaseVarIndex];
  struct ts t;
  char tmpStr[20] = {0};
  DS3231_get(&t);
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(3);
  tft.println("GREEN HOUSE");
    tft.setTextSize(2);
  tft.println();
  tft.setTextColor(ILI9341_YELLOW); 
  sprintf_P(tmpStr, PSTR("%02d/%02d/%04d  %02d:%02d:%02d"),t.mday,t.mon,t.year,t.hour,t.min,t.sec);
  tft.println(tmpStr);
  tft.setTextColor(ILI9341_RED);    
  tft.println();
  sprintf_P(tmpStr, PSTR("Temp: %4.2f"), value);
  tft.println(tmpStr);
  tft.println();
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.println("Program #3");
  tft.println();
  tft.println("Shpinat");
  tft.println();
  tft.println("Pump   ON 08:00");
  tft.println("Pump   ON 12:00");
  tft.println("Pump   ON 16:00");
  tft.println("Pump   ON 20:00");
  tft.println("Light  ON 20:00");
  tft.println("Light  OFF 08:00");
  tft.println();
  tft.println("Fan    ON/OFF 27/25");
  tft.println("Heater ON/OFF 17/20");
}
#endif // USES_P137
