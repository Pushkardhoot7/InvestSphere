#ifndef INVESTSPHERE_LOGINWIDGET_H
#define INVESTSPHERE_LOGINWIDGET_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include "NotificationBanner.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Modern two-panel login widget for InvestSphere.
 * Left panel displays brand identity, tagline, and architectural features.
 * Right panel provides credentials input, show/hide password toggle, and error handling.
 */
class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);

    void resetForm();
    void setRememberedEmail(const QString &email);

signals:
    void loginSuccess(int userId);
    void switchToRegister();

private slots:
    void handleLogin();
    void togglePasswordVisibility();

private:
    void setupUi();
    QWidget* createLeftBrandingPanel();
    QWidget* createRightFormPanel();

    QLineEdit *m_emailInput{nullptr};
    QLineEdit *m_passwordInput{nullptr};
    QPushButton *m_showPasswordBtn{nullptr};
    QCheckBox *m_rememberEmailCheck{nullptr};
    QPushButton *m_loginButton{nullptr};
    NotificationBanner *m_errorBanner{nullptr};
    bool m_passwordVisible{false};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_LOGINWIDGET_H
