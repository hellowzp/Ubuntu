#ifndef WORLDVIEWANALYZER_H
#define WORLDVIEWANALYZER_H


class WorldView;

class WorldViewAnalyzer
{
public:
    WorldViewAnalyzer();
    ~WorldViewAnalyzer();

    bool checkValidation(int x, int y);
    float valueDifference(int x1, int y1, int x2, int y2);
    void setWorld(WorldView *world);

private:
    WorldView const *_targetWordView;
};

#endif // WORLDVIEWANALYZER_H
