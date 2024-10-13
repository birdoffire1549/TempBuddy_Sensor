/*
  Project Name: ... Temp Buddy
  Written By: ..... Scott Griffis
  Email: .......... birdoffire1549@gmail.com
  Date: ........... 08-04-2023

  High-level Overview: 
  This code enables an ESP8266MOD to obtain Temperature and Humidity readings
  from a connected AHT10 Device, it then presents the information upon request to any device which 
  connects to its IP Address via port 80. The data is in the form of a web-page with some formatting, 
  and can be displayed in a web browser.

  SECURITY RELATED PLEASE NOTE: 
  Communication with this Device is NOT encrypted. This means that when configuring the settings of the
  device using the Admin page, the data is transmitted to and from the client in clear text on the network you
  and the device are connected to. The potential for compromise is highest when doing initial settings with the
  device in AP mode as there is no password used for the TempBuddy network. Be cautious as this could result in
  the leaking of target network's SSID and Password when initially setup, also if changing the device's admin
  password don't use one that you use for other sensitive things as it will be sent to the device in clear text.
  [TODO: Add SSL usage]

  Hosted Endpoints:
  /        - This is where the temperature and humidity information is deplayed as a web page
  /admin   - This is where the device's settings are configured. Default Password: admin
  /update  - This page isn't intended to be accessed directly rather it should be accessed through the admin page's save functionality

  More Detailed:
  When device is first programmed it boots up as an AccessPoint that can be connected to using a computer, 
  by connecting to the presented network with a name of 'TempBuddy' and no password. Once connected to the
  device's WiFi network you can connect to it for configuration using a web browser via the URL:
  http://192.168.4.1/admin. This will take you to a page requesting a password. Initially the password is
  simply 'admin' but can be changed. This will display the current device settings and allow the user to make desired
  configuration changes to the device. When the Network settings are changed the device will reboot and 
  attempt to connect to the configured network. This code also allows for the device to be equiped with a 
  factory reset button. To perform a factory reset the factory reset button must supply a HIGH to its input
  while the device is rebooted. Upon reboot if the factory reset button is HIGH the stored settings in flash will
  be replaced with the original factory default setttings. The factory reset button also serves another purpose 
  during the normal operation of the device. If pressed breifly the device will flash out the last octet of its 
  IP Address. It does this using the built-in LED. Each digit of the last octet is flashed out with a breif 
  rapid flash between the blink count for each digit. Once all digits have been flashed out the LED will do
  a long rapid flash. Also, one may use the factory reset button to obtain the full IP Address of the device by 
  keeping the ractory reset button pressed during normal device operation for more than 6 seconds. When flashing
  out the IP address the device starts with the first digit of the first octet and flashes slowly that number of 
  times, then it performs a rapid flash to indicate it is on to the next digit. Once all digits in an octet have
  been flashed out the device performs a second after digit rapid flash to indicate it has moved onto a new 
  octet. 
  
  I will demonstrate how this works below by representting a single flash of the LED as a dash '-'. I will represent
  the post digit rapid flash with three dots '...', and finally I will represent the end of sequence long flash
  using 10 dots '..........'. 

  Using the above the IP address of 192.168.123.71 would be flashed out as follows:
  1                     9       2       . 1               6                   8       . 1       2         3       .             7     1           
  - ... - - - - - - - - - ... - - ... ... - ... - - - - - - ... - - - - - - - - ... ... - ... - - ... - - - ... ... - - - - - - - ... - ..........

  The short button press version of the above would simply be the last octet so in this case it would be:
              7     1
  - - - - - - - ... - ..........

  The last octet is useful if you know the network portion of the IP Address the device would be attaching to but
  are not sure what the assigned host portion of the address is, of course this is for network masks of 255.255.255.0.
*/

// ************************************************************************************
// Include Statements
// ************************************************************************************
#include <Arduino.h>
#include <ESP_EEPROM.h>
#include <ESP8266WiFi.h>
#include <AHT10.h>
#include <ESP8266WebServerSecure.h>

#include <Utils.h>
#include <IpUtils.h>
#include <Settings.h>
#include <ExampleSecrets.h>
#include <Secrets.h>
#include <HtmlContent.h>

#include <WiFiUdp.h>

#define FIRMWARE_VERSION "3.0.1"
#define LED_PIN 2 // Output used for flashing out IP Address
#define RESTORE_PIN 13 // Input used for factory reset button; Normally Low

// ************************************************************************************
// Setup of Services
// ************************************************************************************
Settings settings = Settings();
AHT10 tempSensor = AHT10();
BearSSL::ESP8266WebServerSecure webServer(/*Port*/443);
BearSSL::ServerSessions serverCache(/*Sessions*/4);
WiFiUDP udpService;

// ************************************************************************************
// Global worker variables
// ************************************************************************************
String ipAddr = "0.0.0.0";
String deviceId = "";
float lastTempRead = MAXFLOAT;
float lastHumidityRead = MAXFLOAT;
IPAddress bcastAddress;

void resetOrLoadSettings();
void doStartAHT10();
void doStartNetwork();
void checkIpDisplayRequest();
void endpointHandlerRoot();
void endpointHandlerAdmin();
void endpointHandlerApiInfo();
void notFoundHandler();
void fileUploadHandler();
bool handleAdminPageUpdates();
void signalIpAddress(String ipAddress, bool quick);
void sendHtmlPageUsingTemplate(int code, String title, String heading, String &content);
void displayOctet(int octet);
void displayNextDigitIndicator();
bool displayDigit(int digit);
void displayNextOctetIndicator();
void displayDone();
void doReadSensorData();
void doBroadcast();

/***************************** 
 * SETUP() - REQUIRED FUNCTION
 * ***************************
 * This function is the required setup() function and it is
 * where the initialization of the application happens. 
 */
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED off; High = off and Low = On.
  pinMode(RESTORE_PIN, INPUT);

  /* Generate Device ID Based On MAC Address */
  deviceId = Utils::genDeviceIdFromMacAddr(WiFi.macAddress());

  resetOrLoadSettings();
  doStartAHT10(); // Temp/Humidity device
  doStartNetwork();

  yield();
}

/****************************
 * LOOP() - REQUIRED FUNCTION
 * **************************
 * This is the required loop() function of the applicaiton.
 * Here is where all functionality happens or starts to happen.
 */
void loop() {
  doReadSensorData();
  doBroadcast();
  checkIpDisplayRequest();
  webServer.handleClient();

  yield();
}

/**
 * Check to see if the factory reset pin is being held down during
 * normal operation of the device. If it is, then count for how long
 * it is held down for. If less than 6 seconds then signal the last
 * octet of the IP Address. If longer than 6 seconds then signal the
 * entire IP Address.
 */
void checkIpDisplayRequest() {
  int counter = 0;
  while (digitalRead(RESTORE_PIN) == HIGH) {
    counter++;
    delay(1000);
  }
  
  if (counter > 0 && counter < 6) {
    signalIpAddress(ipAddr, true);
  } else if (counter >= 6) {
    signalIpAddress(ipAddr, false);
  }
}

/**
 * Puts the device into client mode such that it will
 * connect to a specified WiFi network based on its
 * SSID and Password.
 */
void connectToNetwork() {
  // Connect to WiFi network...
  WiFi.setOutputPower(20.5F);
  WiFi.setHostname(settings.getHostname(deviceId).c_str());
  WiFi.mode(WiFiMode::WIFI_STA);
  WiFi.begin(settings.getSsid(), settings.getPwd());
}

/**
 * Puts the device into AP Mode so that user can
 * connect via WiFi directly to the device to configure
 * the device.
 */
void activateAPMode() {
  WiFi.setOutputPower(20.5F);
  WiFi.setHostname(settings.getHostname(deviceId).c_str());
  WiFi.mode(WiFiMode::WIFI_AP);
  WiFi.softAPConfig(
    IpUtils::stringIPv4ToIPAddress(settings.getApNetIp()), 
    IpUtils::stringIPv4ToIPAddress(settings.getApGateway()), 
    IpUtils::stringIPv4ToIPAddress(settings.getApSubnet())
  );

  WiFi.softAP(settings.getApSsid(deviceId), settings.getApPwd());
}

/**
 * Detects and reacts to a reqest for factory reset
 * during the boot-up. Also loads settings from 
 * EEPROM if there are saved settings.
 */
void resetOrLoadSettings() {
  if (digitalRead(RESTORE_PIN) == HIGH) { // Restore button pressed on boot...
    settings.factoryDefault();
    while(digitalRead(RESTORE_PIN) == HIGH) { // Wait for pin to be released to continue...
      yield();
    }
  } else { // Normal load restore pin not pressed...
    // Load from EEPROM if applicable...
    settings.loadSettings();
  }
}

/**
 * Used to startup the network related services.
 * Starts the device in AP mode or connects to 
 * existing WiFi based on settings.
 */
void doStartNetwork() {
  // Connect to wireless or enable AP Mode...
  if (!settings.isNetworkSet()) { // Network settings weren't set...
    activateAPMode();
  } else { // Normal mode connects to WiFi...
    connectToNetwork();
  }
  
  #ifndef Secrets_h
    webServer.getServer().setRSACert(new BearSSL::X509List(SAMPLE_SERVER_CERT), new BearSSL::PrivateKey(SAMPLE_SERVER_KEY));
  #else
    webServer.getServer().setRSACert(new BearSSL::X509List(server_cert), new BearSSL::PrivateKey(server_key));
  #endif
  webServer.getServer().setCache(&serverCache);

  /* Setup Endpoint Handlers */
  webServer.on(F("/"), endpointHandlerRoot);
  webServer.on(F("/admin"), endpointHandlerAdmin);
  webServer.on(F("/api/info"), endpointHandlerApiInfo);
  
  webServer.onNotFound(notFoundHandler);
  webServer.onFileUpload(fileUploadHandler);

  webServer.begin();
  Serial.println(F("\nServer started."));
  
  ipAddr = (
    (WiFi.getMode() == WiFiMode_t::WIFI_AP) 
        ? WiFi.softAPIP().toString() 
        : WiFi.localIP().toString()
  );
  bcastAddress = IpUtils::deriveNetworkBroadcastAddress(ipAddr, WiFi.subnetMask().toString());
}

/**
 * Initializes the AHT10 Temp/Humidity sensor,
 * preparing it for use.
 */
void doStartAHT10() {
  // Initialize the AHT10 Sensor...  
  tempSensor.begin();
}

/**
 * #### API-INFO JSON ####
 * This function handles an endpoint which sends information to the
 * client in the form of JSON.
*/
void endpointHandlerApiInfo() {
  String content = INFO_JSON;
  
  content.replace("${deviceid}", deviceId.c_str());
  content.replace("${humidity}", String(lastHumidityRead).c_str());
  content.replace("${title}", settings.getTitle().c_str());
  content.replace("${heading}", settings.getHeading().c_str());
  content.replace("${hostname}", settings.getHostname(deviceId).c_str());

  if (settings.getIsCelsius()) {
    content.replace("${tempvalue}", String(lastTempRead).c_str());
    content.replace("${tempunit}", "C");
  } else {
    content.replace("${tempvalue}", String(((lastTempRead * 9/5) + 32)).c_str());
    content.replace("${tempunit}", "F");
  }

  webServer.send(200, "application/json", content);
  yield();
}

/******************************************************
 * INFO/ROOT PAGE
 * ****************************************************
 * This function shows the info page to a given client.
 */
void endpointHandlerRoot() {
  // Build and send Information Page...
  String content = ROOT_PAGE;
  if (settings.getIsCelsius()) {
    content.replace("${temp}", String(lastTempRead).c_str());
    content.replace("${unit}", "C");
  } else {
    content.replace("${temp}", String(((lastTempRead * 9/5) + 32)).c_str());
    content.replace("${unit}", "F");
  }
  content.replace("${deviceid}", deviceId.c_str());
  content.replace("${humidity}", String(lastHumidityRead).c_str());
   
  sendHtmlPageUsingTemplate(200, settings.getTitle(), settings.getHeading(), content);
}

/****************************************************
 * ADMIN PAGE
 * **************************************************
 * This function shows the admin page for the device.
 */
void endpointHandlerAdmin() {
  /* Ensure user authenticated */
  if (!webServer.authenticate(settings.getAdminUser().c_str(), settings.getAdminPwd().c_str())) { // User not authenticated...
    
    return webServer.requestAuthentication(DIGEST_AUTH, "AdminRealm", "Authentication failed!");
  }
  
  String content = ADMIN_PAGE;

  content.replace("${ssid}", settings.getSsid());
  content.replace("${pwd}", settings.getPwd());
  content.replace("${title}", settings.getTitle());
  content.replace("${heading}", settings.getHeading());
  content.replace("${adminuser}", settings.getAdminUser());
  content.replace("${adminpwd}", settings.getAdminPwd());
  if (settings.getIsCelsius()) { // Units are in Celsius...
    content.replace("${unitcchecked}", "checked");
    content.replace("${unitfchecked}", "");
  } else { // Units are in Fahrenheit...
    content.replace("${unitcchecked}", "");
    content.replace("${unitfchecked}", "checked");
  }
  
  if (!handleAdminPageUpdates()) { // Client response not yet handled...
    sendHtmlPageUsingTemplate(200, settings.getTitle(), "Device Settings", content);
  }
}

/**
 * Used to handle update requests as well as show a pages that 
 * indicate the results of the requested updates.
 * 
 * @return Returns true if the response to client was handles by this
 * function, otherwise returns false as bool.
 */
bool handleAdminPageUpdates() {
  /* Get Incoming Settings */
  struct NonVolatileSettings example;
  String ssid = webServer.arg("ssid");
  String pwd = webServer.arg("pwd");
  String title = webServer.arg("title");
  String heading = webServer.arg("heading");
  String units = webServer.arg("units");
  String adminUser = webServer.arg("adminuser");
  String adminPwd = webServer.arg("adminpwd");

  bool isUpdate = false;
  bool needReboot = false;

  /* Verify and Set SSID */
  if (!ssid.isEmpty() && ssid.length() < sizeof(example.ssid)) { // Not empty and under size limit...
    if (!settings.getSsid().equals(ssid)) { // Incoming is different than existing...
      isUpdate = true;
      needReboot = true;
      settings.setSsid(ssid.c_str());
    }
  }
  /* Verify and Set PWD */
  if (!pwd.isEmpty() && pwd.length() < sizeof(example.pwd)) { // Not empty and under size limit...
    if (!settings.getPwd().equals(pwd)) { // Incoming is different than existing...
      isUpdate = true;
      needReboot = true;
      settings.setPwd(pwd.c_str());
    }
  }
  /* Verify and Set Title */
  if (!title.isEmpty() && title.length() < sizeof(example.title)) { // Not empty and under size limit...
    if (!settings.getTitle().equals(title)) { // Incoming is different than existing...
      isUpdate = true;
      settings.setTitle(title.c_str());
    }
  }
  /* Verify and Set Heading */
  if (!heading.isEmpty() && heading.length() < sizeof(example.heading)) { // Not empty and under size limit...
    if (!settings.getHeading().equals(heading)) { // Incoming is different than existing...
      isUpdate = true;
      settings.setHeading(heading.c_str());
    }
  }
  /* Verify and Set isCelsius */
  if (!units.isEmpty()) { // Not empty...
    if (
      (
        units.equalsIgnoreCase("fahrenheit") 
        || units.equalsIgnoreCase("f")
      ) && settings.getIsCelsius() == true
    ) { // Incoming is understood and different than existing...
      isUpdate = true;
      settings.setIsCelsius(false);
    } else if (
      (
        units.equalsIgnoreCase("celsius")
        || units.equalsIgnoreCase("c")
      ) && settings.getIsCelsius() == false
    ) { // Incoming is understood and different than existing...
      isUpdate = true;
      settings.setIsCelsius(true);
    }
  }
  /* Verify and Set AdminUser */
  if (!adminUser.isEmpty() && adminUser.length() < sizeof(example.adminUser)) { // Not empty and under size limit...
    if (!settings.getAdminUser().equals(adminUser)) { // Incoming is different than existing...
      isUpdate = true;
      settings.setAdminUser(adminUser.c_str());
    }
  }
  /* Verify and Set AdminPwd */
  if (!adminPwd.isEmpty() && adminPwd.length() < sizeof(example.adminPwd)) { // Not empty and under size limit...
    if (!settings.getAdminPwd().equals(adminPwd)) { // Incoming is different than existing...
      isUpdate = true;
      settings.setAdminPwd(adminPwd.c_str());
    }
  }

  /* Persist Data If Updated */
  if (isUpdate) {
    if (settings.saveSettings()) { // Successful...
      if (needReboot) { // Needs to reboot...
        String content = "<h3>Settings update Successful!</h3><h4>Device will reboot now...</h4>";
        sendHtmlPageUsingTemplate(200, settings.getTitle(), "Update Result", content);
        yield();
        delay(5000);

        ESP.restart();
      } else { // No reboot needed; Send to home page...
        String content = "<h3>Settings update Successful!</h3><a href='/'><h4>Home Page</h4></a>";
        sendHtmlPageUsingTemplate(200, settings.getTitle(), "Update Result", content);

        return true;
      }
    } else { // Error...
      String content = "<h3>Error Saving Settings!!!</h3>";
       sendHtmlPageUsingTemplate(500, settings.getTitle(), "500 - Internal Server Error", content);

      return true;
    }
  }
  
  return false;
}

/**
 * #### HANDLER - NOT FOUND ####
 * This is a function which is used to handle web requests when the requested resource is not valid.
 * 
*/
void notFoundHandler() {
  String content = F("Just kidding...<br>But seriously what you were looking for doesn't exist.");
  
  sendHtmlPageUsingTemplate(404, F("404 Not Found"), F("OOPS! You broke it!!!"), content);
}

/**
 * #### HANDLER - File Upload ####
 * This function handles file upload requests.
*/
void fileUploadHandler() {
  String content = F("Um, I don't want your nasty files, go peddle that junk elsewhere!");
  
  sendHtmlPageUsingTemplate(400, F("400 Bad Request"), F("Uhhh, Wuuuuut!?"), content);
}

/**
 * This function is used to Generate the HTML for a web page where the 
 * title, heading and content is provided to the function as a String 
 * type, then inserted into template HTML and finally sent to client.
 * 
 * @param code The HTTP Code as int.
 * @param title The page title as String.
 * @param heading The page heading as String.
 * @param content A reference to the main content of the page as String.
 */
void sendHtmlPageUsingTemplate(int code, String title, String heading, String &content) {
  String result = HTML_PAGE_TEMPLATE;

  result.replace("${title}", title);
  result.replace("${heading}", heading);
  result.replace("${content}", content);

  webServer.send(code, "text/html", result);
  yield();
}

/**
 * Function handles flashing of the LED for signaling the given IP Address
 * entirely or simply its last octet as determined by the passed boolean 
 * refered to as quick. If quick is TRUE then last Octet is signaled, if 
 * FALSE then entire IP is signaled.
 */
void signalIpAddress(String ipAddress, bool quick) {
  if (!quick) { // Whole IP Requested...
    int octet[3];
    
    int index = ipAddress.indexOf('.');
    int index2 = ipAddress.indexOf('.', index + 1);
    int index3 = ipAddress.lastIndexOf('.');
    octet[0] = ipAddress.substring(0, index).toInt();
    octet[1] = ipAddress.substring(index + 1, index2).toInt();
    octet[2] = ipAddress.substring(index2 + 1, index3).toInt();

    for (int i = 0; i < 3; i++) { // Iterate first 3 octets and signal...
      displayOctet(octet[i]);
      displayNextOctetIndicator();
    }
  }

  // Signals 4th octet regardless of quick or not
  int fourth = ipAddress.substring(ipAddress.lastIndexOf('.') + 1).toInt();
  displayOctet(fourth);
  displayDone(); // Fast blink...
} 

/**
 * This function is in charge of displaying or signaling a single
 * octet of an IP address. 
 */
void displayOctet(int octet) {
  if (displayDigit(octet / 100)) {
    displayNextDigitIndicator();
    octet = octet % 100;
  }
  if (displayDigit(octet / 10)) {
    displayNextDigitIndicator();
  }
  displayDigit(octet % 10);
}

/**
 * This function's job is to generate flashes for 
 * a single digit.
 */
bool displayDigit(int digit) {
  digitalWrite(LED_PIN, HIGH); // off
  bool result = false; // Indicates a non-zero value if true.
  for (int i = 0; i < digit; i++) { // Once per value of the digit...
    result = true;
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
  }

  return result;
}

/**
 * Displays or signals the separator between octets which
 * is simply 2 Next Digit Indicators.
 */
void displayNextOctetIndicator() {
  displayNextDigitIndicator();
  displayNextDigitIndicator();
}

/*
 * This displays the Next Digit Indicator which is simply a way to visually
 * break up digit flashes.
 */
void displayNextDigitIndicator() {
  digitalWrite(LED_PIN, HIGH);
  delay(700);
  for (int i = 0; i < 3; i++) { // Flash 3 times...
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
  delay(900);
}

/*
 * This displays or signals the Display Done flashes.
 * This is a visual way for the Device to say it is done 
 * signaling the requested IP Address or last Octet.
 */
void displayDone() {
  digitalWrite(LED_PIN, HIGH); // Start off
  delay(1000);
  for (int i = 0; i < 20; i++) { // Do 20 flashes...
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}

void doReadSensorData() {
  static ulong lastReadMillis = 0ul;
  if ((millis() < lastReadMillis ? (__LONG_MAX__ - lastReadMillis + millis()) : (millis() - lastReadMillis)) >= 30000ul) {
    // Time to do routine with accounting for rollover
    lastTempRead = tempSensor.readTemperature();
    lastHumidityRead = tempSensor.readHumidity();
    lastReadMillis = millis();
  }
}

void doBroadcast() {
  static ulong lastBCastMillis = 0ul;
  if ((millis() < lastBCastMillis ? (__LONG_MAX__ - lastBCastMillis + millis()) : (millis() - lastBCastMillis)) >= 10000UL) { // Broadcast every 10 seconds...
    udpService.begin(settings.getBcastPort());
    udpService.beginPacket(bcastAddress, settings.getBcastPort());
    udpService.printf(
      "TempBuddy-Sensor::%s::%s::T_%f::H_%f", 
      ipAddr.c_str(), 
      deviceId.c_str(), 
      lastTempRead,
      lastHumidityRead
    );
    udpService.endPacket();
    udpService.stop();

    lastBCastMillis = millis();
  }
}