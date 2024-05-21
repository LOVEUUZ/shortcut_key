#include "mainwindow.h"
#include "my_lineedit_exe_path.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <QKeyEvent>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <utility>
#include <Windows.h>

#include "json.hpp"
#include "substyleditemdelegate.h"
#include "WindowsHookEx.h"

using json = nlohmann::json;

extern HANDLE hMutex;

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
  ui.setupUi(this);
  // My_lineEdit_exe_path* m = new My_lineEdit_exe_path(this);
  // MyLineEdit* m = new MyLineEdit(this);

  // ptr_windows_hook = WindowsHookEx::getWindowHook();
  // ptr_windows_hook->installHook();
  // setFunc();


  setWindowTitle(tr("快捷启动"));

  initDefaultConfigFile();

  initTableView();
  initModel();
  initView();
  initConnect();
  initMenu();
  initContext();
}

MainWindow::~MainWindow() {
  // 在程序退出时释放互斥体
  if (hMutex) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
  }
  qDebug() << "主窗口退出";
}

//获取按下的键盘事件
// void MainWindow::set_key_event(const KeyEvent & key_event) {
//   std::unique_lock<std::mutex> lock(mtx);
//   if (queue_key_event.size() > 20) {
//     queue_key_event.pop();
//   }
//   std::cout << key_event.key_name << " " << key_event.key << " " << key_event.isPressed << "\n";
//   queue_key_event.push(key_event);
// }

// KeyEvent MainWindow::getKeyEvent() {
//   KeyEvent key_event_;
//
//   {
//     std::unique_lock<std::mutex> lock(mtx);
//     if (queue_key_event.empty()) {
//       return KeyEvent();
//     }
//     key_event_ = std::move(queue_key_event.front());
//     queue_key_event.pop();
//   }
//
//   return key_event_;
// }

// void MainWindow::setFunc() {
//   auto func = [&](const KeyEvent & key_event) { set_key_event(key_event); };
//   queue_clear();
//   ptr_windows_hook->setFunc(func);
//   //
//   // // 使用 lambda 表达式绑定成员函数
//   // // bool b = ptr_windows_hook->setFunc([&](KeyEvent k) {
//   // //   std::cout << k.key_name << " " << k.key << " " << k.isPressed << "\n";
//   // // });
// }


void MainWindow::initDefaultConfigFile() {
  default_config_file_json = QCoreApplication::applicationDirPath().toStdString() + str_default_txt_path;
  currentFilePath          = QCoreApplication::applicationDirPath().toStdString() + str_default_config_json_path;

  uintmax_t file_size;
  if (!std::filesystem::exists(default_config_file_json)) {
    //创建的，写入默认指定配置文件
    std::fstream ofs_default_config_file(default_config_file_json, std::ios::out);
    default_config_json["default_config_file_path"] = currentFilePath;
    ofs_default_config_file << std::setw(4) << default_config_json << "\n";
    ofs_default_config_file.close();

    std::ofstream ofs(currentFilePath, std::ios::trunc);
    //构建json
    json jsonObject;
    jsonObject[0]["name"]         = "1";
    jsonObject[0]["L-Ctrl"]       = false;
    jsonObject[0]["L-Alt"]        = false;
    jsonObject[0]["R-Alt"]        = false;
    jsonObject[0]["R-Ctrl"]       = false;
    jsonObject[0]["shortcut_key"] = "";
    jsonObject[0]["path"]         = "double click this";
    jsonObject[0]["desc"]         = "desc";
    jsonObject[0]["trigger"]      = 1;
    jsonObject[0]["enable"]       = false;
    if (ofs) {
      ofs << std::setw(4) << jsonObject << "\n";
      ofs.close();
    }
  }
  file_size = std::filesystem::file_size(default_config_file_json);

  std::fstream ifs_default_config_file(default_config_file_json, std::ios::in);
  if (!ifs_default_config_file) {
    QMessageBox::critical(nullptr, tr("Fail"), tr("open default error"));
    std::exit(0);
  }

  std::string str_default_json;
  str_default_json.resize(file_size);
  ifs_default_config_file.read(&str_default_json.at(0), file_size);
  ifs_default_config_file.close();

  try {
    default_config_json = json::parse(str_default_json);
  }
  catch (const nlohmann::json::parse_error & e) {
    //解析默认配置出现异常，删除重新新建一个
    QMessageBox::critical(nullptr, tr("Fail"), tr("Set default config Fail,because: ") + e.what());
    std::filesystem::remove(default_config_file_json);
    std::ofstream ofs_default_config_file(default_config_file_json, std::ios::out);
    default_config_json["default_config_file_path"] = currentFilePath;
    ofs_default_config_file << std::setw(4) << default_config_json << "\n";
    ofs_default_config_file.close();
  }
  //解析完成，获取默认启动配置文件
  std::string tmp_file_path = default_config_json["default_config_file_path"].get<std::string>();
  if (tmp_file_path.empty()) {
    return;
  }
  else {
    currentFilePath = tmp_file_path;
  }
}

void MainWindow::initMenu() {
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

  // 在文件菜单中添加动作
  QAction* openFileAction = new QAction(tr("&Open File"), this);
  fileMenu->addAction(openFileAction);

  QAction* createAction = new QAction(tr("&New File"), this);
  fileMenu->addAction(createAction);

  QAction* openAction = new QAction(tr("&Import File"), this);
  fileMenu->addAction(openAction);

  fileMenu->addSeparator(); // 添加分隔符

  QAction* exitAction = new QAction(tr("&Quit"), this);
  fileMenu->addAction(exitAction);

  //底部状态栏，用来显示提示信息
  statusBar = new QStatusBar(this);
  setStatusBar(statusBar);

  //打开配置文件
  connect(openFileAction, &QAction::triggered, [&]() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select the configuration file",
                                                    QApplication::applicationDirPath() + "/config/",
                                                    "Text files (*.json)");
    if (filePath.isEmpty()) return;
    qDebug() << filePath;

    // 获取最后一个斜杠的位置
    int lastSlashIndex     = filePath.lastIndexOf('/');
    int lastBackslashIndex = filePath.lastIndexOf('\\');
    // 取最后一个斜杠或反斜杠的较大者
    int     lastIndex = qMax(lastSlashIndex, lastBackslashIndex);
    QString filename;
    if (lastIndex != -1) {
      filename   = filePath.mid(lastIndex + 1);
      int suffix = filename.lastIndexOf(".");
      filename   = filename.left(suffix);
    }

    setWindowTitle(tr("Quick start-Currently in use: ") + filename);
    currentFilePath = filePath.toLocal8Bit().constData(); //指定当前主页面选定配置
    key_map.clear();
    initContext();                                        //将配置文件内容填入
  });

  //新建配置文件
  connect(createAction, &QAction::triggered, [&]() {
    QString dirPath = QCoreApplication::applicationDirPath();
    qDebug() << dirPath;

    std::string configPath = dirPath.toStdString() + "/config";

    auto parentPath = std::filesystem::path(configPath);
    if (!std::filesystem::exists(parentPath)) {
      if (!std::filesystem::create_directories(parentPath)) {
        qDebug() << "Failed to create directory: " << parentPath;
        return;
      }
    }

    while (true) {
      // 显示输入对话框，获取用户输入的文本
      QString text = QInputDialog::getText(nullptr, tr("input"), tr("Please enter the configuration file name:"));
      if (text.isEmpty()) return;
      std::string ABFileName = configPath + "/" + text.toLocal8Bit().constData() + ".json";
      //文件不存在则创建，存在则提示文件已存在请重新输入
      if (std::filesystem::exists(ABFileName)) {
        QMessageBox::warning(this, tr("warning"), tr("The file already exists, please re -enter"));
      }
      else {
        std::ofstream os(ABFileName, std::ios::out);
        if (os.is_open()) {
          os.close();
          currentFilePath = ABFileName; //指定当前主页面选定配置
          setWindowTitle(tr("Quick start-Currently in use: ") + text);
          clearModel();
          initContext();
          key_map.clear();
        }
        else {
          QMessageBox::critical(nullptr, tr("Fail"), tr("File creation failed"));
        }
        break;
      }
    }
  });

  //退出
  connect(exitAction, &QAction::triggered, []() {
    std::exit(0);
  });
}

void MainWindow::initConnect() {}

void MainWindow::initTableView() {
  table_view = new QTableView(this);
  // table_view->horizontalHeader()->setSectionsMovable(true); //列可拖动（拖动会导致拖动后新读取的数据还是按照默认的顺序填入，暂时放弃）
  //设置代理
  delegate_ = new SubStyledItemDelegate(this);
  //因为有个特殊的自定的弹出框失去焦点就会消失，所以在eventFilter中阻断了传播，但是也会导致其他正常的列会出现不打开另一个编辑器，当前*编辑器就无法关闭和提交数据
  // table_view->setItemDelegate(delegate_);
  table_view->setItemDelegateForColumn(1, delegate_);
  table_view->setItemDelegateForColumn(2, delegate_);
  table_view->setItemDelegateForColumn(3, delegate_);
  table_view->setItemDelegateForColumn(4, delegate_);
  table_view->setItemDelegateForColumn(5, delegate_);
  table_view->setItemDelegateForColumn(6, delegate_);
  table_view->setItemDelegateForColumn(8, delegate_);
  table_view->setItemDelegateForColumn(9, delegate_);
  table_view->setItemDelegateForColumn(10, delegate_);

  // 设置列宽自适应
  table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::initView() {
  QGridLayout* layout = new QGridLayout(this);

  layout->addWidget(table_view, 0, 0, 9, 10);

  // 添加弹簧
  QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(horizontalSpacer, 9, 0, 1, 6);

  QHBoxLayout* h_btns_box_layout = new QHBoxLayout();
  QPushButton* btn_add           = new QPushButton("+", this);
  QPushButton* btn_jian          = new QPushButton("-", this);
  QPushButton* btn_save          = new QPushButton(tr("save"), this);
  QPushButton* btn_set_default   = new QPushButton(tr("Set to default"), this);
  h_btns_box_layout->addWidget(btn_add);
  h_btns_box_layout->addWidget(btn_jian);
  h_btns_box_layout->addWidget(btn_save);
  h_btns_box_layout->addWidget(btn_set_default);

  //测试按钮
#ifdef _DEBUG
  QPushButton* btn_test = new QPushButton("test", this);
  h_btns_box_layout->addWidget(btn_test);
  connect(btn_test, &QPushButton::clicked, this, [&]() {
    QModelIndex index      = model_->index(0, 6); // 获取指定行的索引
    QVariant    itemData   = model_->data(index); // 获取数据
    QString     itemString = itemData.toString(); // 将数据转换为字符串
    qDebug() << "path: " << itemString;
    // QProcess* process = new QProcess(this);
    // process->start(itemString);
    QString filePath = QDir::toNativeSeparators(itemString); // 转为本地格式，避免中文路径无法启动
    QUrl    fileUrl  = QUrl::fromLocalFile(filePath);        //转为url方便启动
    QDesktopServices::openUrl(fileUrl);                      //使用该函数可以打开exe，也能打开jpg，txt等文件，更适用这里
    // for (auto map : key_map) {
    //   std::cout << "key: " << map.first << "   value: " << map.second.key_value_total << +" ";
    // }
  });
#endif

  layout->addLayout(h_btns_box_layout, 9, 6, 1, 4);

  ui.centralWidget->setLayout(layout);

  connect(btn_add, &QPushButton::clicked, this, &MainWindow::addViewNewRow);
  connect(btn_save, &QPushButton::clicked, this, &MainWindow::savaConfigJson);
  connect(btn_set_default, &QPushButton::clicked, this, &MainWindow::updateDefaultConfigFile);
}

void MainWindow::initContext() const {
  if (currentFilePath.empty()) return;
  //如果正在编辑的配置文件处于空文件则直接开始准备数据
  if (!std::filesystem::exists(currentFilePath)) {
    qDebug("initContext_文件不存在");
    return;
  }
  //读取文件判断有无内容
  uintmax_t file_size = std::filesystem::file_size(currentFilePath);
  if (file_size > 0) {
    //有内容，开始解析
    json json_ = analyzeJson();
    if (json_ == nullptr) return;
    //根据json内容回填模型数据
    updateModel(json_);
  }
  else {
    //无内容，准备一行数据
    QStandardItem* root              = model_->invisibleRootItem();
    QStandardItem* item_name_string  = new QStandardItem();
    QStandardItem* item_L_ctrl_bool  = new QStandardItem();
    QStandardItem* item_L_alt_bool   = new QStandardItem();
    QStandardItem* item_R_alt_bool   = new QStandardItem();
    QStandardItem* item_R_ctrl_bool  = new QStandardItem();
    QStandardItem* item_shortcut_key = new QStandardItem();
    QStandardItem* item_path_string  = new QStandardItem();
    QStandardItem* item_describe     = new QStandardItem();
    QStandardItem* item_trigger_enum = new QStandardItem();
    QStandardItem* item_enable_bool  = new QStandardItem();

    item_name_string->setData(tr("Please enter the name"), Qt::DisplayRole);
    item_L_ctrl_bool->setData(false, Qt::DisplayRole);
    item_L_alt_bool->setData(false, Qt::DisplayRole);
    item_R_alt_bool->setData(false, Qt::DisplayRole);
    item_R_ctrl_bool->setData(false, Qt::DisplayRole);
    item_shortcut_key->setData(tr("Press trigger"), Qt::ToolTipRole);
    item_path_string->setData(tr("Double-click to choose"), Qt::ToolTipRole);
    item_describe->setData("Please enter the description", Qt::DisplayRole);
    item_trigger_enum->setData(1, Qt::DisplayRole);
    item_enable_bool->setData(false, Qt::DisplayRole);

    root->setChild(0, 0, item_name_string);
    root->setChild(0, 1, item_L_ctrl_bool);
    root->setChild(0, 2, item_L_alt_bool);
    root->setChild(0, 3, item_R_alt_bool);
    root->setChild(0, 4, item_R_ctrl_bool);
    root->setChild(0, 5, item_shortcut_key);
    root->setChild(0, 6, item_path_string);
    root->setChild(0, 7, item_describe);
    root->setChild(0, 8, item_trigger_enum);
    root->setChild(0, 9, item_enable_bool);
  }
}

void MainWindow::initModel() {
  model_ = new QStandardItemModel();
  model_->setRowCount(1);
  model_->setColumnCount(9);
  setTableHead();

  table_view->setModel(model_);
}

json MainWindow::analyzeJson() const {
  std::ifstream iFile(currentFilePath);
  if (!iFile.is_open()) {
    QMessageBox::critical(nullptr, tr("open file error"), tr("open file error"));
    return nullptr;
  }
  uintmax_t file_size = std::filesystem::file_size(currentFilePath);

  // 分配内存
  std::string jsonData;
  jsonData.resize(file_size);
  // 读取文件内容
  iFile.read(&jsonData.at(0), file_size);
  //解析json
  json json_ = nullptr;
  try {
    json_ = json::parse(jsonData);
  }
  catch (const nlohmann::json::parse_error & e) {
    QMessageBox::critical(nullptr, tr("Configuration file parse Exception"), QString::fromStdString(e.what()));
  }
  return json_;
}

void MainWindow::savaConfigJson() const {
  //构建json
  json jsonObject;

  for (int i = 0; i < model_->rowCount(); ++i) {
    // 添加数据到 JSON 对象
    jsonObject[i]["name"]         = model_->index(i, 0).data(Qt::DisplayRole).toString().toStdString();
    jsonObject[i]["L-Ctrl"]       = model_->index(i, 1).data(Qt::DisplayRole).toBool();
    jsonObject[i]["L-Alt"]        = model_->index(i, 2).data(Qt::DisplayRole).toBool();
    jsonObject[i]["R-Alt"]        = model_->index(i, 3).data(Qt::DisplayRole).toBool();
    jsonObject[i]["R-Ctrl"]       = model_->index(i, 4).data(Qt::DisplayRole).toBool();
    jsonObject[i]["shortcut_key"] = model_->index(i, 5).data(Qt::DisplayRole).toString().toStdString();
    jsonObject[i]["path"]         = model_->index(i, 6).data(Qt::DisplayRole).toString().toStdString();
    jsonObject[i]["desc"]         = model_->index(i, 7).data(Qt::DisplayRole).toString().toStdString();
    jsonObject[i]["trigger"]      = model_->index(i, 8).data(Qt::DisplayRole).toInt();
    jsonObject[i]["enable"]       = model_->index(i, 9).data(Qt::DisplayRole).toBool();

    //下面几个用于快捷键的索引，不显示在视图中，但是要写在配置中
    int key = model_->index(i, 5).data(ROLE_KEY).toInt();
    // ShortcutKeyMsg tmp_shortcut_key_msg;
    // if (key_map.count(key) == 1) {
    //   tmp_shortcut_key_msg = key_map.find(key)->second;
    // }
    // else if (key_map.count(key) > 1) {
    //   auto model_vec = model_->index(i, 5).data(ROLE_VEC_KEY_NUM).value<ShortcutKeyMsg>().key_value_serial_number;
    //   auto pair_s    = key_map.equal_range(key);
    //   for (auto it = pair_s.first; it != pair_s.second; ++it) {
    //     auto skm_vec = it->second.key_value_serial_number;
    //     if (skm_vec == model_vec) {
    //       tmp_shortcut_key_msg = it->second;
    //       break;
    //     }
    //   }
    //   if (tmp_shortcut_key_msg.key_value_total == 0) {
    //     qDebug() << "map错误，虽然有该key的value，但对应不上";
    //     return;
    //   }
    // }
    // else {
    //   qDebug() << "map错误，该key: " << key << " 无对应的value";
    //   return;
    // }

    auto tmp_shortcut_key_msg = model_->index(i, 5).data(ROLE_VEC_KEY_NUM).value<ShortcutKeyMsg>();

    jsonObject[i]["key"]                                       = key;
    jsonObject[i]["ShortcutKeyMsg"]["key_value_total"]         = tmp_shortcut_key_msg.key_value_total;
    jsonObject[i]["ShortcutKeyMsg"]["key_value_serial_number"] = tmp_shortcut_key_msg.key_value_serial_number;
    jsonObject[i]["ShortcutKeyMsg"]["str_key_list"]            = tmp_shortcut_key_msg.str_key_list;
  }

  std::ofstream oFile(currentFilePath, std::ios::out | std::ios::trunc);
  if (oFile) {
    oFile << std::setw(4) << jsonObject << "\n";
    oFile.close();
    statusBar->showMessage(tr("save success"));
  }
  else {
    QMessageBox::critical(nullptr, tr("write error"), tr("Error writing configuration file"));
  }
}

void MainWindow::updateModel(const nlohmann::json & json_) const {
  model_->clear();

  setTableHead();

  QStandardItem* root = model_->invisibleRootItem();

  size_t size = json_.size();
  for (int i = 0; i < size; ++i) {
    QStandardItem* item_name_string     = new QStandardItem();
    QStandardItem* item_L_ctrl_bool     = new QStandardItem();
    QStandardItem* item_L_alt_bool      = new QStandardItem();
    QStandardItem* item_R_alt_bool      = new QStandardItem();
    QStandardItem* item_R_ctrl_bool     = new QStandardItem();
    QStandardItem* item_shortcut_key    = new QStandardItem();
    QStandardItem* item_path_string     = new QStandardItem();
    QStandardItem* item_describe_string = new QStandardItem();
    QStandardItem* item_trigger_enum    = new QStandardItem();
    QStandardItem* item_enable_bool     = new QStandardItem();


    item_name_string->setData(QString::fromStdString(json_[i]["name"].get<std::string>()), Qt::DisplayRole);
    item_L_ctrl_bool->setData(json_[i]["L-Ctrl"].get<bool>(), Qt::DisplayRole);
    item_L_alt_bool->setData(json_[i]["L-Alt"].get<bool>(), Qt::DisplayRole);
    item_R_alt_bool->setData(json_[i]["R-Alt"].get<bool>(), Qt::DisplayRole);
    item_R_ctrl_bool->setData(json_[i]["R-Ctrl"].get<bool>(), Qt::DisplayRole);
    item_shortcut_key->setData(QString::fromStdString(json_[i]["shortcut_key"].get<std::string>()), Qt::DisplayRole);
    item_path_string->setData(QString::fromStdString(json_[i]["path"].get<std::string>()), Qt::DisplayRole);
    item_describe_string->setData(QString::fromStdString(json_[i]["desc"].get<std::string>()), Qt::DisplayRole);
    item_trigger_enum->setData(json_[i]["trigger"].get<int>(), Qt::DisplayRole);
    item_enable_bool->setData(json_[i]["enable"].get<bool>(), Qt::DisplayRole);

    root->setChild(i, 0, item_name_string);
    root->setChild(i, 1, item_L_ctrl_bool);
    root->setChild(i, 2, item_L_alt_bool);
    root->setChild(i, 3, item_R_alt_bool);
    root->setChild(i, 4, item_R_ctrl_bool);
    root->setChild(i, 5, item_shortcut_key);
    root->setChild(i, 6, item_path_string);
    root->setChild(i, 7, item_describe_string);
    root->setChild(i, 8, item_trigger_enum);
    root->setChild(i, 9, item_enable_bool);

    //下面这几个是不显示在视图中的内容,属于第6列，快捷键的内容
    QStandardItem* item_key_total_int         = new QStandardItem();
    QStandardItem* item_ShortcutKeyMsg_struct = new QStandardItem();
    item_key_total_int->setData(json_[i]["key"].get<int>(), ROLE_KEY);
    ShortcutKeyMsg shortcut_key_msg;
    shortcut_key_msg.key_value_total         = json_[i]["ShortcutKeyMsg"]["key_value_total"].get<int>();
    shortcut_key_msg.key_value_serial_number = json_[i]["ShortcutKeyMsg"]["key_value_serial_number"].get<std::vector<
      uint32_t>>();
    shortcut_key_msg.str_key_list = json_[i]["ShortcutKeyMsg"]["str_key_list"].get<std::vector<std::string>>();
    item_ShortcutKeyMsg_struct->setData(QVariant::fromValue(shortcut_key_msg), ROLE_VEC_KEY_NUM);

    //同时将这些数据交给全局map提供给 StartQuickly 处理
    key_map.emplace(json_[i]["key"].get<int>(), json_[i]["ShortcutKeyMsg"].get<ShortcutKeyMsg>());
  }
  statusBar->showMessage(tr("read success"));
}

void MainWindow::addViewNewRow() const {
  QStandardItem* item_name_string     = new QStandardItem();
  QStandardItem* item_L_ctrl_bool     = new QStandardItem();
  QStandardItem* item_L_alt_bool      = new QStandardItem();
  QStandardItem* item_R_alt_bool      = new QStandardItem();
  QStandardItem* item_R_ctrl_bool     = new QStandardItem();
  QStandardItem* item_shortcut_key    = new QStandardItem();
  QStandardItem* item_path_string     = new QStandardItem();
  QStandardItem* item_describe_string = new QStandardItem();
  QStandardItem* item_trigger_enum    = new QStandardItem();
  QStandardItem* item_enable_bool     = new QStandardItem();

  item_name_string->setData(tr("Please enter the name"), Qt::DisplayRole);
  item_L_ctrl_bool->setData(false, Qt::DisplayRole);
  item_L_alt_bool->setData(false, Qt::DisplayRole);
  item_R_alt_bool->setData(false, Qt::DisplayRole);
  item_R_ctrl_bool->setData(false, Qt::DisplayRole);
  item_shortcut_key->setData(tr("Press trigger"), Qt::ToolTipRole);
  item_path_string->setData(tr("Double-click to choose"), Qt::ToolTipRole);
  item_describe_string->setData("Please enter the description", Qt::DisplayRole);
  item_trigger_enum->setData(1, Qt::DisplayRole);
  item_enable_bool->setData(false, Qt::DisplayRole);

  // 创建新的一行数据
  QList<QStandardItem*> newRowItems;

  newRowItems << item_name_string << item_L_ctrl_bool << item_L_alt_bool << item_R_alt_bool << item_R_ctrl_bool <<
    item_shortcut_key << item_path_string <<
    item_trigger_enum << item_trigger_enum << item_enable_bool;

  model_->appendRow(newRowItems);
}

void MainWindow::updateDefaultConfigFile() {
  //不知道为什么不能用 fstream，读后，移动指针到头部，再写入也无法更改文件内容
  std::ifstream ifs_tmp_update_default_config_json(default_config_file_json, std::ios::in);
  if (!ifs_tmp_update_default_config_json) {
    QMessageBox::critical(nullptr, tr("Fail"),
                          tr("Set default config Fail,because: ") + tr("fstream exception"));
    return;
  }

  uintmax_t file_size = std::filesystem::file_size(default_config_file_json);
  // 分配内存
  std::string jsonData;
  jsonData.resize(file_size);
  // 读取文件内容
  ifs_tmp_update_default_config_json.read(&jsonData.at(0), file_size);
  ifs_tmp_update_default_config_json.close();
  //解析json
  json json_ = json::parse(jsonData);


  std::ofstream ofs_tmp_update_default_config_json(default_config_file_json, std::ios::out);
  json_["default_config_file_path"] = currentFilePath;
  ofs_tmp_update_default_config_json << std::setw(4) << json_ << std::endl;
  ofs_tmp_update_default_config_json.close();
  default_config_json = json_;
  QMessageBox::information(this, tr("Success"), tr("Operation completed successfully."));
}

void MainWindow::clearModel() const {
  model_->clear();
  model_->setRowCount(1);
  model_->setColumnCount(11);
  setTableHead();
}

void MainWindow::setTableHead() const {
  model_->setHorizontalHeaderLabels(
    QStringList() << tr("Name") << tr("L-Ctrl") << tr("L-Alt") << tr("R-Alt") << tr("R-Ctrl") << tr("Shortcut_key") <<
    tr("Path") << tr("Desc") <<
    tr("Trigger") <<
    tr("Enable") << tr("Delete"));
}
