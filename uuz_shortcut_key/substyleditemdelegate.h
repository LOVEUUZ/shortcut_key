#pragma once

#include <QStyledItemDelegate>

#include "my_dialog_accept_filepath.h"

class SubStyledItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

    // mutable My_dialog_accept_filePath* my_dialog_accept_file_path_;
    mutable QString tmp_new_path;

  public:
    SubStyledItemDelegate(QObject* parent);
    ~SubStyledItemDelegate();

    void     paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    QWidget* createEditor(QWidget*            parent, const QStyleOptionViewItem & option,
                          const QModelIndex & index) const override;
    void setEditorData(QWidget* editor, const QModelIndex & index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex & index) const override;
    void updateEditorGeometry(QWidget*            editor, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const override;

  protected:
    bool editorEvent(QEvent*             event, QAbstractItemModel* model, const QStyleOptionViewItem & option,
                     const QModelIndex & index) override;

    bool eventFilter(QObject* object, QEvent* event) override;

  protected slots:
    void onCloseEditor(QWidget* editor);
    void onCommitData(QWidget* editor);

    void commitAndCloseEditor(const QString & text) {
      QWidget* editor = qobject_cast<QWidget*>(sender());
      emit commitData(editor);
      emit closeEditor(editor, QAbstractItemDelegate::NoHint);
    }
};
