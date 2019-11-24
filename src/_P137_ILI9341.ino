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
#define ALL_00_00             983040l
#define  SCREEN_MAIN          1
#define  SCREEN_CONTROL       2
#define  SCREEN_PROGRAM       3
#define  SCREEN_STAT          4
#define  BUT_PROG_TOUCH       1
#define  BUT_SEL_TOUCH        2
#define  BEEP_COUNT           1  

#include <SPI.h>
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <utf8rus.h>
#include <XPT2046.h>
#include "ds3231.h"

/**************************************************\
Button structure
\**************************************************/
class Button
{
public:
  static bool beepOn;
  int16_t w;
  int16_t h;
  int16_t x,y;
  int16_t text_x;
  int8_t port;
  int16_t color;
  bool state;
  String name;
  Button() {};
  Button(int16_t aX, int16_t aY, int8_t aPort, String aName) : x(aX), y(aY), port(aPort), name(aName)
  {
    w = 230;
    h = 40;
    text_x = 10;
    state = false;
    color = ILI9341_DARKGREY;
  }
  void draw(Adafruit_ILI9341& tft, bool aState)
  {
    state = aState;
    tft.setFont(MONO9);
    if(state)
      tft.fillRect(x, y, w, h, ILI9341_GREEN);
    else
      tft.fillRect(x, y, w, h, ILI9341_DARKGREY);     
    tft.setCursor(x + text_x, y + 25);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println(utf8rus(name));
    tft.setFont(NULL);
  }
  bool touch(int16_t tx, int16_t ty)
  {
    int16_t ax = (int16_t)(tx * 2);
    int16_t ay = (int16_t)(ty * 1);
    if(!((ax >= x)&& (ax < (int16_t)(x + w)) && (ay >= y) && (ay < (int16_t)(y + h))))
      return false;
    state = state ? false : true;
    port_write(60, 1);
    beepOn = true;
    if(port){
      port_write(port, state ? 0 : 1);
    }
    delay(30);
    port_write(60, 0);
    return true;
  }
  bool port_write(int8_t portAddr, int8_t value)
  {
    portStatusStruct tempStatus;
      const uint32_t key = createKey(PLUGIN_ID_019,portAddr);
      tempStatus = globalMapPortStatus[key];
      tempStatus.mode=PIN_MODE_OUTPUT;
      tempStatus.state=value;
      tempStatus.command=1;
      tempStatus.forceEvent=1;
      savePortStatus(key,tempStatus);
      Plugin_019_Write(portAddr, value);
      return true;
  }
};

//*************************************************************
//    ShowText
//*************************************************************
struct ShowText
{
  int16_t x,y;
  int color;
  String text = "";
  const GFXfont *font;

  ShowText(int16_t _x, int16_t _y, String _text, int _color, const GFXfont *_font) 
                  : x(_x),y(_y), color(_color), font(_font) {}

  bool show(Adafruit_ILI9341& tft, String newText, bool renew)
  {
    if((newText == text) && !renew)
      return false;
    tft.setFont(font);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(x, y);
    tft.println(text);
    tft.setTextColor(color);
    tft.setCursor(x, y);
    text = newText;
    tft.println(text);
    tft.setFont(NULL);
    return true;
  }
};

bool doTouch;
int progNo;
int butTouched;
int screenNo = 1;
uint16_t touch_x, touch_y;
ShowText timeStr = ShowText(5, 50, "1970-01-01 00:00:00", ILI9341_CYAN, MONO9);
ShowText tempStr = ShowText(30, 150, "23.3", ILI9341_YELLOW, MONOBI18);
ShowText ipStr = ShowText(5, 190, "(IP unset)", ILI9341_PINK, NULL);
ShowText progStr = ShowText(5, 240, "Программ11", ILI9341_WHITE , MONO9);

float tempValue;
Adafruit_ILI9341 tft = Adafruit_ILI9341(PLUGIN_137_LCD_CS, PLUGIN_137_LCD_DC);
XPT2046 touch(PLUGIN_137_TS_CS, PLUGIN_137_TS_IRQ);

bool Button::beepOn = false;
Button btnPump = Button(5, 40, 57, "ПОЛИВ");
Button btnLight = Button(5, 85, 58, "ОСВЕЩЕНИЕ");
Button btnFan = Button(5, 130, 59, "ВЕНТИЛЯЦИЯ");
Button btnHeat = Button(5, 175, 61, "ПОДОГРЕВ");
Button btnPage = Button(5, 270, 0, "Главный");
Button btnProg = Button(175, 40, 0, ">>");
Button btnSel = Button(175, 90, 0, "OK");
Button btnX[8];

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
        butTouched = 0;
        btnProg.w = 60;
        btnSel.w = 60;
        for(int x=0; x < 8; x++)
        {
          btnX[x].h = 30;
          btnX[x].w = 80;
          btnX[x].x = 0;
          btnX[x].y = 10 + 32 * x;
          btnX[x].text_x = 10;
          btnX[x].name = x + 1;
        }
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
          if(screenNo == SCREEN_CONTROL){
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
          if(btnPage.touch(touch_x, touch_y)){
            log += F(" Page: ");
            tft.fillScreen(ILI9341_BLACK);
            screenNo++;
            screenNo = (screenNo > SCREEN_STAT) ? SCREEN_MAIN : screenNo;
            log += screenNo;
            switch(screenNo)
            {
              case SCREEN_MAIN:
              {
                btnPage.name = "Главный";
                Plugin_137_MainScreen(doTouch);
                break;
              }
              case SCREEN_CONTROL:
              {
                btnPage.name = "Управление";
                Plugin_137_ControlScreen(doTouch);
                break;
              }
              case SCREEN_PROGRAM:
              {
                btnPage.name = "Програма";
                Plugin_137_ProgramScreen(doTouch);
                break;
              }
              case SCREEN_STAT:
              {
                btnPage.name = "Процес";
                Plugin_137_ProgStatScreen(doTouch);
                break;
              }
              default:
                break;
            }
            goto endTouch;
          }
          if(screenNo == SCREEN_PROGRAM){
            if(btnProg.touch(touch_x, touch_y)){
              progNo++;
              progNo = (progNo > 10) ? 1 : progNo;
              butTouched = BUT_SEL_TOUCH;
              goto endTouch;
            }
            if(btnSel.touch(touch_x, touch_y)){
              butTouched = BUT_PROG_TOUCH;
              goto endTouch;
            }
          }
          if(screenNo == SCREEN_STAT){
            butTouched = 0;
            for(int x = 0; x < 8; x++){
              if(btnX[x].touch(touch_x, touch_y)){
                butTouched = x + 1;
                goto endTouch;
              }
            }
          }
        doTouch = false;
        endTouch:
          log += F(" but: ");
          log += butTouched;
          log += F(" touched at: ");
          log += F(" x=");
          log += touch_x;
          log += F(" y=");
          log += touch_y;
          addLog(LOG_LEVEL_INFO, log);
        } 
        switch(screenNo)
        {
          case SCREEN_MAIN:
            Plugin_137_MainScreen(doTouch); 
          break;
          case SCREEN_CONTROL:
            if(doTouch)
              Plugin_137_ControlScreen(doTouch); 
          break;
          case SCREEN_PROGRAM:
            if(doTouch)
            {
              if(butTouched == BUT_SEL_TOUCH)
                Plugin_P137_readProg(doTouch);
              else
                Plugin_137_ProgramScreen(doTouch);
            } 
          break;
          case SCREEN_STAT:
            if(doTouch)
            {
              if(butTouched)
                Plugin_137_ProgStatScreen(doTouch);
            } 
          break;
          default:
          break;
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
  tft.setCursor(40, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(3);
  tft.println("GREEN BOX");
  tft.println();

  tft.setTextSize(2);
  if (systemTimePresent())
  {
    timeStr.show(tft, getValue(LabelType::LOCAL_TIME), true);
  } 

  tempValue = UserVar[BaseVarIndex];
  sprintf_P(tmpBuf, PSTR("%3.1f"), tempValue);
  tempStr.show(tft, utf8rus(tmpBuf), withTouch);

  taskIndex = getTaskIndexByName("Pump");
  sprintf_P(tmpBuf, PSTR("Програма%d"),Settings.TaskDevicePluginConfig[taskIndex][0]);
  progStr.show(tft, utf8rus(tmpBuf), withTouch);
  
  ipStr.show(tft, getValue(LabelType::IP_ADDRESS), withTouch);

  if(withTouch)
  {
    btnPage.draw(tft, false);
    doTouch = false;
  }
}
//********************************************************************************
// Control Screen
//********************************************************************************
void Plugin_137_ControlScreen(bool withTouch)
{

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(80, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(2);
  tft.println("CONTROL");

  if(withTouch)
  {
    btnLight.draw(tft, btnLight.state);
    btnPump.draw(tft, btnPump.state);
    btnFan.draw(tft, btnFan.state);
    btnHeat.draw(tft, btnHeat.state);
    btnPage.draw(tft, false);
    doTouch = false;
  }
}
//********************************************************************************
// Program Screen
//********************************************************************************
void Plugin_137_ProgramScreen(bool withTouch)
{

  String tmpStr;
  int prog_no;
  char tmpBuf[20] = {0};
  String log = F("9341 => "); 
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(60, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(2);
 
  int8_t taskIndex = getTaskIndexByName("pump");
  LoadTaskSettings(taskIndex);
  prog_no = Settings.TaskDevicePluginConfig[taskIndex][0];

  if(butTouched && (progNo != prog_no))
  {
    //Plugin_043_readProg(prog_no - 1, taskIndex, false);
    Plugin_043_readProg(progNo - 1, taskIndex, (butTouched == BUT_PROG_TOUCH));
  }
  LoadTaskSettings(taskIndex);
  progNo = prog_no = Settings.TaskDevicePluginConfig[taskIndex][0];
  sprintf_P(tmpBuf, PSTR("Program: %d"), prog_no);
  tft.println(tmpBuf);
  //tft.println();
  tft.setTextColor(ILI9341_WHITE);
  tft.println("PUMP");
  for (byte x = 0; x < 8; x++)
  {
    if(ExtraTaskSettings.TaskDevicePluginConfigLong[x] == (long)ALL_00_00)
      continue;
    log += F("date: ");
    log += ExtraTaskSettings.TaskDevicePluginConfigLong[x];
    sprintf_P(tmpBuf, PSTR("#%d %s>%s"), (x + 1), 
        timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]).c_str(), 
        (ExtraTaskSettings.TaskDevicePluginConfig[x] == 2) ? "1" : "0");  
    tft.println(tmpBuf);
  }

  tft.println("LIGHT");
  taskIndex = getTaskIndexByName("light");
  LoadTaskSettings(taskIndex);
  for (byte x = 0; x < 2; x++)
  {
    sprintf_P(tmpBuf, PSTR("#%d %s>%s"), (x + 1), 
        timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]).c_str(), 
        (ExtraTaskSettings.TaskDevicePluginConfig[x] == 2) ? "1" : "0");
    tft.println(tmpBuf);
  }
  tft.setCursor(0, 175);
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
    btnPage.draw(tft, false);
    btnProg.draw(tft, false);
    btnSel.draw(tft, false);
    butTouched = 0;
    doTouch = false;
  }
}
//********************************************************************************
// Program Screen
//********************************************************************************
void Plugin_137_ProgStatScreen(bool withTouch)
{
  //char tmpBuf[20] = {0};
  String log = F("9341 => "); 
  
  tft.fillScreen(ILI9341_BLACK);
   tft.setFont(NULL); 
  tft.setTextSize(2);
  tft.setCursor(80, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.println("PROCESS");
  
  int8_t taskIndex = getTaskIndexByName("ProgStat");
  LoadTaskSettings(taskIndex);
    for (byte x = 0; x < 8; x++)
    {
      byte choice = ExtraTaskSettings.TaskDevicePluginConfig[x];
      if(x == butTouched - 1){
        ExtraTaskSettings.TaskDevicePluginConfig[x] = choice ? 0 : 1;
        ExtraTaskSettings.TaskDevicePluginConfigLong[x] = choice ? getRtcTime() : 0;
      }
      String tmpStr;
      bool changed = (choice) ? true : false;
      btnX[x].draw(tft, changed);
      tft.setCursor(100, 31 * (x+1));
      tft.setTextColor(ILI9341_OLIVE);
      if(choice){
        Plugin_138_Calc(tmpStr, ExtraTaskSettings.TaskDevicePluginConfigLong[x], getRtcTime(), true);
        tft.println(tmpStr);
      }
      else{        
        tft.println(String(F("----[--]")));
      }
      //tft.println((String(F("p138_state")) + (x), 2, options, NULL, NULL, choice, false);
    }
  if(butTouched > 0)
    SaveTaskSettings(taskIndex);
  butTouched = 0;
  if(withTouch)
  {
    btnPage.draw(tft, false);
    doTouch = false;
  }
}
//********************************************************************************
// read progs.json
//********************************************************************************
void Plugin_P137_readProg(bool withTouch)
{
  // load form data from flash

  String tmpStr;
  char tmpBuf[20] = {0};
  int size   = 0;
  DynamicJsonDocument doc(8192);
  String log = F("9341 : Read progs: ");
  fs::File file = tryOpenFile("progs.json", "r");

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(60, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(2);
  sprintf_P(tmpBuf, PSTR("Program: %d"), progNo);
  tft.println(tmpBuf);
  tft.setTextColor(ILI9341_ORANGE); 

  if (file)
  {
    size = file.size();
    log += F(" size: ");
    log += size;
    log += F(" content: ");
    DeserializationError error = deserializeJson(doc, file);
    if (error){
      log += (F("Failed to deserialize JSON"));
    }else{
      JsonArray arr = doc.as<JsonArray>();
      JsonObject obj = arr[progNo - 1];
      log += " Program = ";
      log += obj["id"].as<int>();
      log += " Name=";
      log += obj["name"].as<String>();
// pump device
      tft.println("PUMP");
      for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
      {
        String param = obj["pump"][x].as<String>();
        if(param != "00:00"){
          sprintf_P(tmpBuf, PSTR("#%d %s>1"), x, param.c_str());
          tft.println(tmpBuf);
        }
      }
// light device
      tft.println("LIGHT");
      for(int x=0;x<2;x++){
        String param = obj["light"][x].as<String>();
        if(x)
          sprintf_P(tmpBuf, PSTR("#%d %s>0"), x, param.c_str());
        else
          sprintf_P(tmpBuf, PSTR("#%d %s>1"), x, param.c_str());
        tft.println(tmpBuf);
      }
      delay(0);
 // fan device      
      tft.println("FAN");
      float val1 = obj["fan"][0].as<float>();
      log += " f0=";
      log += val1;
      float val2 = obj["fan"][1].as<float>();
      log += " f1=";
      log += val2;
      sprintf_P(tmpBuf, PSTR("set=%3.1f hist=%3.1f"), val1, val2);
      tft.println(tmpBuf);
// heat device
      tft.println("HEAT"); 
      val1 = obj["heat"][0].as<float>();
      log += " f0=";
      log += val1;
      val2 = obj["heat"][1].as<float>();
      log += " f1=";
      log += val2;
      sprintf_P(tmpBuf, PSTR("set=%3.1f hist=%3.1f"), val1, val2);
      tft.println(tmpBuf);
    }
    file.close();
  }else{
    log += F(" can't open file ! ");
  }
  addLog(LOG_LEVEL_INFO, log);
  if(withTouch)
  {
    btnPage.draw(tft, false);
    btnProg.draw(tft, false);
    btnSel.draw(tft, false);
    butTouched = 0;
    doTouch = false;
  }
}

#endif // USES_P137
