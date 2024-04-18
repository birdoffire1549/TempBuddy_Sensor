    /*
    Settings - A class to contain, maintain, store and retreive settings needed
    by the application. This Class object is intented to be the sole manager of 
    data used throughout the applicaiton. It handles storing both volitile and 
    non-volatile data, where by definition the non-volitile data is persisted
    in flash memory and lives beyond the running life of the software and the 
    volatile data is lost and defaulted each time the software runs.

    Written by: Scott Griffis
    Date: 10-01-2023
    */

#ifndef Settings_h
    #define Settings_h

    #include <string.h> // NEEDED by ESP_EEPROM and MUST appear before WString
    #include <ESP_EEPROM.h>
    #include <WString.h>
    #include <core_esp8266_features.h>
    #include <HardwareSerial.h>
    #include <MD5Builder.h>

    // *****************************************************************************
    // Structure used for storing of settings related data and persisted into flash
    // *****************************************************************************
    struct NonVolatileSettings {
        char           ssid             [33]  ; // 32 chars is max size + 1 null
        char           pwd              [64]  ; // 63 chars is max size + 1 null
        char           adminUser        [13]  ;
        char           adminPwd         [13]  ;
        char           title            [51]  ;
        char           heading          [51]  ;
        char           isCelsius        [6]   ;
        char           sentinel         [33]  ; // Holds a 32 MD5 hash + 1
    };

    class Settings {
        private:
            struct NonVolatileSettings nvSettings;
            struct NonVolatileSettings factorySettings = {
                "SET_ME", // <--------------- ssid
                "SET_ME", // <--------------- pwd
                "admin", // <---------------- adminUser
                "admin", // <---------------- adminPwd
                "TempBuddy Sensor", // <----- title
                "Temp Info", // <------------ heading
                "false", // <---------------- isCelsius
                "NA" // <-------------------- sentinel
            };

            // ******************************************************************
            // Structure used for storing of settings related data NOT persisted
            // ******************************************************************

            /*
                PLEASE NOTE: Just below is an example of what I would use if there
                were also some settings which were not constant but also not persisted
                to storage. I left this block here as a reminder incase I want to use 
                it later.
            */

            // struct VolatileSettings {
            //     bool           unused            ;
            // } vSettings;

            struct ConstantSettings {
                String         hostname          ;
                String         apSsid            ;
                String         apPwd             ;
                String         apNetIp           ;
                String         apSubnet          ;
                String         apGateway         ;
            } constSettings = {
                "TempBuddy-Sensor", // <---- hostname
                "TempBuddy_Sensor", // <---- apSsid
                "P@ssw0rd123", // <--------- apPwd 
                "192.168.1.1", // <--------- apNetIp
                "255.255.255.0", // <------- apSubnet
                "0.0.0.0" // <-------------- apGateway
            };
            
            void defaultSettings();


        public:
            Settings();

            bool factoryDefault();
            bool loadSettings();
            bool saveSettings();
            bool isFactoryDefault();
            bool isNetworkSet();

            /*
            =========================================================
                                Getters and Setters 
            =========================================================
            */
            void           setSsid           (const char* ssid)       ;
            String         getSsid           ()                       ;
            void           setPwd            (const char* pwd)        ;
            String         getPwd            ()                       ;
            void           setAdminUser      (const char* user)       ;
            String         getAdminUser      ()                       ;
            void           setAdminPwd       (const char* pwd)        ;
            String         getAdminPwd       ()                       ;     

            void           setTitle          (const char* title)      ;
            String         getTitle          ()                       ;
            void           setHeading        (const char* heading)    ;
            String         getHeading        ()                       ;
            void           setIsCelsius      (bool isCelsius)         ;
            bool           getIsCelsius      ()                       ;
            
            String         getHostname       ()                       ;
            String         getApSsid         ()                       ;
            String         getApPwd          ()                       ;
            String         getApNetIp        ()                       ;
            String         getApSubnet       ()                       ;    
            String         getApGateway      ()                       ;
    };
    
#endif