#include "stdafx.h"

#include <new>

#include "FactoryClass.h"  
#include "lz77_right_menu.h"

LONG g_cObjs;

void DllAddRef() { InterlockedIncrement(&g_cObjs); }
void DllRelease() { InterlockedDecrement(&g_cObjs); }

class CShellExtension :public CLz77RMenu
{
public:
  CShellExtension():m_cRef(1){
    DllAddRef();
  }
  //IUnknown implementation
  virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
  virtual ULONG __stdcall AddRef() { 
    return  InterlockedIncrement(&m_cRef);
  }
  virtual ULONG __stdcall Release() {
    ULONG uCount = InterlockedDecrement(&m_cRef);
    if (uCount==0){
      delete this;
    }
    return uCount;
  }
private:
  ~CShellExtension() {
    DllRelease();
  }
  LONG m_cRef;
};

HRESULT __stdcall CShellExtension::QueryInterface(const IID& iid, void** ppv)
{
  IUnknown *punk = NULL;
  if (iid == IID_IContextMenu) {
    punk = static_cast<IContextMenu*>(this);
  }
  else if (iid == IID_IShellExtInit) {
    punk = static_cast<IShellExtInit*>(this);
  }
  *ppv = punk;
  if (punk) {
    punk->AddRef();
    return S_OK;
  }
  else {
    return E_NOINTERFACE;
  }
}


///////////////IUnknown接口方法  
HRESULT CFactoryClass::QueryInterface(REFIID riid, void **ppv)
{
  IUnknown *punk = NULL;
  if (riid == IID_IUnknown || riid == IID_IClassFactory) {
    punk = static_cast<IClassFactory*>(this);
  }
  *ppv = punk;
  if (punk) {
    punk->AddRef();
    return S_OK;
  }
  else {
    return E_NOINTERFACE;
  }
}
ULONG CFactoryClass::AddRef(){
  return 2;
}
ULONG CFactoryClass::Release(){
  return 1;
}
HRESULT CFactoryClass::LockServer(BOOL fLock){
  if (fLock) DllAddRef();
  else       DllRelease();
  return S_OK;
}
HRESULT CFactoryClass::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
  *ppv = NULL;
  if (pUnkOuter) return CLASS_E_NOAGGREGATION;
  CShellExtension *pse = new(std::nothrow) CShellExtension();
  if (!pse) return E_OUTOFMEMORY;
  HRESULT hr = pse->QueryInterface(riid, ppv);
  pse->Release();
  return hr;
}