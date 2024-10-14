﻿#include "IconButton.h"

#include <utility>

#include "icons_inner_widget.h"

#include "mainwidget.h"
class MainWidget;



int                      IconButton::iconSize = 64;
bool                     IconButton::is_moving = false;


IconButton::IconButton(QWidget* parent, const Config config) : QToolButton(parent), config(config), changeShowNameEdit(nullptr) {

	this->ID = config.id;
	this->filePath = QString::fromUtf8(config.absolutePath);
	showName = QString::fromStdString(config.showName);			//显示名称
	setText(showName);
	parentWidget_content = dynamic_cast<Icons_inner_widget*>(parentWidget());
	vec_coordinate = parentWidget_content->get_vec_coordinate();
	map_index_button = parentWidget_content->get_map_index_button();

	init_icon(filePath);

	// 设置透明背景，只显示图标和文字
	setAttribute(Qt::WA_TranslucentBackground, true);
	setStyleSheet("QToolButton { background: transparent; border: none; }");

	connect(this, &IconButton::sig_changeShowName, this, &IconButton::slot_changeShowName);
}


IconButton::~IconButton() {}


void IconButton::init_icon(const QString& filePath) {
	QFileInfo fileInfo(filePath);
	QString resolvedPath = filePath;

	// 检查是否是快捷方式,如果是则返回真实路径z
	QFileInfo shortcutInfo(filePath);
	if (shortcutInfo.isSymLink()) {

		resolvedPath = shortcutInfo.symLinkTarget();
	}

	QFileIconProvider iconProvider;
	QIcon fileIcon = iconProvider.icon(QFileInfo(resolvedPath));
	QPixmap iconPixmap = fileIcon.pixmap(iconSize, iconSize); // 获取图标并设置大小
	m_icon = QIcon(iconPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 缩放图标

	// 设置图标和样式
	setIcon(m_icon);
	setIconSize(QSize(iconSize, iconSize));
#ifdef _DEBUG
	qDebug() << "名字 ==》 " << QString::fromUtf8(config.fileName);
#endif
	setText(QString::fromUtf8(config.showName));
	setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // 图标在文字上方
	setStyleSheet("QToolButton { background-color: #b7b7b7; border: 1px solid #888; border-radius: 5px; }");
}


//按下事件
void IconButton::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		originalPos = pos(); // 记录初始位置
		// 添加按下效果，改变样式
		setStyleSheet("QToolButton { background-color: #a0a0a0; border: 1px solid #666; border-radius: 5px; }");
		dragStartPos = event->pos();
		setCursor(Qt::ClosedHandCursor);
	}
	QToolButton::mousePressEvent(event);
}

//释放事件
void IconButton::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		if (is_moving) {
			// 计算当前所在的区域
			QPoint globalMousePos = QCursor::pos();               //鼠标全局坐标
			QPoint localMousePos = mapFromGlobal(globalMousePos);//转为鼠标位置相对于按钮的偏移

			QPoint currentPos = pos() + localMousePos;
			int    closestIndex = calculateClosestIndex(currentPos); // 计算最近的索引
			// qDebug() << map_index_button;
			// 适应到新位置
			if (closestIndex != -1) {
				QPoint newCoordinate = QPoint(vec_coordinate->at(closestIndex).first, vec_coordinate->at(closestIndex).second);
				move(newCoordinate); // 移动到新的坐标
				//放置到新位置后，更新索引与按钮的映射

		  //移动完成，通知父窗口修改配置文件
				emit sig_move_modify_config(closestIndex, ID);

#ifdef _DEBUG
				qDebug() << "closestIndex ==> " << closestIndex << "   old_index ==> " << ID;
#endif

		    //记得及时更新自身的配置，特别的移动后需要修改索引id
				ID = closestIndex;
				config.id = ID;
			}
			else move(originalPos);
		}
		else {
			openFile(filePath); // 打开对应程序
		}
		// 恢复样式
		setStyleSheet("QToolButton { background-color: #b7b7b7; border: 1px solid #888; border-radius: 5px; }");
		setCursor(Qt::ArrowCursor);
		is_moving = false; // 状态重置

		// 发出信号通知父窗口隐藏虚线
		emit sig_buttonDragged(false);
	}
	QToolButton::mouseReleaseEvent(event);
}

//移动事件
void IconButton::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		int distance = (event->pos() - dragStartPos).manhattanLength();
		if (distance > 5) {
			is_moving = true;

			// 发出信号通知父窗口显示虚线
			emit sig_buttonDragged(true);

			// 获取父窗口相对位置
			QPoint parentPos = parentWidget()->mapFromGlobal(QCursor::pos());
			QPoint newPos = parentPos - QPoint(iconSize / 2, iconSize / 2);

			// 判断鼠标是否在父窗口中
			if (!parentWidget()->underMouse()) {
				move(originalPos); // 超出边界，回到初始位置
				return;
			}

			move(newPos); // 更新位置
			raise();      // 提升层级
		}
	}
	QToolButton::mouseMoveEvent(event);
}

//鼠标移入的样式
void IconButton::enterEvent(QEnterEvent* event) {
	// 鼠标悬浮时的样式
	setStyleSheet("QToolButton { background: rgba(0, 0, 0, 0.2); border: none; border-radius: 5px }"); // 阴影效果
	QToolButton::enterEvent(event);
}

//鼠标离开的样式
void IconButton::leaveEvent(QEvent* event) {
	// 鼠标离开时的样式
	setStyleSheet("QToolButton { background: transparent; border: none; border-radius: 5px }");
	QToolButton::leaveEvent(event);
}


void IconButton::openFile(const QString& filePath) {
	QString filePath_tmp = QDir::toNativeSeparators(filePath); // 转为本地格式
	QFileInfo fileInfo(filePath_tmp);
	QString directory = fileInfo.absolutePath();

	// 保存当前工作目录
	QString currentDir = QDir::currentPath();

	// 改变当前工作目录（有些汉化组封包的gal就不行，必须要切过去才能运行，要不然会提示各种找不到）
	QDir::setCurrent(directory);

	// 打开文件/文件夹
	QUrl fileUrl = QUrl::fromLocalFile(filePath_tmp);  // 转为url
	QDesktopServices::openUrl(fileUrl);                // 使用该函数可以打开exe，也能打开jpg，txt等文件

	qInfo() << "Starting process:" << filePath_tmp;

	// 恢复原来的工作目录
	QDir::setCurrent(currentDir);

	// 主动失去焦点，隐藏主窗口
	QWidget* widget_ = this->parentWidget()->parentWidget()->parentWidget(); // this->icons_inner_widget->stacked_widget->MainWidget
	MainWidget* mainWidget = qobject_cast<MainWidget*>(widget_);
	if (mainWidget != nullptr) {
		emit mainWidget->sig_move_focus(nullptr);
	}
}



// 计算最近的索引
int IconButton::calculateClosestIndex(const QPoint& pos) {
	for (int i = 0; i < vec_coordinate->size(); ++i) {
		const auto& coordinate = vec_coordinate->at(i);
		QRect        rect(coordinate.first, coordinate.second, 95, 95);
		//找到目前鼠标停留的坐标与之对应的按钮预留位
		if (rect.contains(pos)) {
			//判断当前位置上是否已经有其他图标了，如果有，则放弃本次移动事件
			if (map_index_button->contains(i)) {
				return -1;
			}
			return i; // 返回最近的索引
		}
	}
	return -1; // 没有找到
}

void IconButton::slot_changeShowName() {
	if (changeShowNameEdit == nullptr) {
		changeShowNameEdit = new QLineEdit(this);
		connect(changeShowNameEdit, &QLineEdit::editingFinished, this, [&]() {
			QString newShowName = changeShowNameEdit->text();
			qDebug() << "修改后的名称为 >> " << newShowName;
			setShowName(newShowName);
			setText(newShowName); // 更新按钮显示的文本
			changeShowNameEdit->hide();

			//然后去更新配置文件
			config.showName = newShowName.toStdString();
			emit sig_modify_config(config);
			});
	}

	changeShowNameEdit->setText(showName);

	// 设置编辑框的在底部，宽度为整个对象
	int textY = height() - changeShowNameEdit->height(); // 将编辑框放在按钮的底部
	changeShowNameEdit->setGeometry(0, textY, width(), changeShowNameEdit->height());

	changeShowNameEdit->show();
	changeShowNameEdit->setFocus();
}


