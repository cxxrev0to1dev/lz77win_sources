#include "base/win/windows_version.h"
#include <windows.h>
#include "base/win/registry.h"

namespace base {
  namespace win {
    namespace {
      
      typedef BOOL(WINAPI *GetProductInfoPtr)(DWORD, DWORD, DWORD, DWORD, PDWORD);

      Version MajorMinorBuildToVersion(int major, int minor, int build) {
        if ((major == 5) && (minor > 0)) {
          // Treat XP Pro x64, Home Server, and Server 2003 R2 as Server 2003.
          return (minor == 1) ? VERSION_XP : VERSION_SERVER_2003;
        }
        else if (major == 6) {
          switch (minor) {
          case 0:
            // Treat Windows Server 2008 the same as Windows Vista.
            return VERSION_VISTA;
          case 1:
            // Treat Windows Server 2008 R2 the same as Windows 7.
            return VERSION_WIN7;
          case 2:
            // Treat Windows Server 2012 the same as Windows 8.
            return VERSION_WIN8;
          default:
            if(minor==3)
              return VERSION_WIN8_1;
          }
        }
        else if (major == 10) {
          if (build < 10586) {
            return VERSION_WIN10;
          }
          else if (build < 14393) {
            return VERSION_WIN10_TH2;
          }
          else if (build < 15063) {
            return VERSION_WIN10_RS1;
          }
          else if (build < 16299) {
            return VERSION_WIN10_RS2;
          }
          else if (build < 17134) {
            return VERSION_WIN10_RS3;
          }
          else {
            return VERSION_WIN10_RS4;
          }
        }
        else if (major > 6) {
          return VERSION_WIN_LAST;
        }

        return VERSION_PRE_XP;
      }
      int GetUBR() {
        // The values under the CurrentVersion registry hive are mirrored under
        // the corresponding Wow6432 hive.
        static constexpr wchar_t kRegKeyWindowsNTCurrentVersion[] =
          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

        base::win::RegKey key;
        if (key.Open(HKEY_LOCAL_MACHINE, kRegKeyWindowsNTCurrentVersion,
          KEY_QUERY_VALUE) != ERROR_SUCCESS) {
          return 0;
        }

        DWORD ubr = 0;
        key.ReadValueDW(L"UBR", &ubr);

        return static_cast<int>(ubr);
      }
    }
    OSInfo::OSInfo()
      : version_(VERSION_PRE_XP) {
      OSVERSIONINFOEX version_info = { sizeof version_info };
      ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info));
      version_number_.major = version_info.dwMajorVersion;
      version_number_.minor = version_info.dwMinorVersion;
      version_number_.build = version_info.dwBuildNumber;
      version_number_.patch = GetUBR();
      version_ = MajorMinorBuildToVersion(
        version_number_.major, version_number_.minor, version_number_.build);
      service_pack_.major = version_info.wServicePackMajor;
      service_pack_.minor = version_info.wServicePackMinor;
      service_pack_str_ = version_info.szCSDVersion;


      SYSTEM_INFO system_info = {};
      ::GetNativeSystemInfo(&system_info);
      switch (system_info.wProcessorArchitecture) {
      case PROCESSOR_ARCHITECTURE_INTEL: architecture_ = X86_ARCHITECTURE; break;
      case PROCESSOR_ARCHITECTURE_AMD64: architecture_ = X64_ARCHITECTURE; break;
      case PROCESSOR_ARCHITECTURE_IA64:  architecture_ = IA64_ARCHITECTURE; break;
      }

      GetProductInfoPtr get_product_info;
      DWORD os_type;

      if (version_info.dwMajorVersion == 6 || version_info.dwMajorVersion == 10) {
        // Only present on Vista+.
        get_product_info = reinterpret_cast<GetProductInfoPtr>(
          ::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "GetProductInfo"));

        get_product_info(version_info.dwMajorVersion, version_info.dwMinorVersion,
          0, 0, &os_type);
        switch (os_type) {
        case PRODUCT_CLUSTER_SERVER:
        case PRODUCT_DATACENTER_SERVER:
        case PRODUCT_DATACENTER_SERVER_CORE:
        case PRODUCT_ENTERPRISE_SERVER:
        case PRODUCT_ENTERPRISE_SERVER_CORE:
        case PRODUCT_ENTERPRISE_SERVER_IA64:
        case PRODUCT_SMALLBUSINESS_SERVER:
        case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
        case PRODUCT_STANDARD_SERVER:
        case PRODUCT_STANDARD_SERVER_CORE:
        case PRODUCT_WEB_SERVER:
          version_type_ = SUITE_SERVER;
          break;
        case PRODUCT_PROFESSIONAL:
        case PRODUCT_ULTIMATE:
          version_type_ = SUITE_PROFESSIONAL;
          break;
        case PRODUCT_ENTERPRISE:
        case PRODUCT_ENTERPRISE_E:
        case PRODUCT_ENTERPRISE_EVALUATION:
        case PRODUCT_ENTERPRISE_N:
        case PRODUCT_ENTERPRISE_N_EVALUATION:
//         case PRODUCT_ENTERPRISE_S:
//         case PRODUCT_ENTERPRISE_S_EVALUATION:
//         case PRODUCT_ENTERPRISE_S_N:
//         case PRODUCT_ENTERPRISE_S_N_EVALUATION:
        case PRODUCT_BUSINESS:
        case PRODUCT_BUSINESS_N:
          version_type_ = SUITE_ENTERPRISE;
          break;
//         case PRODUCT_EDUCATION:
//         case PRODUCT_EDUCATION_N:
//           version_type_ = SUITE_EDUCATION;
//           break;
        case PRODUCT_HOME_BASIC:
        case PRODUCT_HOME_PREMIUM:
        case PRODUCT_STARTER:
        default:
          version_type_ = SUITE_HOME;
          break;
        }
      }
      else if (version_info.dwMajorVersion == 5 &&
        version_info.dwMinorVersion == 2) {
        if (version_info.wProductType == VER_NT_WORKSTATION &&
          system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
          version_type_ = SUITE_PROFESSIONAL;
        }
        else if (version_info.wSuiteMask & VER_SUITE_WH_SERVER) {
          version_type_ = SUITE_HOME;
        }
        else {
          version_type_ = SUITE_SERVER;
        }
      }
      else if (version_info.dwMajorVersion == 5 &&
        version_info.dwMinorVersion == 1) {
        if (version_info.wSuiteMask & VER_SUITE_PERSONAL)
          version_type_ = SUITE_HOME;
        else
          version_type_ = SUITE_PROFESSIONAL;
      }
      else {
        // Windows is pre XP so we don't care but pick a safe default.
        version_type_ = SUITE_HOME;
      }
    }
    OSInfo::~OSInfo() {
    }
    OSInfo* OSInfo::GetInstance() {
      static OSInfo* info;
      if (!info) {
        OSInfo* new_info = new OSInfo();
        if (InterlockedCompareExchangePointer(
          reinterpret_cast<PVOID*>(&info), new_info, NULL)) {
          delete new_info;
        }
      }
      return info;
    }
    Version GetVersion() {
      return OSInfo::GetInstance()->version();
    }
    bool IsX86_ARCHITECTURE() {
      return OSInfo::GetInstance()->architecture() == base::win::OSInfo::X86_ARCHITECTURE;
    }
    bool IsServicePack1() {
      OSInfo::ServicePack sp = OSInfo::GetInstance()->service_pack();
      return (sp.major == 1 && sp.minor == 0);
    }
    bool IsServicePack2() {
      OSInfo::ServicePack sp = OSInfo::GetInstance()->service_pack();
      return (sp.major == 2 && sp.minor == 0);
    }
  }
}