#include "mainwidget.h"

#include <iostream>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>

MainWidget::MainWidget(QWidget* parent)
	: QWidget(parent)
{
	// ui.setupUi(this);
	setObjectName("mainWidget");
	// setWindowFlags(Qt::FramelessWindowHint);
	init_layout();



	setStyleSheet("MainWidget{background-color: #b7b7b7;} ");


	

}

MainWidget::~MainWidget()
{

}



// ���ò���
void MainWidget::init_layout() {
	// 1. ���������ֲ������ޱ߾�
	v_search_and_grid = new QVBoxLayout(this);
	v_search_and_grid->setContentsMargins(0, 0, 0, 0); // �����������ޱ߾�
	setLayout(v_search_and_grid);

	// 2. �����ϲ���ֱ���֣������ñ߾�ͼ��
	topLayout = new QVBoxLayout();
	topLayout->setContentsMargins(5, 5, 5, 0); // �����ϲ����������ܵľ���
	topLayout->setSpacing(5); // �����ϲ������ڲ�������Ϊ5

	search_line = new QLineEdit(this);
	search_line->setFixedSize(800, 60); // �����ϲ� QLineEdit �Ĺ̶��߶�Ϊ60
	search_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // ����Ϊ����չ����ȿ�������

	topLayout->addWidget(search_line);

	// 3. �����²�դ�񲼾֣������ñ߾�ͼ��
	icons_widget = new QWidget(this);
	icons_widget->setFixedSize(800, 400);

	iconsLayout = new QVBoxLayout(icons_widget);
	iconsLayout->setContentsMargins(10, 5, 5, 10); // �����ڱ߾�
	iconsLayout->setSpacing(0); // ���ü��Ϊ0

	// 4. �����ڲ� widget �����ô�С����ɫ
	icons_inner_widget = new Icons_inner_widget(icons_widget);
	// icons_inner_widget->setStyleSheet("background-color: #fff111;"); // ���ñ�����ɫ
	icons_inner_widget->setContentsMargins(5, 5, 5, 5);

	iconsLayout->addWidget(icons_inner_widget); // ��ӵ�������

	// 5. ����ϲ����²����ֵ�������
	v_search_and_grid->addLayout(topLayout); // ���ϲ�������ӵ�������
	v_search_and_grid->addWidget(icons_widget); // ���²�widget��ӵ�������

	// �Զ��������ڴ�С
	adjustSize(); // �������ڴ�С����Ӧ����
	setFixedSize(size()); // ����ǰ��С����Ϊ�̶���С
}

