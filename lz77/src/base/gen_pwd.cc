#include "base/gen_pwd.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cctype>

namespace base{
	const unsigned long kMaxTempLength = 1024;
	const unsigned char password_table[] = "abcdef<>@jklmnopqrstuvwxyzABCDEFGH%&/KLMOPQRSTUVWXYZ0123456789^!$IJ(\"N)=??\\*+',27h,'#;,:._-ghi";
  
  GenPwd* GenPwd::GetInstance() {
    static GenPwd* new_p = nullptr;
    if (!new_p){
      new_p = new GenPwd;
    }
    return new_p;
  }
  GenPwd::GenPwd(void):str_(NULL){
		str_ = new char[kMaxTempLength];
    SetPasscode(32);
	}
  GenPwd::~GenPwd(void){
		if(str_){
			delete[] str_;
			str_ = NULL;
		}
		length_ = 0;
	}
	void GenPwd::SetPasscode(const size_t length){
		if(length>0&&length<kMaxTempLength){
			length_ = length;
		}
		memset(str_,0,kMaxTempLength);
		RandomPasscode();
	}
  void GenPwd::reset() {
    if (!str_){
      str_ = new char[kMaxTempLength];
    }
    memset(str_, 0, kMaxTempLength);
    RandomPasscode();
  }
	char* GenPwd::GetPasscode(){
    while (str_&&strlen(str_)!=32){
      reset();
    }
		return str_;
	}
	//referense:http://www.cryptool-online.org/index.php?option=com_cto&view=tool&Itemid=136&lang=en
	void GenPwd::RandomPasscode(){
		srand(static_cast<unsigned int>(time(NULL)));
		for(unsigned int i=0;i<length_;i++){
			int rand_position = (static_cast<int>(rand()%sizeof(password_table)-1));
			str_[i] = password_table[static_cast<int>(floor(static_cast<long double>(rand_position)))];
			while(i>0&&i<length_&&isupper(str_[i-1])&&isupper(str_[i])){
				rand_position = (static_cast<int>(rand()%sizeof(password_table)-1));
				str_[i] = password_table[static_cast<int>(floor(static_cast<long double>(rand_position)))];
			}
			while(i>0&&i<length_&&islower(str_[i-1])&&islower(str_[i])){
				rand_position = (static_cast<int>(rand()%sizeof(password_table)-2));
				str_[i] = password_table[static_cast<int>(floor(static_cast<long double>(rand_position)))];
			}
			while(i>0&&i<length_&&isdigit(str_[i-1])&&isdigit(str_[i])){
				rand_position = (static_cast<int>(rand()%sizeof(password_table)-3));
				str_[i] = password_table[static_cast<int>(floor(static_cast<long double>(rand_position)))];
			}
			while(i>0&&i<length_&&ispunct(str_[i-1])&&ispunct(str_[i])){
				rand_position = (static_cast<int>(rand()%sizeof(password_table)-4));
				str_[i] = password_table[static_cast<int>(floor(static_cast<long double>(rand_position)))];
			}
		}
	}
}
