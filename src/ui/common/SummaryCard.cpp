#include "SummaryCard.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include "Theme.h"

namespace InvestSphere {
namespace Ui {

SummaryCard::SummaryCard(const QString &title,
                         const QString &initialValue,
                         const QString &deltaText,
                         TrendType trend,
                         IconHelper::IconType icon,
                         QWidget *parent)
    : QFrame(parent)
{
    setupUi(icon);
    setTitle(title);
    setValue(initialValue);
    setTrend(deltaText, trend);
}

void SummaryCard::setupUi(IconHelper::IconType icon)
{
    setObjectName(QStringLiteral("SummaryCard"));
    setStyleSheet(QStringLiteral(
        "QFrame#SummaryCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 12px;"
        "   padding: 16px 20px;"
        "}"
        "QFrame#SummaryCard:hover {"
        "   border: 1px solid #CBD5E1;"
        "}"
    ));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    mainLayout->setSpacing(8);

    // Top Row: Title + Icon
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 13px; font-weight: 600; color: #64748B; text-transform: uppercase; letter-spacing: 0.5px;"
    ).arg(Theme::fontFamily()));

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet(QStringLiteral(
        "background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 8px;"
    ));
    m_iconLabel->setPixmap(IconHelper::getPixmap(icon, QColor(37, 99, 235), 18));

    topRow->addWidget(m_titleLabel);
    topRow->addStretch(1);
    topRow->addWidget(m_iconLabel);
    mainLayout->addLayout(topRow);

    // Value Row
    m_valueLabel = new QLabel(this);
    m_valueLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 24px; font-weight: 700; color: #0F172A;"
    ).arg(Theme::fontFamily()));
    mainLayout->addWidget(m_valueLabel);

    // Bottom Row: Trend Badge + Subtitle
    auto *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(8);

    m_trendBadge = new QLabel(this);
    m_trendBadge->setFixedHeight(22);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 11px; color: #94A3B8;"
    ).arg(Theme::fontFamily()));

    bottomRow->addWidget(m_trendBadge);
    bottomRow->addWidget(m_subtitleLabel);
    bottomRow->addStretch(1);
    mainLayout->addLayout(bottomRow);
}

void SummaryCard::setTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void SummaryCard::setValue(const QString &value)
{
    if (m_valueLabel) {
        m_valueLabel->setText(value);
    }
}

void SummaryCard::setTrend(const QString &deltaText, TrendType trend)
{
    updateTrendBadge(deltaText, trend);
}

void SummaryCard::setSubtitle(const QString &subtitle)
{
    if (m_subtitleLabel) {
        m_subtitleLabel->setText(subtitle);
    }
}

void SummaryCard::setIcon(IconHelper::IconType icon)
{
    if (m_iconLabel) {
        m_iconLabel->setPixmap(IconHelper::getPixmap(icon, QColor(37, 99, 235), 18));
    }
}

void SummaryCard::updateTrendBadge(const QString &text, TrendType trend)
{
    if (!m_trendBadge) return;

    QString bgColor;
    QString textColor;
    QString prefix;

    switch (trend) {
    case TrendType::Positive:
        bgColor = QStringLiteral("#DCFCE7");
        textColor = QStringLiteral("#16A34A");
        prefix = QStringLiteral("▲ ");
        break;
    case TrendType::Negative:
        bgColor = QStringLiteral("#FEE2E2");
        textColor = QStringLiteral("#DC2626");
        prefix = QStringLiteral("▼ ");
        break;
    case TrendType::Neutral:
    default:
        bgColor = QStringLiteral("#F1F5F9");
        textColor = QStringLiteral("#64748B");
        prefix = QStringLiteral("• ");
        break;
    }

    m_trendBadge->setText(prefix + text);
    m_trendBadge->setStyleSheet(QStringLiteral(
        "background-color: %1; color: %2; font-family: %3; font-size: 11px; font-weight: 700; "
        "padding: 2px 8px; border-radius: 4px;"
    ).arg(bgColor).arg(textColor).arg(Theme::fontFamily()));
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
