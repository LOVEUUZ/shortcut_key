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
	QPushButton::paintEvent(event); // ���û���Ļ��Ʒ���

	setStyleSheet("IconButton{background-color: #b7b7b7;} ");

	

	// ��ȡ�ļ�ͼ��
	QFileIconProvider iconProvider;
	QIcon fileIcon = iconProvider.icon(QFileInfo(filePath));
	QPixmap pixmap = fileIcon.pixmap(64, 64); // 64x64 ��ͼ��Ĵ�С

	QPainter painter(this);

	// ����ͼ����ı���λ��
	int iconSize = 64; // ͼ���С
	int iconX = (width() - iconSize) / 2; // ˮƽ����
	int iconY = 7; // ���붥���ľ���

	// ����ͼ��
	painter.drawPixmap(iconX, iconY, pixmap);

	// ��������
	painter.drawText(0, iconY + iconSize, width(), height() - iconY - iconSize - 5, Qt::AlignHCenter, "QQ");
}

void IconButton::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		qDebug() << "IconButton clicked!";

	}
	QPushButton::mousePressEvent(event); // ���û����ʵ��
}


void IconButton::mouseMoveEvent(QMouseEvent* event) {
	is_move = true;

	QPushButton::mouseMoveEvent(event);
}


void IconButton::mouseReleaseEvent(QMouseEvent* event) {
	//�ж��Ƿ񳤰���
	if (is_move) {
		is_move = false;
		return;
	}
	//�򿪶�Ӧ����
	openFile(filePath);
	QPushButton::mouseReleaseEvent(event);
}

//todo ��֪��Ϊʲô��������������
void IconButton::openFile(QString& filePath) {
	QString filePath_tmp = QDir::toNativeSeparators(filePath); // תΪ���ظ�ʽ����������·���޷�����
	QUrl    fileUrl = QUrl::fromLocalFile(filePath_tmp);                          //תΪurl��������
	QDesktopServices::openUrl(fileUrl);                                        //ʹ�øú������Դ�exe��Ҳ�ܴ�jpg��txt���ļ�������������

	qDebug() << "Starting process:" << filePath_tmp;
	// if (!checkFile.exists() || !checkFile.isExecutable()) {
	// 	qDebug() << "File does not exist or is not executable";
	// 	return;
	// }
}

void IconButton::updateIcon(const QString& filePath) {
	QFileIconProvider iconProvider;
	QIcon fileIcon = iconProvider.icon(QFileInfo(filePath));
	m_cachedIcon = fileIcon.pixmap(64, 64); // ����ͼ��
}
