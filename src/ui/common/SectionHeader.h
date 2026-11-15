#ifndef INVESTSPHERE_SECTIONHEADER_H
#define INVESTSPHERE_SECTIONHEADER_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace InvestSphere {
namespace Ui {

/**
 * @brief Reusable section header component with title, subtitle, and optional action button.
 */
class SectionHeader : public QWidget {
    Q_OBJECT

public:
    explicit SectionHeader(const QString &title,
                          const QString &subtitle = QString(),
                          const QString &actionText = QString(),
                          QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setSubtitle(const QString &subtitle);
    void setActionButton(const QString &actionText);

signals:
    void actionClicked();

private:
    QLabel *m_titleLabel{nullptr};
    QLabel *m_subtitleLabel{nullptr};
    QPushButton *m_actionButton{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_SECTIONHEADER_H
