#include "Search_content.h"

#include "mainwidget.h"
class MainWidget;

QStringList     Search_content::filter_path_list;
QStringList     Search_content::filter_suffix_list;
Search_content* Search_content::search_content_widget = nullptr;

Search_content::Search_content(QWidget* parent)
  : QTableWidget(parent), isSearching(false), searchThread(nullptr),
    max_results(3000), is_clear(false) {
  search_content_widget = this;

  // 初始化其他配置
  file_filter_path   = QCoreApplication::applicationDirPath() + "/config/config_filter_path.txt";
  file_filter_suffix = QCoreApplication::applicationDirPath() + "/config/config_filter_suffix.txt";
  init_filter_config();
  init_layout();


  connect(this, &Search_content::sig_addItem, this, &Search_content::slot_addItem);
  connect(this, &Search_content::itemDoubleClicked, this, &Search_content::slot_openFile);
  // connect(this, &Search_content::itemDoubleClicked, this, &Search_content::slot_openFile);
}

Search_content::~Search_content() {
  resetSearch();
}

/**调整列宽*/
void Search_content::resizeEvent(QResizeEvent* event) {
  int totalWidth = this->width();
  int col1Width  = totalWidth * 0.05; // 5% 宽度 ps: 有最小像素点限制，所以只能将就了
  int col2Width  = totalWidth * 0.20; // 20% 宽度
  int col3Width  = totalWidth * 0.75; // 75% 宽度
  this->setColumnWidth(0, 5);
  this->setColumnWidth(1, col2Width);
  this->setColumnWidth(2, col3Width);
  QTableWidget::resizeEvent(event);
}

/**槽，搜索栏文本内容改变的时候触发*/
void Search_content::slot_textChange(const QString & text) {
  if (!Everything_IsDBLoaded()) {
    qWarning() << "Everything数据库未加载";
    return;
  }

  if (isSearching) {
    qInfo() << "停止前一次搜索";
    resetSearch(); // 停止前一次搜索
  }

  if (text.length() == 1) {
    QChar ch = text.at(0);
    if (ch.unicode() >= 0 && ch.unicode() <= 127) return; //单个字符且是ASCII的则不搜索，要不然内容多的逆天
  }

  lastSearch  = text; // 保存上次搜索文本
  isSearching = true;

  // 启动新线程进行搜索
  searchThread = QThread::create([this, text]() { performSearch(text); });
  searchThread->start();
  qDebug() << "开始新线程";
}

/**槽，过滤配置条件修改*/
void Search_content::slot_configFilterModify() {
  qDebug() << filter_path_list;
  qDebug() << filter_suffix_list;


  file_config_filter_path->seek(0);
  file_config_filter_suffix->seek(0);

  file_config_filter_path->resize(0);
  file_config_filter_suffix->resize(0);

  QTextStream text_stream(file_config_filter_path);
  text_stream << filter_path_list.join("\n");

  QTextStream text_stream2(file_config_filter_suffix);
  text_stream2 << filter_suffix_list.join("\n");
}


/**
 * 子线程发送搜索的结果信号,在主线程中添加，避免子线程中添加导致主线程卡顿
 * 将每一次添加的个数设置为50个，避免在任务线程中执行clear的时候这里还在执行addItems，同时避免持有锁太久
 *
 * @param path_list 搜索到的文件的路径集合，发送的信号会做分割，大概50个一组，避免阻塞ui线程过久
 */
void Search_content::slot_addItem(const QStringList & path_list) {
  if (!path_list.isEmpty()) {
    // QMutexLocker locker(&mutex);
    if (!is_clear) {
      for (const QString & filePath : path_list) {
        QFileInfo fileInfo(filePath);
        int       row = this->rowCount();
        this->insertRow(row);
        this->setRowHeight(row, 10); // 设置行高为10像素

        // 第一列：文件夹或文件名
        if (fileInfo.isDir()) {
          this->setItem(row, 0, new QTableWidgetItem("  d"));
        }
        else {
          this->setItem(row, 0, new QTableWidgetItem("  -"));
        }

        // 第二列：文件名称
        this->setItem(row, 1, new QTableWidgetItem(fileInfo.fileName()));

        // 第三列：全路径
        this->setItem(row, 2, new QTableWidgetItem(fileInfo.absoluteFilePath()));
      }
    }
  }
}


/**
 * 双击打开对应文件
 * @param item QTableWidget选中的一行
 */
void Search_content::slot_openFile(const QTableWidgetItem* item) {
  int     row          = item->row();
  QString filePath_tmp = QDir::toNativeSeparators(this->item(row, 2)->text()); // 转为本地格式
  QUrl    fileUrl      = QUrl::fromLocalFile(filePath_tmp);                    // 转为url
  QDesktopServices::openUrl(fileUrl);                                          // 使用该函数可以打开exe，也能打开jpg，txt等文件

  qInfo() << "Starting process:" << filePath_tmp;

  //隐藏窗口
  auto main_widget = qobject_cast<MainWidget*>(this->parentWidget()->parentWidget());
  //第一个是stacked_widget，第二个才是MainWidget
  main_widget->hide();
}

/**
  * 通过键盘快捷键打开文件
  *	通过上下键切换搜索内容栏与搜索框的焦点
 */
void Search_content::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Up && currentRow() == 0) {
    clearFocus();
    auto mainWidget = qobject_cast<MainWidget*>(this->parentWidget()->parentWidget());
    //第一个是stacked_widget，第二个才是MainWidget
    if (mainWidget != nullptr) {
      mainWidget->sig_moveFocus(mainWidget->search_line);
    }
  }

  //回车，小键盘的回车，空格
  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
    emit itemDoubleClicked(currentItem());
  }

  QTableWidget::keyPressEvent(event);
}

/** 右键菜单*/
void Search_content::contextMenuEvent(QContextMenuEvent* event) {
  QTableWidgetItem* item = itemAt(event->pos());
  if (item) {
    QMenu    menu(this);
    QAction* act_open_file   = menu.addAction("打开文件");
    QAction* act_open_folder = menu.addAction("打开路径");
    QAction* act_copy        = menu.addAction("复制绝对路径");
    QAction* act_filter_disk = menu.addAction("过滤只显示当前盘符");

    //打开文件
    connect(act_open_file, &QAction::triggered, [this, item]() {
      emit itemDoubleClicked(item);
    });

    //打开文件夹
    connect(act_open_folder, &QAction::triggered, [this, item]() {
      int     row  = item->row();
      QString path = this->item(row, 2)->text(); // 获取路径

      QFileInfo fileInfo(path);

      if (fileInfo.exists()) {
        if (fileInfo.isDir()) {
          // 如果是文件夹，直接打开
          QUrl directoryUrl = QUrl::fromLocalFile(path);
          QDesktopServices::openUrl(directoryUrl);
          qInfo() << "打开文件夹: " << path;
        }
        else {
          // 如果是文件，打开其所在文件夹
          QString directoryPath = fileInfo.absolutePath();
          QUrl    folderUrl     = QUrl::fromLocalFile(directoryPath);
          QDesktopServices::openUrl(folderUrl);
          qInfo() << "打开文件所属文件夹: " << directoryPath;
        }
      }
      else {
        qWarning() << "路径不存在:" << path;
      }
    });

    //复制路径
    connect(act_copy, &QAction::triggered, [this, item]() {
      int     row  = item->row();
      QString path = this->item(row, 2)->text(); // 获取路径

      QClipboard* clipboard = QApplication::clipboard(); //获取剪贴板对象
      clipboard->setText(path);
      qInfo() << "复制路径: " << path;
    });

    //移除非当前盘符
    connect(act_filter_disk, &QAction::triggered, [this, item]() {
      int     row        = item->row();
      QString path       = this->item(row, 2)->text(); // 获取路径
      QChar   first_char = path[0];

      //从后向前遍历(避免索引改变导致漏行)，当前所有行元素的第三列全路径，判断盘符开头，不是则移除
      for (int                row1  = rowCount() - 1; row1 >= 0; --row1) {
        if (QTableWidgetItem* item_ = this->item(row1, 2)) {
          QString text = item_->text();
          if (text[0] != first_char) {
            removeRow(row1);
          }
        }
      }
    });

    menu.exec(event->globalPos());
    menu.deleteLater(); //主动销毁
  }


  QTableWidget::contextMenuEvent(event);
}


/** 线程任务*/
void Search_content::performSearch(const QString & text) {
  {
    // QMutexLocker locker(&mutex);
    is_clear = true;
    this->setRowCount(0);
    // clear();
  }

  if (text.isEmpty()) {
    emit sig_addItem(QStringList()); // 发送空结果
    isSearching = false;
    return;
  }

  // 构建多个过滤规则
  // Everything_SetMax(max_results);           //设置最大搜索结果数量
  Everything_SetSort(EVERYTHING_SORT_DATE_MODIFIED_DESCENDING); //排序方式-修改日期降序
  Everything_SetMatchCase(FALSE);                               // 无视大小写

  qInfo() << "搜索内容 ==> " << text;
  Everything_SetSearchW(text.toStdWString().c_str());
  Everything_QueryW(TRUE); // 等待查询完成

  total_results = Everything_GetNumResults(); //所有结果数量

  if (total_results == 0) {
    handleErrors(Everything_GetLastError(), text);
    emit sig_addItem(QStringList()); // 发送空结果
    isSearching = false;
    return;
  }

  QStringList string_list; // 收集结果

  //遍历结果
  for (DWORD i = 0; i < total_results; ++i) {
    LPCWSTR fileName = Everything_GetResultFileNameW(i);
    LPCWSTR filePath = Everything_GetResultPathW(i);

    // 修改，不分类处理文件和文件夹,没必要
    QString fullPath = QString::fromWCharArray(filePath) + "\\" + QString::fromWCharArray(fileName);

    // 过滤路径
    bool isFiltered = false;
    for (const auto & path : filter_path_list) {
      if (fullPath.contains(path)) {
        isFiltered = true;
        break;
      }
    }

    // 继续检查文件后缀（如果没有被过滤）
    if (!isFiltered) {
      for (const auto & suffix : filter_suffix_list) {
        if (fullPath.endsWith(suffix, Qt::CaseInsensitive)) {
          isFiltered = true;
          break;
        }
      }
    }

    // 只有在未被过滤的情况下才添加到结果列表
    if (!isFiltered) {
      string_list.push_back(fullPath);
      if (string_list.size() >= max_results) break; // 控制最大结果数量
    }
  }

  is_clear = false;
  emit sig_addItem(string_list); // 一次性发送所有结果
  isSearching = false;
  Everything_Reset();   // 重置状态
  Everything_CleanUp(); // 清理并释放资源
}

/** 清理线程任务，如果前一次的还在运行，则直接中断并释放everything的资源*/
void Search_content::resetSearch() {
  qInfo() << "清理线程开始";

  if (searchThread) {
    qInfo() << "终止搜索线程";
    searchThread->quit(); // 停止搜索线程
    searchThread->wait(); // 等待线程结束
    delete searchThread;  // 删除线程对象
    searchThread = nullptr;
  }

  isSearching = false; // 更新状态

  Everything_Reset();   // 重置状态
  Everything_CleanUp(); // 清理并释放资源

  qInfo() << "清理线程结束";
}

/** everything的异常处理*/
void Search_content::handleErrors(DWORD errorCode, const QString & text) {
  switch (errorCode) {
    case EVERYTHING_ERROR_IPC:
      qCritical() << "Everything client is not running.";
      break;
    case EVERYTHING_ERROR_MEMORY:
      qFatal("everything Out of memory.");
      break;
    case EVERYTHING_ERROR_INVALIDCALL:
      qWarning() << "Invalid call.";
      break;
    default:
      qInfo() << "No search results found:" << text;
      break;
  }
}

/** 初始化QTableWidget*/
void Search_content::init_layout() {
  this->setColumnCount(3); // 设置列数为3
  // this->setHorizontalHeaderLabels({ "类型", "文件名称", "全路径" }); // 设置列标题
  this->horizontalHeader()->setVisible(false);           // 隐藏水平标题
  this->verticalHeader()->setVisible(false);             // 隐藏垂直标题
  this->setShowGrid(false);                              // 隐藏网格线
  this->setStyleSheet("QTableWidget { border: none; }"); // 设置无边框
  this->setSelectionBehavior(SelectRows);                // 选择整行
  this->setSortingEnabled(true);                         // 启用排序功能
  this->horizontalHeader()->setSortIndicatorShown(true); // 显示排序指示器
  this->setRowHeight(0, 10);                             // 默认行高为10像素
  this->setEditTriggers(NoEditTriggers);                 //禁止编辑
  QFont font = this->font();                             // 获取当前字体
  font.setPointSize(8);                                  // 设置字体大小
  this->setFont(font);                                   // 应用新的字体
}

/**过滤配置文件的读取*/
void Search_content::init_filter_config() {
  // 获取目录路径
  QDir dir = QFileInfo(file_filter_path).absoluteDir();
  // 检查目录是否存在，如果不存在则创建它及其父文件夹
  if (!dir.mkpath(".")) {
    qWarning() << "Cannot create directory:" << dir.path();
    return;
  }

  // 初始化过滤路径文件
  file_config_filter_path = new QFile(file_filter_path);
  // 可读可写模式打开文件
  if (file_config_filter_path->open(QIODevice::ReadWrite | QIODevice::Text)) {
    // 如果文件无内容，写入基础内容
    if (file_config_filter_path->readAll().isEmpty()) {
      qInfo() << "File created:" << file_filter_path;

      QString userHomePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
      qInfo() << "当前用户路径：" << userHomePath;

      // 构建默认的排除条件
      filter_path_list << userHomePath + "/AppData"
        << userHomePath + "/."
        << "C:/Program Files (x86)"
        << "C:/Program Files"
        << "C:/Windows"
        << "C:/ProgramData"
        << "C:/Recovery";

      // 确保所有路径都是正斜杠
      for (QString & path : filter_path_list) {
        path.replace("/", "\\"); // 将所有斜杠统一为 \ 因为everything那边返回的是 \ 样式
      }

      QTextStream out(file_config_filter_path);
      out << filter_path_list.join("\n"); // 每个元素换行写入
    }
    else {
      // 文件有内容，读取到 filter_path_list
      file_config_filter_path->seek(0);
      QTextStream in(file_config_filter_path);
      while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); // 逐行读取并去掉首尾空格
        if (!line.isEmpty()) {
          filter_path_list << line; // 添加到 QStringList
        }
      }
    }
  }

  // 初始化过滤后缀文件
  file_config_filter_suffix = new QFile(file_filter_suffix);
  // 可读可写模式打开文件
  if (file_config_filter_suffix->open(QIODevice::ReadWrite | QIODevice::Text)) {
    // 如果文件无内容，写入基础内容
    if (file_config_filter_suffix->readAll().isEmpty()) {
      qInfo() << "File created:" << file_filter_suffix;
      filter_suffix_list << ".lnk"
        << ".dll"
        << ".lib";
      QTextStream out(file_config_filter_suffix);
      out << filter_suffix_list.join("\n"); // 每个元素换行写入
    }
    else {
      // 文件有内容，读取到 filter_suffix_list
      file_config_filter_suffix->seek(0);
      QTextStream in(file_config_filter_suffix);
      while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); // 逐行读取并去掉首尾空格
        if (!line.isEmpty()) {
          filter_suffix_list << line; // 添加到 QStringList
        }
      }
    }
  }
}
