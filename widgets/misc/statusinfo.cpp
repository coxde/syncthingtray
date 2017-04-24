#include "./statusinfo.h"

#include "../../connector/syncthingconnection.h"
#include "../../model/syncthingicons.h"

#include <QIcon>
#include <QCoreApplication>

using namespace Data;

namespace QtGui {

StatusInfo::StatusInfo() :
    m_statusText(QCoreApplication::translate("QtGui::StatusInfo", "Initializing ...")),
    m_statusIcon(&statusIcons().disconnected)
{}

void StatusInfo::update(const SyncthingConnection &connection)
{
    switch(connection.status()) {
    case SyncthingStatus::Disconnected:
        if(connection.autoReconnectInterval() > 0) {
            m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Not connected to Syncthing - trying to reconnect every %1 ms")
                    .arg(connection.autoReconnectInterval());
        } else {
            m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Not connected to Syncthing");
        }
        m_statusIcon = &statusIcons().disconnected;
        break;
    case SyncthingStatus::Reconnecting:
        m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Reconnecting ...");
        m_statusIcon = &statusIcons().disconnected;
        break;
    default:
        if(connection.hasOutOfSyncDirs()) {
            switch(connection.status()) {
            case SyncthingStatus::Synchronizing:
                m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Synchronization is ongoing but at least one directory is out of sync");
                m_statusIcon = &statusIcons().errorSync;
                break;
            default:
                m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "At least one directory is out of sync");
                m_statusIcon = &statusIcons().error;
            }
        } else if(connection.hasUnreadNotifications()) {
            m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Notifications available");
            m_statusIcon = &statusIcons().notify;
        } else {
            switch(connection.status()) {
            case SyncthingStatus::Idle:
                m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Syncthing is idling");
                m_statusIcon = &statusIcons().idling;
                break;
            case SyncthingStatus::Scanning:
                m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Syncthing is scanning");
                m_statusIcon = &statusIcons().scanninig;
                break;
            case SyncthingStatus::Paused:
                m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "At least one device is paused");
                m_statusIcon = &statusIcons().pause;
                break;
            case SyncthingStatus::Synchronizing:
                m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Synchronization is ongoing");
                m_statusIcon = &statusIcons().sync;
                break;
            default:
                m_statusText = QCoreApplication::translate("QtGui::StatusInfo", "Status is unknown");
                m_statusIcon = &statusIcons().disconnected;
            }
        }
    }
}

}
