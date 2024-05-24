#pragma once

#include <iostream>
#include <string>
#include <windows.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <future>
#include <QDesktopServices>
#include <QDir>
#include <QProcess>

#include "mainwindow.h"
#include "timedSet.h"

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

extern MainWindow*    globalVar;
extern nlohmann::json glob_json_;

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

    TimedSet<std::string> duplicate_detection;

    //左右ctrl和alt的状态（true是按下，false是没按下）
    std::atomic<bool> L_Ctrl = false;
    std::atomic<bool> L_Alt  = false;
    std::atomic<bool> R_Alt  = false;
    std::atomic<bool> R_Ctrl = false;

    std::queue<KeyEvent>    queue_key_event;
    std::mutex              mtx;
    std::condition_variable isQueueEmpty; //条件变量
    bool                    isEnd;        //控制 identify (识别） 线程在程序结束的时候结束循环
    std::mutex              mtx_identify;

    WindowsHookEx* ptr_windows_hook;

    void     set_key_event(const KeyEvent & key_event);
    KeyEvent getKeyEvent();
    void     identify();
    bool     ctrlAndAltCheck(const int key_index) const;

    void initThread();

    void startProcess(const std::string & path);
};
