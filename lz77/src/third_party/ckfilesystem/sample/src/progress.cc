#include <iostream>
#include "progress.hh"

Progress::Progress() : last_progress_(0)
{
}

void Progress::set_progress(unsigned char progress)
{
    if (progress > last_progress_ + 10)
    {
        std::cout << "Progress: " << (int)progress << "%." << std::endl;
        last_progress_ = progress;
    }
}

void Progress::set_status(const ckcore::tchar *format,...)
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

    std::cout << "Status: " << buffer << std::endl;
}

void Progress::notify(MessageType type,const ckcore::tchar *format,...)
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

    switch (type)
    {
        case ckcore::Progress::ckINFORMATION:
            std::cout << "Information: ";
            break;

        case ckcore::Progress::ckWARNING:
            std::cout << "Warning: ";
            break;

        case ckcore::Progress::ckERROR:
            std::cerr << "Error: ";
            break;

        case ckcore::Progress::ckEXTERNAL:
            std::cout << "External: ";
            break;
    }

    std::cout << buffer << std::endl;
}

bool Progress::cancelled()
{
    return false;
}

