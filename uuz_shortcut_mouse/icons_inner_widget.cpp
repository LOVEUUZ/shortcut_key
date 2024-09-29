#include "icons_inner_widget.h"

#include "mainwidget.h"

using json = nlohmann::json;

Icons_inner_widget::Icons_inner_widget(QWidget* parent) : QWidget(parent), is_showDashedBorder(false) {
	// ui.setupUi(this);

	init_coordinateut();

	file_path = QCoreApplication::applicationDirPath() + "/config/config_icon.json";
	init_config();

	init_rendering();


	for (auto icon_button : vec_iconButton) {
		connect(icon_button, &IconButton::buttonDragged, this, &Icons_inner_widget::slot_showDashedBorder);
	}
}

Icons_inner_widget::~Icons_inner_widget() {
	file_config->close();
}


void Icons_inner_widget::paintEvent(QPaintEvent* event) {
	if (is_showDashedBorder) {
		QPainter painter(this);
		painter.setPen(QPen(Qt::gray, 1, Qt::DashLine)); // 设置为灰色，宽度为1

		// 在icons_inner_widget内部画出小方块的虚线（下面的加减调整是为了完全与按钮占位适应，手调出来的魔法值）
		for (int x = 0; x <= width(); x += icon_button_size) {
			painter.drawLine(x + 12, 0, x + 12, height() - 5); //竖排虚线
		}
		for (int y = 0; y <= height(); y += icon_button_size) {
			painter.drawLine(12, y, width() - 12, y); //横排虚线
		}
	}

	QWidget::paintEvent(event);
}

//右键菜单
void Icons_inner_widget::contextMenuEvent(QContextMenuEvent* event) {
	QMenu contextMenu(this);

	// 添加菜单项
	QAction* action_add_icon = contextMenu.addAction(tr("新增"));
	QAction* action_delete_icon = contextMenu.addAction(tr("删除"));
	QAction* action_config = contextMenu.addAction(tr("设置"));

	//用来判断删除是否禁用
	int id = -1;
	{
		action_delete_icon->setEnabled(false); // 默认禁用
		// 根据鼠标当前相对于该窗口的坐标偏移，可以判断是否在某个格子内，然后判断该格子当前是否有映射
		QPoint localPos = this->mapFromGlobal(QCursor::pos());
		for (int i = 0; i < vec_coordinate.size(); ++i) {
			QRect coordinateRange(vec_coordinate[i].first, vec_coordinate[i].second,
				icon_button_size, icon_button_size); // 修正矩形范围的计算
			if (coordinateRange.contains(localPos)) {
				// 包含在内，只要有一个包含在内就能退出循环了
				action_delete_icon->setEnabled(true);
				id = i; // 假设你需要保存索引
				break;
			}
		}
	}

	//菜单样式
	contextMenu.setStyleSheet(
		"QMenu {"
		"    border: none;"
		// "    border-radius: 10px;"			//无法生效
		"    background-color: #D9D9E2;"
		"}"
		"QMenu::item {"
		"    padding: 7px 15px;"
		"    height: 15px;"
		"    width: 50px;"
		"    background-color: transparent;"
		"}"
		"QMenu::item:selected {"
		"    background-color: rgba(0, 0, 0, 0.1);"
		"}"
	);

	// 执行菜单并处理选择
	QAction* selectedAction = contextMenu.exec(event->globalPos());
	if (selectedAction) {

		if (selectedAction == action_add_icon) {
			slot_add_icon();
		}
		else if (selectedAction == action_delete_icon) {
			slot_delete_icon(id);
		}
		else if (selectedAction == action_config) {}
	}


	QWidget::contextMenuEvent(event);
}

//初始化每个icon_button的左上角(x,y)坐标
void Icons_inner_widget::init_coordinateut() {
	//1. 以 icons_inner_widget 窗口的坐标为基本计算
	for (int i = 0; i < y; ++i) {
		for (int k = 0; k < x; ++k) {
			int             x_tmp = icon_button_size * k + 12; //12是为了坐标计算居中一些
			int             y_tmp = icon_button_size * i;
			QPair<int, int> pair(x_tmp, y_tmp);
			vec_coordinate.append(pair);
		}
	}
}

//创建并渲染按钮
void Icons_inner_widget::init_rendering() {
	for (auto& item : config_jsonArray) {
		Config config = Config::fromJson(item);
		IconButton* tmp_qb = new IconButton(this, config);
		tmp_qb->setGeometry(config.coordinate.x, config.coordinate.y, icon_button_size, icon_button_size);
		qDebug() << "Button ID:" << config.id << "Position:" << config.coordinate.x << config.coordinate.y;
		vec_iconButton.push_back(tmp_qb); // 将按钮添加到 vec_iconButton
		map_index_button[config.id] = tmp_qb; // 使用一个映射来记录按钮和其索引
	}

	// QVector<int> buttonIndices = { 6, 12, 23 }; // 0-based index for 7, 13, 24
	// for (int index : buttonIndices) {
	// 	if (index < vec_coordinate.size()) {
	// 		const auto& coordinate = vec_coordinate[index];
	// 		IconButton* tmp_qb = new IconButton(this, vec_coordinate, "D:/software/winRAR/WinRAR.exe");
	// 		tmp_qb->setGeometry(coordinate.first, coordinate.second, icon_button_size, icon_button_size);
	// 		vec_iconButton.push_back(tmp_qb); // 将按钮添加到 vec_iconButton
	//
	// 		// 记录按钮位置
	// 		button_map[tmp_qb] = index; // 使用一个映射来记录按钮和其索引
	// 	}
	// }
}

//当没有配置文件的时候创建一个默认的
// ReSharper disable once CppMemberFunctionMayBeStatic
QString Icons_inner_widget::first_create_config() {
	// 获取桌面路径和下载文件夹路径
	QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

	// 当前时间 年-月-日 时:分:秒
	QString creationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

	json jsonArray_ = json::array();

	// 桌面
	Config configDesktop;
	configDesktop.id = 0;
	configDesktop.fileName = "Desktop";
	configDesktop.absolutePath = desktopPath.toStdString();
	configDesktop.creationTime = creationTime.toStdString();
	configDesktop.lastMoveTime = creationTime.toStdString();
	configDesktop.coordinate = Coordinate(vec_coordinate[0].first, vec_coordinate[0].second);
	configDesktop.count = 0;
	jsonArray_.push_back(configDesktop.toJson());

	// 下载文件夹
	Config configDownload;
	configDownload.id = 1;
	configDownload.fileName = "Download";
	configDownload.absolutePath = downloadsPath.toStdString();
	configDownload.coordinate = Coordinate(vec_coordinate[1].first, vec_coordinate[1].second);
	configDownload.count = 0;
	jsonArray_.push_back(configDownload.toJson());

	return QString::fromStdString(jsonArray_.dump(4));
}



void Icons_inner_widget::init_config() {
	// 获取目录路径
	QDir dir = QFileInfo(file_path).absoluteDir();
	// 检查目录是否存在，如果不存在则创建它及其父文件夹
	if (!dir.mkpath(".")) {
		qWarning() << "Cannot create directory:" << dir.path();
		return;
	}

	file_config = new QFile(file_path);
	// 可读可写模式打开文件
	if (file_config->open(QIODevice::ReadWrite | QIODevice::Text)) {
		// 如果文件无内容，写入基础内容
		if (file_config->readAll().isEmpty()) {
			qDebug() << "File created:" << file_path;
			QTextStream out(file_config);
			out << first_create_config(); // 写入
		}
	}

	// 读取文件内容
	file_config->seek(0);
	qstr_config_content = file_config->readAll();
	str_config_content = qstr_config_content.toStdString();
	config_jsonArray = json::parse(str_config_content);
}




//拖拽的时候显示虚线和隐藏
void    Icons_inner_widget::slot_showDashedBorder(bool is_moving) {
	if (is_moving) {
		is_showDashedBorder = true; // 显示虚线
		update();
	}
	else {
		is_showDashedBorder = false; // 隐藏虚线
		update();
	}
}

//右键添加icon
void Icons_inner_widget::slot_add_icon() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("所有文件 (*)"));
	//从索引1开始遍历，直到找到空位索引在进行添加
	for (int i = 0; i < SUM; ++i) {
		if (auto it = map_index_button.find(i); it != map_index_button.end()) {
			//查到了,说明该位置有按钮了，跳过继续下一个
			continue;
		}
		else {
			//todo (更改配置文件)找到空位了，放在该位置后手动显示
			Config config;
			QString creationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
			config.id = i;
			config.fileName = fileName.toStdString();
			config.absolutePath = fileName.toStdString();
			config.creationTime = creationTime.toStdString();
			config.lastMoveTime = creationTime.toStdString();
			config.coordinate = Coordinate(vec_coordinate[1].first, vec_coordinate[1].second);;
			config.count = 0;

			IconButton* tmp_qb = new IconButton(this, config);
			tmp_qb->setGeometry(vec_coordinate[i].first, vec_coordinate[i].second, icon_button_size, icon_button_size);
			map_index_button[i] = tmp_qb;
			vec_iconButton.push_back(std::move(tmp_qb)); // 将按钮添加到 vec_iconButton
			return;
		}
	}
}

//右键删除icon
void Icons_inner_widget::slot_delete_icon(int id) {
	auto ptr_icon_button = map_index_button.find(id);
	if (ptr_icon_button != map_index_button.end()) {
		// 移除 QMap 中的元素
		IconButton* buttonToRemove = ptr_icon_button.value();
		map_index_button.remove(id);

		// 遍历 QVector，找到并移除对应的按钮
		for (int i = 0; i < vec_iconButton.size(); ++i) {
			if (vec_iconButton[i] == buttonToRemove) {
				vec_iconButton.removeAt(i);
				break;
			}
		}
		buttonToRemove->deleteLater(); //移除该按钮
	}
}


//右键打开配置，还未用到
void Icons_inner_widget::slot_config() {}
