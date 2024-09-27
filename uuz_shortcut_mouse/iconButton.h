#pragma once

#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QPainter>
// #include <Shlwapi.h>
#include <QMouseEvent>
#include <QProcess>
#include <QDesktopServices>
#include <QDir>

#include "ui_icon_button.h"

class IconButton : public QPushButton
{
	Q_OBJECT

public:
	IconButton(QWidget* parent = nullptr);
	~IconButton();

private:
	// Ui::icon_buttonClass ui;
	// void init_rendering();


	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	static void openFile(QString& filePath);
	void updateIcon(const QString& filePath);


	QLabel* iconLabel;
	QString filePath;
	QIcon m_icon;
	QString m_text;
	QPixmap m_cachedIcon; // »º´æµÄÍ¼±ê

	bool is_move;
};
