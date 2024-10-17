#pragma once

#include <QLineEdit>
#include <QKeyEvent>

#include "ui_Search_line.h"

// class MainWidget;

class Search_line : public QLineEdit {
    Q_OBJECT

  public:
    Search_line(QWidget* parent = nullptr);
    ~Search_line() override;

    // Ui::Search_lineClass ui;


  protected:
    void keyPressEvent(QKeyEvent* event) override;
};
