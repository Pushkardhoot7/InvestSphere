#include "LoginWidget.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QAction>
#include "Theme.h"
#include "IconHelper.h"
#include "AuthenticationService.h"

namespace InvestSphere {
namespace Ui {

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void LoginWidget::setupUi()
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Left Panel (Branding, Tagline, Highlights)
    rootLayout->addWidget(createLeftBrandingPanel(), 4);

    // Right Panel (Login Form)
    rootLayout->addWidget(createRightFormPanel(), 5);
}

QWidget* LoginWidget::createLeftBrandingPanel()
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

    // Tagline Callout
    auto *taglineLabel = new QLabel(QStringLiteral("\"One Platform. Every Investment. Smarter Decisions.\""), panel);
    taglineLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 15px; font-style: italic; color: #93C5FD; line-height: 1.4;"
    ).arg(Theme::fontFamily()));
    taglineLabel->setWordWrap(true);
    layout->addWidget(taglineLabel);

    // Description
    auto *descLabel = new QLabel(
        QStringLiteral("InvestSphere provides an institutional-grade platform to track multi-asset portfolios, "
                       "simulate systematic retirement wealth compounding, and analyze capital performance with complete data sovereignty."),
        panel
    );
    descLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 13px; color: #94A3B8; line-height: 1.5;"
    ).arg(Theme::fontFamily()));
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    layout->addSpacing(8);

    // Feature Highlights
    auto *featuresLabel = new QLabel(QStringLiteral("PLATFORM HIGHLIGHTS"), panel);
    featuresLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 11px; font-weight: 700; color: #64748B; letter-spacing: 1px;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(featuresLabel);

    auto addHighlight = [&](const QString &title, IconHelper::IconType icon) {
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

    addHighlight(QStringLiteral("Consolidated Multi-Asset Portfolio Tracking"), IconHelper::IconType::Portfolio);
    addHighlight(QStringLiteral("Relational 3NF Schema & ACID Integrity"), IconHelper::IconType::Dashboard);
    addHighlight(QStringLiteral("Systematic SIP & SWP Wealth Modeling"), IconHelper::IconType::SipCalculator);
    addHighlight(QStringLiteral("Bank-Grade PBKDF2-HMAC-SHA256 Encryption"), IconHelper::IconType::Settings);
    addHighlight(QStringLiteral("100% Local Privacy & Data Isolation"), IconHelper::IconType::Checkmark);

    layout->addStretch(1);

    // Footer note
    auto *footer = new QLabel(QStringLiteral("InvestSphere v0.4.0 | Academic DBMS Course Architecture"), panel);
    footer->setStyleSheet(QStringLiteral("font-size: 11px; color: #475569;"));
    layout->addWidget(footer);

    return panel;
}

QWidget* LoginWidget::createRightFormPanel()
{
    auto *panel = new QFrame(this);
    panel->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; }"));

    auto *outerLayout = new QVBoxLayout(panel);
    outerLayout->setContentsMargins(56, 48, 56, 48);
    outerLayout->setAlignment(Qt::AlignCenter);

    auto *formContainer = new QWidget(panel);
    formContainer->setMaximumWidth(420);
    auto *layout = new QVBoxLayout(formContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // Heading
    auto *welcomeHeading = new QLabel(QStringLiteral("Welcome Back"), formContainer);
    welcomeHeading->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 24px; font-weight: 700; color: #0F172A;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(welcomeHeading);

    auto *welcomeSub = new QLabel(QStringLiteral("Enter your credentials to access your investment portfolio."), formContainer);
    welcomeSub->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 13px; color: #64748B;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(welcomeSub);

    // Inline Error Banner
    m_errorBanner = new NotificationBanner(QString(), NotificationBanner::BannerType::Error, true, formContainer);
    m_errorBanner->hide();
    layout->addWidget(m_errorBanner);

    layout->addSpacing(8);

    // Email Input
    auto *emailLabel = new QLabel(QStringLiteral("Email Address"), formContainer);
    emailLabel->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1F2937;"));
    layout->addWidget(emailLabel);

    m_emailInput = new QLineEdit(formContainer);
    m_emailInput->setPlaceholderText(QStringLiteral("investor@example.com"));
    m_emailInput->setFixedHeight(40);
    m_emailInput->setStyleSheet(Theme::inputFieldStyle());
    layout->addWidget(m_emailInput);

    // Password Input + Show/Hide
    auto *pwdLabel = new QLabel(QStringLiteral("Password"), formContainer);
    pwdLabel->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1F2937;"));
    layout->addWidget(pwdLabel);

    auto *pwdContainer = new QWidget(formContainer);
    auto *pwdLayout = new QHBoxLayout(pwdContainer);
    pwdLayout->setContentsMargins(0, 0, 0, 0);
    pwdLayout->setSpacing(6);

    m_passwordInput = new QLineEdit(pwdContainer);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setPlaceholderText(QStringLiteral("Enter your password"));
    m_passwordInput->setFixedHeight(40);
    m_passwordInput->setStyleSheet(Theme::inputFieldStyle());
    pwdLayout->addWidget(m_passwordInput, 1);

    m_showPasswordBtn = new QPushButton(QStringLiteral("Show"), pwdContainer);
    m_showPasswordBtn->setFixedHeight(40);
    m_showPasswordBtn->setCursor(Qt::PointingHandCursor);
    m_showPasswordBtn->setStyleSheet(Theme::secondaryButtonStyle());
    connect(m_showPasswordBtn, &QPushButton::clicked, this, &LoginWidget::togglePasswordVisibility);
    pwdLayout->addWidget(m_showPasswordBtn);

    layout->addWidget(pwdContainer);

    // Remember Email Checkbox
    auto *optionsRow = new QHBoxLayout();
    m_rememberEmailCheck = new QCheckBox(QStringLiteral("Remember my email"), formContainer);
    m_rememberEmailCheck->setStyleSheet(QStringLiteral(
        "QCheckBox { font-size: 12px; color: #475569; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #CBD5E1; border-radius: 4px; }"
        "QCheckBox::indicator:checked { background-color: #2563EB; border-color: #2563EB; }"
    ));
    m_rememberEmailCheck->setChecked(true);
    optionsRow->addWidget(m_rememberEmailCheck);
    optionsRow->addStretch(1);
    layout->addLayout(optionsRow);

    layout->addSpacing(8);

    // Login Action Button
    m_loginButton = new QPushButton(QStringLiteral("Sign In to InvestSphere"), formContainer);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setFixedHeight(42);
    m_loginButton->setStyleSheet(Theme::primaryButtonStyle());
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWidget::handleLogin);
    connect(m_passwordInput, &QLineEdit::returnPressed, this, &LoginWidget::handleLogin);
    connect(m_emailInput, &QLineEdit::returnPressed, this, &LoginWidget::handleLogin);
    layout->addWidget(m_loginButton);

    layout->addSpacing(8);

    // Register Link
    auto *registerRow = new QHBoxLayout();
    registerRow->setAlignment(Qt::AlignCenter);
    registerRow->setSpacing(4);

    auto *noAccountLabel = new QLabel(QStringLiteral("Don't have an account?"), formContainer);
    noAccountLabel->setStyleSheet(QStringLiteral("font-size: 13px; color: #64748B;"));

    auto *registerLink = new QPushButton(QStringLiteral("Create an account"), formContainer);
    registerLink->setCursor(Qt::PointingHandCursor);
    registerLink->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; background: transparent; color: #2563EB; font-weight: 600; font-size: 13px; padding: 0; }"
        "QPushButton:hover { text-decoration: underline; color: #1D4ED8; }"
    ));
    connect(registerLink, &QPushButton::clicked, this, &LoginWidget::switchToRegister);

    registerRow->addWidget(noAccountLabel);
    registerRow->addWidget(registerLink);
    layout->addLayout(registerRow);

    outerLayout->addWidget(formContainer);
    return panel;
}

void LoginWidget::togglePasswordVisibility()
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

void LoginWidget::resetForm()
{
    if (m_passwordInput) {
        m_passwordInput->clear();
    }
    if (m_rememberEmailCheck && !m_rememberEmailCheck->isChecked() && m_emailInput) {
        m_emailInput->clear();
    }
    if (m_errorBanner) {
        m_errorBanner->hide();
    }
    if (m_loginButton) {
        m_loginButton->setEnabled(true);
        m_loginButton->setText(QStringLiteral("Sign In to InvestSphere"));
    }
}

void LoginWidget::setRememberedEmail(const QString &email)
{
    if (m_emailInput) {
        m_emailInput->setText(email);
    }
}

void LoginWidget::handleLogin()
{
    m_errorBanner->hide();

    QString email = m_emailInput->text().trimmed();
    QString password = m_passwordInput->text();

    if (email.isEmpty() || password.isEmpty()) {
        m_errorBanner->showMessage(
            QStringLiteral("Please enter both your email address and password."),
            NotificationBanner::BannerType::Error
        );
        return;
    }

    // Prevent duplicate rapid submissions
    m_loginButton->setEnabled(false);
    m_loginButton->setText(QStringLiteral("Authenticating..."));

    Services::AuthenticationService authService;
    auto result = authService.authenticate(email.toStdString(), password.toStdString());

    m_loginButton->setEnabled(true);
    m_loginButton->setText(QStringLiteral("Sign In to InvestSphere"));

    if (result.success) {
        emit loginSuccess(result.userId);
    } else {
        m_errorBanner->showMessage(
            QString::fromStdString(result.message),
            NotificationBanner::BannerType::Error
        );
        m_passwordInput->selectAll();
        m_passwordInput->setFocus();
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
