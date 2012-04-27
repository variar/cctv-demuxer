/*
 * Copyright (c) 2011 Anton Filimonov
 *
 * See the file license.txt for copying permission.
 */

#include <QtCore/QCoreApplication>

#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QVector>
#include <QtCore>

#include <QDebug>

struct IndexEntry
{
    quint8 channel;
    quint32 offset;
    quint32 length;
    quint32 ts;
};

void unpack_channels(QFileInfo indexFile)
{
    QString indexFileName = indexFile.filePath();
    QString dataFileName = indexFile.dir().filePath(indexFile.baseName() + ".ps");

    qDebug()<<"Parsing index "<<indexFileName;

    QFile index(indexFileName);
    index.open(QIODevice::ReadOnly);

    if (index.isOpen()) {

        qDebug()<<"Opened index file"<<index.fileName();

        QList<IndexEntry> entries;

        QDataStream indexStream(&index);
        while(!indexStream.atEnd()) {
            quint8 channel = 0;
            indexStream>>channel;
            indexStream.skipRawData(3);
            quint32 offset = 0;
            indexStream>>offset;

            quint32 date = 0;
            indexStream>>date;

            indexStream.skipRawData(16);

            IndexEntry entry = {channel, offset, -1, date};
            entries.append(entry);
        }

        QList<QByteArray> channels;
        QList< QList<IndexEntry> > channelIndex;
        for (int i=0; i<32; ++i) {
            channels.append(QByteArray());
            channelIndex.append(QList<IndexEntry>());
        }

        QFile data(dataFileName);
        data.open(QIODevice::ReadOnly);
        if (data.isOpen()) {
            qDebug()<<"Opened data file"<<data.fileName();

            for (int i=0; i<entries.size()-1; ++i) {
                IndexEntry e1 = entries[i];
                IndexEntry e2 = entries[i+1];

                quint32 length = e2.offset - e1.offset;
                e1.length = length;

                data.seek(e1.offset);
                QByteArray channelData = data.read(length);

                channels[e1.channel].append(channelData);
                channelIndex[e1.channel].append(e1);
            }

            for (int i=0; i<channelIndex.size(); ++i) {
                if (!channelIndex[i].isEmpty()) {
                    qDebug()<<"channel"<<i<<"frames"<<channelIndex[i].size();
                    IndexEntry e1 = channelIndex[i].first();
                    IndexEntry e2 = channelIndex[i].last();

                    QDateTime t1 = QDateTime::fromTime_t(e1.ts);
                    QDateTime t2 = QDateTime::fromTime_t(e2.ts);

                    int duration = t1.secsTo(t2);

                    qDebug()<<"channel"<<i<<"duration"<<duration;
                    if (duration > 0) {
                        qDebug()<<"channel"<<i<<"fps"<<1.0*channelIndex[i].size()/duration;
                        qDebug()<<"Saving channels data";
                        QByteArray channelData = channels[i];
                        if (!channelData.isEmpty()) {
                            QFile channelFile(QString("ch_%1_%2.mpeg").arg(i).arg(indexFile.baseName()));
                            channelFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
                            channelFile.write(channelData);
                            channelFile.close();
                        }
                    }
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDir dir(argv[1]);
    qDebug()<<"Processing dir"<<dir.absolutePath();
    dir.setNameFilters(QStringList()<<"*.idx");
    QFileInfoList indexFiles = dir.entryInfoList(QDir::NoFilter, QDir::Name);
    qDebug()<<"Found "<<indexFiles.size()<<" index files";
    for (int i=0; i<indexFiles.size(); ++i) {
        unpack_channels(indexFiles.at(i));
    }

    return 0;
}
