#ifdef USES_P043
//#######################################################################################################
//#################################### Plugin 043: Clock Output #########################################
//#######################################################################################################
#define PLUGIN_043
#define PLUGIN_ID_043         43
#define PLUGIN_NAME_043       "Output - Clock"
#define PLUGIN_VALUENAME1_043 "Output"
#define PLUGIN_043_MAX_SETTINGS 8

boolean Plugin_043(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_043;
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
        string = F(PLUGIN_NAME_043);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_043));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Clock Event"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        String options[3];
        options[0] = "";
        options[1] = F("Off");
        options[2] = F("On");

        addHtml("Select program");
         addFormNumericBox(F("Program"), F("pconf_prog"), PCONFIG(0), 1, 10);

        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
        	addFormTextBox(String(F("Day,Time ")) + (x + 1), String(F("p043_clock")) + (x), timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]), 32);
//          addHtml(F("<TR><TD>Day,Time "));
//          addHtml(x+1);
//          addHtml(F(":<TD><input type='text' name='plugin_043_clock"));
//          addHtml(x);
//          addHtml(F("' value='"));
//          addHtml(timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]));
//          addHtml("'>");

          addHtml(" ");
          byte choice = ExtraTaskSettings.TaskDevicePluginConfig[x];
          addSelector(String(F("p043_state")) + (x), 3, options, NULL, NULL, choice, false);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("pconf_prog"));
        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
          String argc = F("p043_clock");
          argc += x;
          String plugin1 = WebServer.arg(argc);
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = string2TimeLong(plugin1);

          argc = F("p043_state");
          argc += x;
          String plugin2 = WebServer.arg(argc);
          ExtraTaskSettings.TaskDevicePluginConfig[x] = plugin2.toInt();
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        success = true;
        break;
      }

    case PLUGIN_CLOCK_IN:
      {
        LoadTaskSettings(event->TaskIndex);
        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
          unsigned long clockEvent = (unsigned long)minute() % 10 | (unsigned long)(minute() / 10) << 4 | (unsigned long)(hour() % 10) << 8 | (unsigned long)(hour() / 10) << 12 | (unsigned long)weekday() << 16;
          unsigned long clockSet = ExtraTaskSettings.TaskDevicePluginConfigLong[x];

          if (matchClockEvent(clockEvent,clockSet))
          {
            byte state = ExtraTaskSettings.TaskDevicePluginConfig[x];
            if (state != 0)
            {
              state--;
              pinMode(CONFIG_PIN1, OUTPUT);
              digitalWrite(CONFIG_PIN1, state);
              UserVar[event->BaseVarIndex] = state;
              String log = F("TCLK : State ");
              log += state;
              addLog(LOG_LEVEL_INFO, log);
              sendData(event);
            }
          }
        }
        break;
      }
    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("setprogram"))
        {
          String taskName = parseString(string, 2);
          int8_t taskIndex = getTaskIndexByName(taskName);
          if (taskIndex != -1)
          {
            float floatValue=0;
            if (string2float(parseString(string, 4),floatValue))
            {
              if (loglevelActiveFor(LOG_LEVEL_INFO))
              {
                String log = F("TCLK: Program ");
                log += parseString(string, 2);
                log += F(" cmd[2] ");
                log += parseString(string, 3);
                log += F(" value[4] = ");
                log += floatValue;
/*
                for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
                {
                  log += F("[");
        	        log += timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]);
                  log += F("=");
                  log += ExtraTaskSettings.TaskDevicePluginConfig[x];
                  log += F("]");
                }
*/                
                addLog(LOG_LEVEL_INFO,log);
              }
              //UserVar[event->BaseVarIndex+event->Par2-1]=floatValue;
              read_program();
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
              }
            }
          }
        }
        break;
      }
  } // case
  return success;
} // function

// read progs.json
bool read_program()
{
  // load form data from flash

  int size   = 0;
  bool ret = false;
  DynamicJsonDocument doc(8192);
  String log = F("TCLK : Read progs: ");
  fs::File file = tryOpenFile("progs.json", "r");

  if (file)
  {
    size = file.size();
    log += F(" size: ");
    log += size;
    log += F(" content: ");
/*
    while (file.available())
    {
      String c((char)file.read());
      log += c;
    }
*/
    DeserializationError error = deserializeJson(doc, file);
    if (error){
      log += (F("Failed to JSON"));
    }else{
      JsonArray arr = doc.as<JsonArray>();
      JsonObject obj = arr[0];
      log += obj["id"].as<int>();
      log += obj["name"].as<String>();
    }
    file.close();
    ret = true;
  }else{
    log += F(" can't open file ! ");
  }
  addLog(LOG_LEVEL_INFO, log);
  return ret;
}
#endif // USES_P043
