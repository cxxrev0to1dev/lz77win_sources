#include "cpu.h"

#include "SmartLock.h"

namespace base {
  TKLong  CPU::s_time;
  TKDelay CPU::s_delay;

  int    CPU::s_count = 0;
  int    CPU::s_index = 0;

  TKLong CPU::s_idleTime;
  TKLong CPU::s_kernelTime;
  TKLong CPU::s_userTime;
  int    CPU::s_lastCpu = 0;
  int    CPU::s_cpu[];

  TKLong CPU::s_kernelTimeProcess;
  TKLong CPU::s_userTimeProcess;
  int    CPU::s_lastCpuProcess;
  int    CPU::s_cpuProcess[];

  TKLong CPU::s_lastUpTime = 0;


  HINSTANCE CPU::s_hKernel = NULL;

  pfnGetSystemTimes CPU::s_pfnGetSystemTimes = NULL;

  CPU::CPU()
  {
    ::InitializeCriticalSection(&m_lock);
    if (s_hKernel == NULL)
    {
      s_hKernel = LoadLibraryW(L"Kernel32.dll");
      if (s_hKernel != NULL)
      {
        s_pfnGetSystemTimes = (pfnGetSystemTimes)GetProcAddress(s_hKernel, "GetSystemTimes");
        if (s_pfnGetSystemTimes == NULL)
        {
          FreeLibrary(s_hKernel); s_hKernel = NULL;
        }
      }
    }

    s_delay.Mark();
  }

  CPU::~CPU()
  {
    if (s_hKernel == NULL)
    {
      FreeLibrary(s_hKernel); s_hKernel = NULL;
    }

    ::DeleteCriticalSection(&m_lock);
  }

  int CPU::GetUsage(int* pSystemUsage, TKTime* pUpTime)
  {
    TKLong sTime;
    int sLastCpu;
    int sLastCpuProcess;
    TKTime sLastUpTime;

    // lock
    {
      SmartLock lock(&m_lock);

      sTime = s_time;
      sLastCpu = s_lastCpu;
      sLastCpuProcess = s_lastCpuProcess;
      sLastUpTime = s_lastUpTime;
    }

    if (s_delay.MSec() <= 200)
    {
      if (pSystemUsage != NULL)
        *pSystemUsage = sLastCpu;

      if (pUpTime != NULL)
        *pUpTime = sLastUpTime;

      return sLastCpuProcess;
    }

    TKLong time;

    TKLong idleTime;
    TKLong kernelTime;
    TKLong userTime;
    TKLong kernelTimeProcess;
    TKLong userTimeProcess;

    GetSystemTimeAsFileTime((LPFILETIME)&time);

    if (sTime == 0)
    {
      // for the system
      if (s_pfnGetSystemTimes != NULL)
      {
        /*BOOL res = */s_pfnGetSystemTimes((LPFILETIME)&idleTime, (LPFILETIME)&kernelTime, (LPFILETIME)&userTime);
      }
      else
      {
        idleTime = 0;
        kernelTime = 0;
        userTime = 0;
      }

      // for this process
      {
        FILETIME createTime;
        FILETIME exitTime;
        GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime,
          (LPFILETIME)&kernelTimeProcess,
          (LPFILETIME)&userTimeProcess);
      }

      // LOCK
      {
        SmartLock lock(&m_lock);

        s_time = time;

        s_idleTime = idleTime;
        s_kernelTime = kernelTime;
        s_userTime = userTime;

        s_kernelTimeProcess = kernelTimeProcess;
        s_userTimeProcess = userTimeProcess;

        s_lastCpu = 0;
        s_lastCpuProcess = 0;

        s_lastUpTime = kernelTime + userTime;

        sLastCpu = s_lastCpu;
        sLastCpuProcess = s_lastCpuProcess;
        sLastUpTime = s_lastUpTime;
      }

      if (pSystemUsage != NULL)
        *pSystemUsage = sLastCpu;

      if (pUpTime != NULL)
        *pUpTime = sLastUpTime;

      s_delay.Mark();
      return sLastCpuProcess;
    }
    /////////////////////////////////////////////////////
    // sTime != 0

    TKLong div = (time - sTime);

    // for the system
    if (s_pfnGetSystemTimes != NULL)
    {
      /*BOOL res = */s_pfnGetSystemTimes((LPFILETIME)&idleTime, (LPFILETIME)&kernelTime, (LPFILETIME)&userTime);
    }
    else
    {
      idleTime = 0;
      kernelTime = 0;
      userTime = 0;
    }

    // for this process
    {
      FILETIME createTime;
      FILETIME exitTime;
      GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime,
        (LPFILETIME)&kernelTimeProcess,
        (LPFILETIME)&userTimeProcess);
    }

    int cpu;
    int cpuProcess;
    // LOCK
    {
      SmartLock lock(&m_lock);

      TKLong usr = userTime - s_userTime;
      TKLong ker = kernelTime - s_kernelTime;
      TKLong idl = idleTime - s_idleTime;

      TKLong sys = (usr + ker);

      if (sys == 0)
        cpu = 0;
      else
        cpu = int((sys - idl) * 100 / sys); // System Idle take 100 % of cpu :-((

      cpuProcess = int((((userTimeProcess - s_userTimeProcess) + (kernelTimeProcess - s_kernelTimeProcess)) * 100) / div);

      s_time = time;

      s_idleTime = idleTime;
      s_kernelTime = kernelTime;
      s_userTime = userTime;

      s_kernelTimeProcess = kernelTimeProcess;
      s_userTimeProcess = userTimeProcess;

      s_cpu[(s_index++) % 5] = cpu;
      s_cpuProcess[(s_index++) % 5] = cpuProcess;
      s_count++;
      if (s_count > 5)
        s_count = 5;

      int i;
      cpu = 0;
      for (i = 0; i < s_count; i++)
        cpu += s_cpu[i];

      cpuProcess = 0;
      for (i = 0; i < s_count; i++)
        cpuProcess += s_cpuProcess[i];

      cpu /= s_count;
      cpuProcess /= s_count;

      s_lastCpu = cpu;
      s_lastCpuProcess = cpuProcess;

      s_lastUpTime = kernelTime + userTime;

      sLastCpu = s_lastCpu;
      sLastCpuProcess = s_lastCpuProcess;
      sLastUpTime = s_lastUpTime;
    }

    //DBGOUT( _T("CPU:%d  sys:%d div %d"), cpuProcess, cpu, div );

    if (pSystemUsage != NULL)
      *pSystemUsage = sLastCpu;

    if (pUpTime != NULL)
      *pUpTime = sLastUpTime;

    s_delay.Mark();
    return sLastCpuProcess;
  }

}