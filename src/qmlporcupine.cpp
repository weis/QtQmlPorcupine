#include <QLibrary>
#include <QTextStream>
#include <QAudioInput>
#include <QBuffer>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QAudioInput>
#else
#include <QAudioSource>
#endif
#include <QAudioFormat>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QAudioInput>
#include <QAudioDeviceInfo>
#else
#include <QMediaDevices>
#include <QAudioDevice>
#endif
#include <QIODevice>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QTimer>
#include <QDirIterator>

#include "porcupine.h"
#include "qmlporcupine.h"

#undef PV_KEYWORDS_PATH

QVector<QString> AudioErrMsg =
{
    QStringLiteral("No Errors"),
    QStringLiteral("An error occurred opening the audio device"),
    QStringLiteral("An error occurred during read/write of audio device"),
    QStringLiteral("Audio data is not being fed to the audio device at a fast enough rate"),
    QStringLiteral("A non-recoverable error has occurred, the audio device is not usable at this time")
};

QVector<QString> pvGetKeywordsFiles(const QString& keywordsDir = QString())
{
    QVector<QString> kwfiles;
    QString pvKwDir;
#ifdef PV_KEYWORDS_PATH
    pvKwDir = keywordsDir.isEmpty() ? PV_KEYWORDS_PATH : keywordsDir;
#else
    pvKwDir = keywordsDir;
#endif
    QDirIterator keyFilesIt(pvKwDir, {"*.ppn"}, QDir::Files);

    while (keyFilesIt.hasNext())
        kwfiles.append(QDir::toNativeSeparators(keyFilesIt.next()));

    return kwfiles;
}

QString pvGetKeywordsDir(const QString& keywordsDir = QString())
{
    QString _keywordsDir;
#ifdef PV_KEYWORDS_PATH
    _keywordsDir = keywordsDir.isEmpty() ? PV_KEYWORDS_PATH : keywordsDir;
#else
    _keywordsDir = keywordsDir;
#endif
    return QDir::toNativeSeparators(_keywordsDir);
}

QString pvGetModelFile(const QString& modelFile)
{
    return QDir::toNativeSeparators(modelFile);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
QAudioFormat preferredAudioFormat(const qint32 sampleRate)
{
    QAudioFormat audioFormat;
    audioFormat.setSampleRate(sampleRate);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleSize(16);
    audioFormat.setSampleType(QAudioFormat::SignedInt);
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setCodec("audio/pcm");
    return audioFormat;
}
#else
static QAudioFormat preferredAudioFormat(const qint32 sampleRate)
{
    QAudioFormat audioFormat;
    audioFormat.setSampleRate(sampleRate);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    return audioFormat;
}
#endif


QmlPorcupine::QmlPorcupine(QObject* parent)
    : QObject{parent}
    , m_sensitivity(0.5)
    , m_inputPacketSize(0)
    , m_keywords(new QStringListModel(this))
    , m_porcupine(nullptr)
    , m_audioEngine(nullptr)
    , m_ioDevice(nullptr)
    , m_error(false)
    , m_engineReady(false)
{
}

QmlPorcupine::~QmlPorcupine()
{
    delete m_porcupine;
}

void QmlPorcupine::classBegin()
{
}

void QmlPorcupine::componentComplete()
{
    QString infoMsg = QString("Using Qt Version %1").arg((QT_VERSION_STR));
    qInfo("%s", qPrintable(infoMsg));
    emit infoMessage(infoMsg);
}


void QmlPorcupine::setPvAccessKey(const QString& pvAccessKey)
{
    if (m_pvAccessKey != pvAccessKey)
    {
        m_pvAccessKey = pvAccessKey;
        emit pvAccessKeyChanged();
    }
}
const QString& QmlPorcupine::pvAccessKey() const
{
    return m_pvAccessKey;
}

void QmlPorcupine::setPvModelPath(const QString& pvModelPath)
{
    if (m_pvModelPath != pvModelPath)
    {
        m_pvModelPath = pvModelPath;
        emit pvModelPathChanged();
    }
}
const QString& QmlPorcupine::pvModelPath() const
{
    return m_pvModelPath;
}

void QmlPorcupine::setPvKeyWordsDir(const QString& pvKeyWordsDir)
{
    if (m_pvKeyWordsDir != pvKeyWordsDir)
    {
        m_pvKeyWordsDir = pvKeyWordsDir;
        m_pvKeyWordsFiles = pvGetKeywordsFiles(pvKeyWordsDir);
        createKeywordsModel();
        emit pvKeyWordsDirChanged();
    }
}
const QString& QmlPorcupine::pvKeyWordsDir() const
{
    return m_pvKeyWordsDir;
}

qreal QmlPorcupine::sensitivity() const
{
    return m_sensitivity;
}

void QmlPorcupine::setSensitivity(qreal sensitivity)
{
    if (m_sensitivity != sensitivity)
    {
        m_sensitivity = sensitivity;
        emit sensitivityChanged();
    }
}

QStringListModel* QmlPorcupine::keywords() const
{
    return m_keywords;
}

QString QmlPorcupine::pvVersion() const
{
    return m_porcupine == nullptr ? QString() : m_porcupine->version();
}

qint32 QmlPorcupine::pvFrameLength() const
{
    return m_porcupine == nullptr ? -1 : m_porcupine->frameLength();
}

qint32 QmlPorcupine::pvSampleRate() const
{
    return m_porcupine == nullptr ? -1 : m_porcupine->sampleRate();
}

int QmlPorcupine::inputPacketSize() const
{
    return m_inputPacketSize / 2;
}

void QmlPorcupine::setInputPacketSize(int size)
{
    if (m_inputPacketSize != size)
    {
        m_inputPacketSize = size;
        emit inputPacketSizeChanged();
    }
}


bool QmlPorcupine::error() const
{
    return m_error;
}

bool QmlPorcupine::engineReady() const
{
    return m_engineReady;
}


const QString& QmlPorcupine::errorMsg() const
{
    return m_errorMsg;
}

void QmlPorcupine::initPv()
{
    m_engineReady = false;
    QVector<qreal> sensitivities = QVector<qreal>(m_pvKeyWordsFiles.size(), m_sensitivity);
    m_porcupine = Porcupine::create(m_pvAccessKey,
                                    m_pvKeyWordsFiles,
                                    m_pvModelPath,
                                    sensitivities,
                                    &m_errorMsg);
    m_error = m_porcupine == nullptr;

    if (m_error)
    {
        emit errorChanged();
    }
    else
    {
        QString message = QString("Porcubine V%1 successfull initialized").arg(m_porcupine->version());
        emit infoMessage(message);
        m_pvAudioFormat = preferredAudioFormat(m_porcupine->sampleRate());
        createKeywordsModel();
    }

    m_engineReady = !m_error;
    emit engineReadyChanged();
    return;
}

void QmlPorcupine::removePv()
{
    delete m_porcupine;
    m_porcupine = nullptr;
    m_engineReady = false;
    return;
}

bool QmlPorcupine::startListening()
{
    initPv();

    if (!m_engineReady)
        return false;

    m_errorMsg = QString();
    m_error = false;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QAudioDeviceInfo device = QAudioDeviceInfo::defaultInputDevice();
    QString deviceName = device.deviceName();
#else
    QAudioDevice device(QMediaDevices::defaultAudioInput());
    QString deviceName = device.description();
#endif

    if (m_audioEngine == nullptr)
    {
        // Get default audio input device
        QString message = QString("Using audio input device: %1").arg(deviceName);
        emit infoMessage(message);
        qInfo("%s", qPrintable(message));

        // Check whether the format is supported
        if (!device.isFormatSupported(m_pvAudioFormat))
        {
            m_error = true;
            m_errorMsg = "Default audio format is not supported.";
            qCritical("%s", qPrintable(m_errorMsg));
            emit errorChanged();
            return false;
        }

        // Instantiate QAudioInput with the settings
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        m_audioEngine = new QAudioInput(device, m_pvAudioFormat, this);
#else
        m_audioEngine = new QAudioSource(device, m_pvAudioFormat, this);
#endif
    }

    // Start receiving data from audio input
    m_ioDevice = m_audioEngine->start();
    m_error = m_ioDevice == nullptr;

    if (m_error)
    {
        m_errorMsg = QString("Cannot start device \"%0\": %1.")
                     .arg(deviceName, AudioErrMsg[m_audioEngine->error()]);
        qCritical("%s", qPrintable(m_errorMsg));
        m_engineReady = false;
        emit engineReadyChanged();
        emit errorChanged();
        return false;
    }

    QObject::connect(m_ioDevice, &QIODevice::readyRead, this, &QmlPorcupine::pvProcess);
    m_porcupine->enable(true);
    emit started();
    return true;
}

void QmlPorcupine::stopListening()
{
    m_audioEngine->stop();

    if (m_ioDevice != nullptr)
        QObject::disconnect(m_ioDevice, nullptr, nullptr, nullptr);

    m_ioDevice = nullptr;
    m_engineReady = false;
    emit engineReadyChanged();
    removePv();
    emit infoMessage("Porcubine Instance deleted.");
    emit stopped();
}

void QmlPorcupine::handleProcessError(const QString& errMsg)
{
    m_error = true;
    m_errorMsg = errMsg;
    qCritical("%s", qPrintable(m_errorMsg));
    // In our test environment we stop further processing
    QTimer* ti = new QTimer();
    ti->setSingleShot(true);
    ti->setInterval(20);
    QObject::connect(ti, &QTimer::timeout, ti, [ti, this]()
    {
        stopListening();
        delete ti;
    });
    ti->start();
    emit errorChanged();
}

void QmlPorcupine::pvProcess()
{
    if (m_error || m_audioEngine == nullptr)
        return;

    if (m_audioEngine->error() != QAudio::NoError)
    {
        handleProcessError(AudioErrMsg[m_audioEngine->error()]);
        return;
    }

    QString errMsg;
    int keywordsIndex;
    QByteArray audioData = m_ioDevice->readAll();
    setInputPacketSize(audioData.size());
    bool success = m_porcupine->process(keywordsIndex, audioData.constData(), audioData.size(), &errMsg);

    if (success && keywordsIndex < 0)
        return;

    if (success && keywordsIndex >= 0)
    {
        emit keyWordDetected(keywordsIndex);
    }
    else
    {
        handleProcessError(errMsg);
    }
}


void QmlPorcupine::createKeywordsModel()
{
    QStringList keywords;

    for (const auto& file : m_pvKeyWordsFiles)
    {
        QFileInfo fi(file);
        keywords.append(fi.baseName().split('_').at(0));
    }

    m_keywords->setStringList(keywords);
}

QString QmlPorcupine::toNativePathSyntax(const QString& urlString)
{
    const QUrl url(urlString);

    if (url.isLocalFile())
    {
        return (QDir::toNativeSeparators(url.toLocalFile()));
    }
    else
    {
        return urlString;
    }
}


