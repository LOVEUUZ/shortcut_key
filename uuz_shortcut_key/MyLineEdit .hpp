#include <QApplication>
#include <QLineEdit>
#include <QKeyEvent>

class MyLineEdit : public QLineEdit {
  public:
    MyLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {
      resize(300, 50);
    }

  protected:
    void keyPressEvent(QKeyEvent* event) override {
      int     key    = event->key();
      QString keyStr = keyToString(key);

      arr[count++] = key;

      if (!keyStr.isEmpty()) {
        key_combination += keyStr;
        setText(key_combination);
      }
    }

    void keyReleaseEvent(QKeyEvent* event) override {
        qDebug() << "";
      count = 0;
      key_combination.clear();
      for (int i : arr) {
        if(i == 0) break;
        qDebug() << i;
      }

      for (int& i : arr) {
          i = 0;
      }
    }

  private:
    QString keyToString(int key) {
      QString keyStr = QKeySequence(key).toString();
      // 这里可以添加额外的逻辑，例如将某些按键转换为特定字符串
      return keyStr;
    }

    int     count = 0;
    QString key_combination;
    int     arr[10];
};
