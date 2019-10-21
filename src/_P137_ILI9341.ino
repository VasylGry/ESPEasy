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
#define PLUGIN_NAME_137       "ILI9341 LCD + PCF8574"
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
        //apply INIT only if PIN is in range. Do not start INIT if pin not set in the device page.
        if (CONFIG_PORT >= 0)
        {
          portStatusStruct newStatus;
          const uint32_t key = createKey(PLUGIN_ID_137,CONFIG_PORT);
          //Read current status or create empty if it does not exist
          newStatus = globalMapPortStatus[key];

          // read and store current state to prevent switching at boot time
          // "state" could be -1, 0 or 1
          newStatus.state = Plugin_137_Read(CONFIG_PORT);
          newStatus.output = newStatus.state;
          (newStatus.state == -1) ? newStatus.mode = PIN_MODE_OFFLINE : newStatus.mode = PIN_MODE_INPUT; 
          newStatus.task++; // add this GPIO/port as a task

          // @giig1967g-20181022: set initial UserVar of the switch
          if (newStatus.state != -1 && Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
            UserVar[event->BaseVarIndex] = !newStatus.state;
          } else {
            UserVar[event->BaseVarIndex] = newStatus.state;
          }

          savePortStatus(key,newStatus);

        }
        success = true;
        break;
      }
    case PLUGIN_TEN_PER_SECOND:
      {
        const int8_t state = Plugin_137_Read(CONFIG_PORT);
        portStatusStruct currentStatus;
        const uint32_t key = createKey(PLUGIN_ID_137,CONFIG_PORT);
        //WARNING operator [],creates an entry in map if key doesn't exist:
        currentStatus = globalMapPortStatus[key];
       //set UserVar and switchState = -1 and send EVENT to notify user
        UserVar[event->BaseVarIndex] = state;
        //switchstate[event->TaskIndex] = state;
        currentStatus.state = state;
        currentStatus.mode = PIN_MODE_OFFLINE;
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("PCF  : Port=");
          log += CONFIG_PORT;
          log += F(" is offline (EVENT= -1)");
          addLog(LOG_LEVEL_INFO, log);
        }
        sendData(event);
        savePortStatus(key,currentStatus);
       
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
// PCF8574 read
//********************************************************************************
//@giig1967g-20181023: changed to int8_t
int8_t Plugin_137_Read(byte Par1)
{
  int8_t state = -1;
  byte unit = (Par1 - 1) / 8;
  byte port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  // get the current pin status
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    state = ((Wire.read() & _BV(port - 1)) >> (port - 1));
  }
  return state;
}

uint8_t Plugin_137_ReadAllPins(uint8_t address)
{
  uint8_t rawState = 0;

  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    rawState =Wire.read();
  }
  return rawState;
}

//********************************************************************************
// PCF8574 write
//********************************************************************************
boolean Plugin_137_Write(byte Par1, byte Par2)
{
  uint8_t unit = (Par1 - 1) / 8;
  uint8_t port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  //generate bitmask
  int i = 0;
  uint8_t portmask = 255;
  unit = unit * 8 + 1; // calculate first pin

  uint32_t key;

  for(i=0; i<8; i++){
    key = createKey(PLUGIN_ID_137,unit+i);

    if (existPortStatus(key) && globalMapPortStatus[key].mode == PIN_MODE_OUTPUT && globalMapPortStatus[key].state == 0)
      portmask &= ~(1 << i); //set port i = 0
  }

  key = createKey(PLUGIN_ID_137,Par1);

  if (Par2 == 1)
    portmask |= (1 << (port-1));
  else
    portmask &= ~(1 << (port-1));

  Wire.beginTransmission(address);
  Wire.write(portmask);
  Wire.endTransmission();

  return true;
}
#endif // USES_P137
