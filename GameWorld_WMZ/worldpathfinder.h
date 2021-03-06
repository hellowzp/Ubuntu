#ifndef WORLDPATHFINDER_H
#define WORLDPATHFINDER_H

#define HEURISTIC_WIGHT 5
#define GIVEN_WIGHT 200

#include "worldviewanalyzer.h"
#include <QVector>


struct PathNode{

    PathNode(int x, int y, int g, int h , PathNode *parent = nullptr) {
        this->x = x;
        this->y = y;
        this->F = g + h;
        this->G = g;
        this->H = h;
        this->parent = parent;
    }
    int x, y, F, G, H;
    PathNode *parent;
};


class WorldPathFinder
{
public:
    WorldPathFinder();
    ~WorldPathFinder();
    void findPath(int xO, int yO, int xD, int yD);
    void setWorld(WorldView *world);

private:
    WorldViewAnalyzer _analyzer;

    QVector<PathNode *> _openList;
    QVector<PathNode *> _closeList;
    QVector<PathNode *> _pathNodes;

    PathNode *_currentNode;
    int _xO, _yO, _xD, _yD;

    PathNode * getFLowestNode();
    bool checkIfOnOpenList(int x, int y);
    bool checkIfOnCloseList(int x, int y);
    void handleAdjPoint(int x, int y);
    void cleanUpNodes();

    friend class OpponentReal;
};



#endif // WORLDPATHFINDER_H
