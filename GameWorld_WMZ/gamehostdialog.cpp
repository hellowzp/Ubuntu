#include "gamehostdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include "gameclient.h"
#include "gamehoster.h"


GameHostDialog::GameHostDialog(bool isEnabled)
{

    QVBoxLayout *mainLayout = new QVBoxLayout();
    GameClient *client = GameClient::Instance();

    this->setLayout(mainLayout);
    this->setWindowTitle(client->getRoom().roomName);

    QScrollArea *playerListScrollArea = new QScrollArea();
    this->_playerListLayout = new QListWidget();

    this->_playerListLayout->addItem("Player                        Role");
    playerListScrollArea->setWidget(_playerListLayout);
    mainLayout->addWidget(playerListScrollArea);

    QHBoxLayout *lowerLayout = new QHBoxLayout();
    mainLayout->addLayout(lowerLayout);
    QPushButton *startButton = new QPushButton("Start");
    GameHoster *hoster = GameHoster::Instance();
    if (hoster != nullptr)
        QObject::connect(startButton, SIGNAL(clicked()), hoster, SLOT(startGame()));
    lowerLayout->addWidget(startButton);

    // connect this ui dialog to the client host
    QObject::connect(client, SIGNAL(needSetMyPlayer(Player)), this, SLOT(myPlayer(Player)));
    QObject::connect(client, SIGNAL(needSetOtherPlayer(Player)), this, SLOT(otherPlayer(Player)));
    QObject::connect(this, SIGNAL(needToRequestOtherPlayers()), client, SLOT(requestOtherPlayers()));
    QObject::connect(client, SIGNAL(needToCloseDialog()), this, SLOT(closeDialog()));

    startButton->setEnabled(isEnabled);

}

GameHostDialog::~GameHostDialog()
{
    qDebug() << "dtor of GameHostDialog";

    delete _myFrame;

    for (PlayerFrame *frame : _otherPlayerFrames) delete frame;

}




void GameHostDialog::otherPlayer(Player player)
{
    bool foundPlayer = false;
    for (PlayerFrame *frame : _otherPlayerFrames) {

        if (frame->getPlayer().id == player.id) {
            foundPlayer = true;
            frame->updataFrame(player);
            break;
        }
    }

    if (!foundPlayer) {

        PlayerFrame *newPlayerFrame = new PlayerFrame(player, false);
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(item->sizeHint().height(),50));
        _playerListLayout->addItem(item);
        _playerListLayout->setItemWidget(item,newPlayerFrame);
        _otherPlayerFrames.push_back(newPlayerFrame);
    }

}

void GameHostDialog::myPlayer(Player player)
{
    _myFrame = new PlayerFrame(player, true);
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(item->sizeHint().height(),50));
    _playerListLayout->addItem(item);
    _playerListLayout->setItemWidget(item,_myFrame);
    QObject::connect(_myFrame, SIGNAL(playerChanged(Player)), GameClient::Instance(), SLOT(playerChanged(Player)));

    //after set my player, we need to request other player also
    emit needToRequestOtherPlayers();
}

void GameHostDialog::closeDialog()
{
    this->close();
    delete this;
}

