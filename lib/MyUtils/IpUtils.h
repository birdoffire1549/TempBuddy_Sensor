#ifndef IpUtils_h
    #define IpUtils_h

    #include <WString.h>
    #include <IPAddress.h>

    class IpUtils {
        private:

        public:
            static IPAddress stringIPv4ToIPAddress(String ip);
    };
#endif