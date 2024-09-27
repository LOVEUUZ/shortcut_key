#pragma once

#include <QStyledItemDelegate>
#include "my_dialog_accept_filepath.h"

//委托
class sub_styled_item_delegate : public QStyledItemDelegate {
    Q_OBJECT

    mutable QString tmp_new_path; //可变
    mutable QComboBox* comboBox = nullptr;

  public:
    sub_styled_item_delegate(QObject* parent);
    ~sub_styled_item_delegate();

    //让符合条件的元素自定显示方式
    void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    //1.创建编辑器
    QWidget* createEditor(QWidget*            parent, const QStyleOptionViewItem & option,
                          const QModelIndex & index) const override;
    //2.更新编辑器位置
    void updateEditorGeometry(QWidget*            editor, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const override;
    //3. 6. 通过索引从模型中获取数据
    void setEditorData(QWidget* editor, const QModelIndex & index) const override;
    //5.  将编辑后的新数据返回模型
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex & index) const override;

  protected:
    bool editorEvent(QEvent*             event, QAbstractItemModel* model, const QStyleOptionViewItem & option,
                     const QModelIndex & index) override;

    bool eventFilter(QObject* object, QEvent* event) override;

  protected slots:
    // 4. 信号  编辑器提交数据
    void onCommitData(QWidget* editor);
    // 7. 信号  关闭编辑器信号
    void onCloseEditor(QWidget* editor);


    //槽: 发送事件:提交和关闭
    void commitAndCloseEditor() {
      QWidget* editor = qobject_cast<QWidget*>(sender());
      emit commitData(editor);
      emit closeEditor(editor, QAbstractItemDelegate::NoHint);
    }
};
