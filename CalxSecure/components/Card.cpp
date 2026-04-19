#include "Card.h"
#include "../../CalxSecure/GlobalStyle.h"

Card::Card(QWidget* parent, bool elevated) : QFrame(parent) {
    setObjectName("Card");
    setStyleSheet(GlobalStyle::getCardStyle());

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(16);
    m_mainLayout->setContentsMargins(24, 24, 24, 24);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #00d4ff;");
    m_descLabel = new QLabel(this);
    m_descLabel->setWordWrap(true);
    m_descLabel->setStyleSheet("color: #a0a0b0;");

    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_descLabel);

    if (elevated) {
        auto* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(20);
        shadow->setXOffset(0);
        shadow->setYOffset(8);
        shadow->setColor(QColor(0, 0, 0, 80));
        setGraphicsEffect(shadow);
    }
}

void Card::setTitle(const QString& title) { m_titleLabel->setText(title); }
void Card::setDescription(const QString& desc) { m_descLabel->setText(desc); }
QVBoxLayout* Card::contentLayout() { return m_mainLayout; }