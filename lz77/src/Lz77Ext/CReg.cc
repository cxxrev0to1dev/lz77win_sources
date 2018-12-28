
#include "stdafx.h"
#include "CReg.h"

LONG CKey::Create(HKEY parentKey, LPCTSTR keyName,
  LPTSTR keyClass, DWORD options, REGSAM accessMask,
  LPSECURITY_ATTRIBUTES securityAttributes, LPDWORD disposition) throw()
{
  DWORD dispositionReal;
  HKEY key = NULL;
  LONG res = RegCreateKeyEx(parentKey, keyName, 0, keyClass,
    options, accessMask, securityAttributes, &key, &dispositionReal);
  if (disposition != NULL)
    *disposition = dispositionReal;
  if (res == ERROR_SUCCESS)
  {
    res = Close();
    _object = key;
  }
  return res;
}

LONG CKey::Open(HKEY parentKey, LPCTSTR keyName, REGSAM accessMask) throw()
{
  HKEY key = NULL;
  LONG res = RegOpenKeyEx(parentKey, keyName, 0, accessMask, &key);
  if (res == ERROR_SUCCESS)
  {
    res = Close();
    _object = key;
  }
  return res;
}

LONG CKey::Close() throw()
{
  LONG res = ERROR_SUCCESS;
  if (_object != NULL)
  {
    res = RegCloseKey(_object);
    _object = NULL;
  }
  return res;
}

LONG CKey::SetValue(LPCTSTR name, LPCTSTR value) throw()
{
  return RegSetValueEx(_object, name, 0, REG_SZ,
    (const BYTE *)value, (lstrlen(value) + 1) * sizeof(TCHAR));
}