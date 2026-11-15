#include "Theme.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QFontDatabase>

namespace InvestSphere {
namespace Ui {

QString Theme::fontFamily()
{
    return QStringLiteral("\"Segoe UI\", -apple-system, BlinkMacSystemFont, \"Roboto\", \"Helvetica Neue\", Arial, sans-serif");
}

QFont Theme::headingFont(int pointSize, int weight)
{
    QFont font(QStringLiteral("Segoe UI"), pointSize, weight);
    font.setStyleStrategy(QFont::PreferAntialias);
    return font;
}

QFont Theme::bodyFont(int pointSize, int weight)
{
    QFont font(QStringLiteral("Segoe UI"), pointSize, weight);
    font.setStyleStrategy(QFont::PreferAntialias);
    return font;
}

QFont Theme::metricFont(int pointSize, int weight)
{
    QFont font(QStringLiteral("Segoe UI"), pointSize, weight);
    font.setStyleStrategy(QFont::PreferAntialias);
    return font;
}

QString Theme::primaryButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "   background-color: %1;"
        "   color: #FFFFFF;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   padding: 8px 16px;"
        "   font-family: %3;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "   background-color: %4;"
        "   border-color: %4;"
        "}"
        "QPushButton:pressed {"
        "   background-color: %5;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #93C5FD;"
        "   border-color: #93C5FD;"
        "   color: #EFF6FF;"
        "}"
    ).arg(ColorPrimaryAccent)
     .arg(RadiusMd)
     .arg(fontFamily())
     .arg(ColorPrimaryHover)
     .arg(ColorPrimaryPressed);
}

QString Theme::secondaryButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: %3px;"
        "   padding: 8px 16px;"
        "   font-family: %4;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "   background-color: #F8FAFC;"
        "   border-color: #CBD5E1;"
        "   color: #0F172A;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #F1F5F9;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #F8FAFC;"
        "   border-color: #E2E8F0;"
        "   color: #94A3B8;"
        "}"
    ).arg(ColorTextPrimary)
     .arg(ColorBorder)
     .arg(RadiusMd)
     .arg(fontFamily());
}

QString Theme::destructiveButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "   background-color: %1;"
        "   color: #FFFFFF;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   padding: 8px 16px;"
        "   font-family: %3;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "   background-color: #B91C1C;"
        "   border-color: #B91C1C;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #991B1B;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #FCA5A5;"
        "   border-color: #FCA5A5;"
        "}"
    ).arg(ColorNegative)
     .arg(RadiusMd)
     .arg(fontFamily());
}

QString Theme::cardFrameStyle()
{
    return QStringLiteral(
        "QFrame.CardWidget, QWidget.CardWidget {"
        "   background-color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: %3px;"
        "}"
    ).arg(ColorCardBackground)
     .arg(ColorCardBorder)
     .arg(RadiusLg);
}

QString Theme::inputFieldStyle()
{
    return QStringLiteral(
        "QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: %4px;"
        "   padding: 7px 12px;"
        "   font-family: %5;"
        "   font-size: 13px;"
        "   selection-background-color: %6;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {"
        "   border: 1px solid %6;"
        "   background-color: #FFFFFF;"
        "}"
        "QLineEdit:disabled, QComboBox:disabled {"
        "   background-color: #F1F5F9;"
        "   color: %7;"
        "}"
    ).arg(ColorInputBackground)
     .arg(ColorTextPrimary)
     .arg(ColorBorder)
     .arg(RadiusMd)
     .arg(fontFamily())
     .arg(ColorPrimaryAccent)
     .arg(ColorTextSecondary);
}

QString Theme::tableWidgetStyle()
{
    return QStringLiteral(
        "QTableWidget {"
        "   background-color: #FFFFFF;"
        "   color: %1;"
        "   gridline-color: %2;"
        "   border: 1px solid %2;"
        "   border-radius: %3px;"
        "   font-family: %4;"
        "   font-size: 13px;"
        "   selection-background-color: #EFF6FF;"
        "   selection-color: %1;"
        "   outline: none;"
        "}"
        "QTableWidget::item {"
        "   padding: 10px 14px;"
        "   border-bottom: 1px solid #F1F5F9;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #EFF6FF;"
        "   color: #1E3A8A;"
        "   font-weight: 500;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: %5;"
        "   font-family: %4;"
        "   font-size: 11px;"
        "   font-weight: 700;"
        "   letter-spacing: 0.5px;"
        "   text-transform: uppercase;"
        "   padding: 10px 14px;"
        "   border: none;"
        "   border-bottom: 1px solid %2;"
        "}"
    ).arg(ColorTextPrimary)
     .arg(ColorBorder)
     .arg(RadiusMd)
     .arg(fontFamily())
     .arg(ColorTextSecondary);
}

QString Theme::getApplicationStyleSheet()
{
    return QStringLiteral(
        "/* InvestSphere Global Application Stylesheet */\n"
        "QWidget {\n"
        "   font-family: %1;\n"
        "   color: %2;\n"
        "}\n"
        "QMainWindow {\n"
        "   background-color: %3;\n"
        "}\n"
        "QScrollArea {\n"
        "   border: none;\n"
        "   background: transparent;\n"
        "}\n"
        "QScrollBar:vertical {\n"
        "   border: none;\n"
        "   background: #F1F5F9;\n"
        "   width: 8px;\n"
        "   margin: 0px 0px 0px 0px;\n"
        "   border-radius: 4px;\n"
        "}\n"
        "QScrollBar::handle:vertical {\n"
        "   background: #CBD5E1;\n"
        "   min-height: 24px;\n"
        "   border-radius: 4px;\n"
        "}\n"
        "QScrollBar::handle:vertical:hover {\n"
        "   background: #94A3B8;\n"
        "}\n"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {\n"
        "   height: 0px;\n"
        "}\n"
        "QScrollBar:horizontal {\n"
        "   border: none;\n"
        "   background: #F1F5F9;\n"
        "   height: 8px;\n"
        "   margin: 0px 0px 0px 0px;\n"
        "   border-radius: 4px;\n"
        "}\n"
        "QScrollBar::handle:horizontal {\n"
        "   background: #CBD5E1;\n"
        "   min-width: 24px;\n"
        "   border-radius: 4px;\n"
        "}\n"
        "QScrollBar::handle:horizontal:hover {\n"
        "   background: #94A3B8;\n"
        "}\n"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {\n"
        "   width: 0px;\n"
        "}\n"
        "QToolTip {\n"
        "   background-color: #0F172A;\n"
        "   color: #FFFFFF;\n"
        "   border: 1px solid #334155;\n"
        "   border-radius: 4px;\n"
        "   padding: 6px 10px;\n"
        "   font-size: 11px;\n"
        "}\n"
        "QDialog {\n"
        "   background-color: #FFFFFF;\n"
        "}\n"
    ).arg(fontFamily())
     .arg(ColorTextPrimary)
     .arg(ColorMainBackground);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
