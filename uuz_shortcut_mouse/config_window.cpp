#include "config_window.h"

Config_window::Config_window(QWidget* parent) : QMainWindow(parent) {
	// 先卸载钩子，记得关闭该页面的时候重新加上钩子
	WindowsHookKeyEx::getWindowHook()->unInstallHook();
	WindowsHookMouseEx::getWindowHook()->unInstallHook();
}


Config_window::~Config_window() {

}

void Config_window::closeEvent(QCloseEvent* event) {
	WindowsHookKeyEx::getWindowHook()->installHook();
	WindowsHookMouseEx::getWindowHook()->installHook();
	qDebug() << "钩子挂上成功";
	QMainWindow::closeEvent(event);
}
