#include <iostream>
#include "log.hh"

void Log::print(const ckcore::tchar *format,...)
{
    va_list args;
    va_start(args,format);

    ckcore::tchar buffer[128];
#ifdef _WINDOWS
#ifdef _UNICODE
    _vsnwprintf(buffer,sizeof(buffer)/sizeof(ckcore::tchar) - 1,format,args);
#else
    _vsnprintf(buffer,sizeof(buffer)/sizeof(ckcore::tchar) - 1,format,args);
#endif
#else
    vsnprintf(buffer,sizeof(buffer)/sizeof(ckcore::tchar) - 1,format,args);
#endif

    std::cout << buffer;
}

void Log::print_line(const ckcore::tchar *format,...)
{
    va_list args;
    va_start(args,format);

    ckcore::tchar buffer[128];
#ifdef _WINDOWS
#ifdef _UNICODE
    _vsnwprintf(buffer,sizeof(buffer)/sizeof(ckcore::tchar) - 1,format,args);
#else
    _vsnprintf(buffer,sizeof(buffer)/sizeof(ckcore::tchar) - 1,format,args);
#endif
#else
    vsnprintf(buffer,sizeof(buffer)/sizeof(ckcore::tchar) - 1,format,args);
#endif

    std::cout << buffer << std::endl;
}

