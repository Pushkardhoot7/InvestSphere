#include "ConfirmDialog.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

ConfirmDialog::ConfirmDialog(const QString &title,
                             const QString &message,
                             const QString &confirmText,
                             const QString &cancelText,
                             bool isDestructive,
                             QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(380);
    setStyleSheet(QStringLiteral("QDialog { background-color: #FFFFFF; border-radius: 12px; }"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Header Row with Icon and Title
    auto *headerRow = new QHBoxLayout();
    headerRow->setSpacing(12);

    auto *iconLabel = new QLabel(this);
    iconLabel->setFixedSize(40, 40);
    iconLabel->setAlignment(Qt::AlignCenter);

    if (isDestructive) {
        iconLabel->setStyleSheet(QStringLiteral("background-color: #FEE2E2; border-radius: 20px;"));
        iconLabel->setPixmap(IconHelper::getPixmap(IconHelper::IconType::AlertCircle, QColor(220, 38, 38), 22));
    } else {
        iconLabel->setStyleSheet(QStringLiteral("background-color: #EFF6FF; border-radius: 20px;"));
        iconLabel->setPixmap(IconHelper::getPixmap(IconHelper::IconType::InfoCircle, QColor(37, 99, 235), 22));
    }
    headerRow->addWidget(iconLabel);

    auto *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;"
    ).arg(Theme::fontFamily()));
    headerRow->addWidget(titleLabel, 1);
    mainLayout->addLayout(headerRow);

    // Message Body
    auto *messageLabel = new QLabel(message, this);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 13px; color: #475569; line-height: 1.4;"
    ).arg(Theme::fontFamily()));
    mainLayout->addWidget(messageLabel);

    mainLayout->addSpacing(8);

    // Buttons Row
    auto *buttonsRow = new QHBoxLayout();
    buttonsRow->setSpacing(12);
    buttonsRow->addStretch(1);

    auto *cancelBtn = new QPushButton(cancelText, this);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(Theme::secondaryButtonStyle());
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonsRow->addWidget(cancelBtn);

    auto *confirmBtn = new QPushButton(confirmText, this);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(isDestructive ? Theme::destructiveButtonStyle() : Theme::primaryButtonStyle());
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonsRow->addWidget(confirmBtn);

    mainLayout->addLayout(buttonsRow);
}

bool ConfirmDialog::confirm(QWidget *parent,
                            const QString &title,
                            const QString &message,
                            const QString &confirmText,
                            bool isDestructive)
{
    ConfirmDialog dialog(title, message, confirmText, QStringLiteral("Cancel"), isDestructive, parent);
    return dialog.exec() == QDialog::Accepted;
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
