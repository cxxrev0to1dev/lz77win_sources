#pragma once

#include "TKTime.h"
#include "base/base_export.h"
#include "base/basictypes.h"

typedef BOOL ( __stdcall * pfnGetSystemTimes)( LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime );
namespace base{
  class BASE_EXPORT CPU
  {
  public:
    CPU(void);
    ~CPU(void);

    // return :
    // % of cpu usage for this process 
    // % cpu systemUsage 
    // uptime
    int GetUsage(int* pSystemUsage, TKTime* pUpTime);
  private:
    static TKDelay s_delay;

    static TKLong s_time;

    static TKLong s_idleTime;
    static TKLong s_kernelTime;
    static TKLong s_userTime;
    static int    s_lastCpu;
    static int    s_cpu[5];

    static TKLong s_kernelTimeProcess;
    static TKLong s_userTimeProcess;
    static int    s_lastCpuProcess;
    static int    s_cpuProcess[5];

    static int    s_count;
    static int    s_index;

    static TKLong s_lastUpTime;

    static HINSTANCE s_hKernel;
    static pfnGetSystemTimes s_pfnGetSystemTimes;

    CRITICAL_SECTION m_lock;
  };
}