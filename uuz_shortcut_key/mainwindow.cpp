#include "mainwindow.h"


using json = nlohmann::json;

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
  ui.setupUi(this);

  initDefaultConfigFile();
  initTranslate();
  initTableView();
  initModel();
  initView();
  initTray();
  initMenu();
  initContext();

  qInfo() << "启动完成";
}

MainWindow::~MainWindow() {
  // 在程序退出时释放互斥体
  if (hMutex) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
  }
  qInfo() << "程序退出";
}

//加载翻译文件
void MainWindow::initTranslate() {
  // 获取系统语言
  QString       locale;
  std::ifstream if_defaultConfig(default_config_file_json);
  if (if_defaultConfig) {
    qInfo() << "加载默认设置语言";
    locale = QString::fromStdString(default_config_json["lang"].get<std::string>());

    // 加载Qt自带翻译文件
    if (qtTranslator.load("uuz_" + locale, QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
      QApplication::installTranslator(&qtTranslator);
    }

    QString translateFilePath = QCoreApplication::applicationDirPath() + "/translations/uuz_" + locale + ".qm";
    if (locale == "en") return;
    qInfo() << "加载翻译文件:" << translateFilePath;
    // 加载应用程序的翻译文件
    if (translator.load(translateFilePath)) {
      QApplication::installTranslator(&translator);
      qInfo() << "翻译文件加载成功";
    }
    else {
      qWarning() << "翻译文件: " << translateFilePath << "未找到。";
    }
  }
}

//首次运行时候创建默认配置
void MainWindow::initDefaultConfigFile() {
  qInfo() << "创建默认配置文件";
  default_config_file_json = QCoreApplication::applicationDirPath().toStdString() + str_default_txt_path;
  currentFilePath          = QCoreApplication::applicationDirPath().toStdString() + str_default_config_json_path;

  uintmax_t file_size;
  if (!std::filesystem::exists(default_config_file_json)) {
    //创建的，写入默认指定配置文件
    std::fstream ofs_default_config_file(default_config_file_json, std::ios::out);
    default_config_json["default_config_file_path"] = currentFilePath;
    default_config_json["isStartUp"]                = false;
    default_config_json["lang"]                     = QLocale::system().name().toStdString();
    ofs_default_config_file << std::setw(4) << default_config_json << "\n";
    ofs_default_config_file.close();

    //空的，除了设置默认json外毫无用处
    std::vector<uint32_t>    empty_vector_int;
    std::vector<std::string> empty_vector_string;

    std::ofstream ofs(currentFilePath, std::ios::trunc);
    //构建json
    json jsonObject;
    jsonObject[0]["name"]                                      = "1";
    jsonObject[0]["L-Ctrl"]                                    = false;
    jsonObject[0]["L-Alt"]                                     = false;
    jsonObject[0]["R-Alt"]                                     = false;
    jsonObject[0]["R-Ctrl"]                                    = false;
    jsonObject[0]["shortcut_key"]                              = "";
    jsonObject[0]["path"]                                      = "";
    jsonObject[0]["desc"]                                      = "desc";
    jsonObject[0]["trigger"]                                   = 1;
    jsonObject[0]["enable"]                                    = false;
    jsonObject[0]["key"]                                       = 0;
    jsonObject[0]["ShortcutKeyMsg"]["key_value_serial_number"] = empty_vector_int;
    jsonObject[0]["ShortcutKeyMsg"]["key_value_total"]         = 0;
    jsonObject[0]["ShortcutKeyMsg"]["str_key_list"]            = empty_vector_string;
    if (ofs) {
      ofs << std::setw(4) << jsonObject << "\n";
      ofs.close();
    }
    //既然首次创建默认配置，那么说明这里是首次运行，显示主界面
    this->show();
  }
  file_size = std::filesystem::file_size(default_config_file_json);

  std::fstream ifs_default_config_file(default_config_file_json, std::ios::in);
  if (!ifs_default_config_file) {
    QMessageBox::critical(nullptr, tr("Fail"), tr("open default error"));
    qCritical() << "默认文件打开错误，退出";
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
    qWarning() << "解析默认配置出现异常";
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

//桌面右下角的托盘
void MainWindow::initTray() {
  // 创建托盘图标
  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setIcon(QIcon(":/res/Resource/uuz_tray.ico")); // 设置托盘图标

  // 创建托盘菜单
  trayIconMenu           = new QMenu(this);
  QAction* restoreAction = new QAction(tr("Display"), this);
  QAction* quitAction    = new QAction(tr("quit"), this);
  connect(restoreAction, &QAction::triggered, this, &MainWindow::onShowMainWindow);
  connect(quitAction, &QAction::triggered, this, &MainWindow::onExitApplication);

  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addAction(quitAction);

  trayIcon->setContextMenu(trayIconMenu);

  // 连接托盘图标的信号
  connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);

  // 显示托盘图标
  trayIcon->show();
}

//初始化菜单栏
void MainWindow::initMenu() {
  //第一个菜单
  {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    // 在文件菜单中添加动作
    openFileAction = new QAction(tr("&Open File"), this);
    openFileAction->setIcon(QIcon(":/res/Resource/open file.svg"));
    fileMenu->addAction(openFileAction);

    QAction* createAction = new QAction(tr("&New File"), this);
    createAction->setIcon(QIcon(":/res/Resource/create file.svg"));
    fileMenu->addAction(createAction);

    // QAction* openAction = new QAction(tr("&Import File"), this);
    // fileMenu->addAction(openAction);

    fileMenu->addSeparator(); // 添加分隔符

    //开机启动
    QAction* startupAction = new QAction(tr("&boot startup"), this);
    {
      std::ifstream readConfig(default_config_file_json);
      if (readConfig) {
        // 分配内存
        std::string jsonData;
        uintmax_t   size = std::filesystem::file_size(default_config_file_json);
        jsonData.resize(size);
        // 读取文件内容
        readConfig.read(&jsonData.at(0), size);
        readConfig.close();
        //解析json
        auto tmp_json = json::parse(jsonData);
        if (tmp_json["isStartUp"].get<bool>()) {
          startupAction->setIcon(QIcon(":/res/Resource/gou.svg"));
        }
      }
    }

    fileMenu->addAction(startupAction);

    fileMenu->addSeparator(); // 添加分隔符

    //导出配置
    QAction* exportConfigAction = new QAction(tr("&Export Config"), this);
    createAction->setIcon(QIcon(":/res/Resource/create file.svg"));
    fileMenu->addAction(exportConfigAction);

    //导入配置
    QAction* importConfigAction = new QAction(tr("&Import Config"), this);
    createAction->setIcon(QIcon(":/res/Resource/create file.svg"));
    fileMenu->addAction(importConfigAction);

    fileMenu->addSeparator(); // 添加分隔符

    QAction* exitAction = new QAction(tr("&Quit"), this);
    exitAction->setIcon(QIcon(":/res/Resource/exit.svg"));
    fileMenu->addAction(exitAction);

    //底部状态栏，用来显示提示信息
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    //打开配置文件
    connect(openFileAction, &QAction::triggered, [&]() {
      QString filePath = QFileDialog::getOpenFileName(this, "Select the configuration file",
                                                      QApplication::applicationDirPath() + "/config/",
                                                      "Text files (*.json)");
      if (filePath == nullptr || filePath.isEmpty()) return;
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
      initContext(); //将配置文件内容填入
      qInfo() << "打开配置文件";
    });

    //新建配置文件
    connect(createAction, &QAction::triggered, [&]() {
      qInfo() << "新建配置文件";
      QString dirPath = QCoreApplication::applicationDirPath();
      qDebug() << dirPath;

      std::string configPath = dirPath.toStdString() + "/config";

      auto parentPath = std::filesystem::path(configPath);
      if (!std::filesystem::exists(parentPath)) {
        if (!std::filesystem::create_directories(parentPath)) {
          qDebug() << "Failed to create directory: " << parentPath;
          qWarning() << "无法新建配置文件";
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
            qWarning() << "新建文件失败";
          }
          break;
        }
      }
    });

    //导出配置
    connect(exportConfigAction, &QAction::triggered, [&]() {
      //1.选择导出位置
      QString exportDir = QFileDialog::getExistingDirectory(this, tr("Select the export position"), QDir::homePath());
      if (exportDir == nullptr || exportDir.isEmpty()) return;
      //2.获取现配置文件夹
      QString configPath = QApplication::applicationDirPath() + "/config/";
      if (!std::filesystem::exists(configPath.toStdString())) {
        //2.1 如何现有配置文件不存在，则终止,提示进行手动移动配置文件
        QMessageBox::warning(this, tr("warning"),
                             tr(
                               "Unable to export the file, please press the following prompts to operate:  "
                               "Find the program EXE directory, select the Config folder, copy and save all the files, move it to the config folder on the new machine"));
        qWarning() << "导出配置文件失败，原因: 文件或文件夹不存在";
        return;
      }
      //3.获取配置文件夹中的所有文件，压缩，将生成的压缩包放在导出位置
      exportDir = exportDir + "/config.jnf";
      Tools::compressFolder(configPath, exportDir);
      QMessageBox::information(this, tr("success"), tr("The export is successful, the path is:") + exportDir);
      qInfo() << "导出配置文件成功，路径为:" + exportDir;
    });

    //导入配置
    connect(importConfigAction, &QAction::triggered, [&]() {
      //1.选择文件位置
      QString configFilePath = QFileDialog::getOpenFileName(this, tr("Select file position"), QDir::homePath(), "config File (*.jnf)");
      //2.解压到指定位置
      Tools::decompressFolder(configFilePath, QApplication::applicationDirPath() + "/config/");
      QMessageBox::information(this, tr("success"), tr("Please re -select the configuration file for enabled"));
      qInfo() << "导入配置文件成功";
      //3.自动弹出配置选择
      openFileAction->triggered();
    });

    //设置为开机启动/取消开机启动
    connect(startupAction, &QAction::triggered, [=] {
      //如果没有设置图标，说明不是开机启动，就要设置为开机启动
      bool isStartUp;
      if (startupAction->icon().isNull()) {
        setStartup();
        startupAction->setIcon(QIcon(":/res/Resource/gou.svg"));
        isStartUp = true;
      }
      else {
        //如果有图标，说明已经是开机启动了，需要关闭开机启动
        unSetStartup();
        //删除图标
        startupAction->setIcon(QIcon());
        isStartUp = false;
      }

      //修改配置文件
      //1.先读出来
      std::ifstream if_defaultConfig(default_config_file_json);
      if (if_defaultConfig) {
        // 分配内存
        std::string jsonData;
        uintmax_t   size = std::filesystem::file_size(default_config_file_json);
        jsonData.resize(size);
        // 读取文件内容
        if_defaultConfig.read(&jsonData.at(0), size);
        if_defaultConfig.close();
        //解析json
        auto tmp_json         = json::parse(jsonData);
        tmp_json["isStartUp"] = isStartUp;


        //2.在写回去
        std::ofstream of_defaultConfig(default_config_file_json, std::ios::trunc);
        if (of_defaultConfig) {
          of_defaultConfig << std::setw(4) << tmp_json << "\n";
          of_defaultConfig.close();
          if (isStartUp) {
            qInfo() << "设置为开机启动";
            statusBar->showMessage(tr("boot startUp set success"));
          }
          else {
            qInfo() << "取消开机启动";
            statusBar->showMessage(tr("Cancel the boot start"));
          }
        }
      }
    });

    //退出
    connect(exitAction, &QAction::triggered, []() {
      qInfo() << "程序退出";
      std::exit(0);
    });
  }

  //第二个菜单 翻译
  {
    QMenu* lang_menu = menuBar()->addMenu(tr("language"));

    QAction* zh_cn_action = new QAction("中文", this);
    zh_cn_action->setData("zh");
    lang_menu->addAction(zh_cn_action);

    QAction* english_action = new QAction("English", this);
    english_action->setData("");
    lang_menu->addAction(english_action);

    if (default_config_json["lang"] == "zh_CN") {
      zh_cn_action->setIcon(QIcon(":/res/Resource/gou.svg"));
    }
    else if (default_config_json["lang"] == "en") {
      english_action->setIcon(QIcon(":/res/Resource/gou.svg"));
    }

    connect(zh_cn_action, &QAction::triggered, [this]() {
      switchLanguage("zh_CN");
    });
    connect(english_action, &QAction::triggered, [this]() {
      switchLanguage("en");
    });
  }
}

//初始化视图，并注册委托
void MainWindow::initTableView() {
  table_view = new QTableView(this);
  // table_view->horizontalHeader()->setSectionsMovable(true); //列可拖动（拖动会导致拖动后新读取的数据还是按照默认的顺序填入，暂时放弃）
  //设置代理
  delegate_ = new sub_styled_item_delegate(this);
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
  new Trigger(); //用于委托的触发选择下拉框的枚举

  // 设置列宽自适应
  table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//初始化页面和简单的按钮绑定
void MainWindow::initView() {
  setWindowTitle(tr("Start quickly"));

  QGridLayout* layout = new QGridLayout;

  layout->addWidget(table_view, 0, 0, 9, 10);

  // 添加弹簧
  QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(horizontalSpacer, 9, 0, 1, 6);

  QHBoxLayout* h_btns_box_layout = new QHBoxLayout();
  QPushButton* btn_add           = new QPushButton(tr("add new row"), this);
  // QPushButton* btn_jian          = new QPushButton("-", this);
  QPushButton* btn_save        = new QPushButton(tr("save"), this);
  QPushButton* btn_set_default = new QPushButton(tr("Set to default"), this);
  h_btns_box_layout->addWidget(btn_add);
  // h_btns_box_layout->addWidget(btn_jian);
  h_btns_box_layout->addWidget(btn_save);
  h_btns_box_layout->addWidget(btn_set_default);

  layout->addLayout(h_btns_box_layout, 9, 6, 1, 4);

  ui.centralWidget->setLayout(layout);

  connect(btn_add, &QPushButton::clicked, this, &MainWindow::addViewNewRow);
  connect(btn_save, &QPushButton::clicked, this, &MainWindow::savaConfigJson);
  connect(btn_set_default, &QPushButton::clicked, this, &MainWindow::updateDefaultConfigFile);
}

//初始化配置内容
void MainWindow::initContext() const {
  if (currentFilePath.empty()) return;
  //如果正在编辑的配置文件处于空文件则直接开始准备数据
  if (!std::filesystem::exists(currentFilePath)) {
    // qDebug("initContext_文件不存在");
    qWarning() << "initContext_文件不存在";
    return;
  }
  //读取文件判断有无内容
  uintmax_t file_size = std::filesystem::file_size(currentFilePath);
  if (file_size > 0) {
    //有内容，开始解析
    json json_ = analyzeJson();
    glob_json_ = json_;
    if (json_ == nullptr) return;
    //根据json内容回填模型数据
    updateModel(json_);
  }
  else {
    //无内容，准备一行数据
    qInfo() << "无内容，准备一行数据";

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

//初始化模型
void MainWindow::initModel() {
  model_ = new QStandardItemModel();
  model_->setRowCount(1);
  model_->setColumnCount(9);
  setTableHead();
  table_view->setModel(model_);
}

//解析json
json MainWindow::analyzeJson() const {
  std::ifstream iFile(currentFilePath);
  if (!iFile.is_open()) {
    QMessageBox::critical(nullptr, tr("open file error"), tr("open file error"));
    qWarning() << "打开文件失败";
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
    qWarning() << "配置文件解析异常";
    QMessageBox::critical(nullptr, tr("Configuration file parse Exception"), QString::fromStdString(e.what()));
  }
  return json_;
}

//保存配置文件
void MainWindow::savaConfigJson() const {
  qInfo() << "保存配置文件";
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

    ShortcutKeyMsg tmp_shortcut_key_msg = model_->index(i, 5).data(ROLE_VEC_KEY_NUM).value<ShortcutKeyMsg>();

    jsonObject[i]["key"]                                       = key;
    jsonObject[i]["ShortcutKeyMsg"]["key_value_total"]         = tmp_shortcut_key_msg.key_value_total;
    jsonObject[i]["ShortcutKeyMsg"]["key_value_serial_number"] = tmp_shortcut_key_msg.key_value_serial_number;
    jsonObject[i]["ShortcutKeyMsg"]["str_key_list"]            = tmp_shortcut_key_msg.str_key_list;

    if (model_->index(i, 6).data(Qt::DisplayRole).toString().toStdString().empty()) {
      QMessageBox::warning(nullptr, tr("path is null"), tr("Please enter the correct path"));
      return;
    }
    // if(key == 0) {
    //     QMessageBox::warning(nullptr, tr("key is null"), tr("Please enter the correct path"));
    // }
  }

  std::ofstream oFile(currentFilePath, std::ios::out | std::ios::trunc);
  if (oFile) {
    oFile << std::setw(4) << jsonObject << "\n";
    oFile.close();
    statusBar->showMessage(tr("save success"));
    glob_json_ = jsonObject;
    updateModel(jsonObject);
  }
  else {
    QMessageBox::critical(nullptr, tr("write error"), tr("Error writing configuration file"));
  }
}

//更新模型数据
void MainWindow::updateModel(const nlohmann::json & json_) const {
  qInfo() << "更新模型数据";
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

    //下面这几个是不显示在视图中的内容,属于第6列，快捷键的内容（不需要重新创建QStandardItem，直接用之前的，否则新对象会覆盖旧对象）
    item_shortcut_key->setData(json_[i]["key"].get<int>(), ROLE_KEY);
    ShortcutKeyMsg shortcut_key_msg;
    shortcut_key_msg.key_value_total         = json_[i]["ShortcutKeyMsg"]["key_value_total"].get<int>();
    shortcut_key_msg.key_value_serial_number = json_[i]["ShortcutKeyMsg"]["key_value_serial_number"].get<std::vector<
      uint32_t>>();
    shortcut_key_msg.str_key_list = json_[i]["ShortcutKeyMsg"]["str_key_list"].get<std::vector<std::string>>();
    item_shortcut_key->setData(QVariant::fromValue(shortcut_key_msg), ROLE_VEC_KEY_NUM);

    //如果是首次创建配置文件，则会出现key为0的情况，会导致首次触发快捷键的时候报错，因此排除掉
    if (json_[i]["key"].get<int>() == 0) continue;

    //同时将这些数据交给全局map提供给 StartQuickly 处理
    key_map.emplace(json_[i]["key"].get<int>(), json_[i]["ShortcutKeyMsg"].get<ShortcutKeyMsg>());
  }
  statusBar->showMessage(tr("read success"));
}

//添加一行新配置
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
    item_shortcut_key << item_path_string << item_describe_string <<
    item_trigger_enum << item_enable_bool;

  model_->appendRow(newRowItems);
}

//更新默认配置文件（txt那个）
void MainWindow::updateDefaultConfigFile() {
  qInfo() << "更新默认配置文件";
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

//清空模型数据
void MainWindow::clearModel() const {
  model_->clear();
  model_->setRowCount(1);
  model_->setColumnCount(11);
  setTableHead();
}

//设置视图的列名称
void MainWindow::setTableHead() const {
  model_->setHorizontalHeaderLabels(
    QStringList() << tr("Name") << "L-Ctrl" << "L-Alt" << "R-Alt" << "R-Ctrl" << tr("Shortcut_key") <<
    tr("Path") << tr("Desc") <<
    tr("Trigger") <<
    tr("Enable") << tr("Delete"));
}

//托盘事件
void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::DoubleClick) {
    show();
    raise();          // 确保窗口在其他窗口之上
    activateWindow(); // 激活窗口
  }
}

// 拦截关闭事件，最小化到托盘而不是退出
void MainWindow::closeEvent(QCloseEvent* event) {
  if (trayIcon->isVisible()) {
    hide();
    event->ignore();
  }
}

//托盘事件，显示主页面
void MainWindow::onShowMainWindow() {
  show();
  raise();
  activateWindow();
}

//托盘事件，退出程序
void MainWindow::onExitApplication() const {
  trayIcon->hide(); // 隐藏托盘图标
  qApp->quit();     // 退出应用程序
}

//槽:开机启动
void MainWindow::setStartup() {
  const QString & targetFilePath   = QCoreApplication::applicationFilePath();
  const QString & shortcutFilePath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup"
                                     + "/" + QFileInfo(targetFilePath).baseName() + ".lnk";;
  const QString & description = "";
  QFile           shortcutFile(shortcutFilePath);

  if (shortcutFile.exists()) {
    qInfo() << "开机启动设置成功.";
  }

  QString vbsScript = QDir::temp().absoluteFilePath("createShortcut.vbs");
  QFile   file(vbsScript);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    QString     targetPath   = targetFilePath;
    QString     shortcutPath = shortcutFilePath;
    targetPath.replace("/", "\\");
    shortcutPath.replace("/", "\\");

    out << "Set oWS = WScript.CreateObject(\"WScript.Shell\")\n";
    out << "sLinkFile = \"" << shortcutPath << "\"\n";
    out << "Set oLink = oWS.CreateShortcut(sLinkFile)\n";
    out << "oLink.TargetPath = \"" << targetPath << "\"\n";
    out << "oLink.WorkingDirectory = \"" << QFileInfo(targetPath).absolutePath().replace("/", "\\") << "\"\n";
    if (!description.isEmpty()) {
      out << "oLink.Description = \"" << description << "\"\n";
    }
    out << "oLink.Save\n";
    file.close();
  }
  else {
    qWarning() << "Failed to create VBS file.开机启动设置失败";
  }

  QProcess process;
  process.start("wscript", QStringList() << vbsScript);
  process.waitForFinished();
  file.remove();
}

//槽:取消开机启动
void MainWindow::unSetStartup() {
  QString targetFilePath   = QCoreApplication::applicationFilePath();
  QString startupDir       = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup";
  QString shortcutFilePath = startupDir + "/" + QFileInfo(targetFilePath).baseName() + ".lnk";

  QFile shortcutFile(shortcutFilePath);
  if (shortcutFile.exists() && shortcutFile.remove()) {
    qInfo() << "取消开机启动设置成功";
  }
  else {
    qInfo() << "取消开机启动设置失败";
  }
}

//切换翻译
void MainWindow::switchLanguage(const QString & lang) {
  std::ifstream if_defaultConfig(default_config_file_json);
  if (if_defaultConfig) {
    // 分配内存
    std::string jsonData;
    uintmax_t   size = std::filesystem::file_size(default_config_file_json);
    jsonData.resize(size);
    // 读取文件内容
    if_defaultConfig.read(&jsonData.at(0), size);
    if_defaultConfig.close();
    //解析json
    auto tmp_json    = json::parse(jsonData);
    tmp_json["lang"] = lang.toStdString();


    //2.在写回去
    std::ofstream of_defaultConfig(default_config_file_json, std::ios::trunc);
    if (of_defaultConfig) {
      of_defaultConfig << std::setw(4) << tmp_json << "\n";
      of_defaultConfig.close();
      qInfo() << "设置默认语言成功";
      statusBar->showMessage(tr("boot startUp set success"));

      //是否立即重启
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Restart Application"), tr("Do you want to restart the application?"),
                                    QMessageBox::Yes | QMessageBox::No);
      if (reply) {
        //重启前释放互斥体，解除不能重复启动的限制
        if (hMutex) {
          ReleaseMutex(hMutex);
          CloseHandle(hMutex);
        }
        // 获取当前应用程序的可执行路径
        QString     program          = QCoreApplication::applicationFilePath();
        QStringList arguments        = QCoreApplication::arguments();
        QString     workingDirectory = QDir::currentPath();
        // 启动新进程
        QProcess::startDetached(program, arguments, workingDirectory);

        // 退出当前进程
        this->close();
        QCoreApplication::quit();
      }
    }
  }
}
