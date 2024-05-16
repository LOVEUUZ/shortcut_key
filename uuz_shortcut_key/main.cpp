#include <iostream>
#include <QApplication>
#include <QMessageBox>
#include <Windows.h>

#include "mainwindow.h"

// 键盘事件处理函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0) {
    if (wParam == WM_KEYDOWN) {
      KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
      DWORD            key       = pKeyboard->vkCode;

      // 使用MapVirtualKey函数获取按键的字符表示
      UINT  scanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);
      WCHAR buff[16];
      int   result = GetKeyNameTextW(scanCode << 16, buff, sizeof(buff));

      // 输出按键的键码和字符表示
      if (result != 0) {
        std::wcout << L"Key pressed - Code: " << key << L", Name: " << buff << std::endl;
      }
      else {
        std::cerr << "Failed to get key name." << std::endl;
      }
    }
  }

  // 将事件传递给下一个钩子
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, char* argv[]) {
  // // 设置键盘钩子
  // HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
  //
  // if (hHook == NULL) {
  //     std::cerr << "Failed to set hook!" << std::endl;
  //     return 1;
  // }
  //
  // // 消息循环，等待退出
  // MSG msg;
  // while (GetMessage(&msg, NULL, 0, 0)) {
  //     TranslateMessage(&msg);
  //     DispatchMessage(&msg);
  // }
  //
  // // 卸载钩子
  // UnhookWindowsHookEx(hHook);
  //
  // return 0;

  QApplication a(argc, argv);
  MainWindow   w;
  w.show();

  return a.exec();
}
