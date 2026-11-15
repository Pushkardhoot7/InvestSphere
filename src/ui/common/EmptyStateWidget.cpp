#include "EmptyStateWidget.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include "Theme.h"

namespace InvestSphere {
namespace Ui {

EmptyStateWidget::EmptyStateWidget(const QString &title,
                                   const QString &description,
                                   const QString &actionText,
                                   IconHelper::IconType icon,
                                   QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 48, 32, 48);
    layout->setSpacing(12);
    layout->setAlignment(Qt::AlignCenter);

    // Icon Circle
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(56, 56);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet(QStringLiteral(
        "background-color: #EFF6FF; border: 1px solid #DBEAFE; border-radius: 28px;"
    ));
    m_iconLabel->setPixmap(IconHelper::getPixmap(icon, QColor(37, 99, 235), 28));
    layout->addWidget(m_iconLabel, 0, Qt::AlignCenter);

    // Title
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 16px; font-weight: 700; color: #1F2937;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

    // Description
    m_descriptionLabel = new QLabel(description, this);
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setMaximumWidth(450);
    m_descriptionLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 13px; color: #64748B; line-height: 1.4;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(m_descriptionLabel, 0, Qt::AlignCenter);

    // Action Button
    m_actionButton = new QPushButton(this);
    m_actionButton->setCursor(Qt::PointingHandCursor);
    m_actionButton->setStyleSheet(Theme::primaryButtonStyle());
    if (!actionText.isEmpty()) {
        m_actionButton->setText(actionText);
        m_actionButton->setVisible(true);
    } else {
        m_actionButton->setVisible(false);
    }

    connect(m_actionButton, &QPushButton::clicked, this, &EmptyStateWidget::actionClicked);
    layout->addSpacing(4);
    layout->addWidget(m_actionButton, 0, Qt::AlignCenter);
}

void EmptyStateWidget::setTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void EmptyStateWidget::setDescription(const QString &description)
{
    if (m_descriptionLabel) {
        m_descriptionLabel->setText(description);
    }
}

void EmptyStateWidget::setActionText(const QString &actionText)
{
    if (m_actionButton) {
        m_actionButton->setText(actionText);
        m_actionButton->setVisible(!actionText.isEmpty());
    }
}

void EmptyStateWidget::setIcon(IconHelper::IconType icon)
{
    if (m_iconLabel) {
        m_iconLabel->setPixmap(IconHelper::getPixmap(icon, QColor(37, 99, 235), 28));
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
