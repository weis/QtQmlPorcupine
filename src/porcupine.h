#ifndef PORCUPINE_H
#define PORCUPINE_H

#include <QByteArray>
#include <QString>
#include <QVector>

class QLibrary;
class QIODevice;

class Porcupine
{

public:
    static Porcupine* create(const QString& accessKey,
                             const QVector<QString>& keywordPaths,
                             const QString& modelPath,
                             const QVector<qreal>& sensitivities,
                             QString* errMsg = nullptr);

    ~Porcupine();

    QString version() const;

    qint32 frameLength() const;

    qint32 bytesFrameLength() const;

    qint32 sampleRate() const;

    bool process(int& keywordIndex, const char* audioData, const int len, QString* errMsg = nullptr);

    void enable(bool enable);

private:
    explicit Porcupine(void* pvInstance, QLibrary* pvLib);

    bool processFrame(const int16_t* pcm, qint32* keywordIndex, QString* errMsg = nullptr);

    void*               m_pvInstance;
    QLibrary*           m_pvLib;
    QByteArray          m_audioBuffer;
    bool                m_pvEnabled;
    const int           m_pvBytesFrameSize;
};

#endif // PORCUPINE_H
