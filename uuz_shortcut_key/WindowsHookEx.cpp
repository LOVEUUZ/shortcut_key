#include "WindowsHookEx.h"

#include <iostream>

#include "keyEvent.h"

WindowsHookEx* WindowsHookEx::ptr_windows_hook = nullptr;


WindowsHookEx* WindowsHookEx::getWindowHook() {
    if (ptr_windows_hook == nullptr) {
        ptr_windows_hook = new WindowsHookEx;
    }
    return ptr_windows_hook;
}

void WindowsHookEx::installHook() {
    getWindowHook();
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
    if (hook == nullptr) {
        std::cerr << "Failed to install hook" << "\n";
        std::exit(0);
    }

    // 消息循环在单独的线程中运行
    msgLoopThread = std::thread(&WindowsHookEx::messageLoop, this);
}

void WindowsHookEx::unInstallHook() {
    stopRequested = false;

    // 向指定线程发送
    PostThreadMessage(msgLoopThreadId, WM_QUIT, 0, 0);

    if (msgLoopThread.joinable()) {
        msgLoopThread.detach();
    }

    if (hook != nullptr) {
        UnhookWindowsHookEx(hook);
        hook = nullptr;
    }
    std::cout << "unInstall hook successful" << std::endl;
}

bool WindowsHookEx::setFunc(const std::function<void(KeyEvent)>& newFunc) {
    // if (func != nullptr) {
        //   return false;
        // }
    func = newFunc;
    return true;
}

void WindowsHookEx::messageLoop() {
    msgLoopThreadId = GetCurrentThreadId();
    while (!stopRequested.load()) {
        while (GetMessage(&msg, nullptr, 0, 0)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

std::string WindowsHookEx::getKeyName(DWORD vkCode) {
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
    // Construct a fake lParam
    LONG lParam = (scanCode << 16) | 1;
    char keyName[128];
    if (GetKeyNameTextA(lParam, keyName, sizeof(keyName)) > 0) {
        return std::string(keyName);
    }
    else {
        return "Unknown";
    }
}

LRESULT WindowsHookEx::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pkbhs = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        std::string      keyName = WindowsHookEx::getKeyName(pkbhs->vkCode);

        std::pair<uint64_t, std::string> p = std::make_pair(pkbhs->vkCode, keyName);

        KeyEvent key_event;
        key_event.key = pkbhs->vkCode;
        key_event.key_name = WindowsHookEx::getKeyName(pkbhs->vkCode);
        if ((pkbhs->vkCode == 164 || pkbhs->vkCode == 165) && wParam == WM_KEYUP) {
            key_event.isPressed = false;
        }
        else if (pkbhs->vkCode == 164 || pkbhs->vkCode == 165) {
            key_event.isPressed = true;
        }
        else {
            if (wParam == WM_KEYDOWN) {
                key_event.isPressed = true;
            }
            else if (wParam == WM_KEYUP) {
                key_event.isPressed = false;
            }
        }
        ptr_windows_hook->func(key_event);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
    // return CallNextHookEx(hook, nCode, wParam, lParam);
}
