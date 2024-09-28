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

    void                   init_coordinateut(); //��ʼ������
    void                   init_rendering();    //��������Լ���Ⱦ��Χ
    QMap<IconButton*, int> button_map;          // �洢��ť��������ӳ��
    QVector<IconButton*>   vec_iconButton;


    QPushButton* q_push_button_arr[x][y];
    QVector<QPair<int, int>> vec_coordinate; //icon_button�����Ͻ�����
    QRect                    first_icon_coordinate;

    bool is_showDashedBorder; // ���ڿ���������ʾ
    QWidget* overlayWidget; // ͸�����ǲ�


public slots:
    void showDashedBorder(bool is_moving) {
        qDebug() << ".........";
        if (is_moving) {
            is_showDashedBorder = true; // ���ñ�־λ
            update();                   // ���½���
        }
        else {
            is_showDashedBorder = false; // ���ñ�־λ
            update();                    // ���½���
        }
    }
};
