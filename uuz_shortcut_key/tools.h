#pragma once

#include <QObject>

#include <QtZlib/zlib.h>   //����ѹ����ѹ�ļ�
#include <QDataStream>
#include <QByteArray>
#include <QFileInfoList>
#include <QDir>

//�����࣬��ʱֻ���������ļ���ѹ�����ѹ
class Tools : public QObject {
    Q_OBJECT

  public:
    Tools() = delete;

    //���뵼�����ã�ѹ����ѹ�ļ����
    static void compressFolder(const QString & folderPath, const QString & archivePath);
    static void decompressFolder(const QString & archivePath, const QString & outputPath);
};
