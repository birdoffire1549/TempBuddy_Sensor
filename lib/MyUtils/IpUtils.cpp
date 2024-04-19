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

unsigned long IpUtils::ipv4ToBinary(String ip) {
    /* Pull Appart IP Into Octets */
    unsigned int oct[4];

    int curIndex = 0;
    String temp = "";
    for (unsigned int i = 0; i < ip.length(); i++) {
        if (ip.charAt(i) != '.') {
            temp.concat(ip.charAt(i));
        } else {
            oct[curIndex] = String(temp).toInt();
            temp = "";
            curIndex ++;
        }
    }
    oct[curIndex] = String(temp).toInt();

    /* Derive Binary From Octets */
    unsigned long ipBin = 0UL;
    for (int i = 0; i < 4; i ++) {
        ipBin = (ipBin << 8);
        ipBin = (ipBin | oct[i]);
    }

    return ipBin;
}

IPAddress IpUtils::deriveNetworkBroadcastAddress(String ip, String subnet) {
    unsigned long ipBin = ipv4ToBinary(ip);
    unsigned long subBin = ipv4ToBinary(subnet);

    /* Calculate Network Broadcast IP */
    unsigned long temp = (ipBin & subBin);
    temp = (temp | (~ subBin));

    /* Pull Octets From Binary */
    unsigned int oct[4];
    for (int i = 3; i >= 0; i --) {
        oct[i] = temp & 255;
        temp = (temp >> 8);
    }

    IPAddress result = IPAddress(oct[0], oct[1], oct[2], oct[3]);

    return result;
}