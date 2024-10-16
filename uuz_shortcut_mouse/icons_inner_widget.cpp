#include "icons_inner_widget.h"

#include "mainwidget.h"

using json = nlohmann::json;

Icons_inner_widget::Icons_inner_widget(QWidget* parent) :
	QWidget(parent), is_showDashedBorder(false) {
	// ui.setupUi(this);

#ifdef _DEBUG       //接收拖拽，release版本通过右键菜单设置或者关闭
	setAcceptDrops(true);
#endif

	init_coordinateut();

	file_path = QCoreApplication::applicationDirPath() + "/config/config_icon.json";
	init_config();

	init_rendering();


	for (auto icon_button : vec_iconButton) {
		init_button_connect(icon_button);
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
static bool is_install_hook = true;

void Icons_inner_widget::contextMenuEvent(QContextMenuEvent* event) {
	QMenu contextMenu(this);

	// 添加菜单项
	QAction* action_open_icon_path = contextMenu.addAction(tr("打开路径"));
	QAction* action_add_icon = contextMenu.addAction(tr("新增文件"));
	QAction* action_add_folder = contextMenu.addAction(tr("新增文件夹"));
	QAction* action_change_show_name = contextMenu.addAction(tr("修改名称"));
	QAction* action_delete_icon = contextMenu.addAction(tr("删除"));
	QAction* action_unInstall_hook = contextMenu.addAction(tr("锁定/恢复界面"));
	QAction* action_config = contextMenu.addAction(tr("设置"));

	if (is_install_hook) {
		action_unInstall_hook->setText(tr("锁定界面"));
	}
	else {
		action_unInstall_hook->setText(tr("恢复界面"));
	}

	//用来判断删除是否禁用
	int id = -1;
	{
		action_open_icon_path->setEnabled(false); // 默认禁用
		action_delete_icon->setEnabled(false);
		action_change_show_name->setEnabled(false);
		// 根据鼠标当前相对于该窗口的坐标偏移，可以判断是否在某个格子内，然后判断该格子当前是否有映射
		QPoint localPos = this->mapFromGlobal(QCursor::pos());

#ifdef _DEBUG
		qDebug() << localPos.x() << "," << localPos.y();
#endif

		for (int i = 0; i < vec_config.size(); ++i) {
			QRect coordinateRange(vec_config[i].coordinate.x, vec_config[i].coordinate.y,
				icon_button_size, icon_button_size);
			if (coordinateRange.contains(localPos)) {
				// 包含在内，只要有一个包含在内就能退出循环了
				action_open_icon_path->setEnabled(true);
				action_delete_icon->setEnabled(true);
				action_change_show_name->setEnabled(true); // 默认禁用

				id = vec_config[i].id;

				break;
			}
		}
	}

#ifdef _DEBUG
	qDebug() << "id ==> " << id;
#endif

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
	if (QAction* selectedAction = contextMenu.exec(event->globalPos())) {
		if (selectedAction == action_open_icon_path) {
			slot_open_icon_path(id);
		}
		if (selectedAction == action_add_icon) {
			slot_add_icon();
		}
		else if (selectedAction == action_add_folder) {
			slot_add_folder();
		}
		else if (selectedAction == action_change_show_name) {
			slot_change_show_name(id);
		}
		else if (selectedAction == action_delete_icon) {
			slot_delete_icon(id);
		}
		else if (selectedAction == action_unInstall_hook) {
			slot_unInstall_hook();
		}
		else if (selectedAction == action_config) {
			slot_config_widget_open();
		}
	}


	QWidget::contextMenuEvent(event);
}

//处理拖动进入窗口的事件
void Icons_inner_widget::dragEnterEvent(QDragEnterEvent* event) {
#ifdef _DEBUG
	qDebug() << "拖入";
#endif
	//当有拖入内容且只有一个内容的时候，接受
	if (event->mimeData()->hasUrls()) {
		QList<QUrl> urlList = event->mimeData()->urls();
		if (urlList.size() == 1) {
			event->acceptProposedAction();
			is_showDashedBorder = true;
			update();
		}
	}
	QWidget::dragEnterEvent(event);
}

void Icons_inner_widget::dragLeaveEvent(QDragLeaveEvent* event) {
#ifdef _DEBUG
	qDebug() << "移出";
#endif
	is_showDashedBorder = false;
	update();

	QWidget::dragLeaveEvent(event);
}


//处理拖动移动窗口的事件
void Icons_inner_widget::dragMoveEvent(QDragMoveEvent* event) {
	//计算鼠标落点，获取对应索引，判断当前位置是否已被占据，如果有，则不显示虚线，且释放会放弃本次拖动
	for (int i = 0; i < vec_coordinate.size(); ++i) {
		const auto& coordinate = vec_coordinate.at(i);
		QRect        rect(coordinate.first, coordinate.second, 95, 95);
		//找到目前鼠标停留的坐标与之对应的按钮预留位
		if (rect.contains(event->position().toPoint())) {
			//判断当前位置上是否已经有其他图标了，如果有，则放弃本次移动事件
			if (map_index_button.contains(i)) {
				is_showDashedBorder = false;
				// is_icon_overlap = true;
#ifdef _DEBUG
				qDebug() << "图标重叠";
#endif
			}
			else {
				// is_icon_overlap = false;
				is_showDashedBorder = true;
			}
		}
	}
	update();
	QWidget::dragMoveEvent(event);
}

//处理拖动释放到窗口的事件
void Icons_inner_widget::dropEvent(QDropEvent* event) {
	int index = -1;		  //存储释放鼠标的时候当前格子索引
	//计算鼠标落点，获取对应索引，判断当前位置是否已被占据，如果有，则不显示虚线，且释放会放弃本次拖动
	for (int i = 0; i < vec_coordinate.size(); ++i) {
		const auto& coordinate = vec_coordinate.at(i);
		QRect        rect(coordinate.first, coordinate.second, 95, 95);
		//找到目前鼠标停留的坐标与之对应的按钮预留位
		if (rect.contains(event->position().toPoint())) {
			//判断当前位置上是否已经有其他图标了，如果有，则放弃本次移动事件
			if (map_index_button.contains(i)) {
#ifdef _DEBUG
				qDebug() << "图标重叠放弃本次拖动事件";
#endif
				return;		//如果重叠放弃本次拖动事件
			}
			else {
				index = i;
			}
		}
	}

	if (index == -1) return;

	if (event->mimeData()->hasUrls()) {
		QList<QUrl> urlList = event->mimeData()->urls();
		//就算是多个也只要第一个(正常来说在dragEnterEvent已经做了过滤了，所以这里出现多个反而是有问题)
		auto fileName = urlList.at(0).toLocalFile();
		qInfo() << "拖入文件路径: " << fileName;

		//处理新增
		handleDroppedItem(fileName, index);
	}

	is_showDashedBorder = false;
	update();
	QWidget::dropEvent(event);
}

void Icons_inner_widget::handleDroppedItem(const QString& fileName, int index) {
	Config config;
	QString creationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	config.id = index;
	config.fileName = fileName.toStdString();
	QString name = QFileInfo(fileName).fileName();
	config.showName = name.toStdString();
	config.absolutePath = fileName.toStdString();
	config.creationTime = creationTime.toStdString();
	config.lastMoveTime = creationTime.toStdString();
	config.coordinate = Coordinate(vec_coordinate[index].first, vec_coordinate[index].second);

	auto tmp_qb = new IconButton(this, config);
	init_button_connect(tmp_qb);
	tmp_qb->setGeometry(vec_coordinate[index].first, vec_coordinate[index].second, icon_button_size, icon_button_size);
	connect(tmp_qb, &IconButton::sig_buttonDragged, this, &Icons_inner_widget::slot_showDashedBorder);
	connect(tmp_qb, &IconButton::sig_move_modify_config, this, &Icons_inner_widget::slot_move_modify_config);
	tmp_qb->show();
	map_index_button[index] = tmp_qb;
	vec_iconButton.push_back(std::move(tmp_qb)); // 将按钮添加到 vec_iconButton
	vec_config.push_back(config);

	//更新配置文件,回写进配置文件
	modify_config(ADD, config);
	qInfo() << "新增成功";
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
	for (const auto config : vec_config) {
		auto tmp_qb = new IconButton(this, config);
		tmp_qb->setGeometry(config.coordinate.x, config.coordinate.y, icon_button_size, icon_button_size);
		qDebug() << "Button ID:" << config.id << "Position:" << config.coordinate.x << config.coordinate.y;
		vec_iconButton.push_back(tmp_qb);     // 将按钮添加到 vec_iconButton
		map_index_button[config.id] = tmp_qb; // 使用一个映射来记录按钮和其索引
	}
}

void Icons_inner_widget::init_button_connect(const IconButton* icon_button) {
	connect(icon_button, &IconButton::sig_buttonDragged, this, &Icons_inner_widget::slot_showDashedBorder);
	connect(icon_button, &IconButton::sig_move_modify_config, this, &Icons_inner_widget::slot_move_modify_config);
	connect(icon_button, &IconButton::sig_modify_config, this, &Icons_inner_widget::slot_modify_config);
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
	configDesktop.showName = "Desktop";
	configDesktop.absolutePath = desktopPath.toStdString();
	configDesktop.creationTime = creationTime.toStdString();
	configDesktop.lastMoveTime = creationTime.toStdString();
	configDesktop.coordinate = Coordinate(vec_coordinate[0].first, vec_coordinate[0].second);
	// configDesktop.count        = 0;
	jsonArray_.push_back(configDesktop.toJson());

	// 下载文件夹
	Config configDownload;
	configDownload.id = 1;
	configDownload.fileName = "Download";
	configDownload.showName = "Download";
	configDownload.absolutePath = downloadsPath.toStdString();
	configDownload.coordinate = Coordinate(vec_coordinate[1].first, vec_coordinate[1].second);
	// configDownload.count        = 0;
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
	json config_jsonArray = json::parse(str_config_content);
	for (auto& config_json : config_jsonArray) {
		vec_config.push_back(Config::fromJson(config_json));
	}
}

//增加/移除 配置项目
bool Icons_inner_widget::modify_config(Config_operate opt, const Config& config) {
	file_config->seek(0);
	QTextStream out(file_config);
	switch (opt) {
	case ADD: {
		// config_jsonArray.push_back(config.toJson());
		json config_jsonArray;
		for (const auto& config1 : vec_config) {
			config_jsonArray.push_back(config1.toJson());
		}
		file_config->resize(0);
		out << QString::fromUtf8(config_jsonArray.dump(4));
		qInfo() << "写入新配置成功";
		return true;
	}
	case DEL: {
		json config_jsonArray;
		for (const auto& config1 : vec_config) {
			config_jsonArray.push_back(config1.toJson());
		}
		file_config->resize(0);
		out << QString::fromUtf8(config_jsonArray.dump(4));
		return true;
	}
	case MODIFY: //修改和移动同性质
	case MOVE: {
		json config_jsonArray;
		for (const auto& config1 : vec_config) {
			config_jsonArray.push_back(config1.toJson());
		}
		file_config->resize(0);
		out << QString::fromUtf8(config_jsonArray.dump(4));
		return true;
	}
	default: { return false; }
	}
}

bool Icons_inner_widget::modify_config(Config_operate opt) {
	Config occupy;
	return modify_config(opt, occupy);
}


//拖拽的时候显示虚线和隐藏
void Icons_inner_widget::slot_showDashedBorder(bool is_moving) {
	if (is_moving) {
		is_showDashedBorder = true; // 显示虚线
		update();
	}
	else {
		is_showDashedBorder = false; // 隐藏虚线
		update();
	}
}

//拖动后修改配置
void Icons_inner_widget::slot_move_modify_config(int new_index, int old_index) {
	//先从保存区获取该按钮对象指针
	auto it = map_index_button.find(old_index);
	if (it == map_index_button.end()) {
		qWarning() << "未找到对应索引, 移动失败";
		return;
	}

	if (old_index != it.key()) {
		qWarning() << "移动的时候未找到对应索引 ==>  old_index = " << old_index << "  it.key = " << it.key();
		return;
	}

	if (map_index_button.contains(new_index)) {
		qWarning() << "新索引已存在，无法移动";
		return;
	}

	IconButton* move_button = it.value();
	map_index_button.remove(it.key());
	map_index_button[new_index] = move_button;


	for (auto& config : vec_config) {
		if (config.id == old_index) {
			config.id = new_index;
			QString creationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
			config.lastMoveTime = creationTime.toStdString();
			//在移动完成后再发的信号，所以直接获取坐标偏移
			config.coordinate.x = move_button->pos().x();
			config.coordinate.y = move_button->pos().y();
			break;
		}
	}
	modify_config(MOVE);
}

void Icons_inner_widget::slot_modify_config(const Config& config_) {
	qInfo() << "修改配置文件";

	//先从保存区获取该按钮对象指针
	auto it = map_index_button.find(config_.id);
	if (it == map_index_button.end()) {
		qWarning() << "未找到对应索引, 修改失败";
		return;
	}

	IconButton* move_button = it.value();
	for (auto& config : vec_config) {
		if (config.id == config_.id) {
			QString creationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
			config.lastMoveTime = creationTime.toStdString();
			//在移动完成后再发的信号，所以直接获取坐标偏移
			config.coordinate.x = move_button->pos().x();
			config.coordinate.y = move_button->pos().y();

			//修改显示名称
			config.showName = config_.showName;

			break;
		}
	}
	modify_config(MODIFY);
}

void Icons_inner_widget::slot_open_icon_path(int id) {
	auto ptr_icon_button = map_index_button.find(id);
	if (ptr_icon_button != map_index_button.end()) {
		auto btn = ptr_icon_button.value();

		QFileInfo fileInfo(btn->getFilePath());
		if (fileInfo.exists()) {
			// 调用Windows资源管理器打开文件夹
			QStringList args;
			args << "/select," << QDir::toNativeSeparators(btn->getFilePath());
			QProcess::startDetached("explorer", args);
		}
		else {
			qWarning() << "路径不存在";
		}
	}
}

//寻找新增可用位置
int Icons_inner_widget::findEmptyPosition() {
	for (int index = 0; index < SUM; ++index) {
		if (auto it = map_index_button.find(index); it != map_index_button.end()) {
			//查到了,说明该位置有按钮了，跳过继续下一个
			continue;
		}
		return index;
	}
	qWarning() << "无可用位置,请检查内容是否多余 " << SUM + 1 << " 个";
	return -1;
}

//右键添加icon
void Icons_inner_widget::slot_add_icon() {
	//记得打开这些新窗口前卸载掉鼠标键盘钩子并在最后加回来
	WindowsHookMouseEx::getWindowHook()->unInstallHook();
	WindowsHookKeyEx::getWindowHook()->unInstallHook();

	QString fileName = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("所有文件 (*)"));
	if (fileName.isEmpty()) return;
	//从索引1开始遍历，直到找到空位索引在进行添加
	int index = findEmptyPosition();
	if (index == -1) {
		qWarning() << "新增失败";
		return;
	}

	//找到空位了，放在该位置后手动显示
	Config  config;
	QString creationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	config.id = index;
	config.fileName = fileName.toStdString();
	QString fileName_ = QFileInfo(fileName).fileName();
	config.showName = fileName_.toStdString();
	config.absolutePath = fileName.toStdString();
	config.creationTime = creationTime.toStdString();
	config.lastMoveTime = creationTime.toStdString();
	config.coordinate = Coordinate(vec_coordinate[index].first, vec_coordinate[index].second);
	// config.count         = 0;

	auto tmp_qb = new IconButton(this, config);
	init_button_connect(tmp_qb);

	tmp_qb->setGeometry(vec_coordinate[index].first, vec_coordinate[index].second, icon_button_size, icon_button_size);
	connect(tmp_qb, &IconButton::sig_buttonDragged, this, &Icons_inner_widget::slot_showDashedBorder);
	connect(tmp_qb, &IconButton::sig_move_modify_config, this, &Icons_inner_widget::slot_move_modify_config);
	tmp_qb->show();
	map_index_button[index] = tmp_qb;
	vec_iconButton.push_back(std::move(tmp_qb)); // 将按钮添加到 vec_iconButton
	vec_config.push_back(config);

	//更新配置文件,回写进配置文件
	modify_config(ADD, config);
	qInfo() << "新增成功";

	WindowsHookMouseEx::getWindowHook()->installHook();
	WindowsHookKeyEx::getWindowHook()->installHook();
}

//新增文件夹
void Icons_inner_widget::slot_add_folder() {
	//记得打开这些新窗口前卸载掉鼠标键盘钩子并在最后加回来
	WindowsHookMouseEx::getWindowHook()->unInstallHook();
	WindowsHookKeyEx::getWindowHook()->unInstallHook();

	QString fileName = QFileDialog::getExistingDirectory(this, tr("选择文件夹"),
		QString(QStandardPaths::writableLocation(
			QStandardPaths::HomeLocation)), //初始路径文档？好像是这个
		QFileDialog::ShowDirsOnly);
	if (fileName.isEmpty()) return;
	//从索引1开始遍历，直到找到空位索引在进行添加
	int index = findEmptyPosition();
	if (index == -1) {
		qWarning() << "新增失败";
		return;
	}

	//处理新增
	handleDroppedItem(fileName, index);

	WindowsHookMouseEx::getWindowHook()->installHook();
	WindowsHookKeyEx::getWindowHook()->installHook();
}

//修改显示名称
void Icons_inner_widget::slot_change_show_name(int id) {
	auto ptr_icon_button = map_index_button.find(id);
	if (ptr_icon_button != map_index_button.end()) {
		IconButton* button = ptr_icon_button.value();
		emit button->sig_changeShowName(); //进行修改
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
		vec_config.removeIf([id](const Config& config) {
			return config.id == id;
			});

		modify_config(DEL);
	}
}

//锁定界面（暂时卸载钩子）
void Icons_inner_widget::slot_unInstall_hook() {
	//锁定，卸载钩子
	if (is_install_hook) {
		WindowsHookMouseEx::getWindowHook()->unInstallHook();
		WindowsHookKeyEx::getWindowHook()->unInstallHook();
		is_install_hook = false;
		setAcceptDrops(true);
	}
	else {
		//注册钩子
		WindowsHookMouseEx::getWindowHook()->installHook();
		WindowsHookKeyEx::getWindowHook()->installHook();
		is_install_hook = true;
		setAcceptDrops(false);
	}
}


//右键打开配置
void Icons_inner_widget::slot_config_widget_open() {
	auto config_window = new Config_window();
	config_window->show();
}


// void Icons_inner_widget::slot_config_widget_close() {
// 	WindowsHookKeyEx::getWindowHook()->installHook();
// 	WindowsHookMouseEx::getWindowHook()->installHook();
// }
