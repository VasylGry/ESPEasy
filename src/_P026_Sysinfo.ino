#ifdef USES_P026
//#######################################################################################################
//#################################### Plugin 026: System Info ##########################################
//#######################################################################################################


#define PLUGIN_026
#define PLUGIN_ID_026         26
#define PLUGIN_NAME_026       "Generic - System Info"

// place sensor type selector right after the output value settings
#define P026_QUERY1_CONFIG_POS  0
#define P026_SENSOR_TYPE_INDEX  P026_QUERY1_CONFIG_POS + VARS_PER_TASK
#define P026_NR_OUTPUT_VALUES   getValueCountFromSensorType(PCONFIG(P026_SENSOR_TYPE_INDEX))

#define P026_NR_OUTPUT_OPTIONS  12

String Plugin_026_valuename(byte value_nr, bool displayString) {
  switch (value_nr) {
    case 0:  return displayString ? F("Uptime") : F("uptime");
    case 1:  return displayString ? F("Free RAM") : F("freeheap");
    case 2:  return displayString ? F("Wifi RSSI") : F("rssi");
    case 3:  return displayString ? F("Input VCC") : F("vcc");
    case 4:  return displayString ? F("System load") : F("load");
    case 5:  return displayString ? F("IP 1.Octet") : F("ip1");
    case 6:  return displayString ? F("IP 2.Octet") : F("ip2");
    case 7:  return displayString ? F("IP 3.Octet") : F("ip3");
    case 8:  return displayString ? F("IP 4.Octet") : F("ip4");
    case 9:  return displayString ? F("Web activity") : F("web");
    case 10: return displayString ? F("Free Stack") : F("freestack");
    case 11: return displayString ? F("None") : F("");
    default:
      break;
  }
  return "";
}

boolean Plugin_026(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_026;
      Device[deviceCount].VType          = SENSOR_TYPE_QUAD;
      Device[deviceCount].ValueCount     = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].FormulaOption  = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_026);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (byte i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P026_NR_OUTPUT_VALUES) {
          const byte pconfigIndex = i + P026_QUERY1_CONFIG_POS;
          byte choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_026_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0) = 0;    // "Uptime"

      for (byte i = 1; i < VARS_PER_TASK; ++i) {
        PCONFIG(i) = 11; // "None"
      }
      PCONFIG(P026_SENSOR_TYPE_INDEX) = SENSOR_TYPE_QUAD;
      success                         = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      sensorTypeHelper_webformLoad_simple(event, P026_SENSOR_TYPE_INDEX);
      String options[P026_NR_OUTPUT_OPTIONS];

      for (byte i = 0; i < P026_NR_OUTPUT_OPTIONS; ++i) {
        options[i] = Plugin_026_valuename(i, true);
      }

      for (byte i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P026_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P026_NR_OUTPUT_OPTIONS, options);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // Save output selector parameters.
      for (byte i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P026_QUERY1_CONFIG_POS;
        const byte choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_026_valuename(choice, false));
      }
      sensorTypeHelper_saveSensorType(event, P026_SENSOR_TYPE_INDEX);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      sensorTypeHelper_setSensorType(event, P026_SENSOR_TYPE_INDEX);
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      for (int i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        UserVar[event->BaseVarIndex + i] = P026_get_value(PCONFIG(i));
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("SYS  : ");

        for (int i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
          if (i != 0) {
            log += ',';
          }
          log += UserVar[event->BaseVarIndex + i];
        }
        addLog(LOG_LEVEL_INFO, log);
      }
      success = true;
      break;
    }
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      #ifdef USES_PACKED_RAW_DATA
      // Matching JS code:
      // return decode(bytes, 
      //  [header, uint24, uint24, int8, vcc, pct_8, uint8, uint8, uint8, uint8, uint24, uint16],
      //  ['header', 'uptime', 'freeheap', 'rssi', 'vcc', 'load', 'ip1', 'ip2', 'ip3', 'ip4', 'web', 'freestack']);
      int index = 0;
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // uptime
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // freeheap
      string += LoRa_addFloat(P026_get_value(index++), PackedData_int8);  // rssi
      string += LoRa_addFloat(P026_get_value(index++), PackedData_vcc);   // vcc
      string += LoRa_addFloat(P026_get_value(index++), PackedData_pct_8); // load
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip1
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip2
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip3
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip4
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // web
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint16);  // freestack
      event->Par1 = index; // valuecount
      success = true;
      #endif // USES_PACKED_RAW_DATA
      break;
    }
  }
  return success;
}

float P026_get_value(int type)
{
  float value = 0;

  switch (type)
  {
    case 0:
    {
      value = (wdcounter / 2);
      break;
    }
    case 1:
    {
      value = ESP.getFreeHeap();
      break;
    }
    case 2:
    {
      value = WiFi.RSSI();
      break;
    }
    case 3:
    {
# if FEATURE_ADC_VCC
      value = vcc;
# else // if FEATURE_ADC_VCC
      value = -1.0;
# endif // if FEATURE_ADC_VCC
      break;
    }
    case 4:
    {
      value = getCPUload();
      break;
    }
    case 5:
    {
      value = WiFi.localIP()[0];
      break;
    }
    case 6:
    {
      value = WiFi.localIP()[1];
      break;
    }
    case 7:
    {
      value = WiFi.localIP()[2];
      break;
    }
    case 8:
    {
      value = WiFi.localIP()[3];
      break;
    }
    case 9:
    {
      value = timePassedSince(lastWeb) / 1000; // respond in seconds
      break;
    }
    case 10:
    {
      value = getCurrentFreeStack();
      break;
    }
  }
  return value;
}

#endif // USES_P026
