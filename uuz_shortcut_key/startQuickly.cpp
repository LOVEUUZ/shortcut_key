#include "startQuickly.h"

#include <iostream>
#include <set>

extern MainWindow*    globalVar;
extern nlohmann::json glob_json_;


StartQuickly* StartQuickly::start_quickly_ = nullptr;

//单例模式
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
  tmp_clear_set();
}

//卸载键盘钩子，并让线程停止循环
StartQuickly::~StartQuickly() {
  ptr_windows_hook->unInstallHook();
  isEnd = false;
}

//向队列末尾添加
void StartQuickly::set_key_event(const KeyEvent & key_event) {
  std::unique_lock<std::mutex> lock(mtx);
  if (queue_key_event.size() > 20) {
    queue_key_event.pop();
  }
#ifdef _DEBUG
  std::cout << key_event.key_name << " " << key_event.key << " " << key_event.isPressed << "\n";
#endif
  queue_key_event.push(key_event);
  isQueueEmpty.notify_one(); //唤醒消费线程
}

//取出队首元素
KeyEvent StartQuickly::getKeyEvent() {
  KeyEvent key_event_;

  {
    // std::unique_lock<std::mutex> lock(mtx);
    if (queue_key_event.empty()) {
      return KeyEvent{};
    }
    key_event_ = std::move(queue_key_event.front());
    queue_key_event.pop();
  }

  return key_event_;
}

//核心处理部分，识别键盘关键字以及打开绑定程序
void StartQuickly::identify() {
  std::unique_lock<std::mutex> lock(mtx_identify);
  static int                   key_total;
  static std::set<uint64_t>    set_key_value;

  static uintmax_t last_key = 0; //前一个key值
  static uintmax_t pre_key  = 0; //最后一个key值

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
        if (key_event.key == 0) continue;
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
#ifdef _DEBUG
              qDebug() << "key_total: " << key_total;
#endif
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

            //3.3 判断是否启用该行配置
            if (!glob_json_[key_index]["enable"].get<bool>()) { continue; }

            pre_key            = last_key;      //前次的键位
            last_key           = key_event.key; //本次按下的键位
            static int count   = 0;
            int        trigger = glob_json_[key_index]["trigger"].get<int>();

            /*
             * 这里根据配置的触发次数来调整。
             * 当触发次数为 1 ，也就是默认的时候，不做判断处理，直接打开文件。
             * 当触发次数为 2 和 3 的时候，分两种情况，一种是按下了alt，另一种是同时按下了 alt 和 ctrl。
             *
             * 当只按下alt的时候，是释放状态，当同时按下ctrl和alt的时候，又是正常的按下和释放状态
             * 同时，因为当没有按下alt 的时候比如当触发次数为 2 的时候，++count == 1 这里是 1 而不是 2。
             * 这是因为第一次按下的时候，前一次与本次键位是不相等的，所以不会++，要等到下一次，也就是第二次的时候才会相等并执行++，所以要减少1次判断
             *
             * 这里有个小问题，不过正常使用不会触发:
             * 当狂搓键盘的时候，即使中途按下了触发的键位，也是没有触发的，要等到前面的检测没有键盘按下的时候 key_total 重新清 0 。
             *
             */
            if (trigger == 1) {
              count = 0;
            }
            else if ((trigger == 2 && !L_Alt && !R_Alt) || (trigger == 2 && L_Ctrl && R_Ctrl)) {
              if (pre_key == last_key) {
                if (++count == 1) {
                  count    = 0;
                  last_key = 0;
                }
                else {
                  continue;
                }
              }
              else {
                count = 0;
                continue;
              }
            }
            else if (trigger == 2 && (L_Alt || R_Alt)) {
              if (key_event.key == last_key) {
                if (++count == 4) {
                  count = 0;
                }
                else {
                  continue;
                }
              }
              else {
                count = 0;
                continue;
              }
            }
            else if ((trigger == 3 && !L_Alt && !R_Alt) || (trigger == 3 && L_Ctrl && R_Ctrl)) {
              if (pre_key == last_key) {
                if (++count == 2) {
                  count    = 0;
                  last_key = 0;
                }
                else {
                  continue;
                }
              }
              else {
                count = 0;
                continue;
              }
            }
            else if (trigger == 3 && (L_Alt || R_Alt)) {
              if (key_event.key == last_key) {
                if (++count == 6) {
                  count = 0;
                }
                else {
                  continue;
                }
              }
              else {
                count = 0;
                continue;
              }
            }


            //3.4 有按下按键，且alt没有处于按下状态
            if (key_event.isPressed && !L_Alt && !R_Alt) {
              //3.4.1 再次和json配置中的四个ctrl和alt对比校验
              if (ctrlAndAltCheck(key_index)) {
                continue;
              }

              //3.4.2 正式打开指定路径
              qInfo() << "打开程序";
              std::string path = glob_json_[key_index]["path"].get<std::string>();
              startProcess(path);
            }

            //3.5 因为alt按住后再按下其他键位，会导致除了alt和ctrl之外的按下都变成释放，所以这里处理快捷键中有alt的情况
            else if (L_Alt || R_Alt) {
              //3.5.1 对比ctrl和alt与json配置中的状态

              if (ctrlAndAltCheck(key_index)) {
                continue;
              }

              //3.5.2 正式打开指定路径
              qInfo() << "alt 打开程序";
              std::string path = glob_json_[key_index]["path"].get<std::string>();
              startProcess(path);
            }
          }
        }
      }
    }
  }
}

//专门处理ctrl和alt的状态和配置文件是否一致的检测
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
  auto res = tmp_set.insert(path);
  if (!res.second) {
    return;
  }

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

//避免程序短时间内重复多次启动
void StartQuickly::tmp_clear_set() {
  std::thread([this]() {
    while (isEnd) {
      tmp_set.clear();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }).detach();
}
