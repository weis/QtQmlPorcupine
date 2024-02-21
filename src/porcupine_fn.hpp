#ifndef PORCUPINE_FN_HPP
#define PORCUPINE_FN_HPP

#include <QLibrary>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QCoreApplication>
#include <pv_porcupine.h>

namespace PV
{

#if defined(_WIN32)
QString pvLibName = QStringLiteral("libpv_porcupine.dll");
#endif

#if defined(__APPLE__)
QString pvLibName = QStringLiteral("libpv_porcupine.dylib");
#endif

typedef const char* (*pv_status_to_string_t)(pv_status_t);
typedef int32_t (*pv_sample_rate_t)();
typedef pv_status_t (*pv_porcupine_init_t)(const char*, const char*, int32_t, const char* const*, const float*, pv_porcupine_t**);
typedef void (*pv_porcupine_delete_t)(pv_porcupine_t*);
typedef pv_status_t (*pv_porcupine_process_t)(pv_porcupine_t*, const int16_t*, int32_t*);
typedef int32_t (*pv_porcupine_frame_length_t)();
typedef const char* (*pv_porcupine_version_t)();
typedef pv_status_t (*pv_get_error_stack_t)(char***, int32_t*);
typedef void (*pv_free_error_stack_t)(char**);

pv_status_to_string_t pv_status_to_string_func;
pv_sample_rate_t pv_sample_rate_func;
pv_porcupine_init_t pv_porcupine_init_func;
pv_porcupine_delete_t pv_porcupine_delete_func;
pv_porcupine_process_t  pv_porcupine_process_func;
pv_porcupine_frame_length_t pv_porcupine_frame_length_func;
pv_porcupine_version_t pv_porcupine_version_func;
pv_get_error_stack_t  pv_get_error_stack_func;
pv_free_error_stack_t pv_free_error_stack_func;

QString pvGetLibpath()
{
    QString appPath = QCoreApplication::applicationDirPath();
    return QDir::toNativeSeparators(QString("%1/%2").arg(appPath, pvLibName));
}

bool errLoadingPorcupino(const QString& fn, QString* errMsg = nullptr)
{
    QString message = QString("Error access wake word engine Porcupine: Failed to load \"%1\".").arg(fn);

    if (errMsg != nullptr)
        *errMsg = message;

    return false;
}

bool  porcupine_fn_init(QLibrary* lib, QString* errMsg = nullptr)
{
    if (!lib->load())
        return errLoadingPorcupino(QStringLiteral("library"), errMsg);

    pv_status_to_string_func = (pv_status_to_string_t) lib->resolve("pv_status_to_string");

    if (!pv_status_to_string_func)
        return errLoadingPorcupino(QStringLiteral("pv_status_to_string"), errMsg);

    pv_sample_rate_func = (pv_sample_rate_t) lib->resolve("pv_sample_rate");

    if (!pv_sample_rate_func)
        return errLoadingPorcupino(QStringLiteral("pv_sample_rate"), errMsg);

    pv_porcupine_init_func = (pv_porcupine_init_t) lib->resolve("pv_porcupine_init");

    if (!pv_porcupine_init_func)
        return errLoadingPorcupino(QStringLiteral("pv_porcupine_init"), errMsg);

    pv_porcupine_delete_func = (pv_porcupine_delete_t) lib->resolve("pv_porcupine_delete");

    if (!pv_porcupine_delete_func)
        return errLoadingPorcupino(QStringLiteral("pv_porcupine_delete"), errMsg);

    pv_porcupine_process_func = (pv_porcupine_process_t) lib->resolve("pv_porcupine_process");

    if (!pv_porcupine_process_func)
        return errLoadingPorcupino(QStringLiteral("pv_porcupine_process"), errMsg);

    pv_porcupine_frame_length_func = (pv_porcupine_frame_length_t) lib->resolve("pv_porcupine_frame_length");

    if (!pv_porcupine_frame_length_func)
        return errLoadingPorcupino(QStringLiteral("pv_porcupine_frame_length"), errMsg);

    pv_porcupine_version_func = (pv_porcupine_version_t) lib->resolve("pv_porcupine_version");

    if (!pv_porcupine_version_func)
        return errLoadingPorcupino(QStringLiteral("pv_porcupine_version"), errMsg);

    pv_get_error_stack_func = (pv_get_error_stack_t) lib->resolve("pv_get_error_stack");

    if (!pv_get_error_stack_func)
        return errLoadingPorcupino(QStringLiteral("pv_get_error_stack"), errMsg);

    pv_free_error_stack_func = (pv_free_error_stack_t) lib->resolve("pv_free_error_stack");

    if (!pv_free_error_stack_func)
        return errLoadingPorcupino(QStringLiteral("pv_free_error_stack"), errMsg);

    return true;
}

QString getMessageDetail(const QString& pvFunc, pv_status_t porcupine_status)
{
    QString msg;
    QTextStream inpErr(&msg);
    char** message_stack = NULL;
    int32_t message_stack_depth = 0;
    pv_status_t error_status = PV_STATUS_RUNTIME_ERROR;
    inpErr <<  QString("'%0' failed with '%1'").arg(pvFunc, pv_status_to_string_func(porcupine_status));
    error_status = pv_get_error_stack_func(&message_stack, &message_stack_depth);

    if (error_status != PV_STATUS_SUCCESS)
    {
        inpErr << QString(".\nUnable to get Porcupine error state with '%1'.\n").arg(pv_status_to_string_func(error_status));
    }
    else if (message_stack_depth > 0)
    {
        inpErr << ":\n";

        for (int32_t i = 0; i < message_stack_depth; i++)
        {
            inpErr << QString("  [%1] %2\n").arg(QString::number(i), message_stack[i]);
        }

        pv_free_error_stack_func(message_stack);
    }
    else
    {
        inpErr <<  ".\n";
    }

    return msg;
}

} // namespace PV

#endif // PORCUPINE_FN_HPP
