#pragma once

#include <qstring.h>
#include <QObject>

class Trigger;

inline Trigger* ptr_trigger;

class Trigger : public QObject {
    Q_OBJECT

  public:
    QString str_once_click   = tr("once_click");
    QString str_double_click = tr("double_click");
    QString str_triple_click = tr("triple_click");

    enum TriggerType {
      once_click   = 1,
      double_click = 2,
      triple_click = 3
    };

    QString triggerToString(TriggerType type) {
      switch (type) {
        case Trigger::once_click: return str_once_click;
        case Trigger::double_click: return str_double_click;
        case Trigger::triple_click: return tr("triple_click");
        default: return "unknown";
      }
    }

    int strToType(const QString& str){
        if (str == str_once_click) return once_click;
        if (str == str_double_click) return double_click;
        if (str == str_triple_click) return triple_click;
        return 0;
    }

    Trigger() {
      ptr_trigger = this;
    }

    ~Trigger() {
      delete ptr_trigger;
      ptr_trigger = nullptr;
    }
};
