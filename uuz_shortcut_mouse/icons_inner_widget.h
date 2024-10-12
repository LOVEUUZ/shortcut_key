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


    static constexpr int x                = 8;
    static constexpr int y                = 4;
    static constexpr int SIZE             = x * y - 1;
    static constexpr int icon_button_size = 95;
    static constexpr int SUM              = x * y - 1;

    void init_coordinateut(); //初始化坐标
    void init_rendering();    //创建并渲染按钮
    void init_button_connect(const IconButton* icon_button);

    QMap<int, IconButton*> map_index_button; //存储按钮与索引的映射
    QVector<IconButton*>   vec_iconButton;   //每个按钮的指针


    QPushButton*             q_push_button_arr[x][y];
    QVector<QPair<int, int>> vec_coordinate; //icon_button的左上角坐标
    QRect                    first_icon_coordinate;

    bool is_showDashedBorder; // 用于控制虚线显示

    //配置文件相关
    QString     file_path;
    QFile*      file_config;
    QString     qstr_config_content;
    std::string str_config_content;
    // nlohmann::json  config_jsonArray;
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
    auto get_vec_coordinate() {
      return &vec_coordinate;
    }

    auto get_map_index_button() {
      return &map_index_button;
    }

  public slots:
    void slot_showDashedBorder(bool is_moving);                 //拖拽的时候显示虚线和隐藏
    void slot_move_modify_config(int new_index, int old_index); //拖动后修改配置
    void slot_modify_config(const Config & config);

    //右键菜单项
    void slot_open_icon_path(int id);               //右键添加icon
    void slot_add_icon();               //右键添加icon
    void slot_add_folder();             //右键添加文件夹
    void slot_change_show_name(int id); //修改显示名称
    void slot_delete_icon(int id);      //右键删除icon
    void slot_config_widget_open();     //右键打开配置
    // void slot_config_widget_close();            //关闭配置页面

    // signals:
    // void sig_config_widget_close();            //关闭配置页面
};
