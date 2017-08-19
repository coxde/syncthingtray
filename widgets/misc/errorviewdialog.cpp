#include "./errorviewdialog.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QStringBuilder>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <algorithm>
#include <limits>

using namespace std;
using namespace ChronoUtilities;
using namespace Data;

namespace QtGui {

ErrorViewDialog *ErrorViewDialog::s_instance = nullptr;
std::vector<InternalError> ErrorViewDialog::s_internalErrors;

ErrorViewDialog::ErrorViewDialog()
    : TextViewDialog(tr("Internal errors"))
    , m_request(tr("Request URL:"))
    , m_response(tr("Response:"))
    , m_statusLabel(new QLabel(this))
{
    if (!s_instance) {
        s_instance = this;
    }

    // add layout to show status and additional buttons
    auto *const buttonLayout = new QHBoxLayout(this);
    buttonLayout->setMargin(0);

    // add label for overall status
    QFont boldFont(m_statusLabel->font());
    boldFont.setBold(true);
    m_statusLabel->setFont(boldFont);
    buttonLayout->addWidget(m_statusLabel);
    updateStatusLabel();

    // add errors to text view
    for (const InternalError &error : s_internalErrors) {
        internalAddError(error);
    }

    // add a button for clearing errors
    if (!s_internalErrors.empty()) {
        auto *const clearButton = new QPushButton(this);
        clearButton->setText(tr("Clear errors"));
        clearButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));
        buttonLayout->setMargin(0);
        buttonLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        buttonLayout->addWidget(clearButton);
        connect(clearButton, &QPushButton::clicked, &ErrorViewDialog::clearErrors);
    }

    layout()->addItem(buttonLayout);
}

ErrorViewDialog::~ErrorViewDialog()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void ErrorViewDialog::addError(InternalError &&newError)
{
    s_internalErrors.emplace_back(newError);
    if (s_instance) {
        s_instance->internalAddError(s_internalErrors.back());
        s_instance->updateStatusLabel();
    }
}

void ErrorViewDialog::internalAddError(const InternalError &error)
{
    browser()->append(QString::fromUtf8(error.when.toString(DateTimeOutputFormat::DateAndTime, true).data()) % QChar(':') % QChar(' ') % error.message
        % QChar('\n') % m_request % QChar(' ') % error.url.toString(QUrl::FullyDecoded) % QChar('\n') % m_response % QChar('\n')
        % QString::fromLocal8Bit(error.response) % QChar('\n'));
}

void ErrorViewDialog::updateStatusLabel()
{
    m_statusLabel->setText(tr("%1 error(s) occured", nullptr, static_cast<int>(min<size_t>(s_internalErrors.size(), numeric_limits<int>::max())))
                               .arg(s_internalErrors.size()));
}

void ErrorViewDialog::clearErrors()
{
    s_internalErrors.clear();
    if (s_instance) {
        s_instance->updateStatusLabel();
        s_instance->browser()->clear();
    }
}
}
