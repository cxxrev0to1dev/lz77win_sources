// dllmain.h : Declaration of module class.

class CLz77ExtModule : public ATL::CAtlDllModuleT< CLz77ExtModule >
{
public :
	DECLARE_LIBID(LIBID_Lz77ExtLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_LZ77EXT, "{E06C59E1-BA0F-449A-9012-E89228C44F1D}")
};

extern class CLz77ExtModule _AtlModule;
