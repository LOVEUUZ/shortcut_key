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
    QWidget::paintEvent(event); // 调用基类的方法

    if (is_showDashedBorder) {
        QPainter painter(this);
        painter.setPen(QPen(Qt::gray, 1, Qt::DashLine)); // 设置为灰色，宽度为1

        // 在icons_inner_widget内部画出小方块的虚线
        for (int x = 0; x <= width(); x += icon_button_size) {
            painter.drawLine(x + 12, 0, x + 12, height()); // 横排虚线右移12像素
        }
        for (int y = 0; y <= height(); y += icon_button_size) {
            painter.drawLine(0, y, width(), y); // 纵排虚线
        }
    }
}





//初始化每个icon_button的左上角(x,y)坐标
void Icons_inner_widget::init_coordinateut() {
  //1. 以 icons_inner_widget 窗口的坐标为基本计算
  for (int i = 0; i < y; ++i) {
    for (int k = 0; k < x; ++k) {
      int             x_tmp = icon_button_size * k + 12; ///12是为了坐标计算居中一些
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
      vec_iconButton.push_back(tmp_qb); // 将按钮添加到 vec_iconButton
    
      // 记录按钮位置
      button_map[tmp_qb] = index; // 使用一个映射来记录按钮和其索引
    }
  }


}
