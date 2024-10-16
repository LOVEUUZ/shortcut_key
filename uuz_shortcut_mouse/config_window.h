#pragma once

#include <QWidget>
#include "ui_config_window.h"

#include "WindowsHookKeyEx.h"
#include "WindowsHookMouseEx.h"

class Config_window : public QWidget
{
	Q_OBJECT

public:
	Config_window(QWidget *parent = nullptr);
	~Config_window();

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	Ui::Config_windowClass ui;
};
