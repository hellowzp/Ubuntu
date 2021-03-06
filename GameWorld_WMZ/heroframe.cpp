#include "heroframe.h"
#include "QLabel"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

HeroFrame::HeroFrame(HeroModel &hero):  _hero(hero)

{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    this->setLayout(mainLayout);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    QVBoxLayout *rightLayout = new QVBoxLayout();
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    QLabel *iconLabel = new QLabel();
    leftLayout->addWidget(iconLabel);
    QLabel *roleLabel = new QLabel();
    if (_hero.role == PLAYER_ROLE_MAGE) {

        iconLabel->setPixmap(QPixmap(":/world/angry-bird-yellow-icon.png").scaled(40, 40));
        roleLabel->setText("Role: Mage");

    } else if (_hero.role == PLAYER_ROLE_PRIEST) {

        iconLabel->setPixmap(QPixmap(":/world/angry-bird-green-icon.png").scaled(40, 40));
        roleLabel->setText("Role: Priest");

    } else if (_hero.role == PLAYER_ROLE_WORRIOR) {

        iconLabel->setPixmap(QPixmap(":/world/angry-bird-icon.png").scaled(40, 40));
        roleLabel->setText("Role: Worrior");
    }

    QLabel *nameLabel = new QLabel();
    nameLabel->setText(QString("Name: ").append(_hero.name));
    rightLayout->addWidget(nameLabel);
    rightLayout->addWidget(roleLabel);

    this->setFixedHeight(60);


}

HeroFrame::~HeroFrame()
{
    qDebug() << "dtor HeroFrame";
}
