#ifndef Utils_h
    #define Utils_h

    #include <Settings.h>

    class Utils {
        private:

        public:
            static String hashNvSettings(struct NonVolatileSettings nvSet);
            static String hashString(String string);
            static String genDeviceIdFromMacAddr(String macAddress);
    };

#endif