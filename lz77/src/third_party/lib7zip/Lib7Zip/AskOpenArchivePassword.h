#ifndef LIB7ZIP_ASK_OPEN_ARCHIVE_PASSWORD_H_
#define LIB7ZIP_ASK_OPEN_ARCHIVE_PASSWORD_H_

#include <string>

class AskOpenArchivePassword
{
public:
  AskOpenArchivePassword() {}
  virtual ~AskOpenArchivePassword() {}
  virtual void AskPasswordUI() = 0;
  virtual const wchar_t* GetPassword() = 0;
  virtual void SetPassword(const wchar_t* pwd) = 0;
private:
  std::wstring password_;
};

#endif // !LIB7ZIP_ASK_OPEN_ARCHIVE_PASSWORD_H_
