#include "startQuickly.h"

#include <iostream>
#include <set>


StartQuickly* StartQuickly::start_quickly_ = nullptr;

StartQuickly* StartQuickly::getStartQuickly() {
  if (start_quickly_ == nullptr) {
    start_quickly_ = new StartQuickly;
  }
  return start_quickly_;
}

StartQuickly::StartQuickly() {
  ptr_windows_hook = WindowsHookEx::getWindowHook();
  ptr_windows_hook->installHook();
  setFunc();
  initThread();
}


StartQuickly::~StartQuickly() {
  ptr_windows_hook->unInstallHook();
}

//向队列末尾添加
void StartQuickly::set_key_event(const KeyEvent & key_event) {
  std::unique_lock<std::mutex> lock(mtx);
  if (queue_key_event.size() > 20) {
    queue_key_event.pop();
  }
  std::cout << key_event.key_name << " " << key_event.key << " " << key_event.isPressed << "\n";
  queue_key_event.push(key_event);
  isQueueEmpty.notify_one(); //唤醒消费线程
}

//取出队首元素
KeyEvent StartQuickly::getKeyEvent() {
  KeyEvent key_event_;

  {
    // std::unique_lock<std::mutex> lock(mtx);
    if (queue_key_event.empty()) {
      return NULL;
    }
    key_event_ = std::move(queue_key_event.front());
    queue_key_event.pop();
  }

  return key_event_;
}

//核心处理部分，识别键盘关键字以及打开绑定程序
void StartQuickly::identify() {
  std::unique_lock<std::mutex> lock(mtx_identify);
  uint8_t                      size;
  static int                   key_total;
  static std::set<uint64_t>    set_key_value;
  while (isEnd) {
    auto result = isQueueEmpty.wait_for(lock, std::chrono::microseconds(500));

    //超时且队列为空
    if (result == std::cv_status::timeout && queue_key_event.empty()) {
      continue;
    }

    //被唤醒
    if (result == std::cv_status::no_timeout && !queue_key_event.empty()) {
      {
        std::unique_lock<std::mutex> lock_read(mtx);

        //检查当前是否有按键被按下，如果当前没有任何按下的，则把存储状态清空
        std::set<int> pressedKeys;
        for (int key = 0x08; key <= 0xFF; ++key) {
          if (GetAsyncKeyState(key) & 0x8000) {
            pressedKeys.insert(key);
          }
        }
        if (pressedKeys.empty()) {
          key_total = 0;
          set_key_value.clear();
        }

        //开始
        //1. 取得键盘事件（似乎正常情况下都是队列只会有1个，除非cpu很慢？)
        KeyEvent key_event = getKeyEvent();
        //2. 先判断是不是ctrl和alt，这两个需要单独处理
        switch (key_event.key) {
          case 162: {
            if (key_event.isPressed == 1) {
              L_Ctrl = true;
            }
            else {
              L_Ctrl = false;
            }
            break;
          }
          case 164: {
            if (key_event.isPressed == 1) {
              L_Alt = true;
            }
            else {
              L_Alt = false;
            }
            break;
          }
          case 165: {
            if (key_event.isPressed == 1) {
              R_Alt = true;
            }
            else {
              R_Alt = false;
            }
            break;
          }
          case 163: {
            if (key_event.isPressed == 1) {
              R_Ctrl = true;
            }
            else {
              R_Ctrl = false;
            }
            break;
          }
          default: {
            //3. 在这里处理除了 ctrl 和 alt 之外的按键，统计所有按下的键位和释放键位
            if (key_event.isPressed) {
              auto res = set_key_value.emplace(key_event.key);
              if (res.second) {
                key_total += key_event.key;
              }
            }
            else {
              if (!L_Alt && !R_Alt) {
                uint64_t row = set_key_value.erase(key_event.key);
                if (row == 1) {
                  key_total -= key_event.key;
                }
              }
              else {
                auto res = set_key_value.emplace(key_event.key);
                if (res.second) {
                  key_total += key_event.key;
                }
              }
              qDebug() << "key_total: " << key_total;
            }

            //3.1 判断是否有和key_map对应的值
            auto res_pair = key_map.equal_range(key_total);
            if (res_pair.first == res_pair.second) { continue; }

            //3.2 遍历当前json配置，找到符合的key总计值
            int key_index = -1;
            for (int i = 0; i < glob_json_.size(); ++i) {
              if (glob_json_[i]["key"].get<int>() == key_total) {
                key_index = i;
                break;
              }
            }

            //3.3 有按下按键，且alt没有处于按下状态
            if (key_event.isPressed && !L_Alt && !R_Alt) {
              //3.3.1 再次和json配置中的四个ctrl和alt对比校验
              if (ctrlAndAltCheck(key_index)) {
                continue;
              }

              //3.3.2 正式打开指定路径
              qDebug() << "打开程序";
              std::string path = glob_json_[key_index]["path"].get<std::string>();
              startProcess(path);
            }

            //3.4 因为alt按住后再按下其他键位，会导致除了alt和ctrl之外的按下都变成释放，所以这里处理快捷键中有alt的情况
            else if (L_Alt || R_Alt) {
              //3.4.1 对比ctrl和alt与json配置中的状态

              if (ctrlAndAltCheck(key_index)) {
                continue;
              }

              //3.4.2 正式打开指定路径
              qDebug() << "alt 打开程序";
              std::string path = glob_json_[key_index]["path"].get<std::string>();
              startProcess(path);
            }
          }
        }
      }
    }
  }
}

bool StartQuickly::ctrlAndAltCheck(const int key_index) const {
  bool json_L_Ctrl = glob_json_[key_index]["L-Ctrl"];
  bool json_L_Alt  = glob_json_[key_index]["L-Alt"];
  bool json_R_Alt  = glob_json_[key_index]["R-Alt"];
  bool json_R_Ctrl = glob_json_[key_index]["R-Ctrl"];

  if (!(json_L_Ctrl == L_Ctrl && json_R_Ctrl == R_Ctrl)) {
    return true;
  }
  if (!(json_L_Alt == L_Alt && json_R_Alt == R_Alt)) {
    return true;
  }
  return false;
}

void StartQuickly::initThread() {
  isEnd = true;
  std::thread(&StartQuickly::identify, this).detach();
}


void StartQuickly::setFunc() {
  auto func = [&](const KeyEvent & key_event) { set_key_event(key_event); };
  queue_clear();
  ptr_windows_hook->setFunc(func);
  //
  // // 使用 lambda 表达式绑定成员函数
  // // bool b = ptr_windows_hook->setFunc([&](KeyEvent k) {
  // //   std::cout << k.key_name << " " << k.key << " " << k.isPressed << "\n";
  // // });
}


//打开程序
void StartQuickly::startProcess(const std::string & path) {
    //按住alt触发的快捷键可能会连续打开，因此用一个计时器控制一下短时间内只能打开一次





  //qt的函数
  // {
  //   QString filePath = QDir::toNativeSeparators(QString::fromStdString(path)); // 转为本地格式，避免中文路径无法启动
  //   QUrl    fileUrl  = QUrl::fromLocalFile(filePath);                          //转为url方便启动
  //   QDesktopServices::openUrl(fileUrl);                                        //使用该函数可以打开exe，也能打开jpg，txt等文件，更适用这里
  //   QProcess process;
  //   QProcess::startDetached(filePath);
  // }


  // (windows的函数)
  // 转换路径字符串为宽字符串
  // 将 UTF-8 编码的字符串转换为 UTF-16 编码的 LPCWSTR
  {
    int  size_needed = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
    auto wpath       = new wchar_t[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wpath, size_needed);

    // 检查路径是否存在
    if (!PathFileExists(wpath)) {
      std::cerr << "Path does not exist: " << path << std::endl;
      return;
    }

    // 获取文件属性
    DWORD attributes = GetFileAttributes(wpath);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
      std::cerr << "Could not get attributes for path: " << path << std::endl;
      return;
    }

    // 检查路径是否为文件夹
    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
      // 打开文件夹
      ShellExecute(NULL, L"open", wpath, NULL, NULL, SW_SHOWDEFAULT);
    }
    else {
      // 获取文件扩展名
      std::wstring extension = PathFindExtension(wpath);

      // 如果是 .exe 文件，直接打开
      if (_wcsicmp(extension.c_str(), L".exe") == 0) {
        ShellExecute(NULL, NULL, wpath, NULL, NULL, SW_SHOWDEFAULT);

        // 构建命令字符串，使用cmd /c来执行exe文件并在完成后关闭cmd窗口
        // 构建命令字符串，先cd到目标目录，然后执行可执行文件
        // std::string command = std::string("cmd /c cd ") + directoryPath + " && " + path;
        // system("cmd /c F:/GAL/星辰恋曲的白色永恒/星辰恋曲的白色永恒/WhiteEternity.exe");
      }
      else {
        // 否则，使用默认程序打开
        ShellExecute(NULL, L"open", wpath, NULL, NULL, SW_SHOWDEFAULT);
      }
    }
  }
}
