#ifndef INVESTSPHERE_REGISTERWIDGET_H
#define INVESTSPHERE_REGISTERWIDGET_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "NotificationBanner.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Modern two-panel registration widget for InvestSphere.
 * Provides client-side validation, PBKDF2 password security,
 * and duplicate email enforcement.
 */
class RegisterWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);

    void resetForm();

signals:
    void registerSuccess(int userId);
    void switchToLogin();

private slots:
    void handleRegister();
    void togglePasswordVisibility();
    void toggleConfirmPasswordVisibility();

private:
    void setupUi();
    QWidget* createLeftBrandingPanel();
    QWidget* createRightFormPanel();

    QLineEdit *m_nameInput{nullptr};
    QLineEdit *m_emailInput{nullptr};
    QLineEdit *m_passwordInput{nullptr};
    QLineEdit *m_confirmPasswordInput{nullptr};
    QPushButton *m_showPasswordBtn{nullptr};
    QPushButton *m_showConfirmPasswordBtn{nullptr};
    QPushButton *m_registerButton{nullptr};
    NotificationBanner *m_errorBanner{nullptr};
    bool m_passwordVisible{false};
    bool m_confirmPasswordVisible{false};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_REGISTERWIDGET_H
