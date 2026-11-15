#include "AuthWindow.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include "Theme.h"

namespace InvestSphere {
namespace Ui {

AuthWindow::AuthWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void AuthWindow::setupUi()
{
    setWindowTitle(QStringLiteral("InvestSphere — Smart Investment Management | Authentication"));
    setMinimumSize(960, 640);
    resize(1080, 700);

    // Apply global stylesheet
    setStyleSheet(Theme::getApplicationStyleSheet());

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_stackedWidget = new QStackedWidget(this);

    m_loginWidget = new LoginWidget(m_stackedWidget);
    m_registerWidget = new RegisterWidget(m_stackedWidget);

    m_stackedWidget->addWidget(m_loginWidget);    // Index 0
    m_stackedWidget->addWidget(m_registerWidget); // Index 1

    connect(m_loginWidget, &LoginWidget::switchToRegister, this, &AuthWindow::showRegister);
    connect(m_registerWidget, &RegisterWidget::switchToLogin, this, [this]() {
        showLogin();
    });

    connect(m_loginWidget, &LoginWidget::loginSuccess, this, &AuthWindow::authenticated);
    connect(m_registerWidget, &RegisterWidget::registerSuccess, this, &AuthWindow::authenticated);

    rootLayout->addWidget(m_stackedWidget);

    showLogin();
}

void AuthWindow::showLogin(const QString &rememberedEmail)
{
    m_loginWidget->resetForm();
    if (!rememberedEmail.isEmpty()) {
        m_loginWidget->setRememberedEmail(rememberedEmail);
    }
    m_stackedWidget->setCurrentIndex(0);
}

void AuthWindow::showRegister()
{
    m_registerWidget->resetForm();
    m_stackedWidget->setCurrentIndex(1);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
