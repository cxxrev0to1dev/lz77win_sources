// TKType.h - basic type
#pragma once

#include <windows.h>

typedef __int64             TKLong;
typedef __int64             TKLongID;
typedef __int64             TKRefCount;
typedef unsigned __int64    TKULong;

typedef DWORD               TKDATE;

// undef const
const int   UNDEF_INT     = 0x80000000;
const DWORD UNDEF_DWORD   = 0xFFFFFFFF;


typedef struct TKuuid
{
    __int64     m_lo;
    __int64     m_hi;
}
    TKuuid;

#define /*BOOL*/ TKuuidEQ(u1,u2) (memcmp( u1, u2, sizeof(TKuuid) )==0)

struct TKTIME
{
    int     m_hour;
    int     m_minute;
    int     m_second;
    int     m_milliseconds;
};

// The TKTime structure holds an unsigned 64-bit date and time value.
// This value could represents the number of 100-nanosecond units since the beginning of January 1, 1601.
class TKTime
{
public:
    TKTime( void );
    TKTime( LPCTSTR szGetSystemTimeDummy );  // szGetSystemTimeDummy is not used it's just to Get the system time in the constructor
    TKTime( const TKTime& time );
    TKTime( const FILETIME& fileTime );
    TKTime( const SYSTEMTIME& sysTime );
    TKTime( DWORD days, DWORD hours, DWORD minutes, DWORD seconds, DWORD millisseconds );
    TKTime( WORD year, WORD month, WORD day, WORD hour, WORD minute, WORD second );
    TKTime( TKLong time );

    ~TKTime( void ) {}

    TKTime& operator =( TKLong time );
    TKTime& operator +=( TKLong time );
    TKTime& operator -=( TKLong time );
    TKTime& operator /=( TKLong time );
    TKTime& operator *=( TKLong time );

    operator FILETIME( void );
    operator SYSTEMTIME( void );
    operator TKTIME( void );
    operator TKLong( void );

    TKTime& Set( DWORD days, DWORD hours, DWORD minutes, DWORD seconds, DWORD milliseconds );
    TKTime& Set( WORD year, WORD month, WORD day, WORD hour, WORD minute, WORD second );
    TKTime& Set( DWORD milliseconds );  // becarefull you set some milliseconds
    TKTime& SetSystemTime( void );

    void LocalToUTC( void );
    void UTCToLocal( void );

private:
    TKLong m_time;
};

