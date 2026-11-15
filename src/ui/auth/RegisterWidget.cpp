#include "RegisterWidget.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include "Theme.h"
#include "IconHelper.h"
#include "AuthenticationService.h"

namespace InvestSphere {
namespace Ui {

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void RegisterWidget::setupUi()
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Left Panel (Branding & Account Benefits)
    rootLayout->addWidget(createLeftBrandingPanel(), 4);

    // Right Panel (Registration Form)
    rootLayout->addWidget(createRightFormPanel(), 5);
}

QWidget* RegisterWidget::createLeftBrandingPanel()
{
    auto *panel = new QFrame(this);
    panel->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: %1;"
        "   border-right: 1px solid #1F2937;"
        "}"
    ).arg(Theme::ColorPrimarySidebar));

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(48, 56, 48, 56);
    layout->setSpacing(24);

    // Brand Logo & Title Row
    auto *brandRow = new QHBoxLayout();
    brandRow->setSpacing(12);

    auto *logoLabel = new QLabel(panel);
    logoLabel->setFixedSize(48, 48);
    logoLabel->setPixmap(IconHelper::getPixmap(IconHelper::IconType::BrandLogo, QColor(37, 99, 235), 44));
    brandRow->addWidget(logoLabel);

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    auto *brandTitle = new QLabel(QStringLiteral("InvestSphere"), panel);
    brandTitle->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 24px; font-weight: 800; color: #FFFFFF; letter-spacing: 0.5px;"
    ).arg(Theme::fontFamily()));

    auto *brandSubtitle = new QLabel(QStringLiteral("Smart Investment Management"), panel);
    brandSubtitle->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 12px; font-weight: 600; color: #60A5FA; letter-spacing: 0.5px;"
    ).arg(Theme::fontFamily()));

    titleLayout->addWidget(brandTitle);
    titleLayout->addWidget(brandSubtitle);
    brandRow->addLayout(titleLayout);
    brandRow->addStretch(1);
    layout->addLayout(brandRow);

    // Security Guarantee Callout
    auto *securityCard = new QFrame(panel);
    securityCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #1E293B;"
        "   border: 1px solid #334155;"
        "   border-radius: 10px;"
        "   padding: 16px;"
        "}"
    ));
    auto *sLayout = new QVBoxLayout(securityCard);
    sLayout->setSpacing(8);

    auto *sHeader = new QLabel(QStringLiteral("Institutional Security Standard"), securityCard);
    sHeader->setStyleSheet(QStringLiteral("color: #38BDF8; font-size: 13px; font-weight: 700;"));
    auto *sBody = new QLabel(
        QStringLiteral("Your passwords are never stored in plaintext. InvestSphere uses PBKDF2-HMAC-SHA256 "
                       "with 100,000 rounds of computation and 128-bit cryptographic salts."),
        securityCard
    );
    sBody->setWordWrap(true);
    sBody->setStyleSheet(QStringLiteral("color: #94A3B8; font-size: 12px; line-height: 1.4;"));

    sLayout->addWidget(sHeader);
    sLayout->addWidget(sBody);
    layout->addWidget(securityCard);

    // Benefits list
    auto *benefitsHeader = new QLabel(QStringLiteral("ACCOUNT BENEFITS"), panel);
    benefitsHeader->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 11px; font-weight: 700; color: #64748B; letter-spacing: 1px;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(benefitsHeader);

    auto addBenefit = [&](const QString &title, IconHelper::IconType icon) {
        auto *row = new QHBoxLayout();
        row->setSpacing(12);

        auto *ico = new QLabel(panel);
        ico->setFixedSize(22, 22);
        ico->setAlignment(Qt::AlignCenter);
        ico->setPixmap(IconHelper::getPixmap(icon, QColor(96, 165, 250), 16));

        auto *lbl = new QLabel(title, panel);
        lbl->setStyleSheet(QStringLiteral(
            "font-family: %1; font-size: 12px; color: #E2E8F0; font-weight: 500;"
        ).arg(Theme::fontFamily()));

        row->addWidget(ico);
        row->addWidget(lbl, 1);
        layout->addLayout(row);
    };

    addBenefit(QStringLiteral("Dedicated Multi-Asset Investment Ledger"), IconHelper::IconType::Portfolio);
    addBenefit(QStringLiteral("Comprehensive Tax Statements (STCG & LTCG)"), IconHelper::IconType::Reports);
    addBenefit(QStringLiteral("Deterministic Inflows & Maturity Timeline"), IconHelper::IconType::MaturityTracker);
    addBenefit(QStringLiteral("Zero Third-Party Cloud Data Transmission"), IconHelper::IconType::Checkmark);

    layout->addStretch(1);

    auto *footer = new QLabel(QStringLiteral("InvestSphere v0.4.0 | User Security & Data Isolation"), panel);
    footer->setStyleSheet(QStringLiteral("font-size: 11px; color: #475569;"));
    layout->addWidget(footer);

    return panel;
}

QWidget* RegisterWidget::createRightFormPanel()
{
    auto *panel = new QFrame(this);
    panel->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; }"));

    auto *outerLayout = new QVBoxLayout(panel);
    outerLayout->setContentsMargins(56, 40, 56, 40);
    outerLayout->setAlignment(Qt::AlignCenter);

    auto *formContainer = new QWidget(panel);
    formContainer->setMaximumWidth(420);
    auto *layout = new QVBoxLayout(formContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    // Heading
    auto *heading = new QLabel(QStringLiteral("Create your Account"), formContainer);
    heading->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 24px; font-weight: 700; color: #0F172A;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(heading);

    auto *subHeading = new QLabel(QStringLiteral("Get started with smart portfolio management in seconds."), formContainer);
    subHeading->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 13px; color: #64748B;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(subHeading);

    // Inline Error Banner
    m_errorBanner = new NotificationBanner(QString(), NotificationBanner::BannerType::Error, true, formContainer);
    m_errorBanner->hide();
    layout->addWidget(m_errorBanner);

    // Full Name
    auto *nameLbl = new QLabel(QStringLiteral("Full Name"), formContainer);
    nameLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 12px; color: #1F2937;"));
    layout->addWidget(nameLbl);

    m_nameInput = new QLineEdit(formContainer);
    m_nameInput->setPlaceholderText(QStringLiteral("Dr. Alexander Wright"));
    m_nameInput->setFixedHeight(38);
    m_nameInput->setStyleSheet(Theme::inputFieldStyle());
    layout->addWidget(m_nameInput);

    // Email
    auto *emailLbl = new QLabel(QStringLiteral("Email Address"), formContainer);
    emailLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 12px; color: #1F2937;"));
    layout->addWidget(emailLbl);

    m_emailInput = new QLineEdit(formContainer);
    m_emailInput->setPlaceholderText(QStringLiteral("alex.wright@investsphere.test"));
    m_emailInput->setFixedHeight(38);
    m_emailInput->setStyleSheet(Theme::inputFieldStyle());
    layout->addWidget(m_emailInput);

    // Password + Show/Hide
    auto *pwdLbl = new QLabel(QStringLiteral("Password (min. 8 characters, letter + number)"), formContainer);
    pwdLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 12px; color: #1F2937;"));
    layout->addWidget(pwdLbl);

    auto *pwdContainer = new QWidget(formContainer);
    auto *pwdLayout = new QHBoxLayout(pwdContainer);
    pwdLayout->setContentsMargins(0, 0, 0, 0);
    pwdLayout->setSpacing(6);

    m_passwordInput = new QLineEdit(pwdContainer);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setPlaceholderText(QStringLiteral("Create a secure password"));
    m_passwordInput->setFixedHeight(38);
    m_passwordInput->setStyleSheet(Theme::inputFieldStyle());
    pwdLayout->addWidget(m_passwordInput, 1);

    m_showPasswordBtn = new QPushButton(QStringLiteral("Show"), pwdContainer);
    m_showPasswordBtn->setFixedHeight(38);
    m_showPasswordBtn->setCursor(Qt::PointingHandCursor);
    m_showPasswordBtn->setStyleSheet(Theme::secondaryButtonStyle());
    connect(m_showPasswordBtn, &QPushButton::clicked, this, &RegisterWidget::togglePasswordVisibility);
    pwdLayout->addWidget(m_showPasswordBtn);

    layout->addWidget(pwdContainer);

    // Confirm Password + Show/Hide
    auto *confirmLbl = new QLabel(QStringLiteral("Confirm Password"), formContainer);
    confirmLbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 12px; color: #1F2937;"));
    layout->addWidget(confirmLbl);

    auto *confirmContainer = new QWidget(formContainer);
    auto *confirmLayout = new QHBoxLayout(confirmContainer);
    confirmLayout->setContentsMargins(0, 0, 0, 0);
    confirmLayout->setSpacing(6);

    m_confirmPasswordInput = new QLineEdit(confirmContainer);
    m_confirmPasswordInput->setEchoMode(QLineEdit::Password);
    m_confirmPasswordInput->setPlaceholderText(QStringLiteral("Re-enter password"));
    m_confirmPasswordInput->setFixedHeight(38);
    m_confirmPasswordInput->setStyleSheet(Theme::inputFieldStyle());
    confirmLayout->addWidget(m_confirmPasswordInput, 1);

    m_showConfirmPasswordBtn = new QPushButton(QStringLiteral("Show"), confirmContainer);
    m_showConfirmPasswordBtn->setFixedHeight(38);
    m_showConfirmPasswordBtn->setCursor(Qt::PointingHandCursor);
    m_showConfirmPasswordBtn->setStyleSheet(Theme::secondaryButtonStyle());
    connect(m_showConfirmPasswordBtn, &QPushButton::clicked, this, &RegisterWidget::toggleConfirmPasswordVisibility);
    confirmLayout->addWidget(m_showConfirmPasswordBtn);

    layout->addWidget(confirmContainer);

    layout->addSpacing(6);

    // Register Button
    m_registerButton = new QPushButton(QStringLiteral("Create Account"), formContainer);
    m_registerButton->setCursor(Qt::PointingHandCursor);
    m_registerButton->setFixedHeight(42);
    m_registerButton->setStyleSheet(Theme::primaryButtonStyle());
    connect(m_registerButton, &QPushButton::clicked, this, &RegisterWidget::handleRegister);
    connect(m_confirmPasswordInput, &QLineEdit::returnPressed, this, &RegisterWidget::handleRegister);
    layout->addWidget(m_registerButton);

    layout->addSpacing(4);

    // Switch to Login Link
    auto *loginRow = new QHBoxLayout();
    loginRow->setAlignment(Qt::AlignCenter);
    loginRow->setSpacing(4);

    auto *haveAccountLabel = new QLabel(QStringLiteral("Already have an account?"), formContainer);
    haveAccountLabel->setStyleSheet(QStringLiteral("font-size: 13px; color: #64748B;"));

    auto *loginLink = new QPushButton(QStringLiteral("Sign In"), formContainer);
    loginLink->setCursor(Qt::PointingHandCursor);
    loginLink->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; background: transparent; color: #2563EB; font-weight: 600; font-size: 13px; padding: 0; }"
        "QPushButton:hover { text-decoration: underline; color: #1D4ED8; }"
    ));
    connect(loginLink, &QPushButton::clicked, this, &RegisterWidget::switchToLogin);

    loginRow->addWidget(haveAccountLabel);
    loginRow->addWidget(loginLink);
    layout->addLayout(loginRow);

    outerLayout->addWidget(formContainer);
    return panel;
}

void RegisterWidget::togglePasswordVisibility()
{
    m_passwordVisible = !m_passwordVisible;
    if (m_passwordVisible) {
        m_passwordInput->setEchoMode(QLineEdit::Normal);
        m_showPasswordBtn->setText(QStringLiteral("Hide"));
    } else {
        m_passwordInput->setEchoMode(QLineEdit::Password);
        m_showPasswordBtn->setText(QStringLiteral("Show"));
    }
}

void RegisterWidget::toggleConfirmPasswordVisibility()
{
    m_confirmPasswordVisible = !m_confirmPasswordVisible;
    if (m_confirmPasswordVisible) {
        m_confirmPasswordInput->setEchoMode(QLineEdit::Normal);
        m_showConfirmPasswordBtn->setText(QStringLiteral("Hide"));
    } else {
        m_confirmPasswordInput->setEchoMode(QLineEdit::Password);
        m_showConfirmPasswordBtn->setText(QStringLiteral("Show"));
    }
}

void RegisterWidget::resetForm()
{
    if (m_passwordInput) m_passwordInput->clear();
    if (m_confirmPasswordInput) m_confirmPasswordInput->clear();
    if (m_errorBanner) m_errorBanner->hide();
    if (m_registerButton) {
        m_registerButton->setEnabled(true);
        m_registerButton->setText(QStringLiteral("Create Account"));
    }
}

void RegisterWidget::handleRegister()
{
    m_errorBanner->hide();

    QString name = m_nameInput->text().trimmed();
    QString email = m_emailInput->text().trimmed();
    QString password = m_passwordInput->text();
    QString confirmPassword = m_confirmPasswordInput->text();

    m_registerButton->setEnabled(false);
    m_registerButton->setText(QStringLiteral("Creating Account..."));

    Services::AuthenticationService authService;
    auto result = authService.registerUser(
        name.toStdString(),
        email.toStdString(),
        password.toStdString(),
        confirmPassword.toStdString()
    );

    m_registerButton->setEnabled(true);
    m_registerButton->setText(QStringLiteral("Create Account"));

    if (result.success) {
        emit registerSuccess(result.userId);
    } else {
        m_errorBanner->showMessage(
            QString::fromStdString(result.message),
            NotificationBanner::BannerType::Error
        );
        // Preserve form contents, just clear password fields if there was a password issue
        if (result.code == Services::AuthResult::ErrorCode::PasswordMismatch ||
            result.code == Services::AuthResult::ErrorCode::WeakPassword) {
            m_passwordInput->selectAll();
            m_passwordInput->setFocus();
        }
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
