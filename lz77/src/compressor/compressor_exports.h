#ifndef COMPRESSOR_COMPRESSOR_EXPORTS_h_
#define COMPRESSOR_COMPRESSOR_EXPORTS_h_

#if defined(COMPONENT_BUILD)
#if defined(OS_WIN)

#if defined(COMPRESSOR_IMPLEMENTATION)
#define COMPRESSOR_EXPORT __declspec(dllexport)
#else
#define COMPRESSOR_EXPORT __declspec(dllimport)
#if defined(OS_WIN_X86)
#pragma comment(lib,"compressor.lib")
#else
#pragma comment(lib,"compressor_x64.lib")
#endif
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#define COMPRESSOR_EXPORT __attribute__((visibility("default")))
#endif

#else  // defined(COMPONENT_BUILD)
#define COMPRESSOR_EXPORT
#endif

#endif