#include "SearchField.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QPainter>
#include <QPaintEvent>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

SearchField::SearchField(const QString &placeholder, QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText(placeholder);
    setClearButtonEnabled(true);
    setFixedHeight(36);

    setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "   background-color: #FFFFFF;"
        "   color: #1F2937;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 8px;"
        "   padding-left: 32px;"
        "   padding-right: 28px;"
        "   font-family: %1;"
        "   font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #2563EB;"
        "   background-color: #FFFFFF;"
        "}"
        "QLineEdit:disabled {"
        "   background-color: #F8FAFC;"
        "   color: #94A3B8;"
        "}"
    ).arg(Theme::fontFamily()));
}

void SearchField::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    // Draw search icon on the left
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPixmap iconPixmap = IconHelper::getPixmap(IconHelper::IconType::Search, QColor(148, 163, 184), 16);
    int yPos = (height() - iconPixmap.height()) / 2;
    painter.drawPixmap(10, yPos, iconPixmap);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
