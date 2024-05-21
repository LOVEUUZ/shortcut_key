#include "startQuickly.h"

#include <iostream>

StartQuickly* StartQuickly::start_quickly_ = nullptr;

StartQuickly* StartQuickly::getStartQuickly() {
  if (start_quickly_ == nullptr) {
    start_quickly_ = new StartQuickly;
  }
  return start_quickly_;
}

StartQuickly::StartQuickly() {
  ptr_windows_hook = WindowsHookEx::getWindowHook();
  ptr_windows_hook->installHook();
  setFunc();
}

StartQuickly::~StartQuickly() {
  ptr_windows_hook->unInstallHook();
}


void StartQuickly::set_key_event(const KeyEvent & key_event) {
  std::unique_lock<std::mutex> lock(mtx);
  if (queue_key_event.size() > 20) {
    queue_key_event.pop();
  }
  std::cout << key_event.key_name << " " << key_event.key << " " << key_event.isPressed << "\n";
  queue_key_event.push(key_event);
  // auto i = key_map;
}

//todo 核心在这
KeyEvent StartQuickly::getKeyEvent() {
  KeyEvent key_event_;

  {
    std::unique_lock<std::mutex> lock(mtx);
    if (queue_key_event.empty()) {
      return NULL;
    }
    key_event_ = std::move(queue_key_event.front());
    queue_key_event.pop();
  }

  return key_event_;
}


void StartQuickly::setFunc() {
  auto func = [&](const KeyEvent & key_event) { set_key_event(key_event); };
  queue_clear();
  ptr_windows_hook->setFunc(func);
  //
  // // 使用 lambda 表达式绑定成员函数
  // // bool b = ptr_windows_hook->setFunc([&](KeyEvent k) {
  // //   std::cout << k.key_name << " " << k.key << " " << k.isPressed << "\n";
  // // });
}
