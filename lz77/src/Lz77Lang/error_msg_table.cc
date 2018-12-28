#include "Lz77Lang/error_msg_table.h"
#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"//rapidxml::file  
#include "rapidxml/rapidxml_print.hpp"//rapidxml::print
#include "base/string_conv.h"


namespace lang {
  namespace internal{
    ErrorMsgTable::ErrorMsgTable(std::basic_istream<char>& in){
      error_msg_table_.clear();
      temp_.resize(0);
      rapidxml::file<> fdoc(in);
      rapidxml::xml_document<> doc;
      doc.parse<0>(fdoc.data());
      base::StringConv str_conv;
      const rapidxml::xml_node<> *cross_platform = doc.first_node("CrossPlatform");
      if (NULL == cross_platform) {
        return;
      }
      rapidxml::xml_node<> *error_msg_table_node = cross_platform->first_node("ErrorMsgTable");
      rapidxml::xml_node<> *datum = (error_msg_table_node ==nullptr?nullptr: error_msg_table_node->first_node("sub"));
      while (error_msg_table_node&&NULL != datum){
        rapidxml::xml_attribute<char>* attr = datum->first_attribute("name");
        while (attr != NULL){
          str_conv.StrToWStr(attr->value());
          std::wstring lvalue = str_conv.wdst();
          attr = attr->next_attribute();
          if (attr->name() == std::string("string")) {
            str_conv.StrToWStr(attr->value());
            error_msg_table_[lvalue] = str_conv.wdst();
          }
         
          attr = attr->next_attribute();
        }
        datum = datum->next_sibling();
      }
      return;
    }
    ErrorMsgTable::~ErrorMsgTable(){
      error_msg_table_.clear();
      temp_.resize(0);
    }
    const std::wstring& ErrorMsgTable::GetErrorMsg(std::wstring msg_index) {
      temp_.resize(0);
      std::map<std::wstring, std::wstring>::iterator it;
      for (it = error_msg_table_.begin();it != error_msg_table_.end();it++) {
        if (msg_index == it->first.c_str()) {
          temp_ = it->second;
          break;
        }
      }
      return temp_;
    }
  }
}