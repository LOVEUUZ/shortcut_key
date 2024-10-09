#include "my_dialog_key_input.h"

my_dialog_key_input::my_dialog_key_input(QWidget* parent): QDialog(parent) {
  resize(600, 100);

  ptr_windows_hook = WindowsHookKeyEx::getWindowHook();
  start_quickly_   = StartQuickly::getStartQuickly();
  // main_window_ = qobject_cast<MainWindow*>(parent);
  // main_window_ = globalVar;
  start_quickly_->queue_clear(); //先清空没有处理的key，确保在这里输入的都是新的按键

  initView();
  initConnect();
  setFunc();
}

my_dialog_key_input::~my_dialog_key_input() {
  qInfo() << "关闭键盘dialog，返还主事件给核心";
  start_quickly_->queue_clear();
  start_quickly_->setFunc(); //关闭dialog的时候重新将主窗口的方法给钩子
}

void my_dialog_key_input::initView() {
  line_edit = new QLineEdit(this);
  line_edit->setEnabled(false); // 完全禁用
  // line_edit->setFocus();
  // 设置样式表来保持禁用时的颜色
  line_edit->setStyleSheet(
    "QLineEdit:disabled {"
    "   color: #000000;"            // 设置禁用时的文本颜色
    "   background-color: #FFFFFF;" // 设置禁用时的背景颜色
    "   border: 1px solid #C0C0C0;" // 设置禁用时的边框颜色
    "}"
  );


  btn_reset = new QPushButton(tr("reset"), this);
  btn_ok    = new QPushButton(tr("OK"), this);

  btn_reset->setFocusPolicy(Qt::NoFocus);
  btn_ok->setFocusPolicy(Qt::NoFocus);
  btn_ok->setEnabled(false);

  layout = new QHBoxLayout(this);

  layout->addWidget(line_edit, 6);
  layout->addWidget(btn_reset, 2);
  layout->addWidget(btn_ok, 2);
}

//信号连接
void my_dialog_key_input::initConnect() {
  //当内容发生变化的时候进行校验，成功的话按钮ok启用，否则禁用
  connect(line_edit, &QLineEdit::textChanged, [&]() {
    //校验下格式，格式暂定为（x+y+q+b）
    QString text = this->line_edit->text();
    if (text.isEmpty() || shortcut_key_msg_.key_value_serial_number.empty()) {
      btn_ok->setEnabled(false);
      return;
    }

    //注意，+的左右两侧都有空格
    tmp_list = text.split(" + ");
    // for (QString chars : tmp_list)
    //   qDebug() << chars;

    if (tmp_list.size() >= 20) {
      btn_ok->setEnabled(false);
      return;
    }

    btn_ok->setEnabled(true);
  });

  //清除当前内容
  connect(btn_reset, &QPushButton::clicked, [&]() {
    str_key_tmp.str(""); //清除字符流中的内容
    shortcut_key_msg_.key_value_total = 0;
    shortcut_key_msg_.key_value_serial_number.clear();
    tmp_list.clear();
    set_.clear();
    line_edit->clear();
    start_quickly_->queue_clear();
  });

  //编辑完成，退出保存
  connect(btn_ok, &QPushButton::clicked, [&]() {
    qInfo() << "编辑完成";
    // tmp_list.sort();
    for (const QString & key_name : tmp_list) {
      shortcut_key_msg_.str_key_list.emplace_back(key_name.toLocal8Bit().constData());
    }

    addNewShortKeyToMap(shortcut_key_msg_);
    //通过信号告知委托编辑完成
    emit editingCompleted(shortcut_key_msg_.str_key_list);
    close();
  });
}

void my_dialog_key_input::closeEvent(QCloseEvent* event) {
  // 安排在事件循环的安全点删除对象
  this->deleteLater();
  QDialog::closeEvent(event);
}

void my_dialog_key_input::setFunc() {
  qInfo() << "注册dialog键盘事件";
  auto func = [&](const KeyEvent& key_event) { set_key_event(key_event); };
  WindowsHookKeyEx::ptr_windows_hook->setFunc(func);
}

//处理按键事件
void my_dialog_key_input::set_key_event(const KeyEvent & key_event) {
  //忽略左右ctlr和alt，因为这四个按钮在主界面勾选
  if (key_event.key == L_Ctrl || key_event.key == L_Alt || key_event.key == R_Ctrl || key_event.key == R_Alt)
    return;

  //如果是重复输入的按钮则排除
  if (!set_.emplace(key_event.key).second)
    return;

  if (!str_key_tmp.str().empty())
    str_key_tmp << " + ";

#ifdef _DEBUG
  std::cout << key_event.key_name << " + " << key_event.isPressed << " + " << key_event.key << "\n";
#endif

  str_key_tmp << key_event.key_name;
  shortcut_key_msg_.key_value_total += key_event.key;
  shortcut_key_msg_.key_value_serial_number.emplace_back(key_event.key);
  this->line_edit->setText(QString::fromStdString(str_key_tmp.str()));
}
