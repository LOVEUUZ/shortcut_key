#pragma once

#include <QPushButton>
#include <QWidget>
#include <QPainter>
#include <QMenu>
#include <QFileDialog>
#include <QFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QProcess>
#include <QMimeData>

#include "iconButton.h"
#include "ui_icons_inner_widget.h"
#include "config.h"
#include "config_window.h"
#include "WindowsHookKeyEx.h"
#include "WindowsHookMouseEx.h"

#include "json.hpp"

// class WindowsHookKeyEx;
// class WindowsHookMouseEx;

class Icons_inner_widget : public QWidget {
    Q_OBJECT

  public:
    Icons_inner_widget(QWidget* parent = nullptr);
    ~Icons_inner_widget() override;

  private:
    // Ui::Icons_inner_widgetClass ui;
    void paintEvent(QPaintEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override; //右键菜单
    void dragEnterEvent(QDragEnterEvent* event) override;     //处理拖动进入窗口的事件
    void dragLeaveEvent(QDragLeaveEvent* event) override;     //处理拖动移出的事件
    void dragMoveEvent(QDragMoveEvent* event) override;       //处理拖动移动窗口的事件
    void dropEvent(QDropEvent* event) override;               //处理拖动释放到窗口的事件

    void handleDroppedItem(const QString & fileName, int index); //构建入的文件或文件夹的配置，并处理


    static constexpr int x                = 8;         //一排8个
    static constexpr int y                = 4;         //总共4行
    static constexpr int SIZE             = x * y - 1; //总数4*8=32，然后-1，是因为索引计算从0开始，总共32个
    static constexpr int icon_button_size = 95;        //每个图标大小是95x95

    static constexpr int SUM = x * y - 1;

    void init_coordinateut(); //初始化坐标
    void init_rendering();    //创建并渲染按钮
    void init_button_connect(const icon_button* icon_button);

    QMap<int, icon_button*> map_index_button; //存储按钮与索引的映射
    QVector<icon_button*>   vec_iconButton;   //每个按钮的指针


    QPushButton*             q_push_button_arr[x][y];
    QVector<QPair<int, int>> vec_coordinate; //icon_button的左上角坐标
    QRect                    first_icon_coordinate;

    bool is_showDashedBorder; // 用于控制虚线显示

    //配置文件相关
    QString         file_path;
    QFile*          file_config;
    QString         qstr_config_content;
    std::string     str_config_content;
    QVector<Config> vec_config;
    QString         first_create_config();
    void            init_config();

    //配置界面
    // QMainWindow* config_window;

    enum Config_operate { ADD, DEL, MOVE, MODIFY };

    bool modify_config(Config_operate opt, const Config & config);
    bool modify_config(Config_operate opt);
    int  findEmptyPosition(); //寻找新增可用位置

  public:
    auto getVecCoordinate() {
      return &vec_coordinate;
    }

    auto getMapIndexButton() {
      return &map_index_button;
    }

  public slots:
    void slot_showDashedBorder(bool is_moving);               //拖拽的时候显示虚线和隐藏
    void slot_moveModifyConfig(int new_index, int old_index); //拖动后修改配置
    void slot_modifyConfig(const Config & config);

    //右键菜单项
    void slot_openIconPath(int id);   //右键添加icon
    void slot_addIcon();              //右键添加icon
    void slot_addFolder();            //右键添加文件夹
    void slot_changeShowName(int id); //修改显示名称
    void slot_deleteIcon(int id);     //右键删除icon
    void slot_unInstallHook();        //锁定界面（暂时卸载或重新挂载钩子）
    void slot_configWidgetOpen();     //右键打开配置

    // void slot_config_widget_close();            //关闭配置页面

    // signals:
    // void sig_config_widget_close();            //关闭配置页面
};
