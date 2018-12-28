#ifndef BASE_GEN_PWD_H_
#define BASE_GEN_PWD_H_
//////////////////////////////////////////////////////////////////////////
#include "base/base_export.h"
#include "base/basictypes.h"
//////////////////////////////////////////////////////////////////////////
namespace base{
	class GenPwd{
	public:
    BASE_EXPORT static GenPwd* GetInstance();
    BASE_EXPORT GenPwd();
    BASE_EXPORT virtual ~GenPwd();
    BASE_EXPORT void reset();
    BASE_EXPORT char* GetPasscode();
	private:
    void SetPasscode(const size_t length);
		void RandomPasscode();
		char* str_;
		size_t length_;
	};
}

#endif