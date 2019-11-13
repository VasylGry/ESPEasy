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

int testCount;
bool doTouch;
uint16_t touch_x, touch_y;
Adafruit_ILI9341 tft = Adafruit_ILI9341(PLUGIN_137_LCD_CS, PLUGIN_137_LCD_DC);
XPT2046 touch(PLUGIN_137_TS_CS, PLUGIN_137_TS_IRQ);
// XPT2046_Touchscreen touch(PLUGIN_137_TS_CS, PLUGIN_137_TS_IRQ);

/**************************************************\
Button structure
\**************************************************/
struct Button
{
  int8_t w;
  int8_t h;
  int8_t x,y;
  int16_t color;
  bool state;
  String name;
  Button(int8_t aX, int8_t aY, String aName) : x(aX), y(aY), name(aName)
  {
    w = 80;
    h = 40;
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
    tft.setCursor(x + 10 , y + 10);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println(name);
  }
  bool touch(int16_t tx, int16_t ty)
  {
    if((tx > x && tx < (x + w)) && (ty > x && ty < (y + h)))
      return true;
    else
      return false;
  }
};

Button btnPump(120, 120, "PUMP");

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
        SPI.setFrequency(ESP_SPI_FREQ);
        tft.begin();
        //Plagin_137_XPT2046_init(PLUGIN_137_TS_CS, PLUGIN_137_TS_IRQ);
        //Plagin_137_XPT2046_PS(PLUGIN_137_TS_CS, PLUGIN_137_TS_IRQ);
        touch.begin(tft.width(), tft.height());
        //touch.setCalibration(255, 1024, 255, 1024);
        tft.fillScreen(ILI9341_BLACK);
        Plugin_137_TestText(testCount);
        btnPump.draw(tft, true);
        success = true;
        break;
      }
    case PLUGIN_TEN_PER_SECOND:
//    case PLUGIN_READ:
      {
        if(touch.isTouching()) // && !doTouch)
        {
          // int x,y;
          doTouch = true;
          String log = F("9341 => ");
          log += F(" touched at: ");
          touch.getPosition(touch_x, touch_y);
          log += F(" touch_x=");
          log += touch_x;
          log += F(" touch_y=");
          log += touch_y;
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }
    case PLUGIN_ONCE_A_SECOND:
      {
        /*
       if(doTouch) {
          if(testCount)
            btnPump.draw(tft, true);
          else
            btnPump.draw(tft, false);
        }
        if(testCount > 10){
          tft.fillRect(225, 10, 230, 20, ILI9341_GREEN);
          Plugin_137_TestText(testCount);
          testCount = 0;
          curX = 0;
        }else{
          curX += 5;
          tft.fillRect(220, 0, 230, 10, ILI9341_BLACK);
        } */
        doTouch = false;
        success = true;
        break;
      }
    case PLUGIN_EXIT:
      {
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
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(3);
  tft.println(" GREEN BOX");
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
//******************************************************************************************
#define TochXSme 170         // начальное смещение по X   конечное значение 1960
#define TochXMax 2000        //  X   конечное значение 1960
#define TochYSme 95          // начальное смещение по Y   конечное значение 1930
#define TochYMax 2000        //  Y   конечное значение 1930
#define TochXProp 5.7        //пропорция пересчета в пикселы экрана Х
#define TochYProp 7.75       //пропорция пересчета в пикселы экрана Y
int iTochXTft,iTochYTft,iTochXAcp,iTochYAcp; 
//******************************************************************************************
void Plagin_137_XPT2046_PS(uint8_t csPin, uint8_t irqPin)
{
  // Issue a throw-away read, with power-down enabled (PD{1,0} == 0b00)
  // Otherwise, ADC is disabled
  int8_t tmp = (0b1101  << 4) | 0b0100;
  digitalWrite(csPin, LOW);
  SPI.transfer(tmp);
  SPI.transfer16(0);  // Flush, just to be sure
  digitalWrite(csPin, HIGH);
}
//******************************************************************************************  
void Plagin_137_XPT2046_init(uint8_t csPin, uint8_t irqPin)
{
  //Initiallize the SPI1 port.
	  SPI.begin();
	  SPI.setClockDivider(SPI_CLOCK_DIV32);  // SPI_CLOCK_DIV8 начинает работать с 8 и выше
	// эффекта не было
	//   SPI.setDataMode(SPI_MODE3);   //SPI_MODE0 SPI_MODE1    SPI_MODE2  SPI_MODE3 
	// инициализация порта выбора устройства 
	  pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
	// инициализация порта на который приходит сигнал прерывания от экрана PIN 
	  pinMode(irqPin, INPUT); 
}
int Plagin_137_XPT2046_ReadXY(int iKol)
{
  float flP;
	  byte  bXH,bXL,bYH,bYL;
	  int x1=0,y1=0,x=0,y=0, iKolRead=20,iR,iKolReadOK=0;
	      iTochXAcp=0;
	      iTochYAcp=0; 
	       if(iKol<2) iKolRead=1;
	       if(iKol>1&&iKol<20)  iKolRead=iKol;
	// выбор устройства    
	       digitalWrite(PLUGIN_137_TS_CS, LOW);
	       for(iR=0;iR<iKolRead;iR++)
	       {
	//даем команду на пересылку позиции Х
	         SPI.transfer(0X90);
	// считываем старший байт
	        bXH = SPI.transfer(0);
          iTochXTft = bXH << 8;
	// убираем не нужный старший бит  !!!!!    
	         bXH=bXH&0X7F;  //
	         x=bXH;
	// сдвигаем в лево на 4
	         x=bXH<<4;           
	// читаем младший байт
	         bXL = SPI.transfer(0);
           iTochXTft += bXL;
	// убираем ненужный старший бит     
	         bXL=bXL<<1;
	// ставим нужные 4 байта в младшие разряды 
	         bXL=bXL>>4;
	         x1=bXL;     
	// совмещаем два байта - данные Х с АЦП
	         x=x|x1;
	//даем команду на пересылку позиции Y
	         SPI.transfer(0XD0);
	// считываем старший байт
	          bYH = SPI.transfer(0);
            iTochYTft = bYH << 8;
	// убираем не нужный старший бит  !!!!!    
	          bYH=bYH&0X7F;  //
	// сдвигаем в лево на 4
	          y=bYH<<4;           
	// читаем младший байт
	          bYL = SPI.transfer(0);
            iTochYTft += bYL;
	// ставим в младшие разряды      
	          bYL=bYL<<1;
	          bYL=bYL>>4;
	          y1=bYL;     
	// совмещаем два байта и получаем данные c АЦП по Y
	          y=y|y1;
	// проверяем данные с АЦП - в ганицах экрана? Если да то берем среднее с предыдущими - правда разници при 1 и несколких считываний я не заметил
	         if(x>=TochXSme&&x<=TochXMax&&y>=TochYSme&&y<=TochYMax) 
	         {  iKolReadOK++;
	            if(iTochXAcp==0)  iTochXAcp=x;
	            else  iTochXAcp=(iTochXAcp+x)/2;
	            if(iTochYAcp==0)  iTochYAcp=y;
	            else  iTochYAcp=(iTochYAcp+y)/2; 
	         }
	       }  
	// отключаем выбор устройства      
	       digitalWrite(PLUGIN_137_TS_CS, HIGH);
         return (1);        
	// проверяем выход за границы
	      if(iTochXAcp>=TochXSme&&iTochXAcp<=TochXMax&&iTochYAcp>=TochYSme&&iTochYAcp<=TochYMax)
	       { flP=(iTochXAcp-TochXSme)/TochXProp;       //пересчет в координаты дисплея
	         iTochXTft=flP;
	         flP=(iTochYAcp-TochYSme)/TochYProp;     
	         iTochYTft=flP;      
	         return(iKolReadOK);
	       } 
	       else
	       {
	         iTochXTft=0;
	         iTochYTft=0;      
	         return(0);
	       }

}
//******************************************************************************************
// получение кординаты X
int Plagin_137_XPT2046GetX(void)
{
  return(iTochXTft);
}
// получение кординаты Y
int Plagin_137_XPT2046GetY(void)
{
  return(iTochYTft);
}

#endif // USES_P137
