#pragma once

#include <QMainWindow>

#include "WindowsHookKeyEx.h"
#include "WindowsHookMouseEx.h"

class Config_window  : public QMainWindow
{
	Q_OBJECT

public:
	Config_window(QWidget *parent);
	~Config_window();


  protected:
  void closeEvent(QCloseEvent* event) override;
};
