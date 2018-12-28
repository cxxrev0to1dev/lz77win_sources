// compress_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib,"zlib.lib")
#include <stdio.h>
#include <string.h>  // for strlen
#include <assert.h>
#include "zlib.h"
#include "ZLibCompress.h"


int _tmain(int argc, _TCHAR* argv[])
{
  std::string ssss = "Hello Hello Hello Hello Hello Hello!";
  std::vector<std::uint8_t> v;
  std::copy(ssss.begin(), ssss.end(), std::back_inserter(v));
  ZLibCompress zzzz(ssss, ZLibCompress::ZLibCompressType::kCompressHTTPGz);
  ZLibCompress xxxx(zzzz.dst(), ZLibCompress::ZLibCompressType::kUncompressHTTPGz);
  if (xxxx.dst()==v)
	  return 0;
  return -1;
}

