#include "gamehoster.h"
#include <iostream>
#include <QList>
#include <QDataStream>
#include <QDebug>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QPixmap>
#include "gameclient.h"

GameHoster *GameHoster::_instance = 0;

GameHoster::GameHoster(Room &room)
{

    this->_broadcaster = new QUdpSocket();
    this->_room = room;

    this->_server = new QTcpServer();
    QObject::connect(_server, SIGNAL(newConnection()), this , SLOT(newClientConnection()));
    if (!this->_server->listen(this->_room.senderAddress, TCP_CONNECTION_PORT)) {

        qDebug() << "Can not open the server";
    }

    QPixmap worldMap;
    if (room.map == 1) {

        worldMap = QPixmap(":/world/world1.jpg");
    } else if (room.map == 2) {

        worldMap = QPixmap(":/world/world2.png");
    } else if (room.map == 3) {

        worldMap = QPixmap(":/world/world3.png");
    }

    _worldWidth = worldMap.width();
    _worldHeight = worldMap.height();

    QObject::connect(this, SIGNAL(needSendBroadcast()), this, SLOT(sendBroadcast()));
    this->start();

}

void GameHoster::run()
{
    while (this->_isBroadcasting) {

        this->sleep(1);

        emit needSendBroadcast();
        qDebug() << "broadcasting";

    }
}

void GameHoster::assignRoles(int round)
{

    //assign heros only in the first round
    if (round == 1) {

        //request heros first

        for (QTcpSocket *socket : _clientSockets) {

            int16_t x;
            int16_t y;
            bool validCoor = false;
            while (!validCoor) {

                x = rand() % _worldWidth;
                y = rand() % _worldHeight;
                validCoor = true;

                for (QPoint &point : _coorsBuffer) {

                    if ((point.rx() == x) && (point.ry() == y)) {
                        validCoor = false;
                        break;
                    }
                }
            }

            _coorsBuffer.push_back(QPoint(x, y));

            QByteArray hero;
            QDataStream heroStream(&hero, QIODevice::WriteOnly);
            int16_t type = TCP_DATA_TYPE_HERO_REQUEST;
            int16_t size = 2 * sizeof(int16_t);
            heroStream << type << size << x << y;
            socket->write(hero.data(), hero.size());

        }
    }

    for (QTcpSocket *socket : _clientSockets) {

        //for the first round, we let each client to create 2 opponents of type 1

        for (int i = 0; i < 2; i++) {

            int16_t x;
            int16_t y;
            bool validCoor = false;
            while (!validCoor) {

                x = rand() % _worldWidth;
                y = rand() % _worldHeight;
                validCoor = true;

                for (QPoint &point : _coorsBuffer) {

                    if ((point.rx() == x) && (point.ry() == y)) {
                        validCoor = false;
                        break;
                    }
                }
            }

            _coorsBuffer.push_back(QPoint(x, y));

            QByteArray opponent;
            QDataStream opponentStream(&opponent, QIODevice::WriteOnly);
            int16_t type = TCP_DATA_TYPE_OPPONENT_REQUEST;
            int16_t size = sizeof(int16_t) * 3;
            int16_t opponentType = round;
            opponentStream << type << size << opponentType << x << y;
            socket->write(opponent.data(), opponent.size());
            socket->flush();

            qDebug() << "sending opponent request " << x << y;
        }

    }
}

bool GameHoster::checkIfRoundOver()
{
    for (OpponentModel &opponent : _opponents) {

        if (opponent.healthPecentage > 0) {

            return false;
        }
    }

    return true;
}

void GameHoster::generateGifts(OpponentModel &opponent)
{


        for (int i = 0; i < _currentRound; i++) {

            int16_t giftType;

            int randNum = rand();
            randNum %= 9;

            if (randNum == 6) {

                giftType = 3;
            } else if (randNum == 7) {

                giftType = 4;
            } else if (randNum == 8) {

                giftType = 5;
            } else {

                if (randNum % 2 == 0) {
                    giftType = 1;
                } else {
                    giftType = 2;
                }
            }

            int16_t x;
            int16_t y;
            bool validCoor = false;
            while (!validCoor) {

                x = opponent.x + rand() % 9 - 4;
                y = opponent.y + rand() % 9 - 4;
                validCoor = true;

                if (x == opponent.x && y == opponent.y) {
                    validCoor = false;
                    continue;
                }

                for (GiftModel &gift : _gifts) {

                    if (x == gift.x && y == gift.y) {
                        validCoor = false;
                        break;
                    }
                }

                if (x < 0 || x > _worldWidth - 1) {
                    validCoor = false;
                    continue;
                }

                if (y < 0 || y > _worldHeight - 1) {
                    validCoor = false;
                    continue;
                }
            }

            int16_t giftId = x * 100 + y;

            GiftModel gift;
            gift.id = giftId;
            gift.type = giftType;
            gift.x = x;
            gift.y = y;

            _gifts.push_back(gift);

            // send out

            QByteArray giftData;
            QDataStream giftStream(&giftData, QIODevice::WriteOnly);
            int16_t type = TCP_DATA_TYPE_NEW_GIFT;
            int16_t size = gift.getByteSize();
            giftStream << type << size << gift;

            for (QTcpSocket *socket : _clientSockets) {

                socket->write(giftData.data(), giftData.size());
                socket->flush();
            }

        }

}


void GameHoster::CreateHoster(Room room)
{
    GameHoster::_instance = new GameHoster(room);

}

void GameHoster::DestoryHoser()
{

    delete _instance;
}

GameHoster *GameHoster::Instance()
{
    return GameHoster::_instance;
}

GameHoster::~GameHoster()
{
    _server->close();
    for (QTcpSocket *socket : _clientSockets) {

        socket->close();
        delete socket;
    }
}

void GameHoster::sendBroadcast()
{

    QByteArray broadcastData;
    QDataStream dataStream(&broadcastData, QIODevice::WriteOnly);

    _room.currentPlayerNum = this->_clientSockets.count();

    dataStream << _room;


    this->_broadcaster->writeDatagram(broadcastData.data(), broadcastData.size(), QHostAddress::Broadcast, BROADCAST_PORT);
    //this->_broadcaster->writeDatagram(broadcastData.data(), broadcastData.size(), QHostAddress::Broadcast, BROADCAST_PORT);

}

void GameHoster::newClientConnection()
{

    qDebug() << "new tcp connection";
    QTcpSocket *clientSocket = this->_server->nextPendingConnection();
    QObject::connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readData()));

    //send back the id of this connection
    int id = clientSocket->socketDescriptor();
    int16_t type= TCP_DATA_TYPE_CON_ID;
    int16_t size = sizeof(int);
    QByteArray data;
    QDataStream dataStream(&data, QIODevice::WriteOnly);
    dataStream << type << size << id;
    clientSocket->write(data.data(), data.size());

    this->_clientSockets.push_back(clientSocket);

}

void GameHoster::readData()
{


    //check which socket has data
    for (QTcpSocket *socket : _clientSockets) {

        while (socket->bytesAvailable()) {
            //read out the data
            QByteArray headerData;
            headerData.resize(sizeof(int16_t) * 2);
            socket->read(headerData.data(), sizeof(int16_t) * 2);
            QDataStream headerDataStream(&headerData, QIODevice::ReadOnly);
            int16_t type;
            int16_t size;
            headerDataStream >> type;
            headerDataStream >> size;

            QByteArray data;
            data.resize(size);
            socket->read(data.data(), size);
            QDataStream dataStream(&data, QIODevice::ReadOnly);

            if (type == TCP_DATA_TYPE_PLAYER) {

                //read out this player
                Player player;
                dataStream >> player;
                bool foundPlayer = false;
                for (Player &myPlayer : _players) {

                    if (myPlayer.id == player.id) {

                        myPlayer = player;
                        foundPlayer = true;
                        break;
                    }
                }
                if (!foundPlayer) {
                    _players.push_back(player); //add the new player the vector
                    _clientStates.push_back(CLIENT_STATE_ID_RECEIVED); //add the corronsponding client state
                }

                //give this information other sockets
                for (QTcpSocket *socket : _clientSockets) {

                    if (socket->socketDescriptor() != player.id) {
                        QByteArray data;
                        QDataStream dataStream(&data, QIODevice::WriteOnly);
                        dataStream << type << size << player;
                        socket->write(data.data(), data.size());
                    }
                }
            } else if (type == TCP_DATA_TYPE_QUERY_PLAYERS) {

                //qDebug() << "request received";

                for (Player &player : _players) {

                    int16_t type = TCP_DATA_TYPE_PLAYER;
                    int16_t size = player.getByteSize();
                    QByteArray playerData;
                    QDataStream playerStream(&playerData, QIODevice::WriteOnly);
                    playerStream << type << size << player;
                    socket->write(playerData.data(), playerData.size());
                }
            } else if (type == TCP_DATA_TYPE_WORLD_CREATED) {

                //then we need assign roles for each client.

                for (Player &player : _players) {

                    if (player.id == socket->socketDescriptor()) {

                        player.state = CLIENT_STATE_WORLD_READY;
                        break;
                    }
                }

                //check if world ready for every one
                bool allReady = true;
                for (Player &player : _players) {

                    if (player.state != CLIENT_STATE_WORLD_READY) {
                        allReady = false;
                        break;
                    }

                }

                if (allReady) {

                    // if all the client are ready to play then create the first round
                    this->assignRoles(1);
                    _currentRound = 1;
                }
            } else if (type == TCP_DATA_TYPE_NEW_HERO) {

                HeroModel hero;
                dataStream >> hero;

                this->_heros.push_back(hero);

                //send the new hero to all other player
                for (QTcpSocket *mySocket : _clientSockets) {

                    if (socket->socketDescriptor() != mySocket->socketDescriptor()) {

                        QByteArray heroData;
                        QDataStream heroStream(&heroData, QIODevice::WriteOnly);
                        int16_t type = TCP_DATA_TYPE_NEW_HERO;
                        int16_t size = hero.getByteSize();
                        heroStream << type << size << hero;
                        mySocket->write(heroData.data(), heroData.size());
                        mySocket->flush();
                    }
                }

            } else if (type == TCP_DATA_TYPE_HERO_MOVE) {

                int16_t id;
                int16_t x;
                int16_t y;
                dataStream >> id >> x >> y;
                for (HeroModel &hero : _heros) {

                    if (hero.id == id) {
                        hero.x = x;
                        hero.y = y;
                    }
                }

                QByteArray moveData;
                QDataStream moveStream(&moveData, QIODevice::WriteOnly);
                moveStream << type << size << id << x << y;


                for (QTcpSocket *mySocket : _clientSockets) {

                    if (mySocket->socketDescriptor() != socket->socketDescriptor()) {

                        mySocket->write(moveData.data(), moveData.size());
                        mySocket->flush();
                    }
                }

            } else if (type == TCP_DATA_TYPE_HERO_MOVING_STOPED) {

                int16_t id;
                dataStream >> id;

                qDebug() << "Server: hero moving stoped " << id;

                for (QTcpSocket *mySocket : _clientSockets) {

                    if (mySocket->socketDescriptor() != socket->socketDescriptor()) {

                        QByteArray movingStoped;
                        QDataStream movingStopedStream(&movingStoped, QIODevice::WriteOnly);
                        movingStopedStream << type << size << id;
                        mySocket->write(movingStoped.data(), movingStoped.size());
                        mySocket->flush();
                    }
                }
            } else if (type == TCP_DATA_TYPE_NEW_OPPONENT) {

                OpponentModel opponent;
                dataStream >> opponent;

                _opponents.push_back(opponent);

                qDebug() << "Server: new opponent " << opponent.id << " current size " << _opponents.size();



                for (QTcpSocket *mySocket : _clientSockets) {

                    if (socket->socketDescriptor() != mySocket->socketDescriptor()) {

                        QByteArray opponentData;
                        QDataStream opponentStream(&opponentData, QIODevice::WriteOnly);
                        opponentStream << type << size << opponent;
                        mySocket->write(opponentData.data(), opponentData.size());
                        mySocket->flush();
                    }
                }

            } else if (type == TCP_DATA_TYPE_OPPONENT_MOVED) {

                int16_t x;
                int16_t y;
                int16_t id;
                dataStream >> id >> x >> y;

                qDebug() << "Server: opponent moved " << id;

                for (OpponentModel &opponent : _opponents) {

                    if (opponent.id == id) {
                        opponent.x = x;
                        opponent.y = y;
                    }
                }

                QByteArray opponentMoved;
                QDataStream movedStream(&opponentMoved, QIODevice::WriteOnly);
                movedStream << type << size << id << x << y;

                for (QTcpSocket *mySocket : _clientSockets) {

                    if (mySocket->socketDescriptor() != socket->socketDescriptor()) {

                        mySocket->write(opponentMoved.data(), opponentMoved.size());
                        mySocket->flush();
                    }
                }
            } else if (type == TCP_DATA_TYPE_HERO_MAGIC) {

                int16_t id;
                int8_t withSword;
                int16_t x;
                int16_t y;
                dataStream >> id >> withSword >> x >> y;

                QByteArray magicData;
                QDataStream magicStream(&magicData, QIODevice::WriteOnly);
                magicStream << type << size << id << withSword << x << y;

                for (QTcpSocket *mySokcet : _clientSockets) {

                    if (mySokcet->socketDescriptor() != socket->socketDescriptor()) {

                        mySokcet->write(magicData.data(), magicData.size());
                        mySokcet->flush();
                    }
                }
            } else if (type == TCP_DATA_TYPE_OPPONENT_INJURED) {

                int16_t id;
                int8_t leftPercentage;
                dataStream >> id >> leftPercentage;

                for (OpponentModel &opponent : _opponents) {

                    if (opponent.id == id) {
                        opponent.healthPecentage = leftPercentage;
                        if (leftPercentage == 0) {

                            this->generateGifts(opponent);
                        }
                    }
                }



                QByteArray injureData;
                QDataStream injureStream(&injureData, QIODevice::WriteOnly);
                injureStream << type << size << id << leftPercentage;

                for (QTcpSocket *mySocket : _clientSockets) {

                    if (mySocket->socketDescriptor() != socket->socketDescriptor()) {

                        mySocket->write(injureData.data(), injureData.size());
                        mySocket->flush();
                    }
                }

                if (this->checkIfRoundOver()) {

                    qDebug() << "round over...............";

                    if (_currentRound == 3) {

                        QByteArray gameOver;
                        QDataStream gameOverStream(&gameOver, QIODevice::WriteOnly);
                        int8_t win = 1; // stands for win
                        int16_t type = TCP_DATA_TYPE_GAME_OVER;
                        int16_t size = sizeof(int8_t);
                        gameOverStream << type << size << win;

                        for (QTcpSocket *mySocket : _clientSockets) {

                            mySocket->write(gameOver.data(), gameOver.size());
                            mySocket->flush();
                        }

                    } else {

                        QByteArray overData;
                        QDataStream overStream(&overData, QIODevice::WriteOnly);
                        int16_t type = TCP_DATA_TYPE_ROUND_OVER;
                        int16_t size = sizeof(int8_t);
                        overStream << type << size << (int8_t)_currentRound;

                        for (QTcpSocket *mySocket : _clientSockets) {

                            mySocket->write(overData.data(), overData.size());
                            mySocket->flush();
                        }

                        //clean up the host
                        this->_opponents.clear();
                    }



                }

            } else if (type == TCP_DATA_TYPE_INJURE_HERO) {

                int16_t id;
                int16_t value;
                dataStream >> id >> value;

                QByteArray injureData;
                QDataStream injureStream(&injureData, QIODevice::WriteOnly);
                injureStream << type << size << id << value;

                for (QTcpSocket *mySocket : _clientSockets) {

                    if (mySocket->socketDescriptor() == id) {

                        mySocket->write(injureData.data(), injureData.size());
                        mySocket->flush();
                    }
                }
            } else if (type == TCP_DATA_TYPE_HERO_INJURED) {

                int16_t id;
                int8_t leftPercentage;
                dataStream >> id >> leftPercentage;

                for (HeroModel &hero : _heros) {

                    if (hero.id == id) {

                        hero.healthPercentage = leftPercentage;
                    }
                }

                QByteArray injureData;
                QDataStream injureStream(&injureData, QIODevice::WriteOnly);
                injureStream << type << size << id << leftPercentage;

                for (QTcpSocket *mySocket : _clientSockets) {

                    if (mySocket->socketDescriptor() != socket->socketDescriptor()) {

                        mySocket->write(injureData.data(), injureData.size());
                        mySocket->flush();
                    }
                }

                //check if lose the game
                bool lose = true;
                for (HeroModel &hero : _heros) {

                    if (hero.healthPercentage > 0) {
                        lose = false;
                        break;
                    }
                }

                if (lose) {

                    qDebug() << "Server: lost the game";

                    QByteArray gameOver;
                    QDataStream gameOverStream(&gameOver, QIODevice::WriteOnly);
                    int8_t win = 0; // stands for win
                    int16_t type = TCP_DATA_TYPE_GAME_OVER;
                    int16_t size = sizeof(int8_t);
                    gameOverStream << type << size << win;

                    for (QTcpSocket *mySocket : _clientSockets) {

                        mySocket->write(gameOver.data(), gameOver.size());
                        mySocket->flush();
                    }
                }


            } else if (type == TCP_DATA_TYPE_GIFT_PICKED) {

                int16_t heroId;
                int16_t giftId;
                int16_t giftType;
                dataStream >> heroId >> giftId >> giftType;


                //remove the gift from the server when picked up
                bool confirmed = false;
                for (int i = 0; i < _gifts.size(); i++) {

                    if (_gifts[i].id == giftId) {

                        qDebug() << "Server remmoving a gift " << giftId;
                        confirmed = true;
                        _gifts.remove(i);
                        break;
                    }
                }

                if (confirmed) {

                    QByteArray giftData;
                    QDataStream giftStream(&giftData, QIODevice::WriteOnly);
                    giftStream << type << size << heroId << giftId << giftType;

                    //send this message to every one
                    for (QTcpSocket *mySocket : _clientSockets) {

                        mySocket->write(giftData.data(), giftData.size());
                        mySocket->flush();
                    }
                }
            } else if (type == TCP_DATA_TYPE_ROUND_OVER_CONFIRMED) {

                int8_t round;
                dataStream >> round;

                qDebug() << "confirmed received" << round;

                for (Player &player : _players) {

                    if (player.id == socket->socketDescriptor()) {

                        if (round == 1) {

                            player.state = CLIENT_STATE_FIRST_ROUND_OVER;
                        } else if (round == 2) {

                            player.state = CLIENT_STATE_SECOND_ROUND_OVER;
                        }
                    }
                }



                if (round == 1) {

                    bool allReady = true;

                    for (Player &player : _players) {

                        if (player.state != CLIENT_STATE_FIRST_ROUND_OVER) {

                            allReady = false;
                            break;
                        }
                    }

                    if (allReady) {

                        assignRoles(round + 1);
                        _currentRound = round + 1;

                        qDebug() << "assigning roles";
                    }

                } else if (round == 2) {

                    bool allReady = true;

                    for (Player &player : _players) {

                        if (player.state != CLIENT_STATE_SECOND_ROUND_OVER) {

                            allReady = false;
                            break;
                        }
                    }

                    if (allReady) {

                        assignRoles(round + 1);
                        _currentRound = round + 1;
                    }
                }
            } else if (type == TCP_DATA_TYPE_HERO_HEALTH_INCREASED || type == TCP_DATA_TYPE_HERO_MAGIC_INCREASED || type == TCP_DATA_TYPE_HERO_MAGIC_USED) {

                int8_t left;
                int16_t id;
                dataStream >> id >> left;

                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << type << size << id << left;
                for (QTcpSocket *mySocket : _clientSockets) {

                    if (socket->socketDescriptor() != mySocket->socketDescriptor()) {

                        mySocket->write(data.data(), data.size());
                        mySocket->flush();
                    }
                }
            } else if (type == TCP_DATA_TYPE_PAUSE_GAME || type == TCP_DATA_TYPE_RESUME_GAME) {

                qDebug() << "Server : pause/resume";

                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << type << size;
                for (QTcpSocket *mySocket : _clientSockets) {

                    mySocket->write(data.data(), data.size());
                    mySocket->flush();
                }
            }
        }

    }
}

void GameHoster::startGame()
{

    qDebug() << "Server: starting game";

    this->terminate();

    // send to all clients to start the game

    QByteArray startGame;
    QDataStream startStream(&startGame, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_NEED_CREATE_WORLD;
    int16_t size = 0;
    startStream << type << size;

    for (QTcpSocket *socket : _clientSockets) {

        socket->write(startGame.data(), startGame.size());
    }


}




//free functions

QDataStream & operator >> ( QDataStream & stream, Room & room )
{
        quint32 ipAddress;
        stream >> ipAddress;
        room.senderAddress = QHostAddress(ipAddress);
        stream >> room.roomName;
        stream >> room.map;
        stream >> room.maxPlayerNum;
        stream >> room.currentPlayerNum;

        return stream;
}

QDataStream & operator << ( QDataStream & stream, Room & room )
{
        stream << room.senderAddress.toIPv4Address();
        stream << room.roomName;
        stream << room.map;
        stream << room.maxPlayerNum;
        stream << room.currentPlayerNum;
        return stream;
}


QDataStream & operator >> ( QDataStream & stream, Player & player) {

    stream >> player.id;
    stream >> player.playerName;
    stream >> player.role;
    stream >> player.state;

    return stream;
}

QDataStream & operator << ( QDataStream & stream, Player & player) {

    stream << player.id;
    stream << player.playerName;
    stream << player.role;
    stream << player.state;

    return stream;
}

QDataStream & operator >> ( QDataStream & stream, HeroModel & hero) {

    stream >> hero.id;
    stream >> hero.name;
    stream >> hero.role;
    stream >> hero.x;
    stream >> hero.y;
    stream >> hero.healthPercentage;
    stream >> hero.magicPercentage;

    return stream;
}

QDataStream & operator << ( QDataStream & stream, HeroModel & hero) {

    stream << hero.id;
    stream << hero.name;
    stream << hero.role;
    stream << hero.x;
    stream << hero.y;
    stream << hero.healthPercentage;
    stream << hero.magicPercentage;

    return stream;
}

QDataStream & operator >> ( QDataStream & stream, OpponentModel & opponent) {

    stream >> opponent.id;
    stream >> opponent.type;
    stream >> opponent.x;
    stream >> opponent.y;
    stream >> opponent.healthPecentage;

    return stream;
}

QDataStream & operator << ( QDataStream & stream, OpponentModel & opponent ) {

    stream << opponent.id;
    stream << opponent.type;
    stream << opponent.x;
    stream << opponent.y;
    stream << opponent.healthPecentage;

    return stream;
}


QDataStream &operator >>(QDataStream &stream, GiftModel &gift)
{
    stream >> gift.id;
    stream >> gift.type;
    stream >> gift.x;
    stream >> gift.y;

    return stream;
}



QDataStream &operator <<(QDataStream &stream, GiftModel &gift)
{
    stream << gift.id;
    stream << gift.type;
    stream << gift.x;
    stream << gift.y;

    return stream;
}
