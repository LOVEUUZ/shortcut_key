#include "icons_inner_widget.h"

#include "mainwidget.h"

Icons_inner_widget::Icons_inner_widget(QWidget* parent): QWidget(parent), is_showDashedBorder(false) {
  // ui.setupUi(this);


  init_coordinateut();
  init_rendering();


  for (auto icon_button : vec_iconButton) {
    connect(icon_button, &IconButton::buttonDragged, this, &Icons_inner_widget::showDashedBorder);
  }
}

Icons_inner_widget::~Icons_inner_widget() {}


void Icons_inner_widget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event); // ���û���ķ���

    if (is_showDashedBorder) {
        QPainter painter(this);
        painter.setPen(QPen(Qt::gray, 1, Qt::DashLine)); // ����Ϊ��ɫ�����Ϊ1

        // ��icons_inner_widget�ڲ�����С���������
        for (int x = 0; x <= width(); x += icon_button_size) {
            painter.drawLine(x + 12, 0, x + 12, height()); // ������������12����
        }
        for (int y = 0; y <= height(); y += icon_button_size) {
            painter.drawLine(0, y, width(), y); // ��������
        }
    }
}





//��ʼ��ÿ��icon_button�����Ͻ�(x,y)����
void Icons_inner_widget::init_coordinateut() {
  //1. �� icons_inner_widget ���ڵ�����Ϊ��������
  for (int i = 0; i < y; ++i) {
    for (int k = 0; k < x; ++k) {
      int             x_tmp = icon_button_size * k + 12; ///12��Ϊ������������һЩ
      int             y_tmp = icon_button_size * i + 3;
      QPair<int, int> pair(x_tmp, y_tmp);
      vec_coordinate.append(pair);
    }
  }
}

void Icons_inner_widget::init_rendering() {

  // for (const auto& coordinate : vec_coordinate) {
  // 	IconButton* tmp_qb = new IconButton(this, vec_coordinate);
  //   vec_iconButton.push_back(std::move(tmp_qb));
  // 	tmp_qb->setGeometry(coordinate.first, coordinate.second, icon_button_size, icon_button_size);
  // }

  QVector<int> buttonIndices = {6, 12, 23}; // 0-based index for 7, 13, 24
  for (int index : buttonIndices) {
    if (index < vec_coordinate.size()) {
      const auto & coordinate = vec_coordinate[index];
      IconButton*  tmp_qb     = new IconButton(this, vec_coordinate, "D:/software/winRAR/WinRAR.exe");
      tmp_qb->setGeometry(coordinate.first, coordinate.second, icon_button_size, icon_button_size);
      vec_iconButton.push_back(tmp_qb); // ����ť��ӵ� vec_iconButton
    
      // ��¼��ťλ��
      button_map[tmp_qb] = index; // ʹ��һ��ӳ������¼��ť��������
    }
  }


}
