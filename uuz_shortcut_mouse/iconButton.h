#pragma once

#include <QToolButton>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QDir>
#include <QLineEdit>
#include <QMouseEvent>

#include "config.h"

#include "json.hpp"

class Icons_inner_widget; // 前向声明


class icon_button : public QToolButton {
    Q_OBJECT

  public:
    icon_button(QWidget* parent, Config config);
    ~icon_button() override;

    QString getShowName() { return showName; }

    bool setShowName(const QString & newName) {
      showName = newName;
      return true;
    }

    int ID; //其实索引，按照格子位置来排的，0-31

    QString getFilePath() { return filePath; }

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

  private:
    void init_icon(const QString & filePath);
    void openFile(const QString & filePath);
    void updateIcon(const QString & filePath);
    int  calculateClosestIndex(const QPoint & pos);

    //父窗口中的内容
    Icons_inner_widget*       parent_widget_content; // 上级窗口
    QVector<QPair<int, int>>* vec_coordinate;
    QMap<int, icon_button*>*  map_index_button; // 存储按钮与索引的映射


    QString     filePath;
    QString     showName;
    QIcon       m_icon; // 文件图标
    static int  iconSize;
    static bool is_moving;      // 是否在移动
    QPoint      drag_start_pos; //移动坐标
    QPoint      original_pos;   // 存储未被拖动前的位置

    //配置相关
    Config config;

    QLineEdit* change_show_name_edit;

  signals:
    void sig_buttonDragged(bool is_moving);
    void sig_moveModifyConfig(int new_index, int old_index);
    void sig_modifyConfig(Config & config); //修改配置文件，和上面的移动的修改配置文件其实重复了

    void sig_changeShowName();

  public slots:
    void slot_changeShowName();
};
