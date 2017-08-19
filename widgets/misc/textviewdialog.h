#ifndef SYNCTHINGWIDGETS_TEXTVIEWDIALOG_H
#define SYNCTHINGWIDGETS_TEXTVIEWDIALOG_H

#include "../global.h"

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTextBrowser)

namespace Data {
struct SyncthingDir;
}

namespace QtGui {

class SYNCTHINGWIDGETS_EXPORT TextViewDialog : public QWidget {
    Q_OBJECT
public:
    TextViewDialog(const QString &title = QString(), QWidget *parent = nullptr);

    QTextBrowser *browser();
    static TextViewDialog *forDirectoryErrors(const Data::SyncthingDir &dir);

Q_SIGNALS:
    void reload();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QTextBrowser *m_browser;
};

inline QTextBrowser *TextViewDialog::browser()
{
    return m_browser;
}
}

#endif // SYNCTHINGWIDGETS_TEXTVIEWDIALOG_H
