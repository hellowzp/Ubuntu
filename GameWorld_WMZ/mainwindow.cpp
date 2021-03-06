#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "world.h"
#include <vector>
#include <iostream>
#include <typeinfo>
#include "chooseroomdialog.h"
#include <QDebug>
#include "gameclient.h"
#include "heroframe.h"
#include "assetsframe.h"
#include <QPushButton>
#include <QMessageBox>
#include <QProgressBar>
#include "worldview.h"
#include "worldview.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QHBoxLayout *mainLayout = new QHBoxLayout;

    /*set the main frame*/
    _mainFrame = new QWidget();
    _mainFrame->setObjectName("The main frame");
    _mainFrame->setLayout(mainLayout);

    _leftLayout = new QVBoxLayout();
    mainLayout->addLayout(_leftLayout);
    _leftLayout->setAlignment(Qt::AlignTop);


    QFont font;
    font.setPointSize(24);
    font.setBold(true);
    _healthBar = new QProgressBar();
    _magicBar = new QProgressBar();
    _healthBarLabel = new QLabel();
    _healthBarLabel->setFont(font);
    _magicBarLabel = new QLabel();
    _magicBarLabel->setFont(font);
    _leftLayout->addWidget(_healthBarLabel);
    _leftLayout->addWidget(_healthBar);
    _leftLayout->addWidget(_magicBarLabel);
    _leftLayout->addWidget(_magicBar);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    _pauseButton = new QPushButton("Pause");
    _pauseButton->setEnabled(false);
    QObject::connect(_pauseButton, SIGNAL(clicked()), this, SLOT(pauseClicked()));
    _resumeButton = new QPushButton("Resume");
    _resumeButton->setEnabled(false);
    QObject::connect(_resumeButton, SIGNAL(clicked()), this, SLOT(resumeClicked()));
    buttonsLayout->addWidget(_pauseButton);
    buttonsLayout->addWidget(_resumeButton);
    _leftLayout->addLayout(buttonsLayout);

    _colorButton = new QPushButton("Change background");
    _leftLayout->addWidget(_colorButton);
    QObject::connect(_colorButton, SIGNAL(clicked()), this, SLOT(colorClicked()));

    _assetsFrame = new AssetsFrame();
    _leftLayout->addWidget(_assetsFrame);

    _heroFrameLayout = new QVBoxLayout();
    _leftLayout->addLayout(_heroFrameLayout);

    _rightLayout = new QVBoxLayout();
    mainLayout->addLayout(_rightLayout);

    _mainFrame->setLayout(mainLayout);
    _mainFrame->setFixedSize(1200, 900);
    _mainFrame->show();

    this->resize(_mainFrame->size());


    this->show();

    ChooseRoomDialog *dialog = new ChooseRoomDialog();
    dialog->setMainWidow(this);
    dialog->show();

    _messageBox = new QMessageBox();
    _messageBox->hide();
    QObject::connect(_messageBox, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(messageConfirmed()));


    _mainFrame->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::enableButtons()
{
    this->_pauseButton->setEnabled(true);
    this->_resumeButton->setEnabled(true);
}

void MainWindow::createWorld(int map)
{
    qDebug() << "Client: creating a new world " << map;

    if (map == 1) {

        _worldView = new WorldView(":/world/world1.jpg");
    } else if (map == 2) {

        _worldView = new WorldView(":/world/world2.png");
    } else if (map == 3) {

        _worldView = new WorldView(":/world/world3.png");
    }
    _worldView->setObjectName("The world view");
    _worldView->show();
    _worldView->setAssetsFrame(_assetsFrame);
    _worldView->setMainWindow(this);


    _rightLayout->addWidget(_worldView);

    //emit the world created signal
    emit worldCreated();

    //connect the signal and slot between the game client and world view
    GameClient *client = GameClient::Instance();
    QObject::connect(client, SIGNAL(needToCreateHero(HeroModel)), _worldView, SLOT(createHero(HeroModel)));
    QObject::connect(_worldView, SIGNAL(heroCreated(HeroModel)), client, SLOT(heroCreated(HeroModel)));
    QObject::connect(client, SIGNAL(needToAddHeroShadow(HeroModel)), _worldView, SLOT(addHeroShadow(HeroModel)));
    QObject::connect(client, SIGNAL(needToMoveShadow(int,int,int)), _worldView, SLOT(moveShadow(int,int,int)));
    QObject::connect(client, SIGNAL(needToCreateOpponent(int,int,int,int)), _worldView, SLOT(createOpponent(int,int,int,int)));
    QObject::connect(_worldView, SIGNAL(OpponentCreated(OpponentModel)), client, SLOT(opponentCreated(OpponentModel)));
    QObject::connect(client, SIGNAL(needToRepathOpponents(int)), _worldView, SLOT(repathOpponents(int)));
    QObject::connect(client, SIGNAL(needToAddOpponentShadow(OpponentModel)), _worldView, SLOT(addOpponentShadow(OpponentModel)));
    QObject::connect(client, SIGNAL(needToShadowHeroMagic(int,int,int,int)), _worldView, SLOT(shadowHeroMagic(int,int,int,int)));
    QObject::connect(client, SIGNAL(needToShadowInjure(int,int)), _worldView, SLOT(shadowInjure(int,int)));
    QObject::connect(client, SIGNAL(needToRoundOver(int)), _worldView, SLOT(roundOver(int)));
    QObject::connect(client, SIGNAL(needToAddGift(GiftModel)), _worldView, SLOT(addGift(GiftModel)));
    QObject::connect(client, SIGNAL(needToRemoveGift(int)), _worldView, SLOT(removeGift(int)));
    QObject::connect(client, SIGNAL(needToAddHeroFrame(HeroModel)), this, SLOT(addHeroFrame(HeroModel)));
    QObject::connect(client, SIGNAL(needToPickGift(int)), _assetsFrame, SLOT(pickGift(int)));
    QObject::connect(_worldView, SIGNAL(roundOverConfirmed(int)), client, SLOT(roundOverConfirmed(int)));
    //QObject::connect(client, SIGNAL(needToGameOver(bool)), _worldView, SLOT(gameOver(bool)));
    QObject::connect(_worldView, SIGNAL(keyPressed(int)), _assetsFrame, SLOT(keyPressed(int)));
    QObject::connect(client, SIGNAL(needToShadowHealthIncrease(int,int)), _worldView, SLOT(shadowHealthIncresed(int,int)));
    QObject::connect(client, SIGNAL(needToShadowMagicIncrease(int,int)), _worldView, SLOT(shadowMagicIncreased(int,int)));
    QObject::connect(client, SIGNAL(needToShadowMagicUsed(int,int)), _worldView, SLOT(shadowMagicUsed(int,int)));
    QObject::connect(client, SIGNAL(needToPauseGame()), _worldView, SLOT(pauseGame()));
    QObject::connect(client, SIGNAL(needToResumeGame()), _worldView, SLOT(resumeGame()));
    QObject::connect(this, SIGNAL(changeBackground(int)), _worldView, SLOT(changeBackground(int)));

    _mainFrame->show();
}

void MainWindow::addHeroFrame(HeroModel hero)
{
    HeroFrame *heroFrame = new HeroFrame(hero);
    _heroFrameLayout->addWidget(heroFrame);

    qDebug() << "adding a frame";
}

void MainWindow::gameOver(bool win)
{

    qDebug() << "should show window";

    if (win) {

        _messageBox->setText("Game Over, You won.");
    } else {

        _messageBox->setText("Game Over, You lost.");
    }

    _messageBox->show();

}

void MainWindow::updateProgressBars(int type, int value, int total)
{

    qDebug() << "progress bar update";

    if (type == 1) {

        _healthBar->setMaximum(total);
        _healthBar->setValue(value);
        _healthBarLabel->setText(QString("Health value : %1/%2").arg(value).arg(total));


    } else {

        _magicBar->setMaximum(total);
        _magicBar->setValue(value);
        _magicBarLabel->setText(QString("Magic value : %1/%2").arg(value).arg(total));
    }
}

void MainWindow::pauseClicked()
{
    emit pauseGame();
}

void MainWindow::resumeClicked()
{
    emit resumeGame();
}

void MainWindow::messageConfirmed()
{
    qDebug() << "message confirmed";


    if (GameHoster::Instance() != nullptr)
        GameHoster::DestoryHoser();

    GameClient::DestoryClient();

    //remove and delete
    _rightLayout->removeWidget(_worldView);
    delete _worldView;

    _mainFrame->hide();

    // delete all item in hero frame layout
    QLayoutItem *item;
    while((item = _heroFrameLayout->takeAt(0))) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    _assetsFrame->clearLabels();

    ChooseRoomDialog *dialog = new ChooseRoomDialog();
    dialog->setMainWidow(this);
    dialog->show();
}

void MainWindow::colorClicked()
{
    static int color = 1;

    color++;
    color %= 3;

    emit changeBackground(color + 1);
}
