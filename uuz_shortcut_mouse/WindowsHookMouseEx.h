#pragma once

#include <functional>
#include <windows.h>
#include <thread>
#include <atomic>

class WindowsHookMouseEx {
public:
    static WindowsHookMouseEx* getWindowHook();   //单例

    // 策略模式，接受一个传进来的方法
    bool setFunc(const std::function<void()>& newFunc);

    void installHook();      //注册钩子
    void unInstallHook();   //卸载钩子

    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam); //回调，在回调中使用注册进来的方法

private:
    WindowsHookMouseEx();
    ~WindowsHookMouseEx();

    static HHOOK hMouseHook;
    static WindowsHookMouseEx* windowsHookMouseEx;

    std::function<void()> func; // 存储当前接收的策略
    std::thread hookThread;
    std::atomic<bool> running;
};