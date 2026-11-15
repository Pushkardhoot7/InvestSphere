#include "SectionHeader.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include "Theme.h"

namespace InvestSphere {
namespace Ui {

SectionHeader::SectionHeader(const QString &title,
                             const QString &subtitle,
                             const QString &actionText,
                             QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 4, 0, 8);
    layout->setSpacing(12);

    auto *textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;"
    ).arg(Theme::fontFamily()));
    textLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 12px; color: #64748B;"
    ).arg(Theme::fontFamily()));
    if (!subtitle.isEmpty()) {
        m_subtitleLabel->setText(subtitle);
        m_subtitleLabel->setVisible(true);
    } else {
        m_subtitleLabel->setVisible(false);
    }
    textLayout->addWidget(m_subtitleLabel);

    layout->addLayout(textLayout);
    layout->addStretch(1);

    m_actionButton = new QPushButton(this);
    m_actionButton->setCursor(Qt::PointingHandCursor);
    m_actionButton->setStyleSheet(Theme::secondaryButtonStyle());
    if (!actionText.isEmpty()) {
        m_actionButton->setText(actionText);
        m_actionButton->setVisible(true);
    } else {
        m_actionButton->setVisible(false);
    }

    connect(m_actionButton, &QPushButton::clicked, this, &SectionHeader::actionClicked);
    layout->addWidget(m_actionButton);
}

void SectionHeader::setTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void SectionHeader::setSubtitle(const QString &subtitle)
{
    if (m_subtitleLabel) {
        m_subtitleLabel->setText(subtitle);
        m_subtitleLabel->setVisible(!subtitle.isEmpty());
    }
}

void SectionHeader::setActionButton(const QString &actionText)
{
    if (m_actionButton) {
        m_actionButton->setText(actionText);
        m_actionButton->setVisible(!actionText.isEmpty());
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
