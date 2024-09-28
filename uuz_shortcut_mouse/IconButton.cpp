#include "IconButton.h"

int                      IconButton::iconSize  = 64;
bool                     IconButton::is_moving = false;
QVector<QPair<int, int>> IconButton::vec_coordinate;

IconButton::IconButton(QWidget* parent, QVector<QPair<int, int>> & vec, QString filePath) : QToolButton(parent),
  filePath(filePath) {
  // filePath = "D:/software/PDF/FoxitPDFEditor2024/FoxitPDFEditor.exe";
  vec_coordinate = vec;

  init_icon(filePath);

  // ����͸��������ֻ��ʾͼ�������
  setAttribute(Qt::WA_TranslucentBackground, true);
  setStyleSheet("QToolButton { background: transparent; border: none; }");
}

IconButton::~IconButton() {}

void IconButton::init_icon(const QString & filePath) {
  QFileIconProvider iconProvider;
  QIcon             fileIcon = iconProvider.icon(QFileInfo(filePath));
  QPixmap           iconPixmap = fileIcon.pixmap(iconSize, iconSize); // ��ȡͼ�겢���ô�С
  m_icon = QIcon(iconPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)); // ����ͼ��

  // ����ͼ�����ʽ
  setIcon(m_icon);
  setIconSize(QSize(iconSize, iconSize));
  setText("QQ");
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // ͼ���������Ϸ�
  setStyleSheet("QToolButton { background-color: #b7b7b7; border: 1px solid #888; border-radius: 5px; }");
}

void IconButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    originalPos = pos(); // ��¼��ʼλ��
    // ��Ӱ���Ч�����ı���ʽ
    setStyleSheet("QToolButton { background-color: #a0a0a0; border: 1px solid #666; border-radius: 5px; }");
    dragStartPos = event->pos();
    setCursor(Qt::ClosedHandCursor);
  }
  QToolButton::mousePressEvent(event);
}

void IconButton::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    if (is_moving) {
      // ���㵱ǰ���ڵ�����
      QPoint globalMousePos = QCursor::pos();               //���ȫ������
      QPoint localMousePos  = mapFromGlobal(globalMousePos);//תΪ���λ������ڰ�ť��ƫ��

      QPoint currentPos   = pos() + localMousePos;
      int    closestIndex = calculateClosestIndex(currentPos); // �������������
      // ��Ӧ����λ��
      if (closestIndex != -1) {
        QPoint newCoordinate = QPoint(vec_coordinate[closestIndex].first, vec_coordinate[closestIndex].second);
        move(newCoordinate); // �ƶ����µ�����
      }
    }
    else {
      openFile(filePath); // �򿪶�Ӧ����
    }
    // �ָ���ʽ
    setStyleSheet("QToolButton { background-color: #b7b7b7; border: 1px solid #888; border-radius: 5px; }");
    setCursor(Qt::ArrowCursor);
    is_moving = false; // ״̬����
  }
  QToolButton::mouseReleaseEvent(event);
}

void IconButton::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    int distance = (event->pos() - dragStartPos).manhattanLength();
    if (distance > 5) {
      // �趨�϶�����ֵ
      is_moving = true;

      // �����ź�֪ͨ��������ʾ����
      emit buttonDragged(true);

      // ��ȡ���������λ��
      QPoint parentPos = parentWidget()->mapFromGlobal(QCursor::pos());
      QPoint newPos    = parentPos - QPoint(iconSize / 2, iconSize / 2);

      // �ж��Ƿ񳬳��߽�
      QRect parentRect = parentWidget()->rect();
      if (!parentRect.contains(newPos)) {
        move(originalPos); // �����߽磬�ص���ʼλ��
        return;
      }

      move(newPos); // ����λ��
      raise();      // �����㼶
    }
  }
  QToolButton::mouseMoveEvent(event);
}


//����������ʽ
void IconButton::enterEvent(QEnterEvent* event) {
  // �������ʱ����ʽ
  setStyleSheet("QToolButton { background: rgba(0, 0, 0, 0.2); border: none; }"); // ��ӰЧ��
  QToolButton::enterEvent(event);
}

//����뿪����ʽ
void IconButton::leaveEvent(QEvent* event) {
  // ����뿪ʱ����ʽ
  setStyleSheet("QToolButton { background: transparent; border: none; }");
  QToolButton::leaveEvent(event);
}


void IconButton::openFile(const QString & filePath) {
  QString filePath_tmp = QDir::toNativeSeparators(filePath); // תΪ���ظ�ʽ
  QUrl    fileUrl      = QUrl::fromLocalFile(filePath_tmp);  // תΪurl
  QDesktopServices::openUrl(fileUrl);                        // ʹ�øú������Դ�exe��Ҳ�ܴ�jpg��txt���ļ�

  qDebug() << "Starting process:" << filePath_tmp;
}


// �������������
int IconButton::calculateClosestIndex(const QPoint & pos) {
  for (int i = 0; i < vec_coordinate.size(); ++i) {
    const auto & coordinate = vec_coordinate[i];
    QRect        rect(coordinate.first, coordinate.second, 95, 95);
    if (rect.contains(pos)) {
      return i; // �������������
    }
  }
  return -1; // û���ҵ�
}
