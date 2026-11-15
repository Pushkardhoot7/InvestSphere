#ifndef INVESTSPHERE_FILTERBAR_H
#define INVESTSPHERE_FILTERBAR_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include "SearchField.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Reusable filter bar combining search, category dropdown, status dropdown, and action button.
 */
class FilterBar : public QWidget {
    Q_OBJECT

public:
    explicit FilterBar(const QString &searchPlaceholder = QStringLiteral("Filter records..."),
                       const QStringList &categories = QStringList(),
                       const QStringList &statuses = QStringList(),
                       const QString &actionButtonText = QString(),
                       QWidget *parent = nullptr);

    QString searchText() const;
    QString selectedCategory() const;
    QString selectedStatus() const;
    void setActionButtonVisible(bool visible);

signals:
    void searchChanged(const QString &text);
    void filterChanged();
    void actionClicked();

private:
    SearchField *m_searchField{nullptr};
    QComboBox *m_categoryCombo{nullptr};
    QComboBox *m_statusCombo{nullptr};
    QPushButton *m_actionButton{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_FILTERBAR_H
