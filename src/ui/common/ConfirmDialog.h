#ifndef INVESTSPHERE_CONFIRMDIALOG_H
#define INVESTSPHERE_CONFIRMDIALOG_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QDialog>
#include <QLabel>
#include <QPushButton>

namespace InvestSphere {
namespace Ui {

/**
 * @brief Professional modal confirmation dialog for actions such as logout, deletion, and resets.
 */
class ConfirmDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfirmDialog(const QString &title,
                          const QString &message,
                          const QString &confirmText = QStringLiteral("Confirm"),
                          const QString &cancelText = QStringLiteral("Cancel"),
                          bool isDestructive = false,
                          QWidget *parent = nullptr);

    static bool confirm(QWidget *parent,
                        const QString &title,
                        const QString &message,
                        const QString &confirmText = QStringLiteral("Confirm"),
                        bool isDestructive = false);
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_CONFIRMDIALOG_H
