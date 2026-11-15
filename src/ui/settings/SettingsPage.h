#ifndef INVESTSPHERE_SETTINGSPAGE_H
#define INVESTSPHERE_SETTINGSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include "NotificationBanner.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Application Settings, Database Diagnostics, User Profile, and Security page.
 */
class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

    void loadCurrentUserData();

signals:
    void logoutRequested();

private slots:
    void handleSaveProfile();
    void handleChangePassword();

private:
    void setupUi();
    QWidget* createProfileCard();
    QWidget* createSecurityCard();
    QWidget* createDatabaseStatusCard();
    QWidget* createPreferencesCard();
    QWidget* createAboutCard();

    QLineEdit *m_nameEdit{nullptr};
    QLineEdit *m_emailEdit{nullptr};
    QLabel *m_createdAtLabel{nullptr};
    NotificationBanner *m_profileBanner{nullptr};

    QLineEdit *m_currentPasswordEdit{nullptr};
    QLineEdit *m_newPasswordEdit{nullptr};
    QLineEdit *m_confirmNewPasswordEdit{nullptr};
    NotificationBanner *m_passwordBanner{nullptr};

    QComboBox *m_currencyCombo{nullptr};
    QComboBox *m_numberSystemCombo{nullptr};
    QComboBox *m_dateFormatCombo{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_SETTINGSPAGE_H
