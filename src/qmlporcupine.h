#ifndef QMLPORCUPINE_H
#define QMLPORCUPINE_H

#include <QObject>
#include <QQmlEngine>
#include <QAudioFormat>
#include <QStringListModel>

class QLibrary;
class QAudioSource;
class QIODevice;
class Porcupine;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
class QAudioInput;
#else
class QAudioSource;
#endif
///
/// \brief Checks and demonstrates the porcupine engine.
/// Based on class Porcupine.
///
class QmlPorcupine : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString pvAccessKey READ pvAccessKey WRITE setPvAccessKey NOTIFY pvAccessKeyChanged)
    Q_PROPERTY(QString pvModelPath READ pvModelPath WRITE setPvModelPath NOTIFY pvModelPathChanged)
    Q_PROPERTY(QString pvKeyWordsDir READ pvKeyWordsDir WRITE setPvKeyWordsDir NOTIFY pvKeyWordsDirChanged)
    Q_PROPERTY(QStringListModel* keywords READ keywords CONSTANT)
    Q_PROPERTY(bool error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool engineReady READ engineReady  NOTIFY engineReadyChanged)
    Q_PROPERTY(int inputPacketSize READ inputPacketSize NOTIFY inputPacketSizeChanged)
    Q_PROPERTY(int pvFrameLength READ pvFrameLength CONSTANT);
    Q_PROPERTY(QString errorMsg READ errorMsg CONSTANT)

    Q_PROPERTY(qreal sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)

    QML_ELEMENT

public:
    explicit QmlPorcupine(QObject *parent = nullptr);
    ~QmlPorcupine();

    void setPvAccessKey(const QString& pvAccessKey);
    const QString& pvAccessKey() const;

    void setPvModelPath(const QString& pvModelPath);
    const QString& pvModelPath() const;

    void setPvKeyWordsDir(const QString &pvKeyWordsPath);
    const QString& pvKeyWordsDir() const;

    qreal sensitivity() const ;
    void setSensitivity(qreal sensitivity);

    QStringListModel* keywords() const;

    QString pvVersion() const;
    qint32 pvFrameLength() const;
    qint32 pvSampleRate() const;

    bool error() const ;
    const QString& errorMsg() const;

    bool engineReady() const ;

    int inputPacketSize() const ;
    void setInputPacketSize(int size);


    void classBegin() override;
    void componentComplete() override;

public slots:

    QString toNativePathSyntax(const QString &urlString);

    bool startListening();
    void stopListening();

signals:
    void pvAccessKeyChanged();
    void pvModelPathChanged();
    void pvKeyWordsDirChanged();
    void sensitivityChanged();
    void rmChanged();
    void inputPacketSizeChanged();
    void keyWordDetected(int keywordIndex);
    void errorChanged();
    void engineReadyChanged();
    void started();
    void stopped();
    void infoMessage(const QString& message);

private slots:
    void pvProcess();


private:
    void createKeywordsModel();
    void initPv();
    void removePv();
    void handleProcessError(const QString& errMsg);

    QString             m_pvAccessKey;
    QString             m_pvModelPath;
    QString             m_pvKeyWordsDir;
    QVector<QString>    m_pvKeyWordsFiles;
    qreal               m_sensitivity;
    int                 m_inputPacketSize;
    QStringListModel*   m_keywords;
    Porcupine*          m_porcupine;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QAudioInput*        m_audioEngine;
#else
    QAudioSource*       m_audioEngine;
#endif
    QAudioFormat        m_pvAudioFormat;
    QIODevice*          m_ioDevice;
    bool                m_error;
    bool                m_engineReady;
    QString             m_errorMsg;

};

#endif // QMLPORCUPINE_H
