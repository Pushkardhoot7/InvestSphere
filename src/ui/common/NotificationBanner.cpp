#include "NotificationBanner.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QHBoxLayout>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

NotificationBanner::NotificationBanner(const QString &message,
                                       BannerType type,
                                       bool dismissable,
                                       QWidget *parent)
    : QFrame(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(10);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_iconLabel);

    m_messageLabel = new QLabel(message, this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet(QStringLiteral("font-family: %1; font-size: 13px; font-weight: 500;")
        .arg(Theme::fontFamily()));
    layout->addWidget(m_messageLabel, 1);

    if (dismissable) {
        m_closeButton = new QPushButton(QStringLiteral("✕"), this);
        m_closeButton->setFixedSize(20, 20);
        m_closeButton->setCursor(Qt::PointingHandCursor);
        m_closeButton->setStyleSheet(QStringLiteral(
            "QPushButton { border: none; background: transparent; color: #64748B; font-size: 11px; font-weight: 700; }"
            "QPushButton:hover { color: #0F172A; }"
        ));
        connect(m_closeButton, &QPushButton::clicked, this, [this]() {
            hide();
            emit dismissed();
        });
        layout->addWidget(m_closeButton);
    }

    if (!message.isEmpty()) {
        showMessage(message, type);
    } else {
        hide();
    }
}

void NotificationBanner::showMessage(const QString &message, BannerType type)
{
    m_messageLabel->setText(message);

    QString bgColor;
    QString borderColor;
    QString textColor;
    IconHelper::IconType iconType = IconHelper::IconType::InfoCircle;
    QColor iconColor;

    switch (type) {
    case BannerType::Success:
        bgColor = QStringLiteral("#ECFDF5");
        borderColor = QStringLiteral("#A7F3D0");
        textColor = QStringLiteral("#065F46");
        iconType = IconHelper::IconType::Checkmark;
        iconColor = QColor(16, 185, 129);
        break;
    case BannerType::Warning:
        bgColor = QStringLiteral("#FFFBEB");
        borderColor = QStringLiteral("#FDE68A");
        textColor = QStringLiteral("#92400E");
        iconType = IconHelper::IconType::AlertCircle;
        iconColor = QColor(217, 119, 6);
        break;
    case BannerType::Error:
        bgColor = QStringLiteral("#FEF2F2");
        borderColor = QStringLiteral("#FECACA");
        textColor = QStringLiteral("#991B1B");
        iconType = IconHelper::IconType::AlertCircle;
        iconColor = QColor(220, 38, 38);
        break;
    case BannerType::Info:
    default:
        bgColor = QStringLiteral("#EFF6FF");
        borderColor = QStringLiteral("#BFDBFE");
        textColor = QStringLiteral("#1E40AF");
        iconType = IconHelper::IconType::InfoCircle;
        iconColor = QColor(37, 99, 235);
        break;
    }

    m_iconLabel->setPixmap(IconHelper::getPixmap(iconType, iconColor, 18));
    setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: 8px;"
        "}"
        "QLabel {"
        "   color: %3;"
        "}"
    ).arg(bgColor).arg(borderColor).arg(textColor));

    show();
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
