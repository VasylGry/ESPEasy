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

#include <SPI.h>
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
//#include <XPT2046_Touchscreen.h>
#include "ds3231.h"

bool doTouch;
int screenNo = 1;
uint16_t touch_x, touch_y;
String timeStr = "1970-01-01 00:00:00";
String ipStr = "(IP unset)";
float tempValue;
Adafruit_ILI9341 tft = Adafruit_ILI9341(PLUGIN_137_LCD_CS, PLUGIN_137_LCD_DC);
XPT2046 touch(PLUGIN_137_TS_CS, PLUGIN_137_TS_IRQ);
// XPT2046_Touchscreen touch(PLUGIN_137_TS_CS, PLUGIN_137_TS_IRQ);

/**************************************************\
Button structure
\**************************************************/
struct Button
{
  int16_t w;
  int16_t h;
  int16_t x,y;
  int16_t text_x;
  int8_t port;
  int16_t color;
  bool state;
  String name;
  Button(int16_t aX, int16_t aY, int8_t aPort, String aName) : x(aX), y(aY), port(aPort), name(aName)
  {
    w = 90;
    h = 40;
    text_x = 10;
    state = false;
    color = ILI9341_DARKGREY;
  }
  void draw(Adafruit_ILI9341& tft, bool aState)
  {
    state = aState;
    if(state)
      tft.fillRect(x, y, w, h, ILI9341_GREEN);
    else
      tft.fillRect(x, y, w, h, ILI9341_DARKGREY);
    tft.setCursor(x + text_x, y + 10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println(name);
    //Plugin_019_Write(60, 0);
  }
  bool touch(int16_t tx, int16_t ty)
  {
    int16_t ax = (int16_t)(tx * 2);
    int16_t ay = (int16_t)(ty * 1);
    if(!((ax >= x)&& (ax < (int16_t)(x + w)) && (ay >= y) && (ay < (int16_t)(y + h))))
      return false;
    state = state ? false : true;
    //Plugin_019_Write(60, 1);
    if(port){
      portStatusStruct tempStatus;
      const uint32_t key = createKey(PLUGIN_ID_019,port);
      tempStatus = globalMapPortStatus[key];
      tempStatus.mode=PIN_MODE_OUTPUT;
      tempStatus.state=state ? 0 : 1;
      tempStatus.command=1;
      tempStatus.forceEvent=1;
      savePortStatus(key,tempStatus);
      Plugin_019_Write(port, (state ? 0 : 1));
    }
    return true;
  }
};

Button btnPump = Button(20, 165, 57, "PUMP");
Button btnLight = Button(140, 165, 58, "LIGHT");
Button btnFan = Button(20, 215, 59, "FAN");
Button btnHeat = Button(140, 215, 61, "HEAT");
Button btnSetup = Button(20, 270, 0, "SETUP");

boolean Plugin_137(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  //static int8_t switchstate[TASKS_MAX];
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_137;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
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
    //    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_137));
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
        doTouch = false;
        btnSetup.w = 210;
        btnSetup.text_x = 70;
        SPI.setFrequency(ESP_SPI_FREQ);
        tft.begin();
        String log = F("9341 => tft.width=");
        log += tft.width();
        log += F(" tft.heigth=");
        log += tft.height();
        addLog(LOG_LEVEL_INFO, log);
        touch.begin(tft.width(), tft.height());
        touch.setRotation(touch.ROT270);
        //touch.setCalibration(2047, 715, 2047, 784);
        tft.fillScreen(ILI9341_BLACK);
        Plugin_137_MainScreen(true);
        success = true;
        break;
      }
    case PLUGIN_TEN_PER_SECOND:
//    case PLUGIN_READ:
      {
        if(touch.isTouching() && !doTouch)
        {
          touch.getPosition(touch_x, touch_y);;
          doTouch = true;
        }
        success = true;
        break;
      }
    case PLUGIN_ONCE_A_SECOND:
      {
        if(doTouch){
          String log = F("9341 => "); 
          if(screenNo == 1){
            if(btnPump.touch(touch_x, touch_y))
            {
              log += F(" Pump ");
              goto endTouch;
            }
          if(btnLight.touch(touch_x, touch_y)){
              log += F(" Light ");
              goto endTouch;
            }
            if(btnFan.touch(touch_x, touch_y)){
              log += F(" Fan ");
              goto endTouch;
            }
            if(btnHeat.touch(touch_x, touch_y)){
              log += F(" Heat ");
              goto endTouch;
            }
          }
          if(btnSetup.touch(touch_x, touch_y)){
            log += F(" Setup ");
            if(screenNo == 1){
              btnSetup.name = "Main";
              screenNo = 2;
            }
            else{
              btnSetup.name = "Setup";
              screenNo = 1;
            }
            tft.fillScreen(ILI9341_BLACK);
          }
        endTouch:
          log += F(" touched at: ");
          log += F(" touch_x=");
          log += touch_x;
          log += F(" touch_y=");
          log += touch_y;
          addLog(LOG_LEVEL_INFO, log);
        }
        if(screenNo == 1)
          Plugin_137_MainScreen(doTouch);
        else{
          if(doTouch)
            Plugin_137_SetupScreen(doTouch);
        }
                
        success = true;
        break;
      }
    case PLUGIN_EXIT:
      {
        success = true;
        break;
      }
  }// switch
  return success;
} // function

//********************************************************************************
// Main Screen
//********************************************************************************
void Plugin_137_MainScreen(bool withTouch)
{
  int8_t taskIndex = getTaskIndexByName("Temperature");
  int8_t BaseVarIndex = taskIndex * VARS_PER_TASK;
  String tmpStr;
  char tmpBuf[20] = {0};
  
  // tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(3);
  tft.println(" GREEN BOX");
  tft.println();

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(5, 40);
  tft.println(timeStr.c_str());
  if (systemTimePresent())
  {
    timeStr = getValue(LabelType::LOCAL_TIME);
  } 
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(5, 40);
  tft.println(timeStr.c_str());

  tft.setTextColor(ILI9341_ORANGE);
  tft.setCursor(5, 65);
  tft.println("Temperature: ");
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(155, 65);
  sprintf_P(tmpBuf, PSTR("%4.2f"), tempValue);
  tft.println(tmpBuf);
  tempValue = UserVar[BaseVarIndex];
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(155, 65);
  sprintf_P(tmpBuf, PSTR("%4.2f"), tempValue);
  tft.println(tmpBuf);
  tft.println();

  taskIndex = getTaskIndexByName("Pump");
  sprintf_P(tmpBuf, PSTR("Program: %d"),Settings.TaskDevicePluginConfig[taskIndex][0]);
  tft.setTextColor(ILI9341_NAVY);
  tft.setCursor(5, 90);
  tft.println(tmpBuf);
  tft.println();
  
  tft.setCursor(5, 115);
  ipStr = getValue(LabelType::IP_ADDRESS);
  tft.setTextColor(ILI9341_MAROON);
  tft.println(ipStr.c_str());

  if(withTouch)
  {
    btnLight.draw(tft, btnLight.state);
    btnPump.draw(tft, btnPump.state);
    btnFan.draw(tft, btnFan.state);
    btnHeat.draw(tft, btnHeat.state);
    btnSetup.draw(tft, false);
    doTouch = false;
  }
}

//********************************************************************************
// Setup Screen
//********************************************************************************
void Plugin_137_SetupScreen(bool withTouch)
{

  String tmpStr;
  char tmpBuf[20] = {0};
  String log = F("9341 => "); 
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(2);
 
  int8_t taskIndex = getTaskIndexByName("pump");
  LoadTaskSettings(taskIndex);
  sprintf_P(tmpBuf, PSTR("Program: %d"),Settings.TaskDevicePluginConfig[taskIndex][0]);
  tft.println(tmpBuf);
  tft.println();
  tft.setTextColor(ILI9341_WHITE);
  tft.println("PUMP");
  for (byte x = 0; x < 8; x++)
  {
    if(ExtraTaskSettings.TaskDevicePluginConfigLong[x] == (long)983040)
      continue;
    log += F("date: ");
    log += ExtraTaskSettings.TaskDevicePluginConfigLong[x];
    sprintf_P(tmpBuf, PSTR("#%d  %s => %s"), (x + 1), 
        timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]).c_str(), 
        (ExtraTaskSettings.TaskDevicePluginConfig[x] == 2) ? "ON" : "OFF");
    tft.println(tmpBuf);
  }

  tft.println("LIGHT");
  taskIndex = getTaskIndexByName("light");
  LoadTaskSettings(taskIndex);
  for (byte x = 0; x < 2; x++)
  {
    sprintf_P(tmpBuf, PSTR("#%d  %s => %s"), (x + 1), 
        timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]).c_str(), 
        (ExtraTaskSettings.TaskDevicePluginConfig[x] == 2) ? "ON" : "OFF");
    tft.println(tmpBuf);
  }

  tft.println("FAN");
//  tft.println("Temperature:")
  taskIndex = getTaskIndexByName("fan");
  LoadTaskSettings(taskIndex);
  sprintf_P(tmpBuf, PSTR("set=%4.2f  hist=%4.2f"),
          Settings.TaskDevicePluginConfigFloat[taskIndex][0],
          Settings.TaskDevicePluginConfigFloat[taskIndex][1]);
  tft.println(tmpBuf);

  tft.println("HEAT");
//  tft.println("Temperature:")
  taskIndex = getTaskIndexByName("heat");
  LoadTaskSettings(taskIndex);
  sprintf_P(tmpBuf, PSTR("set=%4.2f  hist=%4.2f"),
          Settings.TaskDevicePluginConfigFloat[taskIndex][0],
          Settings.TaskDevicePluginConfigFloat[taskIndex][1]);
  tft.println(tmpBuf);
  addLog(LOG_LEVEL_INFO, log);
  if(withTouch)
  {
    btnSetup.draw(tft, true);
    doTouch = false;
  }
}
#endif // USES_P137
