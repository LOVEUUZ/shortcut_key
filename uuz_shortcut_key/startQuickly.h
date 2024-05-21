#pragma once

#include "mainwindow.h"

extern MainWindow* globalVar;

class StartQuickly {
  public:
    //单例模式
    static StartQuickly* getStartQuickly();
    static StartQuickly* start_quickly_;

    void queue_clear() {
      std::unique_lock<std::mutex> lock(mtx);
      for (int i = 0; i < queue_key_event.size(); ++i) {
        queue_key_event.pop();
      }
    }

    void setFunc();

  private:
    StartQuickly();
    ~StartQuickly();


    std::queue<KeyEvent> queue_key_event;
    std::mutex           mtx;

    WindowsHookEx* ptr_windows_hook;

    void     set_key_event(const KeyEvent & key_event);
    KeyEvent getKeyEvent();
};
