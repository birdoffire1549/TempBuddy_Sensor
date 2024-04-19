#include "Utils.h"

/**
 * Used to provide a hash of the given NonVolatileSettings.
 * 
 * @param nvSet An instance of NonVolatileSettings to calculate a hash for.
 * 
 * @return Returns the calculated hash value as String.
*/
String Utils::hashNvSettings(struct NonVolatileSettings nvSet) {
    String content = "";
    content = content + String(nvSet.ssid);
    content = content + String(nvSet.pwd);
    content = content + String(nvSet.adminUser);
    content = content + String(nvSet.adminPwd);
    content = content + String(nvSet.title);
    content = content + String(nvSet.heading);
    content = content + String(nvSet.isCelsius);

    MD5Builder builder = MD5Builder();
    builder.begin();
    builder.add(content);
    builder.calculate();

    return builder.toString();
}

/**
 * Function used to perform a MD5 Hash on a given string
 * the result is the MD5 Hash.
 * 
 * @param string The string to hash as String.
 * 
 * @return Returns the generated MD5 Hash as String.
*/
String Utils::hashString(String string) {
    MD5Builder builder = MD5Builder();
    builder.begin();
    builder.add(string);
    builder.calculate();
    
    return builder.toString();
}

/**
 * Generates a six character Device ID based on the
 * given macAddress.
 * 
 * @param macAddress The device's MAC Address as String.
 * 
 * @return Returns a six digit Device ID as String.
*/
String Utils::genDeviceIdFromMacAddr(String macAddress) {
    String result = hashString(macAddress);
    int len = result.length();
    if (len > 6) {
        result = result.substring((len - 6), len);
    }
    result.toUpperCase();

    return result;
}