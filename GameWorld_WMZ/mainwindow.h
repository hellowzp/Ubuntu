#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QSizePolicy>
#include "assetsframe.h"
#include "gamehoster.h"


#define SCALE_FACTOR 10

class WorldView;
class QPushButton;
class QMessageBox;
class QProgressBar;
class QFrame;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void enableButtons();


signals:
    void worldCreated();
    void pauseGame();
    void resumeGame();
    void changeBackground(int color);

public slots:
    void createWorld(int map);
    void addHeroFrame(HeroModel hero);
    void gameOver(bool win);
    void updateProgressBars(int type, int value, int total);

private slots:
    void pauseClicked();
    void resumeClicked();
    void messageConfirmed();
    void colorClicked();

private:
    Ui::MainWindow *ui;
    QWidget *_mainFrame;
    AssetsFrame *_assetsFrame;

    QVBoxLayout *_leftLayout;
    QVBoxLayout *_rightLayout;

    QPushButton *_pauseButton;
    QPushButton *_resumeButton;
    QPushButton *_colorButton;

    QMessageBox *_messageBox;

    WorldView *_worldView;

    QVBoxLayout *_heroFrameLayout;

    QLabel *_healthBarLabel;
    QLabel *_magicBarLabel;
    QProgressBar *_healthBar;
    QProgressBar *_magicBar;
};

#endif // MAINWINDOW_H
