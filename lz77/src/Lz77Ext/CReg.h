#ifndef CREG_H_
#define CREG_H_

#include <windows.h>

class CKey
{
  HKEY _object;
public:
  CKey() : _object(NULL) {}
  ~CKey() { Close(); }

  operator HKEY() const { return _object; }
  void Attach(HKEY key) { _object = key; }
  HKEY Detach()
  {
    HKEY key = _object;
    _object = NULL;
    return key;
  }

  LONG Create(HKEY parentKey, LPCTSTR keyName,
    LPTSTR keyClass = REG_NONE, DWORD options = REG_OPTION_NON_VOLATILE,
    REGSAM accessMask = KEY_ALL_ACCESS,
    LPSECURITY_ATTRIBUTES securityAttributes = NULL,
    LPDWORD disposition = NULL) throw();
  LONG Open(HKEY parentKey, LPCTSTR keyName, REGSAM accessMask = KEY_ALL_ACCESS) throw();

  LONG Close() throw();
  LONG SetValue(LPCTSTR valueName, LPCTSTR value) throw();
};

#endif // !CREG_H_
