#include "heroreal.h"
#include "mainwindow.h"
#include <QDebug>
#include "gift.h"
#include "hero.h"
#include "worldview.h"

HeroReal::HeroReal(int role, int id, const QString &name, int x, int y, int healthPercentage, int magicPercentage): Hero(role, id, name, x, y, healthPercentage, magicPercentage)
{


    //define attributes according to roles
    if (role == PLAYER_ROLE_MAGE) {

        _totalHealth = 800;
        _totalMagic = 2000;
    } else if (role == PLAYER_ROLE_PRIEST) {

        _totalHealth = 1200;
        _totalMagic = 1500;
    } else if (role == PLAYER_ROLE_WORRIOR) {

        _totalHealth = 2000;
        _totalMagic = 800;
    }

    this->_healthValue = healthPercentage * _totalHealth / 100;
    this->_magicValue = magicPercentage * _totalMagic / 100;

    QObject::connect(this, SIGNAL(needToMove()), this, SLOT(moveOneStep()));

    _sword = false;
    _armor = false;
    _boot = false;

    _isAlive = true;
    _isPaused = false;

    _magicAutoIncreaseTimer = new QTimer();
    _magicAutoIncreaseTimer->setInterval(1000);
    QObject::connect(_magicAutoIncreaseTimer, SIGNAL(timeout()), this, SLOT(magicAutoIncrease()));
    _magicAutoIncreaseTimer->start();


}

HeroReal::~HeroReal()
{
    qDebug() << "dtor of HeroReal";
    _magicAutoIncreaseTimer->stop();
    delete _magicAutoIncreaseTimer;
}


void HeroReal::run(){

    while (true) {

        if (!_isAlive) return;
        if (_isPaused) return;

        int speed = (_boot ? (_speed * 2) : _speed );

        this->msleep(100 + 1000 / speed);
        emit needToMove();
    }
}

void HeroReal::faceToDirection(int d) {

    this->_facingDir = d;
}

void HeroReal::startMoving(){

    this->start();
}

void HeroReal::stopMoving(){

    this->terminate();

    emit heroMovingStoped(this->getId());  //to game client
    emit needToRepathOpponents(this->getId()); // to world view
}

void HeroReal::moveOneStep() {

    int nextX;
    int nextY;

    switch (_facingDir) {
    case 1:
        nextX = this->getXPos();
        nextY = this->getYPos() - 1;
        break;
    case 2:
        nextX = this->getXPos() + 1;
        nextY = this->getYPos() - 1;
        break;
    case 3:
        nextX = this->getXPos() + 1;
        nextY = this->getYPos();
        break;
    case 4:
        nextX = this->getXPos() + 1;
        nextY = this->getYPos() + 1;
        break;
    case 5:
        nextX = this->getXPos();
        nextY = this->getYPos() + 1;
        break;
    case 6:
        nextX = this->getXPos() - 1;
        nextY = this->getYPos() + 1;
        break;
    case 7:
        nextX = this->getXPos() - 1;
        nextY = this->getYPos();
        break;
    case 8:
        nextX = this->getXPos() - 1;
        nextY = this->getYPos() - 1;
        break;
    default:
        break;
    }

    this->setXPos(nextX);
    this->setYPos(nextY);
    this->setPos(this->getXPos() * SCALE_FACTOR, this->getYPos() * SCALE_FACTOR);

    emit heroMoved(this->getId(), this->getXPos(), this->getYPos()); // to game client
    emit needToRepathOpponents(this->getId());

    for (Gift *gift : this->_world->_gifts) {

        if (this->getXPos() == gift->getXPos()) {

            if (this->getYPos() == gift->getYPos()) {

                qDebug() << "Hero : steped on a gift " << gift->getXPos() << " " << gift->getYPos();
                emit giftPicked(this->getId(), gift->getId(), gift->getType());

                break;
            }
        }
    }
}

void HeroReal::injureHero(int value)
{


    if (!_isAlive) return;

    if (_isPaused) return;

    //the harm will be halfed if hero wears a armor
    if (_armor) value /= 2;

    _healthValue -= value;
    if (_healthValue <= 0) {
        _healthValue = 0;

    }


    int percentage = this->calculateHealthPercentage();
    this->setHealthPercentage(percentage);

    emit heroInjured(this->getId(), percentage);
    emit updateProgressBars(1, _healthValue, _totalHealth);

    if (percentage == 0) {
        _isAlive = false;
        _magicAutoIncreaseTimer->stop();
    }


}

void HeroReal::equipmentChanged(bool sword, bool armor, bool boot)
{
    _sword = sword;
    _armor = armor;
    _boot = boot;

    qDebug() << "Hero : equipment changed " << _sword << " " << _armor << " " << _boot;
}

void HeroReal::drinkEnergy(int type)
{
    //return this function the hero is dead
    if (!_isAlive) return;

    if (_isPaused) return;

    if (type == ASSERT_ITEM_HEALTH) {

        _healthValue += 350;
        if (_healthValue > _totalHealth) _healthValue = _totalHealth;
        int percentage = this->calculateHealthPercentage();
        this->setHealthPercentage(percentage);
        emit heroHealthIncreased(this->getId(), percentage);
        emit updateProgressBars(1, _healthValue, _totalHealth);

    } else if (type == ASSERT_ITEM_MAGIC) {

        _magicValue += 400;
        if (_magicValue > _totalMagic) _magicValue = _totalMagic;
        int percentage = this->calculateHealthPercentage();
        this->setMagicPercentage(percentage);
        emit heroMagicIncreased(this->getId(), percentage);
        emit updateProgressBars(2, _magicValue, _totalMagic);
    }
}

void HeroReal::magicAutoIncrease()
{
    if (!_isAlive) return;

    if (_isPaused) return;

    _magicValue += 5;
    if (_magicValue > _totalMagic) _magicValue = _totalMagic;

    emit heroMagicIncreased(this->getId(), this->calculateMagicPercentage());
    emit updateProgressBars(2, _magicValue, _totalMagic);
}

void HeroReal::releaseMagicAt(int x, int y)
{

    if (!_isAlive) return;

    if (_isPaused) return;

    if (_magicValue == 0) return;

    if (!this->getMagic().isRunning()) {
        this->getMagic().releaseAt((_sword ? 1 : 0), x, y);
        emit magicReleased(this->getId(), (_sword ? 1 : 0), x, y);

        _magicValue -= 80;
        if (_magicValue < 0) {
            _magicValue = 0;
        }

        int percentage = _magicValue * 100 / _totalMagic;
        this->setMagicPercentage(percentage);
        emit heroMagicUsed(this->getId(), percentage);
        emit updateProgressBars(2, _magicValue, _totalMagic);

    }
}

bool HeroReal::isAlive()
{
    return _isAlive;
}




int HeroReal::calculateHealthPercentage()
{
    return _healthValue * 100 / _totalHealth;
}

int HeroReal::calculateMagicPercentage() {

    return _magicValue * 100 / _totalMagic;
}

void HeroReal::setPaused(bool paused)
{
    _isPaused = paused;
}

void HeroReal::setProgressBars()
{
    emit updateProgressBars(1, _healthValue, _totalHealth);
    emit updateProgressBars(2, _magicValue, _totalMagic);
}

void HeroReal::healHero(int value)
{
    if (!_isAlive) return;

    _healthValue += value;
    if (_healthValue > _totalHealth) _healthValue = _totalHealth;

    emit heroHealthIncreased(this->getId(), this->calculateHealthPercentage());
    emit updateProgressBars(1, _healthValue, _totalHealth);
}
