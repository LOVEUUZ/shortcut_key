#pragma once

#include <iostream>
#include <string>
#include <future>
#include <set>

#include <windows.h>
#include <shellapi.h>
#include <Shlwapi.h>

#include <QDesktopServices>
#include <QDir>
#include <QProcess>

#include "mainwindow.h"

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")


//核心，处理键盘事件，并打开对应内容
//有两种方式，一种是qt自带的QDesktopServices::openUrl(fileUrl);和QProcess::startDetached(filePath);
//另一种是windows的函数
class StartQuickly {
  public:
    //单例模式
    static StartQuickly* getStartQuickly(); //单例模式
    static StartQuickly* start_quickly_;

    inline void queue_clear() {
      std::unique_lock<std::mutex> lock(mtx);
      for (int i = 0; i < queue_key_event.size(); ++i) {
        queue_key_event.pop();
      }
    }

    void setFunc();

    ~StartQuickly();
  private:
    StartQuickly();

    //左右ctrl和alt的状态（true是按下，false是没按下）
    std::atomic<bool> L_Ctrl = false;
    std::atomic<bool> L_Alt  = false;
    std::atomic<bool> R_Alt  = false;
    std::atomic<bool> R_Ctrl = false;

    std::queue<KeyEvent>    queue_key_event; //存储键盘事件的队列
    std::mutex              mtx;
    std::condition_variable isQueueEmpty; //条件变量
    bool                    isEnd;        //控制 identify (识别） 线程在程序结束的时候结束循环
    std::mutex              mtx_identify;

    WindowsHookEx* ptr_windows_hook; //键盘钩子指针

    void     set_key_event(const KeyEvent & key_event); //策略模式，将执行方法注册到键盘钩子之中
    KeyEvent getKeyEvent();
    void     identify();                                 //核心处理部分，识别键盘关键字
    void     startProcess(const std::string & path);     //打开绑定程序
    bool     ctrlAndAltCheck(const int key_index) const; //专门处理ctrl和alt的状态和配置文件是否一致的检测

    void initThread();

    //这两个用于避免重复启动程序
    std::set<std::string> tmp_set;
    void                  tmp_clear_set();
};
