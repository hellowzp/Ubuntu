#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <QTcpSocket>

#include "gamehoster.h"

class GameClient: public QObject
{

    Q_OBJECT
public:
    static void CreateClient(Room &room, Player &player);
    static void DestoryClient();
    static GameClient *Instance();
    void setPlayer(Player &player);
    ~GameClient();


    const Room & getRoom() const;

signals:

    void needSetMyPlayer(Player player);
    void needSetOtherPlayer(Player player);
    void needToCreateWorld(int map);
    void needToCreateHero(HeroModel hero);
    void needToAddHeroShadow(HeroModel hero);
    void needToMoveShadow(int id, int x, int y);
    void needToCreateOpponent(int type, int id, int x, int y);
    void needToRepathOpponents(int heroId);
    void needToAddOpponentShadow(OpponentModel opponent);
    void needToShadowHeroMagic(int id, int withSword, int x, int y);
    void needToShadowInjure(int id, int leftPercentage);
    void needToShadowHealthIncrease(int id, int leftP);
    void needToShadowMagicIncrease(int id, int leftP);
    void needToShadowMagicUsed(int id, int leftP);
    void needToInjureHero(int value);
    void needToRoundOver(int round);
    void needToAddHeroFrame(HeroModel hero);
    void needToAddGift(GiftModel gift);
    void needToRemoveGift(int giftId);
    void needToPickGift(int type);
    void needToGameOver(bool);
    void needToPauseGame();
    void needToResumeGame();
    void needToCloseDialog();


public slots:

    void playerChanged(Player player);
    void requestOtherPlayers();
    void worldCreated();
    void heroCreated(HeroModel hero);
    void heroMoved(int id, int x, int y);
    void heroMovingStoped(int id);
    void opponentCreated(OpponentModel opponent);
    void opponentMoved(int id, int x, int y);
    void magicReleased(int id, int withSword, int x, int y);
    void opponentInjured(int id, int leftPercentage);
    void injureHero(int id, int value);
    void heroInjured(int id, int leftPercentage);
    void giftPicked(int heroId, int giftId, int giftType);
    void roundOverConfirmed(int round);
    void heroHealthIncreased(int id, int leftP);
    void heroMagicIncreased(int id, int leftP);
    void heroMagicUsed(int id, int leftP);
    void pauseGame();
    void resumeGame();




private slots:
    void readData();

private:
    GameClient(Room &room, Player &player);
    static GameClient *_instance;
    QTcpSocket *_socket;
    Room _room;
    int _connectionId;
    Player _player;
    QVector<HeroModel> _otherHeros;


};


#endif // GAMECLIENT_H
