#include "mainwidget.h"

#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{

	QApplication a(argc, argv);
	MainWidget w;

#ifdef _DEBUG
	w.show();
#endif

	return a.exec();
}
