#ifdef USES_P138
//#######################################################################################################
//#################################### Plugin 138: Program statistics #########################################
//#######################################################################################################
#define PLUGIN_138
#define PLUGIN_ID_138               138
#define PLUGIN_NAME_138             "Prog Stat"
#define PLUGIN_VALUENAME1_138       "Duration1"
#define PLUGIN_VALUENAME2_138       "Duration2"
#define PLUGIN_VALUENAME3_138       "Duration3"
#define PLUGIN_VALUENAME4_138       "Duration4"
#define PLUGIN_VALUENAME5_138       "Duration5"
#define PLUGIN_VALUENAME6_138       "Duration6"
#define PLUGIN_VALUENAME7_138       "Duration7"
#define PLUGIN_VALUENAME8_138       "Duration8"
#define PLUGIN_138_MAX_SETTINGS      8

boolean Plugin_138(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_138;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_138);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_138));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Clock Event"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        String options[2];
        options[0] = F("Off");
        options[1] = F("On");

        for (byte x = 0; x < PLUGIN_138_MAX_SETTINGS; x++)
        {
          byte choice = ExtraTaskSettings.TaskDevicePluginConfig[x];
          String tmpStr;
        	//addFormNote(String(F("Start: ")) + ExtraTaskSettings.TaskDevicePluginConfigLong[x], String(F("p138_clock")) + (x));
          //addHtml(" ");
          if(choice){
            Plugin_138_Calc(tmpStr, ExtraTaskSettings.TaskDevicePluginConfigLong[x], getRtcTime(), false);
            addFormNote(String(F("Start: ")) + tmpStr);
          }
          else
            addFormNote(String(F("Start: -- Duration: --")));
          addHtml(" ");
          addSelector(String(F("p138_state")) + (x), 2, options, NULL, NULL, choice, false);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        for (byte x = 0; x < PLUGIN_138_MAX_SETTINGS; x++)
        {
            String argc = F("p138_state");
            argc += x;
            String plugin2 = WebServer.arg(argc);
            bool changed = ExtraTaskSettings.TaskDevicePluginConfig[x] != plugin2.toInt();
            ExtraTaskSettings.TaskDevicePluginConfig[x] = plugin2.toInt();
            if(changed)
            {
                argc = F("p138_clock");
                argc += x;
                String plugin1 = WebServer.arg(argc);
                ExtraTaskSettings.TaskDevicePluginConfigLong[x] = plugin2.toInt() ? getRtcTime() : 0;
            }
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        success = true;
        break;
      }

    case PLUGIN_READ:
    // case PLUGIN_ONCE_A_SECOND:
      {
        LoadTaskSettings(event->TaskIndex);
        for (byte x = 0; x < PLUGIN_138_MAX_SETTINGS; x++)
        {
            byte state = ExtraTaskSettings.TaskDevicePluginConfig[x];
            if (state != 0)
            {
              state--;
              UserVar[event->BaseVarIndex] = state;
              String log = F("Stat : State ");
              log += state;
              addLog(LOG_LEVEL_INFO, log);
              sendData(event);
            }
        }
        break;
      }
    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("startstat"))
        {
          String taskName = parseString(string, 2);
          int8_t taskIndex = event->TaskIndex; // getTaskIndexByName(taskName);
          if (taskIndex != -1)
          {
            float floatValue=0;
            if (string2float(parseString(string, 4),floatValue))
            {
              if (loglevelActiveFor(LOG_LEVEL_INFO))
              {
                String log = F("TCLK: Program ");
                log += parseString(string, 2);
                log += F(" TaskIndex ");
                log += event->TaskIndex;
                log += F(" value[4] = ");
                log += floatValue;
                addLog(LOG_LEVEL_INFO,log);
                command = F("\nOk\nProgram updated!");
                SendStatus(event->Source, command);
              }
              if((floatValue < 1) || (floatValue > 8)){
                command = F("\nFail\nBad program number!");
                SendStatus(event->Source, command);
              }else
                //Plugin_138_readProg((int)floatValue - 1, event->TaskIndex, true);
              success = true;
            } else { // float conversion failed!
              if (loglevelActiveFor(LOG_LEVEL_ERROR))
              {
                String log = F("TCLK: Program ");
                log += event->Par1;
                log += F(" value ");
                log += event->Par2;
                log += F(" parameter3: ");
                log += parseString(string, 4);
                log += F(" not a float value!");
                addLog(LOG_LEVEL_ERROR,log);
                command = F("\nFail\nBad program number!");
                SendStatus(event->Source, command);
              }
            }
          }
        }
        break;
      }
  } // case
  return success;
} // function
//================================================================================================
// calc scheduling
void Plugin_138_Calc(String& str, const unsigned long startTime, const unsigned long endTime, bool shortMode)
{
  struct tm t;
  char buf[20];
  uint8_t mday;
  uint8_t hour;
  uint8_t min;
  unsigned long sec;
  unsigned long delta =  timeDiff(startTime, endTime);
  mday = delta / SECS_PER_DAY;
  sec = delta - mday * SECS_PER_DAY;
  hour = sec / SECS_PER_HOUR;
  sec = sec - hour * SECS_PER_HOUR;
  min = sec / SECS_PER_MIN;
  breakTime(startTime, t);
  if(!shortMode){
	  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", t.tm_year + 1970, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    str = String(buf) + "[" + mday + "-" + hour + ":" + min + "]";
  }
  else{
    sprintf(buf, "%02d %02d:%02d", mday, hour, min);
    str = String(buf);
  }
  String log = F("Stat:");
  log += startTime;
  log += F(" delta: ");
  log += delta;
  log += F(" => ");
  log += str;  
  addLog(LOG_LEVEL_INFO, log);
  return;
}
#endif // USES_P138
