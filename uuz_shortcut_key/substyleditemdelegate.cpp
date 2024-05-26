#include "substyleditemdelegate.h"

#include <iostream>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QComboBox>
#include <QPainter>
#include <QMessageBox>

#include "mainwindow.h"
#include "my_dialog_accept_filepath.h"
#include "my_dialog_key_input.h"
#include "Trigger.hpp"


// 声明全局变量
extern MainWindow* globalVar;
extern Trigger*    ptr_trigger;

sub_styled_item_delegate::sub_styled_item_delegate(QObject* parent): QStyledItemDelegate(parent) {
  //绑定结束编辑和提交数据信道到自定的槽函数上(目前无用，仅作提示)
  connect(this, qOverload<QWidget*, QAbstractItemDelegate::EndEditHint>(&QAbstractItemDelegate::closeEditor),
          this, qOverload<QWidget*>(&sub_styled_item_delegate::onCloseEditor));

  // connect(this, SIGNAL(commitData(QWidget*)), this, SLOT(onCommitData(QWidget*)));
  connect(this, qOverload<QWidget*>(&QAbstractItemDelegate::commitData), this,
          qOverload<QWidget*>(&sub_styled_item_delegate::onCommitData));
}

sub_styled_item_delegate::~sub_styled_item_delegate() {}

//1.创建编辑器
QWidget* sub_styled_item_delegate::createEditor(QWidget*            parent, const QStyleOptionViewItem & option,
                                                const QModelIndex & index) const {
#ifdef _DEBUG
  qDebug() << "1. 创建编辑器";
#endif

  //快捷键输入dialog
  if (index.column() == 5) {
    my_dialog_key_input* editor = new my_dialog_key_input(parent);
    //当改dialog点击确定的时候发出该信号，触发提交数据
    connect(editor, &my_dialog_key_input::editingCompleted, this, &sub_styled_item_delegate::commitAndCloseEditor);
    return editor;
  }

  //文件输入dialpg
  if (index.column() == 6) {
    my_dialog_accept_file_path* editor = new my_dialog_accept_file_path(parent);

    connect(editor, &my_dialog_accept_file_path::str_path_ok, this, &sub_styled_item_delegate::commitAndCloseEditor);
    // editor->exec();      //记住不能用exec，会提示是外部的窗口调用的提交数据信号，造成直接关闭编辑器
    return editor;
  }

  //触发次数列
  if (index.column() == 8) {
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItem(ptr_trigger->str_once_click, ptr_trigger->strToType(ptr_trigger->str_once_click));
    comboBox->addItem(ptr_trigger->str_double_click, ptr_trigger->strToType(ptr_trigger->str_double_click));
    comboBox->addItem(ptr_trigger->str_triple_click, ptr_trigger->strToType(ptr_trigger->str_triple_click));

    //该函数被const修饰，无法发送信号commitData和closeEditor 所以要const_cast解开
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), const_cast<sub_styled_item_delegate*>(this),
            [&](int type) {
              QWidget* editor = qobject_cast<QWidget*>(sender());
              emit const_cast<sub_styled_item_delegate*>(this)->commitData(editor);
              emit const_cast<sub_styled_item_delegate*>(this)->closeEditor(editor, QAbstractItemDelegate::NoHint);
            });

    return comboBox;
  }

  //最后一列放一个 删除 操作按钮
  if (index.column() == index.model()->columnCount() - 1) {
    return nullptr;
  }


  return QStyledItemDelegate::createEditor(parent, option, index);
}

//2.更新编辑器位置
void sub_styled_item_delegate::updateEditorGeometry(QWidget*            editor, const QStyleOptionViewItem & option,
                                                    const QModelIndex & index) const {
#ifdef _DEBUG
  qDebug() << "2. 更新编辑器位置";
#endif

  if (qobject_cast<my_dialog_accept_file_path*>(editor) || qobject_cast<my_dialog_key_input*>(editor)) {
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
void sub_styled_item_delegate::setEditorData(QWidget* editor, const QModelIndex & index) const {
#ifdef _DEBUG
  qDebug() << "3.6. 设置编辑器数据";
#endif

  QString value = index.model()->data(index, Qt::EditRole).toString();
  if (my_dialog_accept_file_path* tmp_edit = qobject_cast<my_dialog_accept_file_path*>(editor))
    tmp_edit->my_lineEdit_exe_path_->setText(value);

  if (my_dialog_key_input* tmp_edit = qobject_cast<my_dialog_key_input*>(editor)) {
    tmp_edit->line_edit->setText(value);
  }

  //触发次数列
  if (QComboBox* combo_box = qobject_cast<QComboBox*>(editor)) {
    combo_box->setCurrentIndex(index.data().toInt() - 1);
  }

  QStyledItemDelegate::setEditorData(editor, index);
}


// 4.  信号 编辑器提交数据
void sub_styled_item_delegate::onCommitData(QWidget* editor) {
#ifdef _DEBUG
  qDebug() << "4.提交数据信号触发";
#endif
}

//5.  将编辑后的新数据返回模型
void sub_styled_item_delegate::setModelData(QWidget*            editor, QAbstractItemModel* model,
                                            const QModelIndex & index) const {
#ifdef _DEBUG
  qDebug() << "5.设置模型数据";
#endif

  //快捷键接收框
  if (my_dialog_key_input* shortcut_key_editor = qobject_cast<my_dialog_key_input*>(editor)) {
    std::string str_key_ = shortcut_key_editor->str_key_tmp.str();
    QString     text     = QString::fromStdString(str_key_);
    model->setData(index, text);
    model->setData(index, shortcut_key_editor->shortcut_key_msg_.key_value_total, ROLE_KEY);

    auto vec = shortcut_key_editor->shortcut_key_msg_;
    // QVariant::fromValue将任何数据类型存入
    model->setData(index, QVariant::fromValue(vec), ROLE_VEC_KEY_NUM);
    return;
  }

  //文件路径接收框
  if (my_dialog_accept_file_path* multilineEditor = qobject_cast<my_dialog_accept_file_path*>(editor)) {
    model->setData(index, multilineEditor->my_lineEdit_exe_path_->text(), Qt::EditRole);
    return;
  }

  // 触发次数列
  if (index.column() == 8) {
    if (QComboBox* comboBox = qobject_cast<QComboBox*>(editor)) {
      QString selectedText = comboBox->currentText();
      int     triggerType  = ptr_trigger->strToType(selectedText);
      qDebug() << "Selected trigger type:" << triggerType << ", text:" << selectedText;
      model->setData(index, triggerType, Qt::EditRole);
      return;
    }
  }

  QStyledItemDelegate::setModelData(editor, model, index);
}

// 7. 信号  关闭编辑器信号
void sub_styled_item_delegate::onCloseEditor(QWidget* editor) {
#ifdef _DEBUG
  qDebug() << "7.关闭编辑器信号触发";
#endif
}

//让符合条件的元素自定显示方式
void sub_styled_item_delegate::paint(QPainter*           painter, const QStyleOptionViewItem & option,
                                     const QModelIndex & index) const {
  //下拉框将枚举转为对应的string显示
  if (index.column() == 8) {
    // 获取单元格的值
    QString stringValue = ptr_trigger->triggerToString(static_cast<Trigger::TriggerType>(index.data().toInt()));
    // 绘制文本
    painter->drawText(option.rect, Qt::AlignCenter, stringValue);
    return;
  }

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

  //最后一列放一个 删除 操作按钮
  if (index.column() == index.model()->columnCount() - 1) {
    // 设置按钮的绘制区域，带有边距
    QStyleOptionButton buttonOption;
    buttonOption.rect  = option.rect.adjusted(5, 5, -5, -5);
    buttonOption.text  = "Delete";
    buttonOption.state = QStyle::State_Enabled;

    QPushButton button;
    button.setStyle(option.widget->style());
    button.style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, &button);
    return;
  }


  QStyledItemDelegate::paint(painter, option, index);
}

//bool类型的值走这里，不用创建编辑器就能修改值
bool sub_styled_item_delegate::editorEvent(QEvent*                      event, QAbstractItemModel* model,
                                           const QStyleOptionViewItem & option,
                                           const QModelIndex &          index) {
#ifdef _DEBUG
  qDebug() << "编辑器事件";
#endif
  if (index.data().typeId() == QMetaType::Bool) {
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
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

  //最后一行的删除按钮的事件
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (index.column() == index.model()->columnCount() - 1) {
      // 设置按钮的点击区域，带有边距
      int   margin     = 5;
      QRect buttonRect = option.rect.adjusted(margin, margin, -margin, -margin);
      if (buttonRect.contains(mouseEvent->pos())) {
        if (model->rowCount() <= 1) {
          QMessageBox::warning(nullptr, tr("Warning"), tr("Unable to delete last row"));
          return true;
        }
        model->removeRow(index.row());
        return true;
      }
    }
  }

  //如果是第5列(快捷键输入)，则输入不会触发编辑器创建，而是无视(只有双击才能在次处创建编辑器)
  if (index.column() == 5) {
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease || event->type() ==
        QEvent::InputMethod) {
      return true;
    }
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}


//不知道为什么，自定的编辑器失去焦点就会消失的问题重写这个函数然后直接返回false就能解决（返回true也行，只是需要点击其他地方触发文本更新）
bool sub_styled_item_delegate::eventFilter(QObject* object, QEvent* event) {
  // qDebug() << "事件过滤器";
  return false;
  // return QStyledItemDelegate::eventFilter(object, event); //继续传播
}
