#ifndef NEWROOMDIALOG_H
#define NEWROOMDIALOG_H

#include <QDialog>
#include <QComboBox>
#include "mainwindow.h"

class QLineEdit;

class NewRoomDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewRoomDialog();
    ~NewRoomDialog();
    void setMyName(QString name);
    void setMainWindow(MainWindow *mainWindow);

signals:
    void newRoomCreated(QString roomName, int maxPlayerNum, int map);

public slots:

private slots:
    void CreateButtonCreated();
private:

    QLineEdit *_roomNameLineEdit;
    QComboBox *_maxPlayerNumComboBox;
    QComboBox *_mapComboBox;
    QString _myName;
    MainWindow *_mainWindow;


};

#endif // NEWROOMDIALOG_H
