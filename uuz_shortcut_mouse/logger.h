#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QtGlobal>
#include <QDir>
#include <QCoreApplication>
#include <QTimer>

class Logger : public QObject {
    Q_OBJECT

  public:
    static Logger& getLogger();

    // 写入日志
    void logMessage(QtMsgType type, const QString & msg);

    // 保留天数设置
    void set_retention_days(int days);

    // 消息处理函数
    static void messageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg);

  private:
    Logger();
    ~Logger() override;

    // 根据日期创建日志文件名称
    QString logFileNameForDate(const QDate & date) const;

    // 打开日志文件
    void openLogFile();

    // 关闭日志文件
    void closeLogFile();

    // 检查日期是否变化
    void checkDate();

    // 清除过久的日志
    void cleanOldLogs() const;

    QString     logPath;
    QString     logFileName;
    QFile       logFile;
    QMutex      logMutex;
    int         retentionDays;
    QDate       last_checked_date_; //最后一次日期
    QTimer*     timer;
    QTextStream out;
};
