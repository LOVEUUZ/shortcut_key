#include "tools.h"

//导入导出配置(解压/压缩)
void Tools::compressFolder(const QString & folderPath, const QString & archivePath) {
  QFile archiveFile(archivePath);
  if (!archiveFile.open(QIODevice::WriteOnly)) {
    qWarning("Could not open archive file for writing");
    return;
  }
  QDataStream out(&archiveFile);

  QDir    dir(folderPath);
  QString rootPath = dir.absolutePath();

  std::function<void(const QDir &)> processDirectory = [&](const QDir & currentDir) {
    QFileInfoList fileList = currentDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo & fileInfo : fileList) {
      if (fileInfo.isDir()) {
        processDirectory(QDir(fileInfo.filePath())); //递归文件夹
      }
      else {
        QFile file(fileInfo.filePath());
        if (!file.open(QIODevice::ReadOnly)) {
          qWarning() << "Could not open file" << fileInfo.filePath();
          continue;
        }
        QByteArray fileData       = file.readAll();
        QByteArray compressedData = qCompress(fileData);

        QString relativePath = dir.relativeFilePath(fileInfo.filePath());
        out << relativePath;
        out << compressedData;
      }
    }
  };

  processDirectory(dir);
}

void Tools::decompressFolder(const QString & archivePath, const QString & outputPath) {
  QFile archiveFile(archivePath);
  if (!archiveFile.open(QIODevice::ReadOnly)) {
    qWarning("Could not open archive file for reading");
    return;
  }
  QDataStream in(&archiveFile);

  QDir outputDir(outputPath);
  if (!outputDir.exists()) {
    outputDir.mkpath(".");
  }

  while (!in.atEnd()) {
    QString    relativeFilePath;
    QByteArray compressedData;

    in >> relativeFilePath;
    in >> compressedData;

    QByteArray fileData       = qUncompress(compressedData);
    QString    outputFilePath = outputDir.filePath(relativeFilePath);

    QFileInfo fileInfo(outputFilePath);
    QDir      fileDir = fileInfo.dir();
    if (!fileDir.exists()) {
      fileDir.mkpath(".");
    }

    QFile outFile(outputFilePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
      qWarning() << "Could not open file" << outputFilePath << "for writing";
      continue;
    }
    outFile.write(fileData);
  }
}
