#include "SettingsPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include "Theme.h"
#include "IconHelper.h"
#include "StatusBadge.h"
#include "SessionManager.h"
#include "AuthenticationService.h"

namespace InvestSphere {
namespace Ui {

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadCurrentUserData();

    // Listen to profile updates
    connect(&App::SessionManager::instance(), &App::SessionManager::profileUpdated, this, [this](const QString &name) {
        if (m_nameEdit) {
            m_nameEdit->setText(name);
        }
    });
}

void SettingsPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createProfileCard());
    layout->addWidget(createSecurityCard());
    layout->addWidget(createDatabaseStatusCard());
    layout->addWidget(createPreferencesCard());
    layout->addWidget(createAboutCard());

    layout->addStretch(1);
    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

void SettingsPage::loadCurrentUserData()
{
    auto &session = App::SessionManager::instance();
    if (m_nameEdit) {
        m_nameEdit->setText(QString::fromStdString(session.currentUserName()));
    }
    if (m_emailEdit) {
        m_emailEdit->setText(QString::fromStdString(session.currentUserEmail()));
    }
    if (m_createdAtLabel) {
        std::string created = session.currentCreatedAt();
        m_createdAtLabel->setText(QString::fromStdString(created.empty() ? "N/A" : created));
    }
}

QWidget* SettingsPage::createProfileCard()
{
    auto *card = new QFrame(this);
    card->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 12px; padding: 20px; }"));
    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(14);

    auto *title = new QLabel(QStringLiteral("User Profile & Investor Identity"), card);
    title->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 700; color: #0F172A;"));
    layout->addWidget(title);

    m_profileBanner = new NotificationBanner(QString(), NotificationBanner::BannerType::Success, true, card);
    m_profileBanner->hide();
    layout->addWidget(m_profileBanner);

    auto *formLayout = new QGridLayout();
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(12);

    auto *nameLbl = new QLabel(QStringLiteral("Full Name"), card);
    nameLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #374151;"));
    m_nameEdit = new QLineEdit(card);
    m_nameEdit->setStyleSheet(Theme::inputFieldStyle());
    m_nameEdit->setFixedHeight(36);

    auto *emailLbl = new QLabel(QStringLiteral("Email Address (Immutable Identity)"), card);
    emailLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #374151;"));
    m_emailEdit = new QLineEdit(card);
    m_emailEdit->setReadOnly(true);
    m_emailEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { background-color: #F8FAFC; color: #64748B; border: 1px solid #E2E8F0; border-radius: 8px; padding: 7px 12px; font-size: 13px; }"
    ));
    m_emailEdit->setFixedHeight(36);

    auto *createdLbl = new QLabel(QStringLiteral("Member Since:"), card);
    createdLbl->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748B; font-weight: 500;"));
    m_createdAtLabel = new QLabel(QStringLiteral("Active Session"), card);
    m_createdAtLabel->setStyleSheet(QStringLiteral("font-size: 12px; color: #0F172A; font-weight: 600;"));

    formLayout->addWidget(nameLbl, 0, 0);
    formLayout->addWidget(m_nameEdit, 0, 1);
    formLayout->addWidget(emailLbl, 1, 0);
    formLayout->addWidget(m_emailEdit, 1, 1);
    formLayout->addWidget(createdLbl, 2, 0);
    formLayout->addWidget(m_createdAtLabel, 2, 1);

    layout->addLayout(formLayout);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);

    auto *logoutBtn = new QPushButton(QStringLiteral("Log Out"), card);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(Theme::secondaryButtonStyle());
    connect(logoutBtn, &QPushButton::clicked, this, &SettingsPage::logoutRequested);
    btnRow->addWidget(logoutBtn);

    auto *saveBtn = new QPushButton(QStringLiteral("Save Profile Changes"), card);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(Theme::primaryButtonStyle());
    connect(saveBtn, &QPushButton::clicked, this, &SettingsPage::handleSaveProfile);
    btnRow->addWidget(saveBtn);

    layout->addLayout(btnRow);

    return card;
}

void SettingsPage::handleSaveProfile()
{
    QString newName = m_nameEdit->text().trimmed();
    int userId = App::SessionManager::instance().currentUserId();

    Services::AuthenticationService authService;
    auto res = authService.updateProfile(userId, newName.toStdString());

    if (res.success) {
        m_profileBanner->showMessage(QStringLiteral("Profile name updated successfully."), NotificationBanner::BannerType::Success);
    } else {
        m_profileBanner->showMessage(QString::fromStdString(res.message), NotificationBanner::BannerType::Error);
    }
}

QWidget* SettingsPage::createSecurityCard()
{
    auto *card = new QFrame(this);
    card->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 12px; padding: 20px; }"));
    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(14);

    auto *title = new QLabel(QStringLiteral("Account Security & Password Management"), card);
    title->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 700; color: #0F172A;"));
    layout->addWidget(title);

    m_passwordBanner = new NotificationBanner(QString(), NotificationBanner::BannerType::Info, true, card);
    m_passwordBanner->hide();
    layout->addWidget(m_passwordBanner);

    auto *formLayout = new QGridLayout();
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(12);

    auto *currPwdLbl = new QLabel(QStringLiteral("Current Password"), card);
    currPwdLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #374151;"));
    m_currentPasswordEdit = new QLineEdit(card);
    m_currentPasswordEdit->setEchoMode(QLineEdit::Password);
    m_currentPasswordEdit->setPlaceholderText(QStringLiteral("Verify current password"));
    m_currentPasswordEdit->setStyleSheet(Theme::inputFieldStyle());
    m_currentPasswordEdit->setFixedHeight(36);

    auto *newPwdLbl = new QLabel(QStringLiteral("New Password"), card);
    newPwdLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #374151;"));
    m_newPasswordEdit = new QLineEdit(card);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setPlaceholderText(QStringLiteral("Min. 8 characters, letter + number"));
    m_newPasswordEdit->setStyleSheet(Theme::inputFieldStyle());
    m_newPasswordEdit->setFixedHeight(36);

    auto *confirmPwdLbl = new QLabel(QStringLiteral("Confirm New Password"), card);
    confirmPwdLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #374151;"));
    m_confirmNewPasswordEdit = new QLineEdit(card);
    m_confirmNewPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmNewPasswordEdit->setPlaceholderText(QStringLiteral("Re-enter new password"));
    m_confirmNewPasswordEdit->setStyleSheet(Theme::inputFieldStyle());
    m_confirmNewPasswordEdit->setFixedHeight(36);

    formLayout->addWidget(currPwdLbl, 0, 0);
    formLayout->addWidget(m_currentPasswordEdit, 0, 1);
    formLayout->addWidget(newPwdLbl, 1, 0);
    formLayout->addWidget(m_newPasswordEdit, 1, 1);
    formLayout->addWidget(confirmPwdLbl, 2, 0);
    formLayout->addWidget(m_confirmNewPasswordEdit, 2, 1);

    layout->addLayout(formLayout);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    auto *changePwdBtn = new QPushButton(QStringLiteral("Update Password"), card);
    changePwdBtn->setCursor(Qt::PointingHandCursor);
    changePwdBtn->setStyleSheet(Theme::primaryButtonStyle());
    connect(changePwdBtn, &QPushButton::clicked, this, &SettingsPage::handleChangePassword);
    btnRow->addWidget(changePwdBtn);
    layout->addLayout(btnRow);

    return card;
}

void SettingsPage::handleChangePassword()
{
    QString currPwd = m_currentPasswordEdit->text();
    QString newPwd = m_newPasswordEdit->text();
    QString confirmPwd = m_confirmNewPasswordEdit->text();
    int userId = App::SessionManager::instance().currentUserId();

    Services::AuthenticationService authService;
    auto res = authService.changePassword(
        userId,
        currPwd.toStdString(),
        newPwd.toStdString(),
        confirmPwd.toStdString()
    );

    if (res.success) {
        m_passwordBanner->showMessage(QStringLiteral("Password changed successfully using PBKDF2-HMAC-SHA256."), NotificationBanner::BannerType::Success);
        m_currentPasswordEdit->clear();
        m_newPasswordEdit->clear();
        m_confirmNewPasswordEdit->clear();
    } else {
        m_passwordBanner->showMessage(QString::fromStdString(res.message), NotificationBanner::BannerType::Error);
    }
}

QWidget* SettingsPage::createDatabaseStatusCard()
{
    auto *card = new QFrame(this);
    card->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 12px; padding: 20px; }"));
    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(16);

    auto *topRow = new QHBoxLayout();
    auto *title = new QLabel(QStringLiteral("Relational Database & DBMS Diagnostics"), card);
    title->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 700; color: #0F172A;"));
    auto *badge = new StatusBadge(QStringLiteral("ONLINE & CONNECTED"), StatusBadge::BadgeVariant::Success, card);
    topRow->addWidget(title);
    topRow->addStretch(1);
    topRow->addWidget(badge);
    layout->addLayout(topRow);

    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(12);

    auto addDiagRow = [&](const QString &prop, const QString &val, int r, int c) {
        auto *p = new QLabel(prop, card);
        p->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748B; font-weight: 500;"));
        auto *v = new QLabel(val, card);
        v->setStyleSheet(QStringLiteral("font-size: 13px; color: #0F172A; font-weight: 600; font-family: 'Consolas', monospace;"));
        grid->addWidget(p, r, c * 2);
        grid->addWidget(v, r, c * 2 + 1);
    };

    addDiagRow(QStringLiteral("Database Engine:"), QStringLiteral("SQLite 3 (WAL Mode, Foreign Keys: ON)"), 0, 0);
    addDiagRow(QStringLiteral("Target DB File:"), QStringLiteral("investsphere.db"), 0, 1);
    addDiagRow(QStringLiteral("Relational Schema:"), QStringLiteral("3NF Architecture (18 Tables)"), 1, 0);
    addDiagRow(QStringLiteral("Authentication:"), QStringLiteral("PBKDF2-HMAC-SHA256 (100,000 Rounds)"), 1, 1);
    addDiagRow(QStringLiteral("User Isolation:"), QStringLiteral("Enforced via SessionManager & Foreign Keys"), 2, 0);
    addDiagRow(QStringLiteral("Schema Version:"), QStringLiteral("v1.0 (Phase 2 Blueprint)"), 2, 1);

    layout->addLayout(grid);
    return card;
}

QWidget* SettingsPage::createPreferencesCard()
{
    auto *card = new QFrame(this);
    card->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 12px; padding: 20px; }"));
    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("System Display & Currency Preferences"), card);
    title->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 700; color: #0F172A;"));
    layout->addWidget(title);

    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(12);

    auto *currLbl = new QLabel(QStringLiteral("Primary Currency:"), card);
    currLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #374151;"));
    m_currencyCombo = new QComboBox(card);
    m_currencyCombo->addItems({QStringLiteral("₹ INR — Indian Rupee"), QStringLiteral("$ USD — US Dollar"), QStringLiteral("€ EUR — Euro"), QStringLiteral("£ GBP — British Pound")});
    m_currencyCombo->setStyleSheet(Theme::inputFieldStyle());
    m_currencyCombo->setFixedHeight(36);

    auto *numLbl = new QLabel(QStringLiteral("Number Format:"), card);
    numLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #374151;"));
    m_numberSystemCombo = new QComboBox(card);
    m_numberSystemCombo->addItems({QStringLiteral("Indian System (Lakhs & Crores, e.g. 10,00,000)"), QStringLiteral("Western System (Millions & Billions, e.g. 1,000,000)")});
    m_numberSystemCombo->setStyleSheet(Theme::inputFieldStyle());
    m_numberSystemCombo->setFixedHeight(36);

    grid->addWidget(currLbl, 0, 0);
    grid->addWidget(m_currencyCombo, 0, 1);
    grid->addWidget(numLbl, 1, 0);
    grid->addWidget(m_numberSystemCombo, 1, 1);

    layout->addLayout(grid);
    return card;
}

QWidget* SettingsPage::createAboutCard()
{
    auto *card = new QFrame(this);
    card->setStyleSheet(QStringLiteral("QFrame { background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 12px; padding: 20px; }"));
    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(8);

    auto *title = new QLabel(QStringLiteral("InvestSphere — Smart Investment Management System"), card);
    title->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 700; color: #0F172A;"));
    auto *tagline = new QLabel(QStringLiteral("\"One Platform. Every Investment. Smarter Decisions.\""), card);
    tagline->setStyleSheet(QStringLiteral("font-size: 13px; color: #2563EB; font-style: italic;"));
    auto *desc = new QLabel(
        QStringLiteral("Academic DBMS Course Project — Native C++17 Desktop Architecture.\n"
                       "Developed using Qt 6 Widgets, SQLite 3NF Schema with ACID transactions, "
                       "B-Tree index optimization, and high-performance financial calculation algorithms.\n"
                       "Version 0.4.0 | Phase 5 Authentication & Session Management"),
        card
    );
    desc->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748B; line-height: 1.5;"));

    layout->addWidget(title);
    layout->addWidget(tagline);
    layout->addWidget(desc);
    return card;
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
