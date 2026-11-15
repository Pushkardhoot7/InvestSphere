#ifndef INVESTSPHERE_SEARCHFIELD_H
#define INVESTSPHERE_SEARCHFIELD_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QLineEdit>

namespace InvestSphere {
namespace Ui {

/**
 * @brief Modern financial search input field with integrated icon and clean styling.
 */
class SearchField : public QLineEdit {
    Q_OBJECT

public:
    explicit SearchField(const QString &placeholder = QStringLiteral("Search investments, symbols, or records..."),
                        QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_SEARCHFIELD_H
