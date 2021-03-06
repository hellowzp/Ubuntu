#include "gameclient.h"
#include <QDebug>
#include <QDataStream>


GameClient* GameClient::_instance = 0;

GameClient::GameClient(Room &room, Player &player)
{
    this->_socket = new QTcpSocket();

    this->_room = room;
    this->_player = player;
    QObject::connect(_socket, SIGNAL(readyRead()), this, SLOT(readData()));
    this->_socket->connectToHost(_room.senderAddress, TCP_CONNECTION_PORT);
}

void GameClient::CreateClient(Room &room, Player &player)
{

    GameClient::_instance = new GameClient(room, player);
}

void GameClient::DestoryClient()
{
    delete _instance;
}

GameClient *GameClient::Instance()
{
    return GameClient::_instance;
}

void GameClient::setPlayer(Player &player)
{
    this->_player = player;
}

GameClient::~GameClient()
{
    _socket->close();
    delete _socket;
}

const Room &GameClient::getRoom() const
{
    return this->_room;
}

void GameClient::playerChanged(Player player)
{
    _player = player;

    //send the imformation of this player to server
    QByteArray data;
    QDataStream dataStream(&data, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_PLAYER;
    int16_t size = _player.getByteSize();
    dataStream << type << size << _player;
    this->_socket->write(data.data(), data.size());
    this->_socket->flush();
}

void GameClient::requestOtherPlayers()
{

    //request the information of all other player from the server
    QByteArray request;
    QDataStream requestStream(&request, QIODevice::WriteOnly);
    int16_t type1 = TCP_DATA_TYPE_QUERY_PLAYERS;
    int16_t size1= 0;
    requestStream << type1 << size1;
    this->_socket->write(request.data(), request.size());
    this->_socket->flush();

}

void GameClient::worldCreated()
{
    QByteArray created;
    QDataStream createdStream(&created, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_WORLD_CREATED;
    int16_t size = 0;
    createdStream << type << size;
    this->_socket->write(created.data(), created.size());
    this->_socket->flush();

}

void GameClient::heroCreated(HeroModel hero)
{

    //add the hero name to it
    hero.name = _player.playerName;

    emit needToAddHeroFrame(hero);

    QByteArray heroData;
    QDataStream heroStream(&heroData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_NEW_HERO;
    int16_t size = hero.getByteSize();
    heroStream << type << size << hero;
    this->_socket->write(heroData.data(), heroData.size());
    this->_socket->flush();
}

void GameClient::heroMoved(int id, int x, int y)
{
    QByteArray moveData;
    QDataStream moveStream(&moveData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_HERO_MOVE;
    int16_t size = sizeof(int16_t) * 3;
    moveStream << type << size << (int16_t)id << (int16_t)x << (int16_t)y;
    this->_socket->write(moveData.data(), moveData.size());
    this->_socket->flush();
}

void GameClient::heroMovingStoped(int id)
{
    QByteArray movingStoped;
    QDataStream movingStopedStream(&movingStoped, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_HERO_MOVING_STOPED;
    int16_t size = sizeof(int16_t);
    movingStopedStream << type << size << (int16_t)id;
    this->_socket->write(movingStoped.data(), movingStoped.size());
    this->_socket->flush();
}

void GameClient::opponentCreated(OpponentModel opponent)
{
    QByteArray opponentData;
    QDataStream opponentStream(&opponentData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_NEW_OPPONENT;
    int16_t size = opponent.getByteSize();
    opponentStream << type << size << opponent;
    this->_socket->write(opponentData.data(), opponentData.size());
    this->_socket->flush();
}

void GameClient::opponentMoved(int id, int x, int y)
{
    QByteArray opponentMoved;
    QDataStream movedStream(&opponentMoved, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_OPPONENT_MOVED;
    int16_t size = sizeof(int16_t) * 3;
    movedStream << type << size << (int16_t)id << (int16_t)x << (int16_t)y;
    this->_socket->write(opponentMoved.data(), opponentMoved.size());
    this->_socket->flush();
}

void GameClient::magicReleased(int id, int withSword, int x, int y)
{
    QByteArray magicData;
    QDataStream magicStream(&magicData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_HERO_MAGIC;
    int16_t size = sizeof(int16_t) * 3 + sizeof(int8_t);
    magicStream << type << size << (int16_t)id << (int8_t)withSword << (int16_t)x << (int16_t)y;
    this->_socket->write(magicData.data(), magicData.size());
    this->_socket->flush();
}

void GameClient::opponentInjured(int id, int leftPercentage)
{
    QByteArray injureData;
    QDataStream injureStream(&injureData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_OPPONENT_INJURED;
    int16_t size = sizeof(int16_t) + sizeof(int8_t);
    injureStream << type << size << (int16_t)id << (int8_t)leftPercentage;
    this->_socket->write(injureData.data(), injureData.size());
    this->_socket->flush();
}

void GameClient::injureHero(int id, int value)
{
    QByteArray injureData;
    QDataStream injureStream(&injureData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_INJURE_HERO;
    int16_t size = sizeof(int16_t) * 2;
    injureStream << type << size << (int16_t)id << (int16_t)value;
    this->_socket->write(injureData.data(), injureData.size());
    this->_socket->flush();
}

void GameClient::heroInjured(int id, int leftPercentage)
{
    QByteArray injureData;
    QDataStream injureStream(&injureData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_HERO_INJURED;
    int16_t size = sizeof(int16_t) + sizeof(int8_t);
    injureStream << type << size << (int16_t)id << (int8_t)leftPercentage;
    this->_socket->write(injureData.data(), injureData.size());
    this->_socket->flush();
}

void GameClient::giftPicked(int heroId, int giftId, int giftType)
{
    QByteArray giftData;
    QDataStream giftStream(&giftData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_GIFT_PICKED;
    int16_t size = sizeof(int16_t) * 3;
    giftStream << type << size << (int16_t)heroId << (int16_t)giftId << (int16_t)giftType;
    this->_socket->write(giftData.data(), giftData.size());
    this->_socket->flush();
}

void GameClient::roundOverConfirmed(int round)
{
    QByteArray roundData;
    QDataStream roundStream(&roundData, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_ROUND_OVER_CONFIRMED;
    int16_t size = sizeof(int8_t);
    roundStream << type << size << (int8_t)round;
    this->_socket->write(roundData.data(), roundData.size());
    this->_socket->flush();
}

void GameClient::heroHealthIncreased(int id, int leftP)
{
    QByteArray health;
    QDataStream healthStream(&health, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_HERO_HEALTH_INCREASED;
    int16_t size = sizeof(int8_t) + sizeof(int16_t);
    healthStream << type << size << (int16_t)id << (int8_t)leftP;
    this->_socket->write(health.data(), health.size());
    this->_socket->flush();
}

void GameClient::heroMagicIncreased(int id, int leftP)
{
    QByteArray magic;
    QDataStream magicStream(&magic, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_HERO_MAGIC_INCREASED;
    int16_t size = sizeof(int8_t) + sizeof(int16_t);
    magicStream << type << size << (int16_t)id << (int8_t)leftP;
    this->_socket->write(magic.data(), magic.size());
    this->_socket->flush();
}

void GameClient::heroMagicUsed(int id, int leftP)
{
    QByteArray magic;
    QDataStream magicStream(&magic, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_HERO_MAGIC_USED;
    int16_t size = sizeof(int8_t) + sizeof(int16_t);
    magicStream << type << size << (int16_t)id << (int8_t)leftP;
    this->_socket->write(magic.data(), magic.size());
    this->_socket->flush();

    qDebug() << "CLient: magic used percentage " << leftP;
}

void GameClient::pauseGame()
{
    QByteArray pause;
    QDataStream pauseStream(&pause, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_PAUSE_GAME;
    int16_t size = 0;
    pauseStream << type << size;
    this->_socket->write(pause.data(), pause.size());
    this->_socket->flush();
}

void GameClient::resumeGame()
{
    QByteArray resume;
    QDataStream resumeStream(&resume, QIODevice::WriteOnly);
    int16_t type = TCP_DATA_TYPE_RESUME_GAME;
    int16_t size = 0;
    resumeStream << type << size;
    this->_socket->write(resume.data(), resume.size());
    this->_socket->flush();
}

void GameClient::readData()
{

    while (this->_socket->bytesAvailable() > 0) {

        QByteArray headerData;
        headerData.resize(sizeof(int16_t) * 2);
        this->_socket->read(headerData.data(), sizeof(int16_t) * 2);
        QDataStream headerDataStream(&headerData, QIODevice::ReadOnly);
        int16_t type;
        int16_t size;
        headerDataStream >> type;
        headerDataStream >> size;

        QByteArray data;
        data.resize(size);
        this->_socket->read(data.data(), size);
        QDataStream dataStream(&data, QIODevice::ReadOnly);

        if (type == TCP_DATA_TYPE_CON_ID) {


            dataStream >> _connectionId;
            qDebug() << "Client : The connection id is " << _connectionId;

            //set the player id of this player
            this->_player.id = _connectionId;
            this->_player.state = CLIENT_STATE_CONNECTED;

            //send the imformation of this player to server
            QByteArray data;
            QDataStream dataStream(&data, QIODevice::WriteOnly);
            int16_t type = TCP_DATA_TYPE_PLAYER;
            int16_t size = _player.getByteSize();
            dataStream << type << size << _player;
            this->_socket->write(data.data(), data.size());
            this->_socket->flush();

            //add my player into UI
            emit needSetMyPlayer(_player);

        } else if (type == TCP_DATA_TYPE_PLAYER) {


            Player player;
            dataStream >> player;

            if (player.id != _player.id) {
                emit needSetOtherPlayer(player);
                qDebug() << "Client other player received with name:" << player.playerName;
            }
        } else if (type == TCP_DATA_TYPE_NEED_CREATE_WORLD) {

            qDebug() << "Client: start game received";

            //emit the create signal to the main window
            emit needToCreateWorld(_room.map);
            emit needToCloseDialog();

        } else if (type == TCP_DATA_TYPE_HERO_REQUEST) {

            qDebug() << "Client: hero request received";

            //emit the signal to world view
            int16_t x;
            int16_t y;
            dataStream >> x >> y;

            HeroModel hero;
            hero.x = x;
            hero.y = y;
            hero.name = _player.playerName;
            hero.healthPercentage = 100;
            hero.magicPercentage = 100;
            hero.role = _player.role;
            hero.id = _player.id;

            emit needToCreateHero(hero);

        } else if (type == TCP_DATA_TYPE_NEW_HERO) {

            HeroModel hero;
            dataStream >> hero;

            this->_otherHeros.push_back(hero);
            emit needToAddHeroShadow(hero);
            emit needToAddHeroFrame(hero);
            qDebug() << "Client other hero received";

        } else if (type == TCP_DATA_TYPE_HERO_MOVE) {

            int16_t id;
            int16_t x;
            int16_t y;

            dataStream >> id >> x >> y;

            emit needToMoveShadow(id, x, y); // to world view
            emit needToRepathOpponents(id);

        } else if (type == TCP_DATA_TYPE_OPPONENT_REQUEST) {


            int16_t opponentType;
            int16_t x;
            int16_t y;   
            dataStream >> opponentType >> x >> y;
             qDebug() << "Client: opponent request received " << x << " " << y;

            int id = (int)x * 10 + (int)y;

            emit needToCreateOpponent(opponentType, id, x, y);   //to world view

        } else if (type == TCP_DATA_TYPE_HERO_MOVING_STOPED) {

            qDebug() << "Client: oponent moving stoped";

            int16_t id;
            dataStream >> id;

            emit needToRepathOpponents(id); // to world view

        } else if (type == TCP_DATA_TYPE_NEW_OPPONENT) {



            OpponentModel opponent;
            dataStream >> opponent;

            qDebug() << "Client: new opponent " << opponent.id;

            emit needToAddOpponentShadow(opponent);  // to world view

        } else if (type == TCP_DATA_TYPE_OPPONENT_MOVED) {

            int16_t id;
            int16_t x;
            int16_t y;
            dataStream >> id >> x >> y;

            //qDebug() << "client: opponent moved" << id;

            emit needToMoveShadow(id, x ,y);

        } else if (type == TCP_DATA_TYPE_HERO_MAGIC) {

            int16_t id;
            int8_t withSword;
            int16_t x;
            int16_t y;
            dataStream >> id >> withSword >> x >> y;

            emit needToShadowHeroMagic(id, withSword, x, y);

        } else if (type == TCP_DATA_TYPE_OPPONENT_INJURED) {

            int16_t id;
            int8_t leftPercentage;
            dataStream >> id >> leftPercentage;

            emit needToShadowInjure(id, leftPercentage);

        } else if (type == TCP_DATA_TYPE_INJURE_HERO) {

            int16_t id;
            int16_t value;
            dataStream >> id >> value;

            emit needToInjureHero(value);

        } else if (type == TCP_DATA_TYPE_HERO_INJURED) {

            int16_t id;
            int8_t leftPercentage;
            dataStream >> id >> leftPercentage;

            emit needToShadowInjure(id, leftPercentage);

            //qDebug() << "come here";

        } else if (type == TCP_DATA_TYPE_ROUND_OVER) {

            int8_t round;
            dataStream >> round;

            emit needToRoundOver(round);

        } else if (type == TCP_DATA_TYPE_GAME_OVER) {

            int8_t win;
            dataStream >> win;

            qDebug() << "Client: game over received";

            emit needToGameOver(win);


        } else if (type == TCP_DATA_TYPE_NEW_GIFT) {


            GiftModel gift;
            dataStream >> gift;

            emit needToAddGift(gift);

        } else if (type == TCP_DATA_TYPE_GIFT_PICKED) {

            int16_t heroId;
            int16_t giftId;
            int16_t giftType;

            dataStream >> heroId >> giftId >> giftType;

            emit needToRemoveGift(giftId);

            if (_player.id == heroId) {

                emit needToPickGift(giftType);
            }
        } else if (type == TCP_DATA_TYPE_HERO_HEALTH_INCREASED) {

            int16_t id;
            int8_t left;

            dataStream >> id >> left;

            emit needToShadowHealthIncrease(id, left);

        } else if (type == TCP_DATA_TYPE_HERO_MAGIC_INCREASED) {

            int16_t id;
            int8_t left;

            dataStream >> id >> left;

            emit needToShadowMagicIncrease(id, left);

        } else if (type == TCP_DATA_TYPE_HERO_MAGIC_USED) {

            int16_t id;
            int8_t left;

            dataStream >> id >> left;

            emit needToShadowMagicUsed(id, left);

        } else if (type == TCP_DATA_TYPE_PAUSE_GAME) {

            emit needToPauseGame();

        } else if (type == TCP_DATA_TYPE_RESUME_GAME) {

            emit needToResumeGame();
        }





    }


}


