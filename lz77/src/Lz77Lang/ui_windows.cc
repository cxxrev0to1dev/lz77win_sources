#include "Lz77Lang/ui_windows.h"
#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"//rapidxml::file  
#include "rapidxml/rapidxml_print.hpp"//rapidxml::print
#include "base/string_conv.h"
#include "Lz77Lang/lz77_lang.h"


namespace lang {
  namespace internal{
    UIWindows::UIWindows(std::basic_istream<char>& in){
      ui_win_.clear();
      sub_menu_count_ = 0;
      rapidxml::file<> fdoc(in);
      rapidxml::xml_document<> doc;
      doc.parse<0>(fdoc.data());
      const rapidxml::xml_node<> *cross_platform = doc.first_node("CrossPlatform");
      if (NULL == cross_platform) {
        return;
      }
      const rapidxml::xml_node<> *ui_windows = cross_platform->first_node("Windows");
      if (NULL == ui_windows) {
        return;
      }
      rapidxml::xml_node<> *root_shell_externtion = ui_windows->first_node("RootShellExterntion");
      if (NULL == root_shell_externtion) {
        return;
      }
      base::StringConv str_conv;
      rapidxml::xml_attribute<char>* root_shell_externtion_attr = root_shell_externtion->first_attribute("name");
      while (root_shell_externtion_attr != NULL){
        str_conv.StrToWStr(root_shell_externtion_attr->value());
        std::wstring lvalue = str_conv.wdst();
        root_shell_externtion_attr = root_shell_externtion_attr->next_attribute();
        if (root_shell_externtion_attr->name()== std::string("string")){
          str_conv.StrToWStr(root_shell_externtion_attr->value());
          ui_win_[lvalue] = str_conv.wdst();
        }
        root_shell_externtion_attr = root_shell_externtion_attr->next_attribute();
      }
      rapidxml::xml_node<> *sub_shell_externtion = ui_windows->first_node("SubShellExterntion");
      rapidxml::xml_node<> *datum = (sub_shell_externtion==nullptr?nullptr:sub_shell_externtion->first_node("sub"));
      while (sub_shell_externtion&&NULL != datum){
        rapidxml::xml_attribute<char>* attr = datum->first_attribute("name");
        while (attr != NULL){
          str_conv.StrToWStr(attr->value());
          std::wstring lvalue = str_conv.wdst();
          attr = attr->next_attribute();
          if (attr->name() == std::string("string")) {
            str_conv.StrToWStr(attr->value());
            ui_win_[lvalue] = str_conv.wdst();
            ++sub_menu_count_;
          }
         
          attr = attr->next_attribute();
        }
        datum = datum->next_sibling();
      }
      return;
    }
    UIWindows::~UIWindows(){
      ui_win_.clear();
      sub_menu_count_ = 0;
    }
    void UIWindows::MENU_ROOT(std::wstring& str) {
      std::map<std::wstring, std::wstring>::iterator it;
      for (it = ui_win_.begin();it != ui_win_.end();it++) {
        if (std::wstring(kStrMenuRoot)==it->first.c_str()){
          str = it->second;
          break;
        }
      }
    }
    void UIWindows::MENU_ARCHIVE(std::wstring& str) {
      std::map<std::wstring, std::wstring>::iterator it;
      for (it = ui_win_.begin();it != ui_win_.end();it++) {
        if (std::wstring(kStrMenuArchive)==it->first.c_str()) {
          str = it->second;
          break;
        }
      }
    }
    void UIWindows::MENU_EXTRACT(std::wstring& str) {
      std::map<std::wstring, std::wstring>::iterator it;
      for (it = ui_win_.begin();it != ui_win_.end();it++) {
        if (std::wstring(kStrMenuExtract)==it->first.c_str()) {
          str = it->second;
          break;
        }
      }
    }
    void UIWindows::MENU_SENDTO(std::wstring& str) {
      std::map<std::wstring, std::wstring>::iterator it;
      for (it = ui_win_.begin();it != ui_win_.end();it++) {
        if (std::wstring(kStrMenuSendto) == it->first.c_str()) {
          str = it->second;
          break;
        }
      }
    }
  }
}