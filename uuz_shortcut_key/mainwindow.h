#pragma once

#include <mutex>
#include <queue>
#include <fstream>
#include <filesystem>
#include <iostream>

#include <QSystemTrayIcon>  //t托盘类
#include <QDesktopServices> //桌面事件类
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QKeyEvent>
#include <QTableView>
#include <QStandardItemModel>
#include <QLibraryInfo>
#include <QTranslator>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardPaths>
#include <QProcess>


#include <utility>
#include <Windows.h>
#include "WindowsHookKeyEx.h"

#include "substyleditemdelegate.h"
#include "json.hpp"
#include "ui_mainwindow.h"
#include "keyEvent.h"
#include "my_lineedit_exe_path.h"

#include "Trigger.hpp"
#include "tools.h"

//main中定义的全局变量
extern HANDLE         hMutex;
extern nlohmann::json glob_json_;

//委托中自定义角色，第一个是快捷键一列的按下按键的名称，第二个是按下按键对应的key值
constexpr int ROLE_KEY         = Qt::UserRole + 1;
constexpr int ROLE_VEC_KEY_NUM = Qt::UserRole + 2;

//主页面
class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    std::string currentFilePath;

  protected:
    // 拦截关闭事件，最小化到托盘而不是退出
    void closeEvent(QCloseEvent* event) override;

  private:
    Ui::MainWindowClass ui;

    std::string               default_config_file_json;
    nlohmann::json            default_config_json;
    QTableView*               table_view;
    sub_styled_item_delegate* delegate_;
    QStandardItemModel*       model_;
    QStatusBar*               statusBar;

    QSystemTrayIcon* trayIcon;
    QMenu*           trayIconMenu;

    const std::string str_default_txt_path         = "/config/default.txt";
    const std::string str_default_config_json_path = "/config/default_config.json";

    //菜单栏
    QAction* openFileAction; //打开配置文件，暂时只有这一个写在类中，因为要在导入配置的地方模拟触发

    void        initTranslate(); //加载翻译文件
    QTranslator translator;
    QTranslator qtTranslator;

    void initDefaultConfigFile(); //首次运行时候创建默认配置
    void initTray();              //桌面右下角的托盘
    void initMenu();
    void initTableView();
    void initView();
    void initContext() const;
    void initModel();

    nlohmann::json analyzeJson() const;                       //解析json
    void           savaConfigJson() const;                    //存储修改的配置文件到磁盘
    void           updateModel(const nlohmann::json &) const; //更新模型中的数据
    void           addViewNewRow() const;                     //新增一行配置文件
    void           updateDefaultConfigFile();                 //更新默认配置文件(txt)
    void           clearModel() const;                        //清理模型中的数据
    void           setTableHead() const;                      //设置视图的列名称

  public slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason); //托盘事件
    void onShowMainWindow();                                            //托盘事件，显示主页面
    void onExitApplication() const;                                     //托盘事件，退出程序

    //开机启动相关
    void setStartup();
    void unSetStartup();

    //翻译相关
    void switchLanguage(const QString & lang = "zh_CN");
};
