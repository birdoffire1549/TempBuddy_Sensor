#ifndef IpUtils_h
    #define IpUtils_h

    #include <WString.h>
    #include <IPAddress.h>

    class IpUtils {
        private:

        public:
            static IPAddress stringIPv4ToIPAddress(String ip);
            static IPAddress deriveNetworkBroadcastAddress(String ip, String subnet);
            static unsigned long ipv4ToBinary(String ip);
    };
#endif