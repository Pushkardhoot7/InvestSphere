#ifndef INVESTSPHERE_EMPTYSTATEWIDGET_H
#define INVESTSPHERE_EMPTYSTATEWIDGET_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Professional Empty State container with icon, message, and call-to-action button.
 */
class EmptyStateWidget : public QWidget {
    Q_OBJECT

public:
    explicit EmptyStateWidget(const QString &title = QStringLiteral("Your portfolio is ready to begin."),
                              const QString &description = QStringLiteral("Add your first investment to start tracking your portfolio."),
                              const QString &actionText = QStringLiteral("+ Add First Investment"),
                              IconHelper::IconType icon = IconHelper::IconType::Portfolio,
                              QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setDescription(const QString &description);
    void setActionText(const QString &actionText);
    void setIcon(IconHelper::IconType icon);

signals:
    void actionClicked();

private:
    QLabel *m_iconLabel{nullptr};
    QLabel *m_titleLabel{nullptr};
    QLabel *m_descriptionLabel{nullptr};
    QPushButton *m_actionButton{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_EMPTYSTATEWIDGET_H
