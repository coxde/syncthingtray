#include "./syncthinglauncher.h"

#include <QtConcurrentRun>

#include <algorithm>
#include <limits>

using namespace std;
using namespace std::placeholders;
using namespace ChronoUtilities;

namespace Data {

SyncthingLauncher *SyncthingLauncher::s_mainInstance = nullptr;

SyncthingLauncher::SyncthingLauncher(QObject *parent)
    : QObject(parent)
    , m_useLibSyncthing(false)
{
    connect(&m_process, &SyncthingProcess::readyRead, this, &SyncthingLauncher::handleProcessReadyRead);
    connect(&m_process, static_cast<void (SyncthingProcess::*)(int exitCode, QProcess::ExitStatus exitStatus)>(&SyncthingProcess::finished), this,
        &SyncthingLauncher::handleProcessFinished);
    connect(&m_process, &SyncthingProcess::confirmKill, this, &SyncthingLauncher::confirmKill);
}

bool SyncthingLauncher::isLibSyncthingAvailable()
{
#ifdef SYNCTHING_WIDGETS_USE_LIBSYNCTHING
    return true;
#else
    return false;
#endif
}

/*!
 * \brief Launches a Syncthing instance using the specified \a arguments.
 * \remarks To use the internal library, leave \a program empty. Otherwise it must be the path the external Syncthing executable.
 */
void SyncthingLauncher::launch(const QString &program, const QStringList &arguments)
{
    if (isRunning()) {
        return;
    }
    m_manuallyStopped = false;
    if (!program.isEmpty()) {
        m_process.startSyncthing(program, arguments);
    } else {
        vector<string> utf8Arguments{ "-no-restart", "-no-browser" };
        utf8Arguments.reserve(utf8Arguments.size() + static_cast<size_t>(arguments.size()));
        for (const auto &arg : arguments) {
            const auto utf8Data(arg.toUtf8());
            utf8Arguments.emplace_back(utf8Data.data(), utf8Data.size());
        }
        m_future = QtConcurrent::run(
            this, static_cast<void (SyncthingLauncher::*)(const std::vector<std::string> &)>(&SyncthingLauncher::runLibSyncthing), utf8Arguments);
    }
}

/*!
 * \brief Launches a Syncthing instance using the internal library with the specified \a runtimeOptions.
 */
void SyncthingLauncher::launch(const LibSyncthing::RuntimeOptions &runtimeOptions)
{
    if (isRunning()) {
        return;
    }
    m_manuallyStopped = false;
    m_future = QtConcurrent::run(
        this, static_cast<void (SyncthingLauncher::*)(const LibSyncthing::RuntimeOptions &)>(&SyncthingLauncher::runLibSyncthing), runtimeOptions);
}

void SyncthingLauncher::terminate()
{
    if (m_process.isRunning()) {
        m_manuallyStopped = true;
        m_process.stopSyncthing();
    } else if (m_future.isRunning()) {
        m_manuallyStopped = true;
#ifdef SYNCTHING_WIDGETS_USE_LIBSYNCTHING
        LibSyncthing::stopSyncthing();
#endif
    }
}

void SyncthingLauncher::kill()
{
    if (m_process.isRunning()) {
        m_manuallyStopped = true;
        m_process.stopSyncthing();
    } else if (m_future.isRunning()) {
        m_manuallyStopped = true;
#ifdef SYNCTHING_WIDGETS_USE_LIBSYNCTHING
        // FIXME: any change to try harder?
        LibSyncthing::stopSyncthing();
#endif
    }
}

void SyncthingLauncher::handleProcessReadyRead()
{
    emit outputAvailable(m_process.readAll());
}

void SyncthingLauncher::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit runningChanged(false);
    emit exited(exitCode, exitStatus);
}

static const char *const logLevelStrings[] = {
    "[DEBUG]   ",
    "[VERBOSE] ",
    "[INFO]    ",
    "[WARNING] ",
    "[FATAL]   ",
};

void SyncthingLauncher::handleLoggingCallback(LibSyncthing::LogLevel level, const char *message, size_t messageSize)
{
#ifdef SYNCTHING_WIDGETS_USE_LIBSYNCTHING
    if (level < LibSyncthing::LogLevel::Info) {
        return;
    }
    QByteArray messageData;
    messageSize = min<size_t>(numeric_limits<int>::max() - 20, messageSize);
    messageData.reserve(static_cast<int>(messageSize) + 20);
    messageData.append(logLevelStrings[static_cast<int>(level)]);
    messageData.append(message, static_cast<int>(messageSize));
    messageData.append('\n');

    emit outputAvailable(move(messageData));
#else
    VAR_UNUSED(level)
    VAR_UNUSED(message)
    VAR_UNUSED(messageSize)
#endif
}

void SyncthingLauncher::runLibSyncthing(const LibSyncthing::RuntimeOptions &runtimeOptions)
{
#ifdef SYNCTHING_WIDGETS_USE_LIBSYNCTHING
    LibSyncthing::setLoggingCallback(bind(&SyncthingLauncher::handleLoggingCallback, this, _1, _2, _3));
    const auto exitCode = LibSyncthing::runSyncthing(runtimeOptions);
    emit exited(static_cast<int>(exitCode), exitCode == 0 ? QProcess::NormalExit : QProcess::CrashExit);
#else
    VAR_UNUSED(runtimeOptions)
    emit outputAvailable("libsyncthing support not enabled");
    emit exited(-1, QProcess::CrashExit);
#endif
}

void SyncthingLauncher::runLibSyncthing(const std::vector<string> &arguments)
{
#ifdef SYNCTHING_WIDGETS_USE_LIBSYNCTHING
    LibSyncthing::setLoggingCallback(bind(&SyncthingLauncher::handleLoggingCallback, this, _1, _2, _3));
    const auto exitCode = LibSyncthing::runSyncthing(arguments);
    emit exited(static_cast<int>(exitCode), exitCode == 0 ? QProcess::NormalExit : QProcess::CrashExit);
#else
    VAR_UNUSED(arguments)
    emit outputAvailable("libsyncthing support not enabled");
    emit exited(-1, QProcess::CrashExit);
#endif
}

SyncthingLauncher &syncthingLauncher()
{
    static SyncthingLauncher launcher;
    return launcher;
}

} // namespace Data