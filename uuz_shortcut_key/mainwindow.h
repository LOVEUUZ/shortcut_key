#pragma once

#include <QTableView>
#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <fstream>

#include "json.hpp"
#include "ui_mainwindow.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    std::string currentFilePath;

  private:
    Ui::MainWindowClass ui;
    void                keyPressEvent(QKeyEvent* event) override;

    std::string         default_config_file_json;
    nlohmann::json      default_config_json;
    QTableView*         table_view;
    QStandardItemModel* model_;
    QStatusBar*         statusBar;



    void initDefaultConfigFile();
    void initMenu();
    void initConnect();
    void initTableView();
    void initView();
    void initContext();
    void initModel();

    nlohmann::json analyzeJson() const;
    void           savaConfigJson() const;
    void           updateModel(const nlohmann::json &);
    void           addViewNewRow();
    void           updateDefaultConfigFile();
};
