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