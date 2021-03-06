#include "playerframe.h"
#include <QHBoxLayout>

#include "gamehoster.h"

PlayerFrame::PlayerFrame(Player &player, bool enabled)
{

    QHBoxLayout *layout = new QHBoxLayout();
    this->setLayout(layout);

    _nameLabel = new QLabel(player.playerName);
    layout->addWidget(_nameLabel);

    this->_roleComboBox = new QComboBox();
    this->_roleComboBox->addItem(QIcon(QPixmap(":/world/angry-bird-yellow-icon.png").scaled(30 ,30)),"Mage", PLAYER_ROLE_MAGE);
    this->_roleComboBox->addItem(QIcon(QPixmap(":/world/angry-bird-icon.png").scaled(30 ,30)),"Worrior", PLAYER_ROLE_WORRIOR);
    this->_roleComboBox->addItem(QIcon(QPixmap(":/world/angry-bird-green-icon.png").scaled(30 ,30)),"Priest", PLAYER_ROLE_PRIEST);
    layout->addWidget(_roleComboBox);
    QObject::connect(_roleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(roleChanged(int)));


    this->_roleComboBox->setCurrentIndex(player.role - 1);
    this->_roleComboBox->setEnabled(enabled);

    _player = player;
}

PlayerFrame::~PlayerFrame()
{
    qDebug() << "dtor of PlayerFrame";
}

Player &PlayerFrame::getPlayer()
{
    return _player;
}

void PlayerFrame::updataFrame(Player &player)
{
    this->_roleComboBox->setCurrentIndex(player.role - 1);
}

void PlayerFrame::roleChanged(int index) {

   _player.role = index + 1;

   if (this->_roleComboBox->isEnabled()) {

       emit playerChanged(_player);
   }

}





