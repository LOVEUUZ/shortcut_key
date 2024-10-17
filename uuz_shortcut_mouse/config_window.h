#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDialog>

#include "mainwidget.h"
#include "ui_config_window.h"
#include "json.hpp"
#include "WindowsHookKeyEx.h"
#include "WindowsHookMouseEx.h"
#include "Logger.h"


class MainWidget;

class Config_window : public QWidget {
	Q_OBJECT

public:
	Config_window(QWidget* parent = nullptr);
	~Config_window() override;

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	Ui::Config_windowClass ui;
	void                   init_config();
	void                   init_translate();

	//json所需的key
	const std::string is_boot_start = "is_boot_start";
	const std::string log_retain_day = "log_retain_day";

	//配置
	bool config_is_boot_start;

	//来自主窗口的指针，负责配置修改
	MainWidget* main_widget;
	//来自search_content窗口的过滤引用，负责配置修改
	QStringList& filter_path_list;
	QStringList& filter_suffix_list;


	int state;

signals:
	void sig_checkBoxIsBootStart();

private slots:
	void slot_checkBoxIsBootStart(int state);
	void slot_spinBoxValueChanged(int day);
	void slot_openConfigPath();
	void slot_aboutDialog();
};
