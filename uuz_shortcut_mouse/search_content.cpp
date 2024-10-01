#include "Search_content.h"

Search_content::Search_content(QWidget* parent) : QListWidget(parent), isSearching(false), searchThread(nullptr),
                                                  max_results(1500), is_clear(false) {
  // setStyleSheet("background-color: #123456;");

  file_filter_path   = QCoreApplication::applicationDirPath() + "/config/config_filter_path.txt";
  file_filter_suffix = QCoreApplication::applicationDirPath() + "/config/config_filter_suffix.txt";
  init_filter_config();
  init_layout();

  connect(this, &Search_content::sig_addItem, this, &Search_content::slot_addItem);
  connect(this, &Search_content::itemClicked, this, &Search_content::slot_open_file);
  connect(this, &Search_content::itemEntered, this, &Search_content::slot_open_file);
  // connect(this, &Search_content::itemDoubleClicked, this, &Search_content::slot_open_file);
}

Search_content::~Search_content() {
  resetSearch();
}

void Search_content::slot_text_change(const QString & text) {
  if (!Everything_IsDBLoaded()) {
    qDebug() << "Everything数据库未加载";
    return;
  }


  if (isSearching) {
    qDebug() << "停止前一次搜索";
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

// 子线程发送搜索的结果信号,在主线程中添加，避免子线程中添加导致主线程卡顿
//将每一次添加的个数设置为50个，避免在任务线程中执行clear的时候这里还在执行addItems，同时避免持有锁太久
void Search_content::slot_addItem(const QStringList & path_list) {
  qDebug() << "调用addItem";
  if (!path_list.isEmpty()) {
    static constexpr int chunkSize = 50; // 每组的大小
    for (int i = 0; i < path_list.size(); i += chunkSize) {
      // 计算当前组的结束索引
      int end = qMin(i + chunkSize, path_list.size());
      // 创建子列表
      QStringList subList = path_list.mid(i, end - i);
      // 上锁，添加子列表
      {
        QMutexLocker locker(&mutex);
        if (!is_clear) addItems(subList);
        else return;
      }
    }
  }
}

//单击打开对应文件
void Search_content::slot_open_file(const QListWidgetItem* item) {
  QString filePath_tmp = QDir::toNativeSeparators(item->text()); // 转为本地格式
  QUrl    fileUrl      = QUrl::fromLocalFile(filePath_tmp);      // 转为url
  QDesktopServices::openUrl(fileUrl);                            // 使用该函数可以打开exe，也能打开jpg，txt等文件

  qInfo() << "Starting process:" << filePath_tmp;
}

//右键点击事件
void Search_content::contextMenuEvent(QContextMenuEvent* event) {
  QListWidgetItem* item = itemAt(event->pos());
  // if (item) {
  //   QMenu    menu(this);
  //   QAction* action = menu.addAction("打开文件");
  //   QAction* action = menu.addAction("打开路径");
  //   QAction* action = menu.addAction("复制绝对路径和文件名");
  //   connect(action, &QAction::triggered, this, [this, item]() {
  //     // 在这里处理自定义操作
  //   });
  //   menu.exec(event->globalPos());
  //   menu.deleteLater(); //主动销毁
  // }


  QListWidget::contextMenuEvent(event);
}


// 线程任务
void Search_content::performSearch(const QString & text) {
  {
    QMutexLocker locker(&mutex);
    is_clear = true;
    clear();
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

  qDebug() << "搜索内容 ==> " << text;
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

    // 分类处理文件和文件夹
    QString fullPath;
    if (Everything_IsFolderResult(i)) {
      // 文件夹的绝对路径
      fullPath = QString::fromWCharArray(filePath);
    }
    else {
      // 获取文件的绝对路径
      fullPath = QString::fromWCharArray(filePath) + "\\" + QString::fromWCharArray(fileName);
    }

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


void Search_content::resetSearch() {
  qDebug() << "清理线程开始";

  if (searchThread) {
    qDebug() << "终止搜索线程";
    searchThread->quit(); // 停止搜索线程
    searchThread->wait(); // 等待线程结束
    delete searchThread;  // 删除线程对象
    searchThread = nullptr;
  }

  isSearching = false; // 更新状态

  Everything_Reset();   // 重置状态
  Everything_CleanUp(); // 清理并释放资源

  qDebug() << "清理线程结束";
}


void Search_content::handleErrors(DWORD errorCode, const QString & text) {
  switch (errorCode) {
    case EVERYTHING_ERROR_IPC:
      qDebug() << "Everything client is not running.";
      break;
    case EVERYTHING_ERROR_MEMORY:
      qDebug() << "Out of memory.";
      break;
    case EVERYTHING_ERROR_INVALIDCALL:
      qDebug() << "Invalid call.";
      break;
    default:
      qInfo() << "No search results found:" << text;
      break;
  }
}

void Search_content::init_layout() {}

//过滤配置文件的读取
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
      qDebug() << "当前用户路径：" << userHomePath;

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
