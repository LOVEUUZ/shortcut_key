#include "substyleditemdelegate.h"

#include <iostream>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QMouseEvent>

#include "mainwindow.h"
#include "my_dialog_accept_filepath.h"

// 声明全局变量
extern MainWindow* globalVar;

SubStyledItemDelegate::SubStyledItemDelegate(QObject* parent): QStyledItemDelegate(parent) {
  connect(this, qOverload<QWidget*, QAbstractItemDelegate::EndEditHint>(&QAbstractItemDelegate::closeEditor),
          this, qOverload<QWidget*>(&SubStyledItemDelegate::onCloseEditor));

  // connect(this, SIGNAL(commitData(QWidget*)), this, SLOT(onCommitData(QWidget*)));
  connect(this, qOverload<QWidget*>(&QAbstractItemDelegate::commitData), this,
          qOverload<QWidget*>(&SubStyledItemDelegate::onCommitData));
}

SubStyledItemDelegate::~SubStyledItemDelegate() {}


//1.创建编辑器
QWidget* SubStyledItemDelegate::createEditor(QWidget*            parent, const QStyleOptionViewItem & option,
                                             const QModelIndex & index) const {
  qDebug() << "1. 创建编辑器";
  if (index.column() == 4) {
    My_dialog_accept_filePath* editor = new My_dialog_accept_filePath(parent);

    connect(editor, &My_dialog_accept_filePath::str_path_ok, this,
            &SubStyledItemDelegate::commitAndCloseEditor);
    // editor->exec();      //记住不能用exec，会提示是外部的窗口调用的提交数据信号，造成直接关闭编辑器
    return editor;
  }
  return QStyledItemDelegate::createEditor(parent, option, index);
}

//2.更新编辑器位置
void SubStyledItemDelegate::updateEditorGeometry(QWidget*            editor, const QStyleOptionViewItem & option,
                                                 const QModelIndex & index) const {
  qDebug() << "2. 更新编辑器位置";

  if (qobject_cast<My_dialog_accept_filePath*>(editor)) {
    //获取主窗口的位置和尺寸
    QRect main_window_geometry = globalVar->geometry();
    //移动对话框到中心位置
    editor->move(main_window_geometry.x() + main_window_geometry.width() / 2 - editor->width() / 2,
                 main_window_geometry.y() + main_window_geometry.height() / 2 - editor->height() / 2);
    return;
  }


  QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

//3. 6. 通过索引从模型中获取数据
void SubStyledItemDelegate::setEditorData(QWidget* editor, const QModelIndex & index) const {
  qDebug() << "3.6. 设置编辑器数据";
  QString                    value           = index.model()->data(index, Qt::EditRole).toString();
  My_dialog_accept_filePath* multilineEditor = qobject_cast<My_dialog_accept_filePath*>(editor);
  if (multilineEditor)
    multilineEditor->my_lineEdit_exe_path_->setText(value);

  QStyledItemDelegate::setEditorData(editor, index);
}


// 4.  信号 编辑器提交数据
void SubStyledItemDelegate::onCommitData(QWidget* editor) {
  qDebug() << "4.提交数据信号触发";
}

//5.  将编辑后的新数据返回模型
void SubStyledItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex & index) const {
  qDebug() << "5.设置模型数据";
  My_dialog_accept_filePath* multilineEditor = qobject_cast<My_dialog_accept_filePath*>(editor);
  if (multilineEditor) {
    model->setData(index, multilineEditor->my_lineEdit_exe_path_->text(), Qt::EditRole);
    return;
  }


  QStyledItemDelegate::setModelData(editor, model, index);
}

// 7. 信号  关闭编辑器信号
void SubStyledItemDelegate::onCloseEditor(QWidget* editor) {
  qDebug() << "7.关闭编辑器信号触发";
}

//让符合条件的元素自定显示方式
void SubStyledItemDelegate::paint(QPainter*           painter, const QStyleOptionViewItem & option,
                                  const QModelIndex & index) const {
  if (index.data().typeId() == QMetaType::Bool) {
    bool data = index.data().toBool();

    QStyleOptionButton checkBoxStyle;
    checkBoxStyle.state = data ? QStyle::State_On : QStyle::State_Off;
    checkBoxStyle.state |= QStyle::State_Enabled;
    checkBoxStyle.rect = option.rect;
    checkBoxStyle.rect.setX(option.rect.x() + option.rect.width() / 2 - 6);
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxStyle, painter);

    //不建议，因为每一个 QCheckBox 都是一个 QWidget，开销十分的大，且 会引入额外的事件处理逻辑，而且违反 模型-视图的分离原则
    // QCheckBox checkBox;
    // checkBox.setChecked(data);
    // // 绘制 QCheckBox
    // QStyleOptionButton checkBoxOption;
    // checkBoxOption.rect = option.rect;
    // checkBoxOption.state = QStyle::State_Enabled;
    // if (data) {
    //     checkBoxOption.state |= QStyle::State_On;
    // }
    // else {
    //     checkBoxOption.state |= QStyle::State_Off;
    // }
    // // 使用 QCheckBox 的样式来绘制
    // checkBox.style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);

    //测试用
    // QStyleOptionTab tabOption;
    // tabOption.rect = option.rect;
    // tabOption.text = index.data().toString();
    // tabOption.state = QStyle::State_Enabled;
    //
    // // 可以在这里设置其他属性，比如是否被选中、鼠标悬停状态等
    // if (option.state & QStyle::State_Selected) {
    //     tabOption.state |= QStyle::State_Selected;
    // }
    // if (option.state & QStyle::State_MouseOver) {
    //     tabOption.state |= QStyle::State_MouseOver;
    // }
    // // 使用 QApplication 的样式来绘制标签页
    // QApplication::style()->drawControl(QStyle::CE_TabBarTab, &tabOption, painter);

    return;
  }

  QStyledItemDelegate::paint(painter, option, index);
}

//bool类型的值走这里，不用创建编辑器就能修改值
bool SubStyledItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem & option,
                                        const QModelIndex & index) {
  qDebug() << "编辑器事件";

  QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
  if (index.data().typeId() == QMetaType::Bool) {
    if (mouseEvent) {
      if (event->type() == QEvent::MouseButtonPress && option.rect.contains(mouseEvent->pos())) {
        bool data = model->data(index, Qt::DisplayRole).toBool();
        model->setData(index, !data, Qt::DisplayRole);
        return true;
      }
      else {
        return true; //双击不走上面的if，会唤醒默认的下拉框，所以用这个阻止
      }
    }
  }


  return QStyledItemDelegate::editorEvent(event, model, option, index);
}


//不知道为什么，自定的编辑器失去焦点就会消失的问题重写这个函数然后直接返回false就能解决（返回true也行，只是需要点击其他地方触发文本更新）
bool SubStyledItemDelegate::eventFilter(QObject* object, QEvent* event) {
  // qDebug() << "事件过滤器";

  return false; //继续传播
}
