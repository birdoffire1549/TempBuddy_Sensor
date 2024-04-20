# TempBuddy Sensor
> Document Last Updated for:    
> Firmware Version: 2.2.1

## Overview
This software was written to be utilized by an ESP8266 with an HT10 Device in a specialized hardware configuration known as a TempBuddy Sensor unit.

## High-level Overview
This code enables an ESP8266 to obtain Temperature and Humidity readings from a connected AHT10 Device, it then presents the information upon request to any device which connects to its IP Address via HTTPS port 443. On the root path the data is in the form of a web-page with some formatting, and can be displayed in a web browser. There is also an API path that can be used to fetch data in a JSON format.

> [!IMPORTANT]
> The device is configured to use HTTPS for its internal webserver by default, 
> however the provided SSL Certs are obviously not secure so you will need to 
> generate your own Certificates and add them to a Secrets.h file in the project 
> before it is built. See the comments in the ExampleSecrets.h file for instructions
> on how to generate your own certificates, or look further below in this document as
> I have updated this README to also contain the instructions.
>
> Sorry, but certs are baked into the binary when compiled and cannot
> be changed out during runtime. The ESP8266 is just too limited to have all
> the bells and whistles. There is more I wanted to do but the hardware is just so
> limited.

### Hosted Endpoints:
| Endpoint | Description | 
| --- | --- |
| / | This is where the temperature and humidity information is deployed as a web page. |
| /admin | This is where the device's settings are configured. Default User: `admin`; Default Password: `admin` |
| /api/info | This allows for information to be fetch from the device in a JSON format. |

## More Details
When device is first programmed it boots up as an Access Point that can be connected to using a computer, by connecting to the presented network with a name of `TempBuddy_Sensor_<deviceId>` and a default password of `P@ssw0rd123`. The `<deviceId>` portion of the SSID will be a kind of unique 6 character device ID. Once connected to the device's WiFi network you can connect to the device for configuration using a web browser via the URL: `https://192.168.1.1/admin`. This will cause you to get an authentication popup. Initially the user is `admin` and password is also `admin` but can be changed. After a successful authentication the current device settings will be displayed and the user will be allowed to make desired configuration changes to the device. When the Network settings are changed the device will reboot and attempt to connect to the configured network. This code also allows for the device to be equipped with a factory-reset button. To perform a factory-reset the factory-reset button must supply a HIGH to its input while the device is rebooted. Upon reboot if the factory-reset button is HIGH the stored settings in flash will be replaced with the original factory default settings. The factory-reset button also serves another purpose during the normal operation of the device. If pressed briefly the device will flash out the last octet of its IP Address. It does this using the device's built-in LED. Each digit of the last octet is flashed out with a brief rapid flash between the blink count for each digit. Once all digits have been flashed out the LED will do a long rapid flash. Also, one may use the factory-reset button to obtain the full IP Address of the device by keeping the factory-reset button pressed during normal device operation for more than 6 seconds. When flashing out the IP address the device starts with the first digit of the first octet and flashes slowly that number of times, then it performs a rapid flash to indicate it is on to the next digit. Once all digits in an octet have been flashed out the device performs a second after digit rapid flash to indicate it has moved onto a new octet. 
  
I will demonstrate how this works below by representing a single flash of the LED as a dash `-`. I will represent the post digit rapid flash with three dots `...`, and finally I will represent the end of sequence long flash using 10 dots `..........`. 

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

## Other Less Noticeable Features
### API Endpoint
As shown under the section above which talks about hosted endpoints, there is an API endpoint which serves additional information in a JSON format. 

That JSON looks something like this:
```
}
  "title_text": "TempBuddy Sensor",
  "heading_text": "Bedroom Info",
  "device_id": "A4C372",
  "hostname": "TempBuddyA4C372",
  "temp": 60.13,
  "temp_unit": "F",
  "humidity_percent": 34.55
}
```

As you can see from the above, a little more information about the device is also provided in addition to the current Temperature and Humidity information. This endpoint was included to allow for a more uniform and stable interaction between this device and other network devices or applications which may be created to obtain information from this sensor unit.

### A Broadcast Capability
The TempBuddy Sensor also has a broadcast capability which may be useful should one wish to use the device with other devices and/or applications. The broadcast feature causes the unit to send out a network wide broadcast once every 10 seconds. The broadcast is a UDP broadcast on port 61549 that will look something like this:

```
TempBuddy-Sensor::192.168.123.31::A4C372
```

The first part of the string message will always be the text `TempBuddy-Sensor` followed by `::`. Actually, the message is made up of 3 parts, each separated by double-colons. The first part is the unchanging text mentioned prior. The second part is the IP Address of the device. The third part is the Device ID.

## Building the Unit's Hardware
I have documented the hardware build process and design for the TempBuddy Sensor unit as an Instructables Page. 
That page and information can be found here:

https://www.instructables.com/WiFi-Enabled-TemperatureHumidity-Sensor-Using-ESP8/

## Generating Your Own Certs
To generate your own private certificates for the SSL encryption of the web traffic to and from this unit you can do the following...

### CA Cert and Key
Generate a private certificate-authority certificate and key:
```
openssl genrsa -traditional -out ca_key.pem 2048
openssl req -x509 -new -nodes -key ca_key.pem -days 4096 -out ca_cer.pem
```     

> [!IMPORTANT]
> Keep ca_key.pem absolutely secure, with it someone could make certs 
> that are signed by you!!! Never upload the ca_key.pem to the ESP8266. 
> 
> The ca_cert.pem file is the public certificate for your CA and should be shared.

### Server Key and Request for Cert
Genereate a server key and request file (You need the CA artifacts to do this):
```
openssl genrsa -traditional -out server_key.pem 2048
openssl req -out server_req.csr -key server_key.pem -new
```

> [!NOTE]
> Leave 'extra' attributes blank when filling out information for CSR! 

### Server Cert Using CA Creds
Use CA creds to generate a server certificate from the request:
```
openssl x509 -req -in server_req.csr -out server_cer.pem -sha256 -CAcreateserial -days 4000 -CA ca_cer.pem -CAkey ca_key.pem
```

> [!IMPORTANT]
> Keep server_key.pem secure, this will be used by your server but never
> should it be shared with anyone else!!! The server_cer.pem is the certificate and
> must be shared with others.

## Adding Custom SSL Certificates to Project
To add your own SSL Certificates to the project, simply add a file called `Secrets.h` in the `lib/Secrets` folder of the project. In the Secrets.h file store your Server Certificate using the variable name: `server_cert` and the Server's Private Key using the variable name: `server_key`. The project's `.gitignore` file is setup so that if you choose to upload the code to a git repo the Secrets.h file will not be included, this is to help keep your cert and key from being publicly exposed.

Both variables should be defined similarly to below:
```
#ifndef Secrets_h
#define Secrets_h
#include <pgmspace.h>

const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
BLA,BLA,BLA CERT TEXT HERE...
-----END CERTIFICATE-----
)EOF";

const char server_key[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
BLA,BLA,BLA KEY TEXT HERE...
-----END RSA PRIVATE KEY-----
)EOF";

#endif
```

> [!IMPORTANT]
> !!!ONE MORE THING!!!
> See how the cert and key are defined above??? Proper whitespace is critical for your
> certs to function properly. ESP8266 gives NO ERRORS when they are improperly 
> formatted.
>
> So rather than wasting DAYS trying to figure out why your certs aren't working like 
> I did, just go ahead and throw your neat code indentions out the window and make 
> sure the lines are all shoved to the left of the document. They should work that 
> way.
