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
    QIcon       m_icon; // �ļ�ͼ��
    static int  iconSize;
    static bool is_moving;    // �Ƿ����ƶ�
    QPoint      dragStartPos; //�ƶ�����
    QPoint      originalPos;  // �洢δ���϶�ǰ��λ��

  signals:
    void buttonDragged(bool is_moving);
};
