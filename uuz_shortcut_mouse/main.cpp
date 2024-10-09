#include "mainwidget.h"

#include <QtWidgets/QApplication>
#include <QSharedMemory>

#include "logger.h"

// #include "logger.h"

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	//避免重复启动
	QSharedMemory sharedMemory("uuz_shortcut_mouse");
	// 尝试附加到现有的共享内存
	if (sharedMemory.attach()) {
		// 如果附加成功，说明已有实例在运行
		// QMessageBox::warning(nullptr, "Warning", "Application is already running.");
		qInfo() << "已有实例正在运行,本进程实例退出";
		return 0;
	}

	// 如果没有实例在运行，创建
	sharedMemory.create(1);

	// 设置全局应用程序图标
	QApplication::setWindowIcon(QIcon(":/res/Resource/uuz_logo.ico"));

#ifdef _RELEASE
	// 设置日志保留天数
	Logger::getLogger().set_retention_days(7);
	// // 安装自定义消息处理程序
	qInstallMessageHandler(Logger::messageHandler);
#endif


	MainWidget w;

#ifdef _DEBUG
	w.show();
#endif

	return a.exec();
}
