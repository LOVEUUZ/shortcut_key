#include "config_window.h"


using json = nlohmann::json;

Config_window::Config_window(QWidget* parent) :
  QWidget(parent), filter_path_list(Search_content::get_filter().first),
  filter_suffix_list(Search_content::get_filter().second) {
  ui.setupUi(this);

  main_widget = MainWidget::get_mainWidget();
  if (main_widget == nullptr)
    qFatal("main_widget 为空!!!");

  // 先卸载钩子，记得关闭该页面的时候重新加上钩子
  WindowsHookKeyEx::getWindowHook()->unInstallHook();
  WindowsHookMouseEx::getWindowHook()->unInstallHook();

  setAttribute(Qt::WA_ShowModal, true); //模态

#ifdef NDEBUG
  //置顶
  this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
#endif


  init_config();

  init_translate();

  //开机自启配置修改
  connect(ui.checkBox_is_boot_start, &QCheckBox::stateChanged, this, &Config_window::slot_checkBoxIsBootStart);
  connect(this, &Config_window::sig_checkBoxIsBootStart, main_widget, &MainWidget::slot_modifyConfig);

  //日志保留天数配置修改
  connect(ui.spinBox_log_day, &QSpinBox::valueChanged, this, &Config_window::slot_spinBoxValueChanged);

  //打开配置文件路径
  connect(ui.btn_openFilePath, &QPushButton::clicked, this, &Config_window::slot_openConfigPath);

  //过滤相关配置修改
}

Config_window::~Config_window() {}

void Config_window::closeEvent(QCloseEvent* event) {
  WindowsHookKeyEx::getWindowHook()->installHook();
  WindowsHookMouseEx::getWindowHook()->installHook();
#ifdef _DEBUG
	qDebug() << "钩子挂上成功";
#endif
  QWidget::closeEvent(event);
}

/**读取配置并赋值*/
void Config_window::init_config() {
  json & json_config = main_widget->get_jsonConfig();

  //开机启动填充
  if (!json_config.contains(is_boot_start)) {
    json_config[is_boot_start] = false;
    emit sig_checkBoxIsBootStart();
  }

  if (json_config.at(is_boot_start).get<bool>()) {
    config_is_boot_start = true;
    ui.checkBox_is_boot_start->setCheckState(Qt::Checked);
  }
  else {
    config_is_boot_start = false;
    ui.checkBox_is_boot_start->setCheckState(Qt::Unchecked);
  }

  //todo 设置语言  翻译暂时没做


  //日志保留天数填充
  if (!json_config.contains(log_retain_day)) {
    json_config[log_retain_day] = 7;
    emit sig_checkBoxIsBootStart();
  }
  ui.spinBox_log_day->setValue(json_config[log_retain_day].get<int>());


  //两个文本编辑器内容填充
  QString filter_path   = filter_path_list.join("\n");
  QString filter_suffix = filter_suffix_list.join("\n");

  ui.textEdit_filterPath->setText(filter_path);
  ui.textEdit_filterSuffix->setText(filter_suffix);
}

/**初始化翻译相关*/
void Config_window::init_translate() {
  ui.checkBox_is_boot_start->setText(tr("开启自启动"));
  ui.label_setLanguage->setText(tr("设置语言"));
  ui.label_log_text->setText(tr("日志保留天数"));
  ui.btn_openFilePath->setText(tr("打开配置文件目录"));
  ui.label_filterPath->setText(tr("过滤路径"));
  ui.label_filterSuffix->setText(tr("过滤后缀"));
}


/** 开机自启设置 */
void Config_window::slot_checkBoxIsBootStart(int state) {
  json & json_config = main_widget->get_jsonConfig();
  //未勾选,取消开机启动
  if (state == 0 && config_is_boot_start) {
    json_config[is_boot_start] = false;
    config_is_boot_start       = false;
    {
      QString targetFilePath   = QCoreApplication::applicationFilePath();
      QString startupDir       = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup";
      QString shortcutFilePath = startupDir + "/" + QFileInfo(targetFilePath).baseName() + ".lnk";

      QFile shortcutFile(shortcutFilePath);
      if (shortcutFile.exists() && shortcutFile.remove()) {
        qInfo() << "取消开机启动设置成功";
      }
      else {
        qWarning() << "取消开机启动设置失败";
        QMessageBox::warning(this, tr("警告"), tr("取消开机启动设置失败"));
      }
    }
  }
  //勾选，设置开机启动
  else if (state == 2 && !config_is_boot_start) {
    json_config["is_boot_start"] = true;
    config_is_boot_start         = true;
    {
      const QString & targetFilePath   = QCoreApplication::applicationFilePath();
      const QString & shortcutFilePath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) +
                                         "/Startup"
                                         + "/" + QFileInfo(targetFilePath).baseName() + ".lnk";
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
  }

  emit sig_checkBoxIsBootStart();
}

void Config_window::slot_spinBoxValueChanged(int day) {
  json & json_config          = main_widget->get_jsonConfig();
  json_config[log_retain_day] = day;
  Logger::getLogger().set_retention_days(7);

  emit sig_checkBoxIsBootStart();
}

void Config_window::slot_openConfigPath() {
  QString filePath_tmp = QCoreApplication::applicationDirPath() + "/config";
  QUrl    fileUrl      = QUrl::fromLocalFile(filePath_tmp);
  QDesktopServices::openUrl(fileUrl);
  qInfo() << "打开配置文件路径: " << filePath_tmp;
}
