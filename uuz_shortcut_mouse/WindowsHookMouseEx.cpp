#include "WindowsHookMouseEx.h"
#include <iostream>

HHOOK WindowsHookMouseEx::hMouseHook = NULL;
WindowsHookMouseEx* WindowsHookMouseEx::windowsHookMouseEx = nullptr;

WindowsHookMouseEx::WindowsHookMouseEx() : running(false) {}

WindowsHookMouseEx::~WindowsHookMouseEx() {
	if (running) {
		unInstallHook();
	}
}

//单例
WindowsHookMouseEx* WindowsHookMouseEx::getWindowHook() {
	if (windowsHookMouseEx == nullptr) {
		windowsHookMouseEx = new WindowsHookMouseEx();
	}
	return windowsHookMouseEx;
}

bool WindowsHookMouseEx::setFunc(const std::function<void()>& newFunc) {
	func = newFunc;
	return true;
}

//注册钩子
void WindowsHookMouseEx::installHook() {
#ifdef NDEBUG		  //注意调试的时候尽量不编译，要不然鼠标一卡一卡的心烦
	std::cout << "mouse hook install" << "\n";
	if (!running) {
		running = true;
		hookThread = std::thread([this]() {
			hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
			if (hMouseHook == NULL) {
				std::cerr << "Failed to set mouse hook!" << std::endl;
				running = false;
				return;
			}
			MSG msg;
			while (running && GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			unInstallHook();
			});
		hookThread.detach();
	}
#endif
}

//卸载钩子
void WindowsHookMouseEx::unInstallHook() {
#ifdef NDEBUG
	if (running) {
		running = false;
		if (hMouseHook != NULL) {
			UnhookWindowsHookEx(hMouseHook);
			std::cout << "mouse hook unInstall" << "\n";
			hMouseHook = NULL;
		}
	}
#endif
}

//回调
LRESULT CALLBACK WindowsHookMouseEx::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0 && (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN)) {
		if (windowsHookMouseEx && windowsHookMouseEx->func) {
			windowsHookMouseEx->func();
		}
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}
