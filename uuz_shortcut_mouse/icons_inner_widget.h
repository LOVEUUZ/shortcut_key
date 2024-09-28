#pragma once

#include <QPushButton>
#include <QWidget>
#include <QPainter>

#include "iconButton.h"
#include "ui_icons_inner_widget.h"

class Icons_inner_widget : public QWidget
{
	Q_OBJECT

public:
	Icons_inner_widget(QWidget *parent = nullptr);
	~Icons_inner_widget();

private:
	// Ui::Icons_inner_widgetClass ui;
    void paintEvent(QPaintEvent* event) override;


    static constexpr int x = 8;
    static constexpr int y = 4;
    static constexpr int icon_button_size = 95;
    static constexpr int sum = x * y;

    void                   init_coordinateut(); //初始化坐标
    void                   init_rendering();    //检测坐标以及渲染范围
    QMap<IconButton*, int> button_map;          // 存储按钮与索引的映射
    QVector<IconButton*>   vec_iconButton;


    QPushButton* q_push_button_arr[x][y];
    QVector<QPair<int, int>> vec_coordinate; //icon_button的左上角坐标
    QRect                    first_icon_coordinate;

    bool is_showDashedBorder; // 用于控制虚线显示
    QWidget* overlayWidget; // 透明覆盖层


public slots:
    void showDashedBorder(bool is_moving) {
        qDebug() << ".........";
        if (is_moving) {
            is_showDashedBorder = true; // 设置标志位
            update();                   // 更新界面
        }
        else {
            is_showDashedBorder = false; // 设置标志位
            update();                    // 更新界面
        }
    }
};
