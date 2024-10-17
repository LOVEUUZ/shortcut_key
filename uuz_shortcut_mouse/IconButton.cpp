#include "IconButton.h"

#include <utility>

#include "icons_inner_widget.h"

#include "mainwidget.h"
class MainWidget;


int  icon_button::iconSize  = 64;
bool icon_button::is_moving = false;


icon_button::icon_button(QWidget* parent, const Config config) : QToolButton(parent), config(config),
                                                                 change_show_name_edit(nullptr) {
  this->ID       = config.id;
  this->filePath = QString::fromUtf8(config.absolutePath);
  showName       = QString::fromStdString(config.showName); //显示名称
  setText(showName);
  parent_widget_content = dynamic_cast<Icons_inner_widget*>(parentWidget());
  vec_coordinate        = parent_widget_content->getVecCoordinate();
  map_index_button      = parent_widget_content->getMapIndexButton();

  init_icon(filePath);

  // 设置透明背景，只显示图标和文字
  setAttribute(Qt::WA_TranslucentBackground, true);
  setStyleSheet("QToolButton { background: transparent; border: none; }");

  connect(this, &icon_button::sig_changeShowName, this, &icon_button::slot_changeShowName);
}


icon_button::~icon_button() {}


void icon_button::init_icon(const QString & filePath) {
  QFileInfo fileInfo(filePath);
  QString   resolvedPath = filePath;

  // 检查是否是快捷方式,如果是则返回真实路径z
  QFileInfo shortcutInfo(filePath);
  if (shortcutInfo.isSymLink()) {
    resolvedPath = shortcutInfo.symLinkTarget();
  }

  QFileIconProvider iconProvider;
  QIcon             fileIcon = iconProvider.icon(QFileInfo(resolvedPath));
  QPixmap           iconPixmap = fileIcon.pixmap(iconSize, iconSize); // 获取图标并设置大小
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
void icon_button::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    original_pos = pos(); // 记录初始位置
    // 添加按下效果，改变样式
    setStyleSheet("QToolButton { background-color: #a0a0a0; border: 1px solid #666; border-radius: 5px; }");
    drag_start_pos = event->pos();
    setCursor(Qt::ClosedHandCursor);
  }
  QToolButton::mousePressEvent(event);
}

//释放事件
void icon_button::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    if (is_moving) {
      // 计算当前所在的区域
      QPoint globalMousePos = QCursor::pos();                //鼠标全局坐标
      QPoint localMousePos  = mapFromGlobal(globalMousePos); //转为鼠标位置相对于按钮的偏移

      QPoint currentPos   = pos() + localMousePos;
      int    closestIndex = calculateClosestIndex(currentPos); // 计算最近的索引
      // qDebug() << map_index_button;
      // 适应到新位置
      if (closestIndex != -1) {
        auto newCoordinate = QPoint(vec_coordinate->at(closestIndex).first, vec_coordinate->at(closestIndex).second);
        move(newCoordinate); // 移动到新的坐标
        //放置到新位置后，更新索引与按钮的映射

        //移动完成，通知父窗口修改配置文件
        emit sig_moveModifyConfig(closestIndex, ID);

#ifdef _DEBUG
				qDebug() << "closestIndex ==> " << closestIndex << "   old_index ==> " << ID;
#endif

        //记得及时更新自身的配置，特别的移动后需要修改索引id
        ID        = closestIndex;
        config.id = ID;
      }
      else move(original_pos);
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
void icon_button::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    int distance = (event->pos() - drag_start_pos).manhattanLength();
    if (distance > 5) {
      is_moving = true;

      // 发出信号通知父窗口显示虚线
      emit sig_buttonDragged(true);

      // 获取父窗口相对位置
      QPoint parentPos = parentWidget()->mapFromGlobal(QCursor::pos());
      QPoint newPos    = parentPos - QPoint(iconSize / 2, iconSize / 2);

      // 判断鼠标是否在父窗口中
      if (!parentWidget()->underMouse()) {
        move(original_pos); // 超出边界，回到初始位置
        return;
      }

      move(newPos); // 更新位置
      raise();      // 提升层级
    }
  }
  QToolButton::mouseMoveEvent(event);
}

//鼠标移入的样式
void icon_button::enterEvent(QEnterEvent* event) {
  // 鼠标悬浮时的样式
  setStyleSheet("QToolButton { background: rgba(0, 0, 0, 0.2); border: none; border-radius: 5px }"); // 阴影效果
  QToolButton::enterEvent(event);
}

//鼠标离开的样式
void icon_button::leaveEvent(QEvent* event) {
  // 鼠标离开时的样式
  setStyleSheet("QToolButton { background: transparent; border: none; border-radius: 5px }");
  QToolButton::leaveEvent(event);
}


void icon_button::openFile(const QString & filePath) {
  QString   filePath_tmp = QDir::toNativeSeparators(filePath); // 转为本地格式
  QFileInfo fileInfo(filePath_tmp);
  QString   directory = fileInfo.absolutePath();

  // 保存当前工作目录
  QString currentDir = QDir::currentPath();

  // 改变当前工作目录（有些汉化组封包的gal就不行，必须要切过去才能运行，要不然会提示各种找不到）
  QDir::setCurrent(directory);

  // 打开文件/文件夹
  QUrl fileUrl = QUrl::fromLocalFile(filePath_tmp); // 转为url
  QDesktopServices::openUrl(fileUrl);               // 使用该函数可以打开exe，也能打开jpg，txt等文件

  qInfo() << "Starting process:" << filePath_tmp;

  // 恢复原来的工作目录
  QDir::setCurrent(currentDir);

  // 主动失去焦点，隐藏主窗口
  QWidget* widget_ = this->parentWidget()->parentWidget()->parentWidget();
  // this->icons_inner_widget->stacked_widget->MainWidget
  auto mainWidget = qobject_cast<MainWidget*>(widget_);
  if (mainWidget != nullptr) {
    emit mainWidget->sig_moveFocus(nullptr);
  }
}


// 计算最近的索引
int icon_button::calculateClosestIndex(const QPoint & pos) {
  for (int i = 0; i < vec_coordinate->size(); ++i) {
    const auto & coordinate = vec_coordinate->at(i);
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

void icon_button::slot_changeShowName() {
  if (change_show_name_edit == nullptr) {
    change_show_name_edit = new QLineEdit(this);
    connect(change_show_name_edit, &QLineEdit::editingFinished, this, [&]() {
      QString newShowName = change_show_name_edit->text();
      qInfo() << "修改后的名称为 >> " << newShowName;
      setShowName(newShowName);
      setText(newShowName); // 更新按钮显示的文本
      change_show_name_edit->hide();

      //然后去更新配置文件
      config.showName = newShowName.toStdString();
      emit sig_modifyConfig(config);
    });
  }

  change_show_name_edit->setText(showName);

  // 设置编辑框的在底部，宽度为整个对象
  int textY = height() - change_show_name_edit->height(); // 将编辑框放在按钮的底部
  change_show_name_edit->setGeometry(0, textY, width(), change_show_name_edit->height());

  change_show_name_edit->show();
  change_show_name_edit->setFocus();
}
