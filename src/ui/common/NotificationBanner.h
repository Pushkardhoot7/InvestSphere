#ifndef INVESTSPHERE_NOTIFICATIONBANNER_H
#define INVESTSPHERE_NOTIFICATIONBANNER_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QFrame>
#include <QLabel>
#include <QPushButton>

namespace InvestSphere {
namespace Ui {

/**
 * @brief Inline dismissable status message banner for notifications, alerts, and confirmations.
 */
class NotificationBanner : public QFrame {
    Q_OBJECT

public:
    enum class BannerType {
        Info,
        Success,
        Warning,
        Error
    };

    explicit NotificationBanner(const QString &message = QString(),
                                BannerType type = BannerType::Info,
                                bool dismissable = true,
                                QWidget *parent = nullptr);

    void showMessage(const QString &message, BannerType type = BannerType::Info);

signals:
    void dismissed();

private:
    QLabel *m_iconLabel{nullptr};
    QLabel *m_messageLabel{nullptr};
    QPushButton *m_closeButton{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_NOTIFICATIONBANNER_H
