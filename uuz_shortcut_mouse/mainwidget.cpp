#include "mainwidget.h"

using json                           = nlohmann::json;
MainWidget* MainWidget::main_widget_ = nullptr;

MainWidget::MainWidget(QWidget* parent) : QWidget(parent) {
  // ui.setupUi(this);
  setObjectName("mainWidget");
  // setWindowFlags(Qt::FramelessWindowHint);
  main_widget_ = this;

  init_coordinate();

  init_shortcutKey();

  init_layout();

  init_search_line();

  connect(this, &MainWidget::sig_moveFocus, this, &MainWidget::slot_moveFocus);

  init_tray();


#ifdef NDEBUG
  //置顶 禁用最大最小化
  this->setWindowFlags(
    this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
#endif

  // setStyleSheet("MainWidget{background-color: #b7b7b7;} ");


  //在这里设置一下日志天数，懒得创建新东西了
  Logger::getLogger().set_retention_days(json_config["log_retain_day"].get<int>());
}

MainWidget::~MainWidget() {}


void MainWidget::init_search_line() {
  //样式
  QFont font;
  font.setPointSize(16);
  search_line->setFont(font);


  //用于避免定时器
  searchTimer = new QTimer(this);
  searchTimer->setSingleShot(true); // 定时器只执行一次

  //当定时器倒计时结束后触发搜索任务
  connect(searchTimer, &QTimer::timeout, this, [this]() {
    // slot_showStackedWidgetIndex(text.isEmpty() ? 0 : 1);
    emit search_inner_widget->slot_textChange(search_line->text()); // 发出信号进行搜索
  });

  //当搜索栏文本改变后触发定时器倒计时一次
  connect(search_line, &QLineEdit::textChanged, this, [this]() {
    searchTimer->start(300); // 启动定时器，300毫秒后触发
  });

  //控制 stacked_widget 显示为 search_inner_widget
  connect(search_line, &QLineEdit::textChanged, this, [this]() {
    QString text = this->search_line->text();

    int index = 0;
    if (text.isEmpty()) index = 0; // 文本为空，显示  icons_inner_widget 图标显示窗口
    else index                = 1; // 显示为 search_inner_widget 搜索内容窗口

    slot_showStackedWidgetIndex(index);
  });
}


// 设置布局
void MainWidget::init_layout() {
  // 1. 创建主布局并设置无边距
  v_search_and_grid = new QVBoxLayout(this);
  v_search_and_grid->setContentsMargins(0, 0, 0, 0); // 设置主布局无边距
  setLayout(v_search_and_grid);

  // 2. 创建上部垂直布局，并设置边距和间距
  topLayout = new QVBoxLayout();
  topLayout->setContentsMargins(5, 5, 5, 0); // 设置上部布局与四周的距离
  topLayout->setSpacing(5);                  // 设置上部布局内部组件间距为5

  search_line = new Search_line(this);
  search_line->setFixedSize(800, 60);                                     // 设置上部 QLineEdit 的固定高度为60
  search_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 设置为可扩展，宽度可以填满

  topLayout->addWidget(search_line);

  // 3. 下半部分布局，并设置边距和间距
  stacked_widget = new QStackedWidget(this);
  stacked_widget->setFixedSize(800, 400);
  stacked_widget->setContentsMargins(10, 5, 5, 10); // 设置内边距

  iconsLayout = new QVBoxLayout(stacked_widget);
  iconsLayout->setContentsMargins(10, 5, 5, 10); // 设置内边距
  iconsLayout->setSpacing(0);                    // 设置间距为0

  // 4. 创建内部 widget 并设置大小和颜色,添加进入栈窗口显示
  icons_inner_widget = new Icons_inner_widget(stacked_widget);
  // icons_inner_widget->setStyleSheet("background-color: #123456;"); // 设置背景颜色
  // icons_inner_widget->setContentsMargins(5, 5, 5, 5);
  // iconsLayout->addWidget(icons_inner_widget); // 添加到布局中

  search_inner_widget = new Search_content(stacked_widget);
  // search_inner_widget->setStyleSheet("background-color: red;"); // 设置背景颜色
  // search_inner_widget->setContentsMargins(5, 5, 5, 5);
  // iconsLayout->addWidget(search_inner_widget); // 添加到布局中

  stacked_widget->addWidget(icons_inner_widget);
  stacked_widget->addWidget(search_inner_widget);


  // 5. 添加上部和下部布局到主布局
  v_search_and_grid->addLayout(topLayout);      // 将上部布局添加到主布局
  v_search_and_grid->addWidget(stacked_widget); // 将下部widget添加到主布局

  // 自动调整窗口大小
  adjustSize();         // 调整窗口大小以适应内容
  setFixedSize(size()); // 将当前大小设置为固定大小
}

void MainWidget::init_shortcutKey() {
  //键盘捕获部分
  ctrlPressTimer = new QTimer(this);
  ctrlPressTimer->setInterval(300); // 0.3秒
  ctrlPressTimer->setSingleShot(true);
  connect(ctrlPressTimer, &QTimer::timeout, [&]() {
    ctrlPressCount = 0;
  });

  setKeyEvent();

  //鼠标钩子
  setMouseEvent();
}


void MainWidget::setKeyEvent() {
  //注册给钩子那边调用的函数
  std::function<void(KeyEvent)> func = [&](const KeyEvent & event) {
#ifdef _DEBUG
		std::cout << event.key << "\n";
#endif
    if ((event.key == 162 && event.isPressed) || (event.key == 163 && event.isPressed)) {
      //左右的ctrl
      ctrlPressTimer->start();
      ctrlPressCount++;
      if (ctrlPressCount == 2) {
        //双击唤醒/隐藏窗口
        if (isHidden()) {
          this->raise();
          this->activateWindow();
          show();
        }
        else hide();
      }
    }
    //显示状态的时候单次按下 esc按键 则隐藏
    else if (event.key == 27) {
      if (!this->isHidden()) {
        hide();
      }
    }
  };

  windowsKeyHookEx = WindowsHookKeyEx::getWindowHook();
  windowsKeyHookEx->installHook();
  windowsKeyHookEx->setFunc(func);
}

void MainWidget::setMouseEvent() {
  //鼠标事件捕获部分
  windowsMouseHook = WindowsHookMouseEx::getWindowHook();
  ////回调注册方法,当鼠标不在程序内部点击的时候，隐藏程序窗口
  std::function<void()> func = [&]() {
    // 获取鼠标的全局坐标
    auto mouse_coordinate = QCursor::pos();
    // 获取窗口左上角的全局坐标
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    // 创建一个包含窗口大小的矩形，并向上扩展 45px(大概是windows自带的最上面那一条的高度)
    QSize size_contain_top_edge_size = size();
    size_contain_top_edge_size.setHeight(size_contain_top_edge_size.height() + 45);

    // 调整窗口左上角的坐标
    globalPos.setY(globalPos.y() - 45);

    QRect globalRect(globalPos, size_contain_top_edge_size);

    // 判断鼠标坐标是否在窗口的全局矩形内
    if (!globalRect.contains(mouse_coordinate)) {
      emit sig_moveFocus(nullptr); // 如果不在窗口内，发出信号
    }
  };


  windowsMouseHook->setFunc(func);

#ifdef _DEBUG
	windowsMouseHook->installHook(); //安装鼠标钩子。当窗口显示的时候安装，隐藏的时候卸载
#endif
}

void MainWidget::showEvent(QShowEvent* event) {
  //判断鼠标当前在哪一个屏幕上，然后显示窗口显示在该屏幕记录的坐标
  QPoint          cursorPos = QCursor::pos();
  QList<QScreen*> screens   = QGuiApplication::screens();
  for (int i = 0; i < screens.size(); ++i) {
    QScreen* screen = screens[i];
    if (screen->geometry().contains(cursorPos)) {
      this->move(screens_coordinate[i]);
    }
  }

  windowsMouseHook->installHook(); //安装鼠标钩子

  //windows api，强制输入框获取焦点，参考：https://learn.microsoft.com/zh-cn/windows/win32/api/winuser/nf-winuser-setfocus
  SetFocus((HWND)this->search_line->winId());

  QWidget::showEvent(event);
}

void MainWidget::hideEvent(QHideEvent* event) {
  windowsMouseHook->unInstallHook(); //卸载鼠标钩子

  search_line->clear(); //清空搜索栏，并让界面重回图标界面

  QWidget::hideEvent(event);
}

void MainWidget::moveEvent(QMoveEvent* event) {
#ifdef _DEBUG
	qDebug() << "移动窗口";
#endif

  // 多屏幕检测
  QPoint cursorPos = QCursor::pos();
  // int index = -1;
  QList<QScreen*> screens = QGuiApplication::screens();
  for (int i = 0; i < screens.size(); ++i) {
    QScreen* screen = screens[i];
    if (screen->geometry().contains(cursorPos)) {
#ifdef _DEBUG
			qDebug() << "当前在屏幕 ==> " << i;
#endif

      QPoint coordinate;

      //判断当前屏幕是否记录了坐标，没有则添加新坐标并更新json
      auto it = screens_coordinate.find(i);
      if (it == screens_coordinate.end()) {
        coordinate.setX((screen->size().width()) / 2 - (this->size().width() / 2));
        coordinate.setY((screen->size().height()) / 2 - (this->size().height() / 2));
        screens_coordinate[i]                     = coordinate;
        json_config["coordinate"][i]["screen_id"] = i;
        json_config["coordinate"][i]["x"]         = coordinate.x();
        json_config["coordinate"][i]["y"]         = coordinate.y();
      }

      coordinate                                                               = this->pos();
      screens_coordinate[json_config["coordinate"][i]["screen_id"].get<int>()] = coordinate;

      json_config["coordinate"][i]["x"] = coordinate.x();
      json_config["coordinate"][i]["y"] = coordinate.y();

#ifdef _DEBUG
			qDebug() << "coordinate （" << coordinate.x()
				<< "," << coordinate.y() << ")";

			qDebug() << "json （" << json_config["coordinate"][i]["x"].get<int>()
				<< "," << json_config["coordinate"][i]["y"].get<int>() << ")";

			qDebug() << "";
#endif

      break;
    }
  }


  // 写入文件
  file_config->resize(0); // 清空文件
  QTextStream config_content(file_config);
  config_content << QString::fromStdString(json_config.dump(4));

  QWidget::moveEvent(event); // 调用基类的 moveEvent
}


void MainWidget::init_coordinate() {
  file_path = QCoreApplication::applicationDirPath() + "/config/shortcut_mouse_config.json";
  // 获取目录路径
  QDir dir = QFileInfo(file_path).absoluteDir();
  // 检查目录是否存在，如果不存在则创建它及其父文件夹
  if (!dir.mkpath(".")) {
    qWarning() << "Cannot create directory:" << dir.path();
    return;
  }

  file_config = new QFile(file_path);
  // 可读可写模式打开文件
  file_config->open(QIODevice::ReadWrite | QIODevice::Text);

  // 读取文件内容
  file_config->seek(0);
  QString qstr_config_content = file_config->readAll();
  if (qstr_config_content.isEmpty()) return;


  std::string str_config_content = qstr_config_content.toStdString();
  json_config                    = json::parse(str_config_content);

  for (int i = 0; i < json_config["coordinate"].size(); ++i) {
    QPoint coordinate;
    coordinate.setX(json_config["coordinate"][i]["x"].get<int>());
    coordinate.setY(json_config["coordinate"][i]["y"].get<int>());
    screens_coordinate[json_config["coordinate"][i]["screen_id"].get<int>()] = coordinate;
  }

  // 多屏幕检测，判断鼠标当前在哪，然后将
  QPoint          cursorPos = QCursor::pos();
  QList<QScreen*> screens   = QGuiApplication::screens();
  for (int i = 0; i < screens.size(); ++i) {
    QScreen* screen = screens[i];
    if (screen->geometry().contains(cursorPos)) {
#ifdef _DEBUG
			qDebug() << "当前在屏幕 ==> " << i;
#endif

      this->move(screens_coordinate[i]);
      break;
    }
  }


  // 连接应用程序退出信号到保存坐标的槽函数
  // connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [&]() {
  // file_config->resize(0); // 清空文件
  // file_config->seek(0);
  // QTextStream config_content(file_config);
  // qDebug() << QString::fromStdString(json_config.dump(4));
  // config_content << QString::fromStdString(json_config.dump(4));
  // }
  // );
}

void MainWidget::closeEvent(QCloseEvent* event) {
  this->hide();
  event->ignore(); // 忽略关闭事件，避免消息传递导致程序退出
}

/**初始化系统右下角托盘*/
void MainWidget::init_tray() {
  trayIcon.setIcon(QIcon(":/res/Resource/uuz_tray.ico"));

  // 添加菜单项
  auto act_show = new QAction("Show", &trayMenu);
  // auto act_enable = new QAction("enable/disable", &trayMenu);
  auto act_exit = new QAction("Exit", &trayMenu);

  trayMenu.addAction(act_show);
  trayMenu.addAction(act_exit);

  // 设置托盘图标的上下文菜单
  trayIcon.setContextMenu(&trayMenu);

  //双击显示界面
  connect(&trayIcon, &QSystemTrayIcon::activated, this, &MainWidget::slot_onTrayIconActivated);

  // 连接菜单项的信号和槽
  connect(act_show, &QAction::triggered, [&]() {
    this->show();
  });

  // connect(act_enable, &QAction::triggered, [&]() {
  //   static bool enable = true;
  //   if (enable) {
  //     
  //   }
  //   else {
  //     
  //   }
  // });

  connect(act_exit, &QAction::triggered, [&]() {
    QApplication::quit();
  });

  // 显示托盘图标
  trayIcon.show();
}

/**槽，托盘点击事件,控制显示或者隐藏*/
void MainWidget::slot_onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::DoubleClick) {
    if (this->isHidden()) {
      show();
    }
    else {
      hide();
    }
    //
    // raise();          // 确保窗口在其他窗口之上
    // activateWindow(); // 激活窗口
  }
}

/**槽，主要是设置界面修改json触发这里*/
void MainWidget::slot_modifyConfig() {
  file_config->resize(0); // 清空文件
  QTextStream config_content(file_config);
  config_content << QString::fromStdString(json_config.dump(4));
}


void MainWidget::slot_showStackedWidgetIndex(const int index) const {
  stacked_widget->setCurrentIndex(index);
}

void MainWidget::slot_moveFocus(QWidget* widget) {
  if (widget != nullptr) {
    widget->setFocus();
  }
  else {
    this->hide(); //隐藏主窗口
  }
}
