#ifndef LZ77LANG_LZ77_LANG_EXPORTS_h_
#define LZ77LANG_LZ77_LANG_EXPORTS_h_

#if defined(COMPONENT_BUILD)
#if defined(OS_WIN)

#if defined(LZ77_LANG_IMPLEMENTATION)
#define LZ77_LANG_EXPORT __declspec(dllexport)
#else
#if defined(OS_WIN_X86)
#pragma comment(lib,"Lz77Lang.lib")
#else
#pragma comment(lib,"Lz77Lang_x64.lib")
#endif // WIN32
#define LZ77_LANG_EXPORT __declspec(dllimport)
#endif  // defined(LZ77_LANG_IMPLEMENTATION)

#else  // defined(WIN32)
#define LZ77_LANG_EXPORT __attribute__((visibility("default")))
#endif

#else  // defined(COMPONENT_BUILD)
#define LZ77_LANG_EXPORT
#endif

#endif