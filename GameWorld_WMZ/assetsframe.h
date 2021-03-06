#ifndef ASSETSFRAME_H
#define ASSETSFRAME_H

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVector>
#include "assetlabel.h"

class AssetsFrame : public QFrame
{
    Q_OBJECT
public:
    explicit AssetsFrame();
    ~AssetsFrame();

    void setDragSource(AssetLabel *source);
    AssetLabel *getDragSource() const;
    void equipmentChanged();
    void clearLabels();

signals:
    void equipmentChanged(bool sword, bool armor, bool boot);
    void drinkEnergy(int type);

public slots:

    void pickGift(int type);
    void keyPressed(int key);

private:



    QHBoxLayout *_energyLayout;
    QHBoxLayout *_equipmentLayout;
    QGridLayout *_assetsLayout;

    QVector<AssetLabel *> _energyLabels;
    QVector<AssetLabel *> _assetsLabels;


    AssetLabel *_swordLabel;
    AssetLabel *_armorLabel;
    AssetLabel *_bootLabel;


    AssetLabel *_dragSource;

};

#endif // ASSETSFRAME_H
