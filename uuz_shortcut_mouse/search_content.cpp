#include "Search_content.h"

Search_content::Search_content(QWidget* parent): QListWidget(parent), isSearching(false), searchThread(nullptr) {
  // setStyleSheet("background-color: #123456;");
    init_layout();
}

Search_content::~Search_content() {
  resetSearch();
}

void Search_content::slot_text_change(const QString & text) {
  if (isSearching) resetSearch(); // 停止前一次搜索

  lastSearch  = text; // 保存上次搜索文本
  isSearching = true;

  // 启动新线程进行搜索
  searchThread = QThread::create([this, text]() { performSearch(text); });
  searchThread->start();
}



void Search_content::performSearch(const QString & text) {
  if (text.isEmpty()) {

    return;
  }

  Everything_SetSearchW(text.toStdWString().c_str());
  Everything_SetMatchCase(FALSE); // 无视大小写
  Everything_QueryW(TRUE);        // 等待查询完成

  DWORD totalResults = Everything_GetNumResults();
  if (totalResults == 0) {
    handleErrors(Everything_GetLastError());
    isSearching = false;
    return;
  }

  for (DWORD i = 0; i < totalResults; ++i) {
    LPCWSTR fileName = Everything_GetResultFileNameW(i);
    LPCWSTR filePath = Everything_GetResultPathW(i);

    // 转换为 QString
    QString qFileName = QString::fromWCharArray(fileName);
    QString qFilePath = QString::fromWCharArray(filePath);

    // 分类处理文件和文件夹
    if (Everything_IsFolderResult(i)) {
      // 输出文件夹的绝对路径
      qDebug() << "Folder:" << qFilePath;
    }
    else {
      // 输出文件夹和文件的绝对路径
      QString folderPath = QString::fromWCharArray(Everything_GetResultPathW(i));
      qDebug() << "File:" << qFilePath + qFileName << ", Folder:" << folderPath;
    }
  }

  // emit sig_show(true);

  isSearching = false;
}


void Search_content::resetSearch() {
  Everything_Reset();   // 重置状态
  Everything_CleanUp(); // 清理并释放资源
  if (searchThread) {
    searchThread->quit(); // 停止搜索线程
    searchThread->wait(); // 等待线程结束
    delete searchThread;  // 删除线程对象
    searchThread = nullptr;
  }
}

void Search_content::handleErrors(DWORD errorCode) {
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
      qDebug() << "Unknown error:" << errorCode;
      break;
  }
}

void Search_content::init_layout() {
    
    
}
