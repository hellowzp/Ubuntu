#include "roomframe.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include "gameclient.h"
#include "gamehostdialog.h"



RoomFrame::RoomFrame(Room &room)
{

    this->_room = room;

    QHBoxLayout *layout = new QHBoxLayout();
    this->setLayout(layout);

    QLabel *roomNameLabel = new QLabel();
    roomNameLabel->setText(room.roomName);
    layout->addWidget(roomNameLabel);

    QLabel *ipLabel = new QLabel();
    ipLabel->setText(room.senderAddress.toString());
    layout->addWidget(ipLabel);

    _currentPlayerNumLabel = new QLabel();
    _currentPlayerNumLabel->setText(QString::number(room.currentPlayerNum));
    _currentPlayerNumLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(_currentPlayerNumLabel);

    QLabel *maxPlayerNumLabel = new QLabel();
    maxPlayerNumLabel->setText(QString::number(room.maxPlayerNum));
    maxPlayerNumLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(maxPlayerNumLabel);

    QLabel *mapLabel = new QLabel;
    if (room.map == 1) {
        mapLabel->setText("map 1");
    } else if (room.map == 2) {
        mapLabel->setText("map 2");
    } else if (room.map == 3) {
        mapLabel->setText("map 3");
    }
    layout->addWidget(mapLabel);

    _joinButton = new QPushButton();
    _joinButton->setText("Join");

    layout->addWidget(_joinButton);


    QObject::connect(_joinButton, SIGNAL(clicked()), this, SLOT(joinGame()));

}

RoomFrame::~RoomFrame()
{
    qDebug() << "dtor of RoomFrame";
}

void RoomFrame::setMyName(QString &name)
{
    _myName = name;
}

void RoomFrame::setMainWindow(MainWindow *mainWindow)
{
    _mainWindow = mainWindow;
}

void RoomFrame::updataFrame(Room &room) {

    _room = room;

    if (room.maxPlayerNum == room.currentPlayerNum)
        _joinButton->setEnabled(false);

    _currentPlayerNumLabel->setText(QString::number(room.currentPlayerNum));
}

void RoomFrame::joinGame()
{

    Player player;
    player.id = 0;
    player.playerName = _myName;
    player.role = PLAYER_ROLE_MAGE;

    qDebug() << "enter room" << player.playerName;

    GameClient::CreateClient(_room, player);
    GameClient *client = GameClient::Instance();
    QObject::connect(client, SIGNAL(needToCreateWorld(int)), _mainWindow, SLOT(createWorld(int)));
    QObject::connect(_mainWindow, SIGNAL(worldCreated()), client, SLOT(worldCreated()));
    QObject::connect(client, SIGNAL(needToGameOver(bool)), _mainWindow, SLOT(gameOver(bool)));


    //create the room dialog and disable the start button
    GameHostDialog *dialog = new GameHostDialog(false);
    dialog->show();

    emit closeRoomDialog();

}

