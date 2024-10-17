#pragma once

#include <QTextEdit>
#include <QStringList>

#include "search_content.h"


class TextEdit : public QTextEdit {
    Q_OBJECT

  public:
    TextEdit(QWidget* parent = nullptr);
    ~TextEdit() override;

  protected:
    void focusOutEvent(QFocusEvent* event) override;

  private:
    QStringList content;

  signals:
    void sig_editModify();
};
