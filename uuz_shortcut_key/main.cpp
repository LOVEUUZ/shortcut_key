#include <iostream>
#include <QApplication>
#include <Windows.h>
#include <filesystem>
#include <thread>

#include <QTranslator>
#include <QLibraryInfo>

#include "logger.h"
#include "mainwindow.h"
#include "startQuickly.h"

#include <QApplication>
#include <QFileIconProvider>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>

//程序启动互斥体
HANDLE hMutex;
// 主窗口全局变量
MainWindow* globalVar;
//当前的配置json
nlohmann::json glob_json_;

void isRepeat();
void initDir();


int main(int argc, char* argv[]) {
  // SetConsoleOutputCP(CP_UTF8);
  // _setmode_fileno(stdout), _O_U16TEXT);

  //避免重复启动
  isRepeat();

  QApplication a(argc, argv);
  // 设置全局应用程序图标
  QApplication::setWindowIcon(QIcon(":/res/Resource/uuz_logo.ico"));

  //创建配置文件夹
  initDir();

  // 设置日志保留天数
  Logger::getLogger().set_retention_days(7);
  // 安装自定义消息处理程序
  qInstallMessageHandler(Logger::messageHandler);


  StartQuickly* start_quickly_1 = StartQuickly::getStartQuickly();


  MainWindow w;
  globalVar = &w;

  //Release情况下默认不显示主窗口，只显示托盘图标
#ifdef _DEBUG
  w.show();
#endif

  a.exec();

  //卸载钩子，终止windows键盘循环等各种线程循环
  delete start_quickly_1;

  return 0;
}

//避免重复启动
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