/*
    Written by: Scott Griffis
    Date: 04/10/2024
*/

#include "IpUtils.h"

IPAddress IpUtils::stringIPv4ToIPAddress(String ip) {
    String oct[4];

    int curIndex = 0;
    String temp = "";
    for (unsigned int i = 0; i < ip.length(); i++) {
        if (ip.charAt(i) != '.') {
            temp.concat(ip.charAt(i));
        } else {
            oct[curIndex] = String(temp);
            temp = "";
            curIndex ++;
        }
    }
    oct[curIndex] = String(temp);

    return IPAddress(oct[0].toInt(), oct[1].toInt(), oct[2].toInt(), oct[3].toInt());
}