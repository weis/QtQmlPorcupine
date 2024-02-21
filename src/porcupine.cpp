#include <QLibrary>
#include <QFileInfo>
#include <QIODevice>

#include "porcupine_fn.hpp"
#include "porcupine.h"

///
/// \brief Porcupine Wake Word Qt API
///
Porcupine::Porcupine(void* pvInstance, QLibrary* pvLib)
    : m_pvInstance(pvInstance)
    , m_pvLib(pvLib)
    , m_pvEnabled(false)
    , m_pvBytesFrameSize(bytesFrameLength())
{
}

Porcupine::~Porcupine()
{
    if (m_pvInstance != nullptr)
        PV::pv_porcupine_delete_func(static_cast<pv_porcupine_t*>(m_pvInstance));

    delete m_pvLib;
}

static Porcupine* errPorcupino(const QString& message,
                               QString* errMsg = nullptr)
{
    qCritical("%s", qPrintable(message));

    if (errMsg != nullptr)
        *errMsg = message;

    return nullptr;
}


static Porcupine* errPorcupino(const QString& message,
                               QLibrary* pvLib,
                               QString* errMsg = nullptr)
{
    errPorcupino(message, errMsg);
    delete pvLib;
    return nullptr;
}

///
/// \brief Creates an instance of the Porcupine wake word engine.
/// \param accessKey AccessKey obtained from Picovoice Console (https://picovoice.ai/console/)
/// \param keywordPaths A list of absolute paths to keyword model files.
/// \param modelPath Absolute path to file containing model parameters.
/// \param sensitivities  A list of sensitivity values for each keyword.
/// A higher sensitivity value lowers miss rate at the cost of increased
/// false alarm rate. A sensitivity value should be within [0, 1].
/// \param errMsg optional output of error messages.
/// \return A Porcupine instance pointer on success or a nullptr on error.
///
Porcupine* Porcupine::create(const QString& accessKey,
                             const QVector<QString>& keywordPaths,
                             const QString& modelPath,
                             const QVector<qreal>& sensitivities,
                             QString* errMsg)
{
    QString message;
    //
    // 1. Initialize porcubine library
    //
    QString pvLibPath = PV::pvGetLibpath();
    QFileInfo fi(pvLibPath);

    if (!QLibrary::isLibrary(pvLibPath) || !fi.exists())
        return errPorcupino("Failed running Porcubine, no accessible runtime library.", errMsg);

    QLibrary* pvLib = new QLibrary(pvLibPath);

    if (!PV::porcupine_fn_init(pvLib, &message))
        return errPorcupino(message, pvLib, errMsg);

    //
    // 2. Initialize wake word detection
    //

    if (accessKey.isEmpty())
        return errPorcupino("No accesskey provided to Porcupine", pvLib, errMsg);

    if (keywordPaths.isEmpty())
        return errPorcupino("No keyword file paths were provided to Porcupine", pvLib, errMsg);

    if (modelPath.isEmpty())
        return errPorcupino("No model file path was provided to Porcupine", pvLib, errMsg);

    if (!QFileInfo::exists(modelPath))
        return errPorcupino(QString("Couldn't find model file at \"%1\"").arg(modelPath), pvLib, errMsg);

    QVector<QByteArray> pvBuffer;
    std::vector<const char*> pvKeywordPaths;

    for (const auto& p : keywordPaths)
    {
        if (QFileInfo::exists(p))
        {
            QByteArray buffer = p.toUtf8();
            pvBuffer.append(buffer);
            pvKeywordPaths.push_back(buffer.constData());
        }
        else
            return errPorcupino(QString("Couldn't find keyword file at \"%1\"").arg(p), pvLib, errMsg);
    }

    std::vector<float> pv_sensitivities;

    if (sensitivities.isEmpty())
        pv_sensitivities = std::vector<float>(keywordPaths.size(), 0.5f);
    else
        for (const auto& sensitive : sensitivities)
            pv_sensitivities.push_back(static_cast<float>(sensitive));

    pv_porcupine_t* porcupine = NULL;
    pv_status_t porcupine_status = PV::pv_porcupine_init_func(
                                       accessKey.toUtf8().constData(),
                                       modelPath.toUtf8().constData(),
                                       (int32_t) pvKeywordPaths.size(),
                                       pvKeywordPaths.data(),
                                       pv_sensitivities.data(),
                                       &porcupine);
    bool success = porcupine_status == PV_STATUS_SUCCESS;
    void* pvInstance = nullptr;

    if (success)
    {
        pvInstance = (void*) porcupine;
        qInfo("Wake word engine Porcubine V%s successfull initialized.", PV::pv_porcupine_version_func());
    }
    else
        return errPorcupino(PV::getMessageDetail("porcupine_init", porcupine_status), pvLib, errMsg);

    //
    // 3. Initialize Porcubine class
    //
    return new Porcupine(pvInstance, pvLib);
}


///
/// \brief Gets the version string.
/// \return The version string.
///
QString Porcupine::version() const
{
    return QString(PV::pv_porcupine_version_func());
}

///
/// \brief Gets the number of 16bit audio samples per frame.
/// \return Number of audio samples per frame.
///
qint32 Porcupine::frameLength() const
{
    return PV::pv_porcupine_frame_length_func();
}

///
/// \brief Gets the number of bytes per frame.
/// \return Number of of bytes per frame.
///
qint32 Porcupine::bytesFrameLength() const
{
    return 2 * PV::pv_porcupine_frame_length_func();
}

///
/// \brief Gets for audio sample rate accepted by Porcupine.
/// \return Audio sample rate accepted by Porcupine.
///
qint32 Porcupine::sampleRate() const
{
    return PV::pv_sample_rate_func();
}

//
// Internal processes a frame of the incoming audio stream and emits the detection result.
//
bool Porcupine::processFrame(const int16_t* pcm, qint32* keywordIndex, QString* errMsg)
{
    pv_status_t porcupine_status = PV::pv_porcupine_process_func(
                                       static_cast<pv_porcupine_t*>(m_pvInstance),
                                       pcm,
                                       keywordIndex);
    bool success = porcupine_status == PV_STATUS_SUCCESS;

    if (success)
    {
        if (*keywordIndex >= 0)
            qInfo("Porcupine detected keyword, index = %d.", *keywordIndex);
    }
    else
    {
        QString message = PV::getMessageDetail("Processing porcubine audiodata", porcupine_status);
        qCritical("%s", qPrintable(message));

        if (errMsg != nullptr)
            *errMsg = message;
    }

    return success;
}

///
/// \brief Processes an incoming audio stream and returns the detection result.
/// \param keywordIndex Outputs a 0-based index of the first observed keyword.
/// If no keyword is detected then value of keywordIndex is -1.
/// \param audioData Raw audio data stream.
/// \param len Length in bytes of the data stream.
/// \param errMsg If not null, an optional output of error messages.
/// \return true on success otherwise false.
///
bool Porcupine::process(int& keywordIndex, const char* audioData, const int len, QString* errMsg)
{
    bool success = true;
    keywordIndex = -1;

    if (m_pvEnabled)
    {
        m_audioBuffer.append(audioData, len);

        if (m_audioBuffer.size() >= m_pvBytesFrameSize)
        {
            int bytesProcessed = 0;
            const char* ptr = m_audioBuffer.constData();
            const char* const end = ptr + m_audioBuffer.size() - m_pvBytesFrameSize + 1;

            while (ptr < end)
            {
                const int16_t* pcm = reinterpret_cast<const int16_t*>(ptr);
                int32_t keyword_index = -1;

                if ((success = processFrame(pcm, &keyword_index, errMsg)) && keyword_index >= 0)
                    keywordIndex = keyword_index;

                bytesProcessed += m_pvBytesFrameSize;

                // Break loop if a keyword is found or on error
                if (keyword_index >= 0 || !success)
                    break;

                ptr += m_pvBytesFrameSize;
            }

            // Remove processed audio data
            m_audioBuffer.remove(0, bytesProcessed);
        }
    }

    return success;
}

///
/// \brief Enables or disables processing of audio frames.
/// \param enable True if enable, otherwise false.
///
void  Porcupine::enable(bool enable)
{
    if (m_pvEnabled != enable)
    {
        m_audioBuffer.clear();
        m_pvEnabled = enable;
    }
}
