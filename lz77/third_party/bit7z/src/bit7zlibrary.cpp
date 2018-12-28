#include "../include/bit7zlibrary.hpp"

#include "../include/bitexception.hpp"
#include "../include/bitguids.hpp"

#include <sstream>

using namespace bit7z;
using std::ostringstream;

#ifdef USE_STATIC_7Z_COMPONENT
#include "compressor/lib7z_exports.h"
#endif // !USE_STATIC_7Z_COMPONENT

Bit7zLibrary::Bit7zLibrary( const std::wstring &dll_path ) : mLibrary( LoadLibrary( dll_path.c_str() ) ) {
#if !defined(USE_STATIC_7Z_COMPONENT)
  if (!mLibrary) {
    ostringstream os;
    os << GetLastError();
    throw BitException("Cannot load 7-zip library (error " + os.str() + ")");
  }
  mCreateObjectFunc = reinterpret_cast<CreateObjectFunc>(GetProcAddress(mLibrary, "CreateObject"));
#else
  mCreateObjectFunc = reinterpret_cast<CreateObjectFunc>(CreateObject);
#endif
    
    

    if ( !mCreateObjectFunc ) {
        ostringstream os;
        os << GetLastError();
        throw BitException( "Cannot get CreateObject (error " + os.str() + ")" );
    }
}

Bit7zLibrary::~Bit7zLibrary() {
#if !defined(USE_STATIC_7Z_COMPONENT)
  FreeLibrary(mLibrary);
#endif
}

void Bit7zLibrary::createArchiveObject( const GUID *format_ID, const GUID *interface_ID, void **out_object ) const {
    if ( mCreateObjectFunc( format_ID, interface_ID, out_object ) != S_OK ) {
        throw BitException( "Cannot get class object" );
    }
}
