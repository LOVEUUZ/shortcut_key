#include "config_window.h"

Config_window::Config_window(QWidget* parent): QWidget(parent) {
  ui.setupUi(this);

  // 先卸载钩子，记得关闭该页面的时候重新加上钩子
  WindowsHookKeyEx::getWindowHook()->unInstallHook();
  WindowsHookMouseEx::getWindowHook()->unInstallHook();

  setAttribute(Qt::WA_ShowModal, true);

#ifdef NDEBUG
  //置顶 禁用最大最小化
  this->setWindowFlags(
    this->windowFlags() | Qt::WindowStaysOnTopHint);
#endif
}

Config_window::~Config_window() {}

void Config_window::closeEvent(QCloseEvent* event) {
  WindowsHookKeyEx::getWindowHook()->installHook();
  WindowsHookMouseEx::getWindowHook()->installHook();
#ifdef _DEBUG
	qDebug() << "钩子挂上成功";
#endif
  QWidget::closeEvent(event);
}
