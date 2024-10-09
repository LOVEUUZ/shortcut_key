#include "Logger.h"

Logger::Logger() : retentionDays(7), last_checked_date_(QDate::currentDate()) {
    logPath = QCoreApplication::applicationDirPath() + "/log/";

    // 创建日志目录
    QDir dir;
    if (!dir.exists(logPath)) {
        if (!dir.mkpath(logPath)) {
            qCritical() << "无法创建日志目录";
        }
    }

    // 默认保留 7 天的日志
    logFileName = logFileNameForDate(QDate::currentDate());
    openLogFile();

    // 设置定时器，每分钟检查一次日期是否发生变化
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Logger::checkDate);
    timer->start(60000);
}

Logger::~Logger() {
    timer->stop();
    closeLogFile();
}

// 单例
Logger& Logger::getLogger() {
    static Logger instance;
    return instance;
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Logger::getLogger().logMessage(type, msg);
}

// 写入日志
void Logger::logMessage(QtMsgType type, const QString& msg) {
    QMutexLocker locker(&logMutex);

    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtInfoMsg:
        txt = QString("Info: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        abort();
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString message = QString("%1 - %2").arg(timestamp).arg(txt);

    out << message << "\n";
    out.flush(); // 确保消息立即写入
}

// 根据日期创建日志文件名称
QString Logger::logFileNameForDate(const QDate& date) const {
    return logPath + date.toString("yyyy-MM-dd") + ".log";
}

// 打开日志文件
void Logger::openLogFile() {
    logFile.setFileName(logFileName);
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qCritical() << "无法打开日志文件进行写入: " << logFile.errorString();
    }
    else {
        out.setDevice(&logFile);
    }
}

// 关闭日志文件
void Logger::closeLogFile() {
    if (logFile.isOpen()) {
        logFile.close();
    }
}

// 检查日期是否变化
void Logger::checkDate() {
    QDate current_date = QDate::currentDate();
    if (current_date != last_checked_date_) {
        // 日期变化，关闭当前日志文件并打开新的日志文件
        closeLogFile();
        logFileName = logFileNameForDate(current_date);
        openLogFile();
        last_checked_date_ = current_date;
    }

    // 清除过久的日志
    cleanOldLogs();
}

// 清除过久的日志
void Logger::cleanOldLogs() const {
    QDir log_dir(logPath);
    QStringList filters;
    filters << "*.log";
    log_dir.setNameFilters(filters);
    QFileInfoList fileList = log_dir.entryInfoList(filters, QDir::Files, QDir::Time);

    QDateTime currentTime = QDateTime::currentDateTime();
    for (const QFileInfo& fileInfo : fileList) {
        if (fileInfo.lastModified().daysTo(currentTime) > retentionDays) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

// 保留天数设置
void Logger::set_retention_days(int days) {
    QMutexLocker locker(&logMutex);
    retentionDays = days;
}
