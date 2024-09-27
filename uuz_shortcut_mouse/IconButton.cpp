#include "IconButton.h"

IconButton::IconButton(QWidget* parent) : QPushButton(parent) {
	// ui.setupUi(this);
	// init_rendering();

	filePath = "D:/softWare/VSC/Code.exe";
	updateIcon(filePath);
}

IconButton::~IconButton()
{}

void IconButton::paintEvent(QPaintEvent* event) {
	QPushButton::paintEvent(event); // 调用基类的绘制方法

	setStyleSheet("IconButton{background-color: #b7b7b7;} ");

	

	// 获取文件图标
	QFileIconProvider iconProvider;
	QIcon fileIcon = iconProvider.icon(QFileInfo(filePath));
	QPixmap pixmap = fileIcon.pixmap(64, 64); // 64x64 是图标的大小

	QPainter painter(this);

	// 计算图标和文本的位置
	int iconSize = 64; // 图标大小
	int iconX = (width() - iconSize) / 2; // 水平居中
	int iconY = 7; // 距离顶部的距离

	// 绘制图标
	painter.drawPixmap(iconX, iconY, pixmap);

	// 绘制文字
	painter.drawText(0, iconY + iconSize, width(), height() - iconY - iconSize - 5, Qt::AlignHCenter, "QQ");
}

void IconButton::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		qDebug() << "IconButton clicked!";

	}
	QPushButton::mousePressEvent(event); // 调用基类的实现
}


void IconButton::mouseMoveEvent(QMouseEvent* event) {
	is_move = true;

	QPushButton::mouseMoveEvent(event);
}


void IconButton::mouseReleaseEvent(QMouseEvent* event) {
	//判断是否长按过
	if (is_move) {
		is_move = false;
		return;
	}
	//打开对应程序
	openFile(filePath);
	QPushButton::mouseReleaseEvent(event);
}

//todo 不知道为什么会运行两个窗口
void IconButton::openFile(QString& filePath) {
	QString filePath_tmp = QDir::toNativeSeparators(filePath); // 转为本地格式，避免中文路径无法启动
	QUrl    fileUrl = QUrl::fromLocalFile(filePath_tmp);                          //转为url方便启动
	QDesktopServices::openUrl(fileUrl);                                        //使用该函数可以打开exe，也能打开jpg，txt等文件，更适用这里

	qDebug() << "Starting process:" << filePath_tmp;
	// if (!checkFile.exists() || !checkFile.isExecutable()) {
	// 	qDebug() << "File does not exist or is not executable";
	// 	return;
	// }
}

void IconButton::updateIcon(const QString& filePath) {
	QFileIconProvider iconProvider;
	QIcon fileIcon = iconProvider.icon(QFileInfo(filePath));
	m_cachedIcon = fileIcon.pixmap(64, 64); // 缓存图标
}
