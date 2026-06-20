#ifndef POSITION_LOGGER_H
#define POSITION_LOGGER_H

#include <fstream>
#include <iostream>
#include <cstdarg>
#include <vector>
#include <string>
using namespace std;
#define Logger CLogger::GetLogger()

class CLogger
{
public:
    void Configure(const string& level);
    void log(const string& sMessage);
    void log(const char * format, ...);
    CLogger& operator<<(const string& sMessage);
    static CLogger* GetLogger();
    void clearLog();
private:
    CLogger();

public:
    const vector<string> &getLoglog() const;

private:
    CLogger(const CLogger&){};             // copy constructor is private
    CLogger& operator=(const CLogger&){ return *this; };  // assignment operator is private
    vector<string> loglog;
    static const string m_sFileName;
    static CLogger* m_pThis;
    static ofstream m_Logfile;
    string level;
    static const std::string CurrentDateTime();
    static int __vscprintf (const char * format, va_list pargs);
};

#endif //POSITION_LOGGER_H
