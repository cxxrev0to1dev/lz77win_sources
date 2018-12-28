#include <string>

namespace base {
  namespace win {
    enum Version {
      VERSION_PRE_XP = 0,  // Not supported.
      VERSION_XP = 1,
      VERSION_SERVER_2003 = 2,  // Also includes XP Pro x64 and Server 2003 R2.
      VERSION_VISTA = 3,        // Also includes Windows Server 2008.
      VERSION_WIN7 = 4,         // Also includes Windows Server 2008 R2.
      VERSION_WIN8 = 5,         // Also includes Windows Server 2012.
      VERSION_WIN8_1 = 6,       // Also includes Windows Server 2012 R2.
      VERSION_WIN10 = 7,        // Threshold 1: Version 1507, Build 10240.
      VERSION_WIN10_TH2 = 8,    // Threshold 2: Version 1511, Build 10586.
      VERSION_WIN10_RS1 = 9,    // Redstone 1: Version 1607, Build 14393.
      VERSION_WIN10_RS2 = 10,   // Redstone 2: Version 1703, Build 15063.
      VERSION_WIN10_RS3 = 11,   // Redstone 3: Version 1709, Build 16299.
      VERSION_WIN10_RS4 = 12,   // Redstone 4: Version 1803, Build 17134.
                                // On edit, update tools\metrics\histograms\enums.xml "WindowsVersion" and
                                // "GpuBlacklistFeatureTestResultsWindows2".
                                VERSION_WIN_LAST,  // Indicates error condition.
    };

    // A rough bucketing of the available types of versions of Windows. This is used
    // to distinguish enterprise enabled versions from home versions and potentially
    // server versions. Keep these values in the same order, since they are used as
    // is for metrics histogram ids.
    enum VersionType {
      SUITE_HOME = 0,
      SUITE_PROFESSIONAL,
      SUITE_SERVER,
      SUITE_ENTERPRISE,
      SUITE_EDUCATION,
      SUITE_LAST,
    };
    class OSInfo
    {
    public:
      struct VersionNumber {
        int major;
        int minor;
        int build;
        int patch;
      };

      struct ServicePack {
        int major;
        int minor;
      };
      // The processor architecture this copy of Windows natively uses.  For
      // example, given an x64-capable processor, we have three possibilities:
      //   32-bit Chrome running on 32-bit Windows:           X86_ARCHITECTURE
      //   32-bit Chrome running on 64-bit Windows via WOW64: X64_ARCHITECTURE
      //   64-bit Chrome running on 64-bit Windows:           X64_ARCHITECTURE
      enum WindowsArchitecture {
        X86_ARCHITECTURE,
        X64_ARCHITECTURE,
        IA64_ARCHITECTURE,
        OTHER_ARCHITECTURE,
      };

      // Whether a process is running under WOW64 (the wrapper that allows 32-bit
      // processes to run on 64-bit versions of Windows).  This will return
      // WOW64_DISABLED for both "32-bit Chrome on 32-bit Windows" and "64-bit
      // Chrome on 64-bit Windows".  WOW64_UNKNOWN means "an error occurred", e.g.
      // the process does not have sufficient access rights to determine this.
      enum WOW64Status {
        WOW64_DISABLED,
        WOW64_ENABLED,
        WOW64_UNKNOWN,
      };
      static OSInfo* GetInstance();
      Version version() const { return version_; }
      VersionNumber version_number() const { return version_number_; }
      VersionType version_type() const { return version_type_; }
      ServicePack service_pack() const { return service_pack_; }
      std::wstring service_pack_str() const { return service_pack_str_; }
      WindowsArchitecture architecture() const { return architecture_; }
    private:
      OSInfo();
      ~OSInfo();
      Version version_;
      VersionNumber version_number_;
      VersionType version_type_;
      ServicePack service_pack_;
      std::wstring service_pack_str_;
      WindowsArchitecture architecture_;
    };
    Version GetVersion();
    bool IsX86_ARCHITECTURE();
    bool IsServicePack1();
    bool IsServicePack2();
  }
}