#ifndef INVESTSPHERE_AUTHWINDOW_H
#define INVESTSPHERE_AUTHWINDOW_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QStackedWidget>
#include "LoginWidget.h"
#include "RegisterWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Window container coordinating Login and Registration flows.
 * Handles transitions between login and registration panels and emits authenticated signal.
 */
class AuthWindow : public QWidget {
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);

    void showLogin(const QString &rememberedEmail = QString());
    void showRegister();

signals:
    void authenticated(int userId);

private:
    void setupUi();

    QStackedWidget *m_stackedWidget{nullptr};
    LoginWidget *m_loginWidget{nullptr};
    RegisterWidget *m_registerWidget{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_AUTHWINDOW_H
