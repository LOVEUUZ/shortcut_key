#include "IconButton.h"

int                      IconButton::iconSize  = 64;
bool                     IconButton::is_moving = false;
QVector<QPair<int, int>> IconButton::vec_coordinate;

IconButton::IconButton(QWidget* parent, QVector<QPair<int, int>> & vec, QString filePath) : QToolButton(parent),
  filePath(filePath) {
  // filePath = "D:/software/PDF/FoxitPDFEditor2024/FoxitPDFEditor.exe";
  vec_coordinate = vec;

  init_icon(filePath);

  // 设置透明背景，只显示图标和文字
  setAttribute(Qt::WA_TranslucentBackground, true);
  setStyleSheet("QToolButton { background: transparent; border: none; }");
}

IconButton::~IconButton() {}

void IconButton::init_icon(const QString & filePath) {
  QFileIconProvider iconProvider;
  QIcon             fileIcon = iconProvider.icon(QFileInfo(filePath));
  QPixmap           iconPixmap = fileIcon.pixmap(iconSize, iconSize); // 获取图标并设置大小
  m_icon = QIcon(iconPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 缩放图标

  // 设置图标和样式
  setIcon(m_icon);
  setIconSize(QSize(iconSize, iconSize));
  setText("QQ");
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // 图标在文字上方
  setStyleSheet("QToolButton { background-color: #b7b7b7; border: 1px solid #888; border-radius: 5px; }");
}

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

void IconButton::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    if (is_moving) {
      // 计算当前所在的区域
      QPoint globalMousePos = QCursor::pos();               //鼠标全局坐标
      QPoint localMousePos  = mapFromGlobal(globalMousePos);//转为鼠标位置相对于按钮的偏移

      QPoint currentPos   = pos() + localMousePos;
      int    closestIndex = calculateClosestIndex(currentPos); // 计算最近的索引
      // 适应到新位置
      if (closestIndex != -1) {
        QPoint newCoordinate = QPoint(vec_coordinate[closestIndex].first, vec_coordinate[closestIndex].second);
        move(newCoordinate); // 移动到新的坐标
      }
    }
    else {
      openFile(filePath); // 打开对应程序
    }
    // 恢复样式
    setStyleSheet("QToolButton { background-color: #b7b7b7; border: 1px solid #888; border-radius: 5px; }");
    setCursor(Qt::ArrowCursor);
    is_moving = false; // 状态重置
  }
  QToolButton::mouseReleaseEvent(event);
}

void IconButton::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    int distance = (event->pos() - dragStartPos).manhattanLength();
    if (distance > 5) {
      // 设定拖动的阈值
      is_moving = true;

      // 发出信号通知父窗口显示虚线
      emit buttonDragged(true);

      // 获取父窗口相对位置
      QPoint parentPos = parentWidget()->mapFromGlobal(QCursor::pos());
      QPoint newPos    = parentPos - QPoint(iconSize / 2, iconSize / 2);

      // 判断是否超出边界
      QRect parentRect = parentWidget()->rect();
      if (!parentRect.contains(newPos)) {
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
  setStyleSheet("QToolButton { background: rgba(0, 0, 0, 0.2); border: none; }"); // 阴影效果
  QToolButton::enterEvent(event);
}

//鼠标离开的样式
void IconButton::leaveEvent(QEvent* event) {
  // 鼠标离开时的样式
  setStyleSheet("QToolButton { background: transparent; border: none; }");
  QToolButton::leaveEvent(event);
}


void IconButton::openFile(const QString & filePath) {
  QString filePath_tmp = QDir::toNativeSeparators(filePath); // 转为本地格式
  QUrl    fileUrl      = QUrl::fromLocalFile(filePath_tmp);  // 转为url
  QDesktopServices::openUrl(fileUrl);                        // 使用该函数可以打开exe，也能打开jpg，txt等文件

  qDebug() << "Starting process:" << filePath_tmp;
}


// 计算最近的索引
int IconButton::calculateClosestIndex(const QPoint & pos) {
  for (int i = 0; i < vec_coordinate.size(); ++i) {
    const auto & coordinate = vec_coordinate[i];
    QRect        rect(coordinate.first, coordinate.second, 95, 95);
    if (rect.contains(pos)) {
      return i; // 返回最近的索引
    }
  }
  return -1; // 没有找到
}
