#pragma once

#include <QWidget>
#include <QString>
#include <QThread>
#include <atomic>
#include "Everything.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QListWidget>


class Search_content : public QListWidget {
    Q_OBJECT

  public:
    Search_content(QWidget* parent = nullptr);
    ~Search_content() override;

  public slots:
    void slot_text_change(const QString & text);


  signals:
     void sig_show(bool is_show);
     // void itemClicked(QListWidgetItem* item);

  private:
    std::atomic<bool> isSearching;  // 跟踪搜索状态
    QThread*          searchThread; // 异步搜索线程
    QString           lastSearch;   // 上次搜索的文本

    void performSearch(const QString & text);
    void resetSearch(); // 重置状态
    void handleErrors(DWORD errorCode);

  //布局相关
  
    void init_layout();

};
