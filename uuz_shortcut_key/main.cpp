#include <iostream>
#include <QApplication>
#include <QMessageBox>
#include <Windows.h>
#include <filesystem>

#include "mainwindow.h"
#include "Trigger.hpp"


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

//程序启动互斥体
HANDLE hMutex;
// 主窗口全局变量
MainWindow* globalVar;


void isRepeat();
void initDir();


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

  isRepeat();

  QApplication a(argc, argv);

  initDir();

  new Trigger();

  MainWindow w;
  globalVar = &w;
  w.show();

  return a.exec();
}

void isRepeat() {
  // 创建一个互斥体避免重复启动
  hMutex = CreateMutex(NULL, TRUE, L"shortcut_key");

  // hMutex 创建失败，退出
  if (hMutex == NULL) {
    MessageBox(NULL, L"Unable to create a handle", L"error", MB_ICONERROR | MB_OK);
    std::exit(1);
  }

  // 如果互斥体已经存在，退出
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    // QMessageBox::warning(nullptr, "Warning", "The application is already running.");
    std::exit(1);
  }
}

//创建文件夹路径
void initDir() {
  std::string dir_path = QApplication::applicationDirPath().toLocal8Bit().constData();
  dir_path             = dir_path + "/config";
  //先看文件夹是否存在，不存在则创建
  if (!std::filesystem::exists(dir_path) && !std::filesystem::create_directories(dir_path)) {
    MessageBox(NULL, L"Directory creation failed", L"error", MB_ICONERROR | MB_OK);
    return std::exit(0);
  }
}
