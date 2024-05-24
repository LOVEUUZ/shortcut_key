#pragma once

#include <QTableView>
#include <QStandardItemModel>
#include <fstream>
#include <mutex>
#include <queue>

#include "json.hpp"
#include "ui_mainwindow.h"
#include "substyleditemdelegate.h"
#include "keyEvent.h"
#include "WindowsHookEx.h"

extern HANDLE         hMutex;
extern nlohmann::json glob_json_;

constexpr int ROLE_KEY = Qt::UserRole + 1;
constexpr int ROLE_VEC_KEY_NUM = Qt::UserRole + 2;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    std::string    currentFilePath;
    // WindowsHookEx* ptr_windows_hook;

    // void     set_key_event(const KeyEvent & key_event);
    // KeyEvent getKeyEvent();
    //
    // void queue_clear() {
    //   std::unique_lock<std::mutex> lock(mtx);
    //   for (int i = 0; i < queue_key_event.size(); ++i) {
    //     queue_key_event.pop();
    //   }
    // }
    //
    // void setFunc();

  private:
    Ui::MainWindowClass ui;

    std::string            default_config_file_json;
    nlohmann::json         default_config_json;
    QTableView*            table_view;
    SubStyledItemDelegate* delegate_;
    QStandardItemModel*    model_;
    QStatusBar*            statusBar;

    // std::queue<KeyEvent> queue_key_event;
    // std::mutex           mtx;

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
    void           setTableHead() const;

  public slots:
};
