#ifndef HtmlContent_h
    #define HtmlContent_h

    #include <WString.h>
    #include <pgmspace.h>

    const char PROGMEM HTML_PAGE_TEMPLATE[] = {
        "<!DOCTYPE HTML> "
        "<html lang=\"en\"> "
        "<head> "
            "<title>${title}</title> "
            "<style> "
            "body { background-color: #FFFFFF; color: #000000; } "
            "h1 { text-align: center; background-color: #5878B0; color: #FFFFFF; border: 3px; border-radius: 15px; } "
            "h2 { text-align: center; background-color: #58ADB0; color: #FFFFFF; border: 3px; } "
            "#wrapper { background-color: #E6EFFF; padding: 20px; margin-left: auto; margin-right: auto; max-width: 700px; box-shadow: 3px 3px 3px #333 } "
            "#info { font-size: 30px; font-weight: bold; line-height: 150%; } "
            "</style> "
        "</head> "
        ""
        "<div id=\"wrapper\"> "
            "<h1>${heading}</h1> "
            "<div id=\"info\">${content}</div> "
        "</div> "
        "</html>"
    };

    const char PROGMEM ADMIN_PAGE[] = {
        "<form name=\"settings\" method=\"post\" id=\"settings\" action=\"admin\"> "
            "<h2>WiFi</h2> "
            "SSID: <input maxlength=\"32\" type=\"text\" value=\"${ssid}\" name=\"ssid\" id=\"ssid\"> <br> "
            "Password: <input maxlength=\"63\" type=\"text\" value=\"${pwd}\" name=\"pwd\" id=\"pwd\"> <br> "
            "<h2>Application</h2> "
            "Title: <input maxlength=\"50\" type=\"text\" value=\"${title}\" name=\"title\" id=\"title\"> <br> "
            "Heading: <input maxlength=\"50\" type=\"text\" value=\"${heading}\" name=\"heading\" id=\"heading\"> <br> "
            "Units:<br>"
            "<input type=\"radio\" id=\"celsius\" name=\"units\" value=\"celsius\" ${unitcchecked}>"
            "<label for=\"celsius\">Celsius</label>"
            "<br>"
            "<input type=\"radio\" id=\"fahrenheit\" name=\"units\" value=\"fahrenheit\" ${unitfchecked}>"
            "<label for=\"fahrenheit\">Fahrenheit</label>"
            "<h2>Admin</h2> "
            "Admin User: <input maxlength=\"12\" type=\"text\" value=\"${adminuser}\" name=\"adminuser\" id=\"adminuser\"> <br> "
            "Admin Password: <input maxlength=\"12\" type=\"text\" value=\"${adminpwd}\" name=\"adminpwd\" id=\"adminpwd\"> <br> "
            "<br> "
            "<input type=\"submit\"> <a href='/'><h4>Home</h4></a>"
        "</form>"
    };

    const char PROGMEM ROOT_PAGE[] = {
        "Temperature:\t${temp}&deg;${unit}<br>"
        "Humidity:\t${humidity}%<br><br>"
    };

#endif