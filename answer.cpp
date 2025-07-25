/*
 * Copyright (c) 2011-2012, Christian Lange
 * (chlange) <chlange@htwg-konstanz.de> <Christian_Lange@hotmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Christian Lange nor the names of its
 *       contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CHRISTIAN LANGE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QElapsedTimer>
#include <QDebug>
#include "answer.h"

#include <algorithm> // for std::min

#define stringify(x) #x
#define FILE2(a) stringify(ui_answer_qt ## a.h)
#define FILE(a) FILE2(a)
#include FILE(QT_VERSION_MAJOR)

void Answer::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

Answer::Answer(QWidget *parent, QString file, int round, Player *players, int playerNr, bool sound, int currentPlayerId) :
        QDialog(parent), ui(new Ui::Answer), round(round), playerNr(playerNr),points(0), currentPlayerId(currentPlayerId),
        winner(NO_WINNER), keyLock(false), sound(sound), doubleJeopardy(false), result(), fileString(file), players(players), currentPlayer(), dj(NULL)
{
    ui->setupUi(this);

    this->time = new QElapsedTimer();
    this->time->start();
    timer = new QTimer();
    timer->setInterval(1*1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start();

    this->hideButtons();
    get_view()->setVisible(false);

#if QT_VERSION_MAJOR == 6
    this->musicPlayer = new QMediaPlayer(this);
    this->audioOutput = new QAudioOutput(this);

    QVideoWidget* videoWidget = findChild<QVideoWidget*>("videoPlayer");
    assert(videoWidget != nullptr);
    this->videoWidget = videoWidget;
    this->videoPlayer = new QMediaPlayer(this);

    videoWidget->setVisible(false);


    if(sound) {
        musicPlayer->setSource(QUrl::fromLocalFile("sound/jeopardy.wav"));
        audioOutput->setVolume(100);
        musicPlayer->play();
    }
#elif QT_VERSION_MAJOR == 5
    ui->videoPlayer->setVisible(false);
    video = new QMediaPlayer();
    video->setVideoOutput(ui->videoPlayer);

    if(sound)
        this->music = new QSound("sound/jeopardy.wav");
#else
    #error "Unsupported Qt Version"
#endif

    this->isVideo = false;
}

Answer::~Answer()
{
    delete ui;
#if QT_VERSION_MAJOR == 6
    if(this->sound) {
        delete this->musicPlayer;
        delete this->audioOutput;
    }
#elif QT_VERSION_MAJOR == 5
    if(this->sound)
        delete this->music;
#else
    #error "Unsupported Qt Version"
#endif

    if(this->dj != NULL)
        delete this->dj;

    delete this->time;
    delete timer;
}

void Answer::updateTime()
{
    int seconds = 31 - this->time->elapsed() / 1000;
    if(seconds >= 0)
        ui->time->setText(QString("%1").arg(std::min(30, seconds), 2));
    else
        ui->time->setText(QString("Ended..."));
}

int Answer::getWinner()
{
    return this->winner;
}

int Answer::getPoints()
{
    return this->points;
}

QString Answer::getResult()
{
    return this->result;
}

void Answer::setAnswer(int category, int points)
{
    this->points = points;
    QString answer;

    if(this->getAnswer(category, points, &answer) != true)
    {
        this->winner = NO_WINNER;
        done(0);
    }

    // Comments: e.g., ## some text ##
    QRegularExpression comment(R"(##.+##)");

    // Tags at the beginning of lines:
    QRegularExpression imgTag(R"(^\[img\])");
    QRegularExpression videoTag(R"(^\[video\])");
    QRegularExpression soundTag(R"(^\[sound\])");

    // Alignment or formatting tags (anywhere in line):
    QRegularExpression alignLeftTag(R"(\[l\])");
    QRegularExpression doubleJeopardyTag(R"(\[dj\])");
    QRegularExpression lineBreakTag(R"(\[b\])");
    QRegularExpression noEscape(R"(\[nE\])");
    QRegularExpression space(R"(\[s\])");

    answer.remove(comment);
    answer.replace(lineBreakTag,"<br>");
    answer.replace(space, "&nbsp;");

    if(answer.contains(alignLeftTag))
        this->processAlign(&answer);

    if(answer.contains(noEscape))
    {
        answer.remove(noEscape);
        ui->answer->setTextFormat(Qt::PlainText);
    }

    if(answer.contains(doubleJeopardyTag))
        this->processDoubleJeopardy(&answer);

    answer = answer.trimmed();

    if(answer.contains(imgTag))
    {
        if(this->sound){
#if QT_VERSION_MAJOR == 6
            this->musicPlayer->setAudioOutput(audioOutput);
#endif
            get_musicplayer()->play();
        }

        answer.remove(imgTag);
        answer = answer.trimmed();
        this->processImg(&answer);
    }
    else if(answer.contains(soundTag))
    {
        answer.remove(soundTag);
        answer = answer.trimmed();
        this->processSound(&answer);
    }
    else if(answer.contains(videoTag))
    {
        answer.remove(videoTag);
        answer = answer.trimmed();
        this->processVideo(&answer);
    }
    else
    {
        if(this->sound){
#if QT_VERSION_MAJOR == 6
            this->musicPlayer->setAudioOutput(audioOutput);
#endif
            get_musicplayer()->play();
        }

        this->processText(&answer);
    }
}

void Answer::processAlign(QString *answer)
{
    QRegularExpression alignLeftTag(R"(\[l\])");
    answer->remove(alignLeftTag);
    ui->answer->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void Answer::processDoubleJeopardy(QString *answer)
{
    QRegularExpression doubleJeopardyTag(R"(\[dj\])");
    answer->remove(doubleJeopardyTag);
    this->openDoubleJeopardy();
}

#if QT_VERSION_MAJOR == 6
QLabel *Answer::get_view()
{
    return ui->imageView;
}
QMediaPlayer *Answer::get_musicplayer()
{
    return this->musicPlayer;
}
#elif QT_VERSION_MAJOR == 5
QGraphicsView *Answer::get_view()
{
    return ui->graphicsView;
}
QSound *Answer::get_musicplayer()
{
    return this->music;
}
#else
    #error "Unsupported Qt Version"
#endif

void Answer::processImg(QString *answer)
{
    this->prependDir(answer);

    get_view()->setVisible(true);

    QPixmap pic(*answer);

    if(pic.height() > get_view()->height())
        pic = pic.scaledToHeight(get_view()->height() - 10);

    if(pic.width() > get_view()->width())
        pic = pic.scaledToWidth(get_view()->width() - 10);

#if QT_VERSION_MAJOR == 6
    ui->answer->setVisible(false);
    ui->imageView->setPixmap(pic);
#elif QT_VERSION_MAJOR == 5
    QGraphicsScene *scene = new QGraphicsScene(ui->graphicsView);
    scene->addPixmap(pic);
    ui->graphicsView->setScene(scene);
#else
    #error "Unsupported Qt Version"
#endif
    get_view()->show();
}

void Answer::processSound(QString *answer)
{
    this->prependDir(answer);

    this->sound = true;
#if QT_VERSION_MAJOR == 6
    this->musicPlayer->stop(); // stop default melody
    this->musicPlayer->setAudioOutput(this->audioOutput);
    this->musicPlayer->setSource(QUrl::fromLocalFile(*answer));
    this->audioOutput->setVolume(100);
    this->musicPlayer->play();

    QTimer::singleShot(30000, this->musicPlayer, SLOT(stop()));
#elif QT_VERSION_MAJOR == 5
    this->music = new QSound(*answer);
    this->music->play();
    QTimer::singleShot(30000, this->music, SLOT(stop()));
#else
    #error "Unsupported Qt Version"
#endif
}

void Answer::processVideo(QString *answer)
{
    this->isVideo = true;
    this->prependDir(answer);

#if QT_VERSION_MAJOR == 6
    this->videoWidget->setVisible(true);

    this->videoPlayer->setVideoOutput(videoWidget);
    this->videoPlayer->setAudioOutput(audioOutput);
    this->videoPlayer->setSource(QUrl::fromLocalFile(*answer));
    this->videoPlayer->play();


    QTimer::singleShot(30000, ui->videoPlayer, SLOT(stop()));
#elif QT_VERSION_MAJOR == 5
    video->setMedia(QMediaContent(QUrl::fromLocalFile(*answer)));
    ui->videoPlayer->setVisible(true);
    video->play();
    QTimer::singleShot(30000, video, SLOT(stop()));
#else
    #error "Unsupported Qt Version"
#endif

}

void Answer::processText(QString *answer)
{
    int count = answer->count("<br>");
    ui->answer->setFont(this->measureFontSize(count));
    ui->answer->setText(*answer);
}

void Answer::prependDir(QString *answer)
{
    answer->prepend(QString("/answers/%1/").arg(this->round));
    answer->prepend(QDir::currentPath());
}

void Answer::keyPressEvent(QKeyEvent *event)
{
    int key;
    int player = -1;

    if(this->sound && event->key() == Qt::Key_Shift)
    {
#if QT_VERSION_MAJOR == 6
        if(this->isVideo == true)
        {
            this->videoPlayer->stop();
            this->videoPlayer->setPosition(0);
            QTimer::singleShot(100, this->videoPlayer, SLOT(play()));
            QTimer::singleShot(30000, this->videoPlayer, SLOT(stop()));
        }
        else
        {
            this->musicPlayer->stop();
            QTimer::singleShot(100, this->musicPlayer, SLOT(play()));
            QTimer::singleShot(30000, this->musicPlayer, SLOT(stop()));
        }
#elif QT_VERSION_MAJOR == 5
        if(this->isVideo == true)
        {
            video->stop();
            video->setPosition(0);
            QTimer::singleShot(100, video, SLOT(play()));
            QTimer::singleShot(30000, video, SLOT(stop()));
        }
        else
        {
            this->music->stop();
            QTimer::singleShot(100, this->music, SLOT(play()));
            QTimer::singleShot(30000, this->music, SLOT(stop()));
        }
#else
        #error "Unsupported Qt Version"
#endif

        this->time->start();
    }

    if(event->key() == Qt::Key_Escape)
        this->on_buttonEnd_clicked();

    if(this->keyListenerIsLocked() == true)
        return;
    else
        this->lockKeyListener();

    key = event->key();

    for(int i = 0; i <  this->playerNr; i++)
        if(key == this->players[i].getKey())
            player = i;

    if(player != -1)
        this->processKeypress(player);
    else
        this->releaseKeyListener();
}

void Answer::processKeypress(int player)
{
    if(this->time->elapsed() < 31000)
    {
        if(this->isVideo == true)
        {
            this->videoPlayer->pause();
        }
        if (sound)
        {
            this->musicPlayer->pause();
        }
        this->currentPlayer = this->players[player];
        ui->currentPlayer->setText(this->currentPlayer.getName());

        this->showButtons();
    }
}

bool Answer::keyListenerIsLocked()
{
    return this->keyLock;
}

void Answer::lockKeyListener()
{
    this->keyLock = true;
}

void Answer::releaseKeyListener()
{
    this->keyLock = false;
}

void Answer::showButtons()
{
    /* Show by color and name which player should ask */
    QString color = QString("QLabel { background-color : %1; }").arg(this->currentPlayer.getColor());
    ui->currentPlayer->setStyleSheet(color);

    ui->buttonCancel->setVisible(true);
    ui->buttonRight->setVisible(true);
    ui->buttonWrong->setVisible(true);
    ui->currentPlayer->setVisible(true);
    ui->currentPlayer->raise();
}

void Answer::hideButtons()
{
    ui->buttonCancel->setVisible(false);
    ui->buttonRight->setVisible(false);
    ui->buttonWrong->setVisible(false);
    ui->currentPlayer->setVisible(false);
}

QFont Answer::measureFontSize(int count)
{
    QFont font;

    if(count > MANY_LINE_BREAKS)
        font.setPointSize(7);
    else if(count > MORE_LINE_BREAKS)
        font.setPointSize(15);
    else if(count > SOME_LINE_BREAKS)
        font.setPointSize(20);
    else
        font.setPointSize(28);

    return font;
}

QString Answer::getRoundFile()
{
    return this->fileString;
}

int Answer::getCategoryLine(int category)
{
    int categoryLine;

    categoryLine = NUMBER_MAX_CATEGORIES * (category - 1) + 1;

    return categoryLine;
}

bool Answer::getAnswer(int category, int points, QString *answer)
{
    int categoryFileLine;
    QString currentLine;
    QString delimiter;

    QFile file(this->getRoundFile());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open round file"));
        return false;
    }

    /* Calculate round answer line */
    categoryFileLine = this->getCategoryLine(category);
    QTextStream in(&file);

    /* Step to appropriate category section */
    for(int lineNr = 0; lineNr != categoryFileLine; lineNr++)
        currentLine = in.readLine();

    /* Prepare answer and delimiter variable (Points: Answer)*/
    delimiter = QString("%1:").arg(points);

    for(int i = 0; i < points / 100; i++)
        currentLine = in.readLine();

    /* Remove preceding points */
    *answer = currentLine;

    answer->remove(0, ANSWER_POINTS_INDICATOR_LENGTH);
    
    return true;
}

void Answer::openDoubleJeopardy()
{
    this->lockKeyListener();
    this->dj = new DoubleJeopardy(this, points / 2, points * 2, this->players, this->playerNr, this->currentPlayerId);
    dj->init();
    dj->show();
    this->currentPlayerId = dj->getPlayer();
    this->points = dj->getPoints();
    this->doubleJeopardy = true;

    this->time->restart(); // ignore selection time

    this->processKeypress(this->currentPlayerId);
}

void Answer::on_buttonEnd_clicked()
{
    this->releaseKeyListener();

    if(this->isVideo == true)
    {
        this->videoPlayer->pause();
    }
    if (sound)
    {
        this->musicPlayer->pause();
    }

    QMessageBox msgBox;
    msgBox.setText("Are you sure?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgBox.setDefaultButton(QMessageBox::Abort);
    int ret = msgBox.exec();

    if(ret == QMessageBox::Yes)
    {
        if(this->sound) {
#if QT_VERSION_MAJOR == 6
            this->musicPlayer->stop();
#elif QT_VERSION_MAJOR == 5
            this->music->stop();
#else
            #error "Unsupported Qt Version"
#endif
        }
        this->winner = NO_WINNER;
        done(0);
    }

    if(isVideo == true) {
        this->videoPlayer->play(); // resumes
    }
    if (sound == true)
    {
        musicPlayer->play(); // resume
    }
}

void Answer::on_buttonRight_clicked()
{
    QString resultTmp;
    resultTmp = QString("%1").arg(this->currentPlayer.getId());
    resultTmp.append(WON);
    this->result.append(resultTmp);
    this->releaseKeyListener();
    if(this->sound) {
#if QT_VERSION_MAJOR == 6
        this->musicPlayer->stop();
#elif QT_VERSION_MAJOR == 5
        this->music->stop();
#else
        #error "Unsupported Qt Version"
#endif
    }
    this->winner = this->currentPlayer.getId() - OFFSET;
    done(0);
}

void Answer::on_buttonWrong_clicked()
{
    QString resultTmp;
    resultTmp = QString("%1").arg(this->currentPlayer.getId());
    resultTmp.append(LOST);
    this->result.append(resultTmp);
    this->hideButtons();
    this->releaseKeyListener();
    if(this->doubleJeopardy)
    {
        if(this->sound) {
#if QT_VERSION_MAJOR == 6
            this->musicPlayer->stop();
#elif QT_VERSION_MAJOR == 5
            this->music->stop();
#else
            #error "Unsupported Qt Version"
#endif
        }
        this->winner = NO_WINNER;
        done(0);
    }
    if(isVideo == true) {
        this->videoPlayer->play(); // resumes
    }
    if (sound == true)
    {
        musicPlayer->play(); // resume
    }
}

void Answer::on_buttonCancel_clicked()
{
    this->hideButtons();
    this->releaseKeyListener();

    if(isVideo == true) {
        this->videoPlayer->play(); // resumes
    }
    if (sound == true)
    {
        musicPlayer->play(); // resume
    }
}
