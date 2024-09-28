#pragma once

#include <QToolButton>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QDir>
#include <QVector>
#include <QMouseEvent>

class IconButton : public QToolButton {
    Q_OBJECT

  public:
    IconButton(QWidget* parent, QVector<QPair<int, int>> & vec, QString filePath = "");
    ~IconButton();

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

  private:
    void        init_icon(const QString & filePath);
    static void openFile(const QString & filePath);
    void        updateIcon(const QString & filePath);
    int         calculateClosestIndex(const QPoint & pos);


    static QVector<QPair<int, int>> vec_coordinate;

    QString     filePath;
    QIcon       m_icon; // 文件图标
    static int  iconSize;
    static bool is_moving;    // 是否在移动
    QPoint      dragStartPos; //移动坐标
    QPoint      originalPos;  // 存储未被拖动前的位置

  signals:
    void buttonDragged(bool is_moving);
};
