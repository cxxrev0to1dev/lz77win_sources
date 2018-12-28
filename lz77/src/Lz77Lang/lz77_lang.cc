#if defined(OS_WIN)
#include "Lz77Lang/win/stdafx.h"
#endif
#include "Lz77Lang/lz77_lang.h"
#include <sstream>
#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"//rapidxml::file  
#include "rapidxml/rapidxml_print.hpp"//rapidxml::print  
#include "Lz77Lang/lz77_stream.h"
#include "Lz77Lang/ui_windows.h"
#include "base/string_conv.h"
#include "base/path.h"

namespace lang {
  std::wstring GetLocale() {
    std::wstring result;
#if defined(OS_WIN)
//     wcout << "GetUserDefaultUILanguage:   " << GetUserDefaultUILanguage() << endl;
//     wcout << "GetSystemDefaultUILanguage: " << GetSystemDefaultUILanguage() << endl;
//     wcout << endl;
//     wcout << "GetUserDefaultLangID:       " << GetUserDefaultLangID() << endl;
//     wcout << "GetSystemDefaultLangID:     " << GetSystemDefaultLangID() << endl;
//     wcout << endl;
//     wcout << "GetUserDefaultLCID:         " << GetUserDefaultLCID() << endl;
//     wcout << "GetSystemDefaultLCID:       " << GetSystemDefaultLCID() << endl;
//     wcout << endl;
    switch (GetUserDefaultUILanguage() & 0xff){
    case LANG_CHINESE:
      result = L"zh_CN.UTF-8";
      break;
    case LANG_ENGLISH:
      result = L"en_US.UTF-8";
      break;
    default:
      result = L"unknown";
      break;
    }
#endif
    return result;
  }
  LZ77Language* LZ77Language::GetInstannce() {
    static LZ77Language* ref_instance = nullptr;
    if (!ref_instance){
#if defined(OS_WIN)
      LZ77Language* new_info = new LZ77Language();
      if (InterlockedCompareExchangePointer(
        reinterpret_cast<PVOID*>(&ref_instance), new_info, NULL)) {
        delete new_info;
      }
#else
      ref_instance = new LZ77Language();
#endif
      ref_instance->InitVectorMenu();
    }
    return ref_instance;
  }
  LZ77Language::LZ77Language() {
    temp_.resize(0);
    all_sub_menu_.resize(0);
    base::Path path;
    path.InitModulePathDir(kModuleName);
    const std::wstring language = GetLocale();
    path.AppendPath(kResDirName, nullptr);
    path.AppendPath(language.c_str(), nullptr);
    std::string xml_lang = lang::internal::ReadFile(path.FullPath());
    std::istringstream is(xml_lang);
#if defined(OS_WIN)
    ui_win_ = new lang::internal::UIWindows(is);
#endif
    std::istringstream is2(xml_lang);
    error_msg_table_ = new lang::internal::ErrorMsgTable(is2);
    is_initia_ = false;
    return;
  }
  LZ77Language::~LZ77Language() {
    temp_.resize(0);
    all_sub_menu_.resize(0);
#if defined(OS_WIN)
    if (ui_win_!=nullptr){
      delete ui_win_;
      ui_win_ = nullptr;
    }
#endif
    if (error_msg_table_ != nullptr) {
      delete error_msg_table_;
      error_msg_table_ = nullptr;
    }
    is_initia_ = false;
  }
  std::uint32_t LZ77Language::SubMenuCount() {
    std::uint32_t count = 0;
#if defined(OS_WIN)
    count = ui_win_->SubMenuCount();
#endif
    return count;
  }
  const std::wstring& LZ77Language::MENU_ROOT() {
    temp_.resize(0);
#if defined(OS_WIN)
    ui_win_->MENU_ROOT(temp_);
#endif
    return temp_;
  }
  const std::wstring& LZ77Language::MENU_ARCHIVE() {
    temp_.resize(0);
#if defined(OS_WIN)
    ui_win_->MENU_ARCHIVE(temp_);
#endif
    return temp_;
  }
  const std::wstring& LZ77Language::MENU_EXTRACT() {
    temp_.resize(0);
#if defined(OS_WIN)
    ui_win_->MENU_EXTRACT(temp_);
#endif
    return temp_;
  }
  const std::wstring& LZ77Language::MENU_SENDTO() {
    temp_.resize(0);
#if defined(OS_WIN)
    ui_win_->MENU_SENDTO(temp_);
#endif
    return temp_;
  }
  const std::wstring& LZ77Language::IndexVectorMenu(int i) {
    temp_.resize(0);
    if (i<all_sub_menu_.size()){
      temp_ = all_sub_menu_[i];
    }
    return temp_;
  }
  const std::wstring& LZ77Language::GetErrorMsg(const std::wstring& msg_index) {
    temp_.resize(0);
    temp_ = error_msg_table_->GetErrorMsg(msg_index);
    return temp_;
  }
  void LZ77Language::WriteInvokeCommand(const std::wstring& outfile,
    const std::vector<std::wstring>& files,
    const std::wstring& instruction) {
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<>* rot = doc.allocate_node(rapidxml::node_pi, doc.allocate_string("xml version='1.0' encoding='utf-8'"));
    doc.append_node(rot);
    rapidxml::xml_node<>* invoke_command = doc.allocate_node(rapidxml::node_element, kConstStrInvokeCommand, nullptr);
    rapidxml::xml_node<>* command = doc.allocate_node(rapidxml::node_element, kConstStrCommand, NULL);
    rapidxml::xml_node<>* path_list = doc.allocate_node(rapidxml::node_element, kConstStrPathList, NULL);
    doc.append_node(invoke_command);
    invoke_command->append_node(command);
    command->append_node(path_list);
    const std::string execute_instruction = base::StringConv::narrow(instruction);
    command->append_attribute(doc.allocate_attribute(kConstStrOp,execute_instruction.c_str()));
    for (std::vector<std::wstring>::const_iterator it = files.begin(); it != files.end(); it++){
      rapidxml::xml_node<>* path = doc.allocate_node(rapidxml::node_element, kConstStrPath, NULL);
      std::string path_str = base::StringConv::narrow(it->c_str());
      char* xxxxxxx = doc.allocate_string(path_str.c_str());
      path->append_attribute(doc.allocate_attribute(kConstStrString, xxxxxxx));
      path_list->append_node(path);
    }
    std::ofstream out(outfile);
    out << doc;
    out.close();
  }
  void LZ77Language::InitVectorMenu() {
    if (!is_initia_) {
      all_sub_menu_.resize(0);
      all_sub_menu_.insert((all_sub_menu_.begin() + (int)lang::WIN_MENU_TABLE::kMENU_ARCHIVE), MENU_ARCHIVE());
      all_sub_menu_.insert((all_sub_menu_.begin() + (int)lang::WIN_MENU_TABLE::kMENU_EXTRACT), MENU_EXTRACT());
      all_sub_menu_.insert((all_sub_menu_.begin() + (int)lang::WIN_MENU_TABLE::kMENU_SENDTO), MENU_SENDTO());
      is_initia_ = true;
    }
  }
}