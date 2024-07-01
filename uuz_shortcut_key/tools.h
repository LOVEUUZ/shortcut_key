#pragma once

#include <QObject>

#include <QtZlib/zlib.h>   //用于压缩解压文件
#include <QDataStream>
#include <QByteArray>
#include <QFileInfoList>
#include <QDir>

//工具类，暂时只用于配置文件的压缩与解压
class Tools : public QObject {
    Q_OBJECT

  public:
    Tools() = delete;

    //导入导出配置，压缩解压文件相关
    static void compressFolder(const QString & folderPath, const QString & archivePath);
    static void decompressFolder(const QString & archivePath, const QString & outputPath);
};
