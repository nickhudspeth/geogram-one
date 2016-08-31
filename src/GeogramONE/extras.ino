#define ASSMS 1
#define ASHTTP 2
#define JSONBUFFERSIZE 4096

char phone_number[] = "+12345678910";
const char url[] = "http://mobilytedev.com/resq/app/insert_device_location_info";

int sendJSON(int mode)
{
  StaticJsonBuffer<JSONBUFFERSIZE> jsonBuffer;
  JsonObject& root_json = jsonBuffer.createObject();
  String email;
  //char output[JSONBUFFERSIZE];
  String output;
  int outputSize;
  int result = -1;
  
  EEPROM_readAnything(RETURNADDCONFIG,email);
  root_json["email_id"] = email;

  JsonObject& device_json = root_json.createNestedObject("device");
  char imei[16];
  sim900.getIMEI(imei);
  device_json["imei"] = String(imei);
  device_json["number"] = String(smsData.smsNumber);
  
  JsonObject& gps_json = root_json.createNestedObject("gps");
  JsonObject & date_json = gps_json.createNestedObject("date");
  date_json["year"] = String(lastValid.year);
  date_json["month"] = String(lastValid.month);
  date_json["day"] = String(lastValid.day);
  JsonObject & time_json = gps_json.createNestedObject("time");
  time_json["hour"] = String(lastValid.hour);
  time_json["minute"] = String(lastValid.minute);
  time_json["second"] = String(lastValid.seconds);
  time_json["am_pm"] = (lastValid.amPM == 'a') ? "am" : "pm";
  #if USESPEEDKNOTS
    gps_json["speed"] = String(lastValid.speedKnots);
  #endif
  #if USESPEEDKMR
    gps_json["speed"] = String(lastValid.speedKmr);
  #endif
  #if USECOURSE
    JsonObject & course_json = gps_json.createNestedObject("course");
    course_json["heading"] = String(lastValid.course);
    course_json["direction"] = String(lastValid.courseDirection);
  #endif
  gps_json["battery"] = String(max17043.getBatterySOC()/100);
  gps_json["rssi"] = String(sim900.signalQuality(1));
  #if USESATELLITESUSED
    gps_json["satellites"] = String(lastValid.satellitesUsed);
  #endif
  gps_json["power"] = (charge == 1) ? "CHG" : "BAT";
  gps_json["sos"] = (digitalRead(4) == HIGH) ? "TRUE" : "FALSE";
  JsonObject& adc1_json = root_json.createNestedObject("adc_1");
  adc1_json["value"] = String(analogRead(1));
  adc1_json["threshold"] = "";
  JsonObject& adc2_json = root_json.createNestedObject("adc_2");
  adc2_json["value"] = String(analogRead(2));
  adc2_json["threshold"] = "";
  JsonObject& adc3_json = root_json.createNestedObject("adc_3");
  adc3_json["value"] = String(analogRead(3));
  adc3_json["threshold"] = "";
  JsonObject& sms_json = root_json.createNestedObject("sms");
  sms_json["client_primary"] = "";
  sms_json["client_secondary"] = "";

  outputSize = root_json.printTo(output);
  if(mode == ASSMS)
  {
    int len = output.length() * sizeof(char);
    char *buf = (char*) malloc(len);
    output.toCharArray(buf, len);
    result = sim900.sendMessage(1, phone_number, buf);
    switch(result){
      case 0:
        // Message sent successfully.
        break;
      case 1:
        // Timeout waiting for '>', message not sent.
        break;
      case 2:
        // Error sending SMS.
        break;
      case 3:
        // Timeout waiting for acknowledgement of sent SMS.
        break;
      case 4:
        // No GSM signal
        break;
    }
    free(buf);
    return 0;
  }
  else if(mode == ASHTTP)
  {
    sim900.gsmSleepMode(0);
  
  GSM.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sim900.confirmAtCommand("OK",5000);
  
  GSM.println("AT+SAPBR=3,1,\"APN\",\"epc.tmobile.com\""); //need to put your provider's APN here
  sim900.confirmAtCommand("OK",5000);
  
  GSM.println("AT+SAPBR=1,1");
  sim900.confirmAtCommand("OK",5000);// Tries to connect GPRS 
  
  GSM.println("AT+HTTPINIT");
  sim900.confirmAtCommand("OK",5000);
  
  GSM.println("AT+HTTPPARA=\"CID\",1");
  sim900.confirmAtCommand("OK",5000);
  
  GSM.println("AT+HTTPPARA=\"URL\",\"www.yourserver.com\""); //web address to send data to
  sim900.confirmAtCommand("OK",5000);

  GSM.println("AT+HTTPDATA="+String(outputSize)+",10000");
  //GSM.println("AT+HTTPDATA=100,10000"); //100 refers to how many bytes you're sending.  You'll probably have to tweak or just put a large #
  sim900.confirmAtCommand("DOWNLOAD",5000);
  
  GSM.println("put the data to send here"); //ie latitude,longitude,etc...
  sim900.confirmAtCommand("OK",5000);
  
  GSM.println("AT+HTTPACTION=1"); //POST the data
  sim900.confirmAtCommand("ACTION:",5000);
  
  GSM.println("AT+HTTPTERM"); //terminate http
  sim900.confirmAtCommand("OK",5000);
  
  GSM.println("AT+SAPBR=0,1");// Disconnect GPRS
  sim900.confirmAtCommand("OK",5000);
  sim900.confirmAtCommand("DEACT",5000);
  
  sim900.gsmSleepMode(2);

    return 0;
  }
  else
  {
    return -1;
  }
  
}

int readJson(String input)
{
  StaticJsonBuffer<JSONBUFFERSIZE> jsonBuffer;
  JsonObject & root = jsonBuffer.parseObject(input);
  
  return 0;
}


