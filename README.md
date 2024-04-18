# TempBuddy Sensor
> [!IMPORTANT]
> This project is in the process of being uplifted from an older version of the
> firmware which I wrote some time ago. This uplifted version of the code uses
> SSL and better security than the old code, but as of yet is not complete and 
> as such might or not contain bugs and perhaps doesn't work properly.
> Once the code is complete and tested then I will remove this flag from the
> README document.

## High-level Overview
This code enables an ESP8266 to obtain Temperature and Humidity readings from a connected AHT10 Device, it then presents the information upon request to any device which connects to its IP Address via HTTPS port 443. The data is in the form of a web-page with some formatting, and can be displayed in a web browser.

### Hosted Endpoints:
| Endpoint | Description | 
| --- | --- |
| / | This is where the temperature and humidity information is deplayed as a web page. |
| /admin | This is where the device's settings are configured. Default User: `admin`; Default Password: `admin` |

## More Details
When device is first programmed it boots up as an AccessPoint that can be connected to using a computer, by connecting to the presented network with a name of `TempBuddy_Sensor` and a default password of `P@ssw0rd123`. Once connected to the device's WiFi network you can connect to it for configuration using a web browser via the URL: `http://192.168.1.1/admin`. This will cause you to get an authentication popup. Initially the user is `admin` and password is also `admin` but can be changed. After a successful authentication the current device settings will be displayed and the user will be allowed to make desired configuration changes to the device. When the Network settings are changed the device will reboot and attempt to connect to the configured network. This code also allows for the device to be equiped with a factory-reset button. To perform a factory-reset the factory-reset button must supply a HIGH to its input while the device is rebooted. Upon reboot if the factory-reset button is HIGH the stored settings in flash will be replaced with the original factory default setttings. The factory-reset button also serves another purpose during the normal operation of the device. If pressed breifly the device will flash out the last octet of its IP Address. It does this using the device's built-in LED. Each digit of the last octet is flashed out with a breif rapid flash between the blink count for each digit. Once all digits have been flashed out the LED will do a long rapid flash. Also, one may use the factory-reset button to obtain the full IP Address of the device by keeping the factory-reset button pressed during normal device operation for more than 6 seconds. When flashing out the IP address the device starts with the first digit of the first octet and flashes slowly that number of times, then it performs a rapid flash to indicate it is on to the next digit. Once all digits in an octet have been flashed out the device performs a second after digit rapid flash to indicate it has moved onto a new octet. 
  
I will demonstrate how this works below by representting a single flash of the LED as a dash `-`. I will represent the post digit rapid flash with three dots `...`, and finally I will represent the end of sequence long flash using 10 dots `..........`. 

Using the above an IP address of `192.168.123.71` would be flashed out as follows:
```
  1                     9       2    .    1               6                   8    . 
  - ... - - - - - - - - - ... - - ... ... - ... - - - - - - ... - - - - - - - - ... ... 
  1       2         3    .                7     1           
  - ... - - ... - - - ... ... - - - - - - - ... - ..........
```

The short button press version of the above would simply be the last octet so in this case it would be:
```
              7     1
  - - - - - - - ... - ..........
```

The last octet is useful if you know the network portion of the IP Address the device would be attaching to but are not sure what the assigned host portion of the address is, of course this is for network masks of `255.255.255.0`.



## Features Coming Soon
- The ability to be discovered on the network by a TempBuddy Control Unit.
- An endpoint that will output a JSON response containing data.