#include "clearthread.h"
#include <QSettings>
#include <QDir>
#include <QApplication>
#include <QMessageBox>

void ClearThread::run()
{
    QString ClientDirectory = qApp->applicationDirPath() + "/work";

    QDir dir;
    dir.setPath(ClientDirectory);

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString file = fileInfo.fileName();
        dir.remove(file);
    }
}
