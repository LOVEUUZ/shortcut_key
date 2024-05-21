#include <iostream>
#include <QApplication>
#include <Windows.h>
#include <filesystem>
#include <thread>

#include "mainwindow.h"
#include "Trigger.hpp"
#include "startQuickly.h"


//程序启动互斥体
HANDLE hMutex;
// 主窗口全局变量
MainWindow* globalVar;

void isRepeat();
void initDir();


int main(int argc, char* argv[]) {
  isRepeat();

  QApplication a(argc, argv);

  initDir();

  new Trigger();

  StartQuickly* start_quickly_1 = StartQuickly::getStartQuickly();

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
