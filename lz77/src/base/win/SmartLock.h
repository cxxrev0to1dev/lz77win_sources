#pragma once

class SmartLock
{
public:
    SmartLock( LPCRITICAL_SECTION sect ){  EnterCriticalSection( pLock=sect ); };
    ~SmartLock(){ LeaveCriticalSection( pLock ); };
private:
    LPCRITICAL_SECTION      pLock;
};

