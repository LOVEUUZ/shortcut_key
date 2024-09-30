#pragma once

#include <QToolButton>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QDir>
#include <QMouseEvent>

#include "config.h"

#include "json.hpp"


// #include "icons_inner_widget.h"
class Icons_inner_widget; // 前向声明


class IconButton : public QToolButton {
	Q_OBJECT

public:
	IconButton(QWidget* parent, const Config config);
	~IconButton();

	int ID;

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	void        init_icon(const QString& filePath);
	static void openFile(const QString& filePath);
	void        updateIcon(const QString& filePath);
	int         calculateClosestIndex(const QPoint& pos);

	//父窗口中的内容
	Icons_inner_widget* parentWidget_content;	  // 上级窗口
	QVector<QPair<int, int>>* vec_coordinate;
	QMap<int, IconButton*>* map_index_button;   // 存储按钮与索引的映射


	QString     filePath;
	QIcon       m_icon;				// 文件图标
	static int  iconSize;
	static bool is_moving;    // 是否在移动
	QPoint      dragStartPos; //移动坐标
	QPoint      originalPos;  // 存储未被拖动前的位置

	//配置相关
	Config config;

signals:
	void sig_buttonDragged(bool is_moving);
	void sig_move_modify_config(int new_index, int old_index);
};
