#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "json.hpp"

constexpr int L_Ctrl = 162;
constexpr int L_Alt  = 164;
constexpr int R_Ctrl = 165;
constexpr int R_Alt  = 163;

struct KeyEvent {
  uint64_t    key;
  std::string key_name;
  bool        isPressed;

  KeyEvent(uint64_t k = 0, const std::string & name = "", bool pressed = false)
    : key(k), key_name(name), isPressed(pressed) {}
};


struct ShortcutKeyMsg {
  uint32_t                 key_value_total = 0;     //存储下方vector的总计值
  std::vector<uint32_t>    key_value_serial_number; //存储快捷键的按键的数值标识，格式如下 [10,51,64,31]
  std::vector<std::string> str_key_list;            //存储快捷键的字符串表示形式，格式如下 {"A","B","Tab","Space"}

  friend void from_json(const nlohmann::json & j, ShortcutKeyMsg & msg) {
    j.at("key_value_total").get_to(msg.key_value_total);
    j.at("key_value_serial_number").get_to(msg.key_value_serial_number);
    j.at("str_key_list").get_to(msg.str_key_list);
  }
};

extern std::multimap<uint32_t, ShortcutKeyMsg> key_map;

inline void addNewShortKeyToMap(ShortcutKeyMsg & shortcut_key_msg) {
  std::sort(shortcut_key_msg.key_value_serial_number.begin(), shortcut_key_msg.key_value_serial_number.end());
  key_map.emplace(shortcut_key_msg.key_value_total, shortcut_key_msg);
}
