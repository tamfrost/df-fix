#include "Logger.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EM_JS(void, console_log, (const char *level, const char *message), {
   console.log('[%cWASM ' + UTF8ToString(level) + '%c]: ' + UTF8ToString(message), 'color:red', 'color:black');
});
#endif

CLogger* CLogger::m_pThis = nullptr;
CLogger::CLogger() = default;
CLogger* CLogger::GetLogger(){
    if (m_pThis == nullptr){
        m_pThis = new CLogger();
    }
    return m_pThis;
}

void CLogger::Configure(const string& level)
{
//    cout << "LOGGER LEVEL SET TO:" << level << endl;
    this->level = level;
    loglog.clear();
}

void CLogger::log(const char * format, ...)
{
    if (this->level != "DEBUG") return;
    char* sMessage = nullptr;
    int nLength = 0;
    va_list args;
    va_start(args, format);
        nLength = __vscprintf(format, args) + 1;
        sMessage = new char[nLength];
        vsprintf(sMessage, format, args);
        loglog.emplace_back(sMessage);
        #ifdef __EMSCRIPTEN__
            console_log(level.c_str(), sMessage);
        #else
            cout << level << ": ";
            cout << sMessage << endl;
        #endif
    va_end(args);

    delete [] sMessage;
}

void CLogger::log(const string& sMessage)
{
    if (this->level != "DEBUG") return;
//    cout <<  Util::CurrentDateTime() << " " << level << " :\t";
    loglog.emplace_back(sMessage);
    #ifdef __EMSCRIPTEN__
        console_log(level.c_str(), sMessage.c_str());
    #else
        cout << level << ": ";
        cout << sMessage << endl;
    #endif
}

const std::string CLogger::CurrentDateTime()
{
//        time_t     now = time(NULL);
//        struct tm  tstruct;
//        char       buf[80];
//        localtime_s(&tstruct, &now);
//        strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
//        return buf;
    return "HH:mm:ss";
}

int CLogger::__vscprintf (const char * format, va_list pargs) {
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}


CLogger& CLogger::operator<<(const string& sMessage)
{
    if (this->level != "DEBUG") return *this;
//    cout << "\n" << Util::CurrentDateTime() << " " << level << " :\t";
    cout << level << ": ";
    cout << sMessage << endl;
    return *this;
}

const vector<string> &CLogger::getLoglog() const {
    return loglog;
}

void CLogger::clearLog() {
    loglog.clear();
}