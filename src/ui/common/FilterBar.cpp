#include "FilterBar.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include "Theme.h"

namespace InvestSphere {
namespace Ui {

FilterBar::FilterBar(const QString &searchPlaceholder,
                     const QStringList &categories,
                     const QStringList &statuses,
                     const QString &actionButtonText,
                     QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 12);
    layout->setSpacing(12);

    // Search Field
    m_searchField = new SearchField(searchPlaceholder, this);
    connect(m_searchField, &QLineEdit::textChanged, this, &FilterBar::searchChanged);
    connect(m_searchField, &QLineEdit::textChanged, this, &FilterBar::filterChanged);
    layout->addWidget(m_searchField, 2);

    // Category ComboBox
    if (!categories.isEmpty()) {
        m_categoryCombo = new QComboBox(this);
        m_categoryCombo->setFixedHeight(36);
        m_categoryCombo->addItems(categories);
        m_categoryCombo->setStyleSheet(Theme::inputFieldStyle());
        connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &FilterBar::filterChanged);
        layout->addWidget(m_categoryCombo, 1);
    }

    // Status ComboBox
    if (!statuses.isEmpty()) {
        m_statusCombo = new QComboBox(this);
        m_statusCombo->setFixedHeight(36);
        m_statusCombo->addItems(statuses);
        m_statusCombo->setStyleSheet(Theme::inputFieldStyle());
        connect(m_statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &FilterBar::filterChanged);
        layout->addWidget(m_statusCombo, 1);
    }

    // Action Button
    if (!actionButtonText.isEmpty()) {
        m_actionButton = new QPushButton(actionButtonText, this);
        m_actionButton->setFixedHeight(36);
        m_actionButton->setCursor(Qt::PointingHandCursor);
        m_actionButton->setStyleSheet(Theme::primaryButtonStyle());
        connect(m_actionButton, &QPushButton::clicked, this, &FilterBar::actionClicked);
        layout->addWidget(m_actionButton);
    }
}

QString FilterBar::searchText() const
{
    return m_searchField ? m_searchField->text().trimmed() : QString();
}

QString FilterBar::selectedCategory() const
{
    return m_categoryCombo ? m_categoryCombo->currentText() : QString();
}

QString FilterBar::selectedStatus() const
{
    return m_statusCombo ? m_statusCombo->currentText() : QString();
}

void FilterBar::setActionButtonVisible(bool visible)
{
    if (m_actionButton) {
        m_actionButton->setVisible(visible);
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
