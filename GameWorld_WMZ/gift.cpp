#include "gift.h"
#include "assetlabel.h"
#include "mainwindow.h"

Gift::Gift(int id, int type, int x, int y) : Tile(x, y, 0), _id(id), _type(type)
{

    if (type == ASSERT_ITEM_HEALTH) {

        this->setPixmap(QPixmap(":/world/energy-icon.png").scaled(30 ,30));
    } else if (type == ASSERT_ITEM_MAGIC) {

         this->setPixmap(QPixmap(":/world/magic-icon.png").scaled(30 ,30));
    } else if (type == ASSERT_ITEM_SWORD) {

        this->setPixmap(QPixmap(":/world/sword-icon.png").scaled(30 ,30));
    } else if (type == ASSERT_ITEM_ARMOR) {

        this->setPixmap(QPixmap(":/world/armor-icon.png").scaled(30 ,30));
    } else if (type == ASSERT_ITEM_BOOT) {

        this->setPixmap(QPixmap(":/world/boot-icon.png").scaled(30 ,30));
    }

    this->setX(x * SCALE_FACTOR);
    this->setY(y * SCALE_FACTOR);

}

int Gift::getId() const
{
    return _id;
}

int Gift::getType() const
{
    return _type;
}

