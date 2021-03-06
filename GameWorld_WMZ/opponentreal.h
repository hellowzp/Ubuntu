#ifndef OPPONENTREAL_H
#define OPPONENTREAL_H

#include <QThread>
#include "opponent.h"
#include "worldviewanalyzer.h"
#include <QMutex>

#define STEP_COST 100
class Hero;

class OpponentReal: public QThread, public Opponent
{
    Q_OBJECT

public:
    OpponentReal(int type, int id, const int &x, const int &y, int healthPercentage);
    ~OpponentReal();

    void activate();
    void deactivate();

    void redirectTo(int x, int y);
    void directToTarget();
    bool makeInjure(int value);
    void setWorld(WorldView *world);

    Hero *getTargetHero();
    void setTargetHero(Hero *);

    void setPause(bool pause);


signals:
    void positionChanged();
    void opponentMoved(int id, int x, int y);
    void opponentInjured(int id, int leftPercentage);
    void injureHero(int id, int value);

public slots:
    void refreshPosition();


private:

    void run();

    WorldView *_world;

    WorldPathFinder _finder;
    QMutex _pathFindingLocker;

    int _totalHealth;
    int _currentHealth;
    int _speed = 5;
    int _preX;
    int _preY;
    WorldViewAnalyzer _analyzer;

    Hero *_targetHero;

    bool _isAlive;
    bool _isPaused;
};

#endif // OPPONENTREAL_H
