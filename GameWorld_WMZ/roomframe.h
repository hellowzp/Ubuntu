#ifndef ROOMFRAME_H
#define ROOMFRAME_H

#include <QFrame>
#include "gamehoster.h"
#include "mainwindow.h"

class RoomFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RoomFrame(Room &room);
    ~RoomFrame();
    void setMyName(QString &name);
    void setMainWindow(MainWindow *mainWindow);
    void updataFrame(Room &room);

signals:
    void closeRoomDialog();
public slots:

private slots:
    void joinGame();

private:
    Room _room;
    QString _myName;

    MainWindow *_mainWindow;

    QPushButton *_joinButton;
    QLabel *_currentPlayerNumLabel;

};

#endif // ROOMFRAME_H
