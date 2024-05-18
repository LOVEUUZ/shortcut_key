#pragma once

#include <QTableView>
#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <fstream>

#include "json.hpp"
#include "ui_mainwindow.h"
#include "substyleditemdelegate.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();


    std::string currentFilePath;

  private:
    Ui::MainWindowClass ui;
    void                keyPressEvent(QKeyEvent* event) override;

    std::string            default_config_file_json;
    nlohmann::json         default_config_json;
    QTableView*            table_view;
    SubStyledItemDelegate* delegate_;
    QStandardItemModel*    model_;
    QStatusBar*            statusBar;

    const std::string str_default_txt_path         = "/config/default.txt";
    const std::string str_default_config_json_path = "/config/default_config.json";

    void initDefaultConfigFile();
    void initMenu();
    void initConnect();
    void initTableView();
    void initView();
    void initContext() const;
    void initModel();

    nlohmann::json analyzeJson() const;
    void           savaConfigJson() const;
    void           updateModel(const nlohmann::json &) const;
    void           addViewNewRow() const;
    void           updateDefaultConfigFile();
    void           clearModel() const;
};
