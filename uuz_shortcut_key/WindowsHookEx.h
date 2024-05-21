#pragma once

#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>

#include "keyEvent.h"


class WindowsHookEx {
  public:
    static WindowsHookEx* ptr_windows_hook;

    static WindowsHookEx* getWindowHook();

    void installHook();

    void unInstallHook();

    //策略模式，接受一个传进来的方法
    bool setFunc(const std::function<void(KeyEvent)> & newFunc);

  private:
    HHOOK             hook;
    MSG               msg;
    std::thread       msgLoopThread;
    std::atomic<bool> stopRequested = false;
    DWORD             msgLoopThreadId; //存储线程id

    std::queue<KeyEvent>          queue_; //按键队列
    std::mutex                    mtx_write;
    std::function<void(KeyEvent)> func = nullptr;

    explicit WindowsHookEx() = default;

    ~WindowsHookEx() = default;

    void messageLoop();

    static std::string getKeyName(DWORD vkCode);

    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
};
