#include "worldviewanalyzer.h"
#include "opponent.h"
#include "opponentreal.h"
#include "opponentshadow.h"
#include "heroreal.h"
#include "heroshadow.h"
#include "worldview.h"
#include "worldbase.h"

WorldViewAnalyzer::WorldViewAnalyzer()
{
}

WorldViewAnalyzer::~WorldViewAnalyzer()
{
    qDebug() << "dtor of WorldViewAnalyzer";
}

bool WorldViewAnalyzer::checkValidation(int x, int y) {

    if (x < 0 || x > this->_targetWordView->_world->getCols() - 1) return false;
    if (y < 0 || y > this->_targetWordView->_world->getRows() - 1) return false;

//    HeroReal *hero = this->_targetWordView->_hero;
//    if ((x == hero->getXPos()) && (y == hero->getYPos())) return false;

//    for (HeroShadow *shadow : this->_targetWordView->_heroShadows) {

//        if ((x == shadow->getXPos()) && (y == shadow->getYPos())) return false;
//    }

//    for (Opponent *opponent : this->_targetWordView->_opponents) {

//        if ((x == opponent->getXPos()) && (y == opponent->getYPos())) return false;
//    }

//    for (OpponentShadow *shadow : this->_targetWordView->_opponentShadows) {

//        if ((x == shadow->getXPos()) && (y == shadow->getYPos())) return false;
//    }

    return true;
}

float WorldViewAnalyzer::valueDifference(int x1, int y1, int x2, int y2) {

    float value1 = this->_targetWordView->getWorldBaseAtIndex(y1 * this->_targetWordView->_world->getCols() + x1)->getValue();
    float value2 = this->_targetWordView->getWorldBaseAtIndex(y2 * this->_targetWordView->_world->getCols() + x2)->getValue();

    if ((value2 - value1) > 0) return value2 - value1;
    else return 0;
}

void WorldViewAnalyzer::setWorld(WorldView *world)
{
    this->_targetWordView = world;
}
