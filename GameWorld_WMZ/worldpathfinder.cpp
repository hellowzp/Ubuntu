#include "worldpathfinder.h"
#include <QtCore/QtMath>
#include <iostream>
#include <QDebug>



WorldPathFinder::WorldPathFinder()
{

}

WorldPathFinder::~WorldPathFinder()
{
    qDebug() << "dtor of WorldPathFinder";

    for (PathNode *node : _openList) delete node;
    for (PathNode *node : _closeList) delete node;
}

void WorldPathFinder::findPath(int xO, int yO, int xD, int yD) {

    this->cleanUpNodes();

    this->_openList.clear();
    this->_closeList.clear();
    this->_pathNodes.clear();

    this->_xO = xO;
    this->_yO = yO;
    this->_xD = xD;
    this->_yD = yD;


    PathNode *startNode = new PathNode(xO, yO, 0, 0, nullptr);
    this->_openList.push_back(startNode);
    _currentNode = startNode;

    while (this->_openList.size() >0 && !(_currentNode->x == _xD && _currentNode->y == _yD)) {


        this->_currentNode = this->getFLowestNode();
       // std::cout << _currentNode->x << "   "<<_currentNode->y << std::endl;
        this->_openList.remove(this->_openList.indexOf(this->_currentNode));
        this->_closeList.push_back(this->_currentNode);

        int adjX, adjY;
        adjX = _currentNode->x + 1;
        adjY = _currentNode->y - 1;
        this->handleAdjPoint(adjX, adjY);
        adjX = _currentNode->x + 1;
        adjY = _currentNode->y;
        this->handleAdjPoint(adjX, adjY);
        adjX = _currentNode->x + 1;
        adjY = _currentNode->y + 1;
        this->handleAdjPoint(adjX, adjY);
        adjX = _currentNode->x;
        adjY = _currentNode->y + 1;
        this->handleAdjPoint(adjX, adjY);
        adjX = _currentNode->x - 1;
        adjY = _currentNode->y + 1;
        this->handleAdjPoint(adjX, adjY);
        adjX = _currentNode->x - 1;
        adjY = _currentNode->y;
        this->handleAdjPoint(adjX, adjY);
        adjX = _currentNode->x - 1;
        adjY = _currentNode->y - 1;
        this->handleAdjPoint(adjX, adjY);
        adjX = _currentNode->x;
        adjY = _currentNode->y - 1;
        this->handleAdjPoint(adjX, adjY);
    }


    PathNode *onPathNode = _currentNode;
    while (!(onPathNode->x == _xO && onPathNode->y == _yO)) {
        //std::cout << onPathNode->x << "  " << onPathNode->y << std::endl;
        this->_pathNodes.push_back(onPathNode);
        onPathNode = onPathNode->parent;
    }



    //std::cout << "Recalculated " << std::endl;

}

void WorldPathFinder::setWorld(WorldView *world)
{
    this->_analyzer.setWorld(world);
}

PathNode *WorldPathFinder::getFLowestNode(){

    PathNode *lowestNode = nullptr;
    if (_openList.size() > 0) {

        lowestNode = _openList[0];

        for (PathNode *node : _openList) {
            if (node->F < lowestNode->F) lowestNode = node;
        }
    }

    return lowestNode;
}



bool WorldPathFinder::checkIfOnOpenList(int x, int y) {

    for (PathNode *node : _openList) {

        if (node->x == x && node->y == y) return true;
    }


    return false;
}

bool WorldPathFinder::checkIfOnCloseList(int x, int y) {

    for (PathNode *node : _closeList) {

        if (node->x == x && node->y == y) return true;
    }

    return false;
}

void WorldPathFinder::handleAdjPoint(int x, int y) {

    if (! checkIfOnCloseList(x, y)) {

        if (this->_analyzer.checkValidation(x, y)) {

            if (checkIfOnOpenList(x, y)) {
                //do nothing here in this case
            } else {
                /*Add the adj node to the open list*/
                int heuristicCost = (qFabs(x - _xD) + qFabs(y - _yD)) * HEURISTIC_WIGHT;
                int givenCost = _currentNode->G + this->_analyzer.valueDifference(_currentNode->x, _currentNode->y, x, y) * GIVEN_WIGHT;
                PathNode *newNode = new PathNode(x, y, givenCost, heuristicCost, _currentNode);
                this->_openList.push_back(newNode);
            }
        }
    }
}

void WorldPathFinder::cleanUpNodes()
{
    for (PathNode * node : this->_closeList)
        delete node;
    for (PathNode * node : this->_openList)
        delete node;


}





