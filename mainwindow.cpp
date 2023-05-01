#include "mainwindow.h"
#include "profilewindow.h"
#include "ui_mainwindow.h"
#include "lib/gui_lib.h"
#include<QString>
#include <QScreen>
#include <QPixmap>
#include <QDateTime>
#include "lib/filesystem_lib.h"
#include "addcontact.h"
#include <QPropertyAnimation>
#include "startnewchat.h"
#include "application/application.h"
#include <QScrollBar>
#include "logic/conversation.h"
#include "logic/message.h"
#include "customGUI/qclickablegroubox.h"
#include <QTimer>
#include <QMessageBox>
#include "lib/gui_render.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    ui->setupUi(this);
    // Set the window to open as full screen

    ui->stackedWidget->setCurrentIndex(0);
    connect(ui->sendMessageLineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::on_pushButton_7_clicked);
    GUI_lib::setUpWindow(this, "Chat Vibes", ":/imgs/logo.png");
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    ui->scrollAreaWidgetContents->setContentsMargins(9,20,9,40);
    ui->scrollArea_2->verticalScrollBar()->setSingleStep(6);

    // Create a QPropertyAnimation object to animate the window's opacity
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(150);  // set the animation duration to 500 milliseconds
    animation->setStartValue(0.0);  // set the start opacity value to 0
    animation->setEndValue(1.0);  // set the end opacity value to 1

    // Connect the finished() signal of the animation to a slot that shows the window
    //    connect(animation, &QPropertyAnimation::finished, this, &MainWindow::show);

    animation->start(QAbstractAnimation::DeleteWhenStopped);

    // Make a copy of the original stack
    std::stack<Conversation*> tempConversations ;

    qDebug()<<Application::loggedUser->getUserName() ;
    qDebug()<<Application::loggedUser->getConversations().size();

    // Render the copied conversations
    while (!Application::loggedUser->getConversations().empty()) {

        qDebug()<<"con Name: "<<Application::loggedUser->getConversations().top()->getName();
        Conversation* conversationPtr = (Application::loggedUser->getConversations().top());
        tempConversations.push(conversationPtr);

        // Convert the address to a string
        std::stringstream ss;
        ss << conversationPtr;
        std::string conversationAddress = ss.str();

        // Create the QClickableGroupBox widget
        QClickableGroupBox *renderConversation = Application::renderConversation(conversationPtr);
        conversationPtr->setConversationGroupBoxAddress(renderConversation);
        renderConversation->setObjectName(QString::fromStdString(conversationAddress));
        ui->contactsCont->layout()->addWidget(renderConversation);
        renderConversation->setEnabled(true);

        // Connect the clicked() signal to a lambda function
        connect(renderConversation, &QClickableGroupBox::clicked, [=]() {
            handleClickedConversation(renderConversation);
        });

        Application::loggedUser->getConversations().pop();
    }

    while(!tempConversations.empty()){
        Application::loggedUser->getConversations().push(tempConversations.top());
        tempConversations.pop();
    }


}

Ui::MainWindow* MainWindow::getUI(){
    return this->ui;
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::renderLoggedUserStory(Story *newStory) {

    QLayout* layout = ui->horizontalGroupBox_3->layout();
    QLayoutItem* item;
    while((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QClickableGroupBox* storyBox = GUI_render::renderStory(newStory);
    if (Application::stories[Application::loggedUser->getUserName()].size() > 1) {
         MainWindow::handleStoryClicked(storyBox , Application::stories[Application::loggedUser->getUserName()] , this);
    }

    MainWindow::connect(storyBox, &QClickableGroupBox::clicked, [=]() {
        MainWindow::handleStoryClicked(storyBox , Application::stories[Application::loggedUser->getUserName()] , this);
    });
    ui->horizontalGroupBox_3->layout()->addWidget(storyBox) ;
}


void MainWindow::handleClickedConversation(QGroupBox *renderConversation) {

    // Set the current index to 1 to show a loading screen
    ui->stackedWidget->setCurrentIndex(1);

    // Create a QTimer object and set it to a single-shot timer with a 2-second delay
    QTimer::singleShot(1, [=]() {

        if (!renderConversation) {
            qDebug() << "Render conversation is null";
            return;
        }

        // Create a stringstream object from the string.
        std::stringstream ss(renderConversation->objectName().toStdString());

        void* address;
        ss >> address;// Read the address from the stringstream.

        // Cast the void* pointer to the desired type.
        Conversation* conversation = static_cast<Conversation*>(address);

        qDebug()<< (Application::currentConversation );
        Application::currentConversation =  conversation;
        qDebug()<< (Application::currentConversation );


        QLayoutItem *item;
        while ((item = ui->verticalGroupBox_3->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        ui->horizontalLayout_5->layout()->setSpacing(10);
        ui->ContactName->setText(conversation->getName().c_str());

        QString IMG_PATH =QString::fromStdString(conversation->getReceiver()->getImgPath());
        QSize imgMaxSize = (IMG_PATH == ":/imgs/Profile (2).png")? *new QSize(40,40) : *new QSize(50,60);

        if(IMG_PATH == ":/imgs/Profile (2).png")
            ui->ContactIMG->setMaximumSize(imgMaxSize);

        ui->ContactIMG->setMinimumSize(imgMaxSize);

        QString imgType = IMG_PATH == ":/imgs/Profile (2).png"? "image" : "border-image";
        ui->ContactIMG->setStyleSheet( imgType+  ":url(" + IMG_PATH + ");border-radius:8px");
        for (auto &conv : conversation->getMessages()) {
            ui->verticalGroupBox_3->layout()->addWidget(Application::renderMessage(*conv));
        }

        // Set the current index to 2 to show the conversation
        ui->stackedWidget->setCurrentIndex(3);
    });
}






void MainWindow::on_pushButton_3_clicked()
{

    //    Application::conversations.push(Conversation(Contact("sdfs")));
    fileSystem_lib::saveData();
    this->close();
}

void MainWindow::renderContactMain() {

    Conversation* conversationPtr = (Application::loggedUser->getConversations().top());
    std::stringstream ss;
    ss << conversationPtr;
    std::string conversationAddress = ss.str();

    // Create the QClickableGroupBox widget
    QClickableGroupBox *conv = Application::renderConversation(conversationPtr);
    conv->setObjectName(QString::fromStdString(conversationAddress));

    connect(conv, &QClickableGroupBox::clicked, [=]() {
        handleClickedConversation(conv);
    });

    ui->contactsCont->layout()->addWidget(conv);

}

void MainWindow::on_pushButton_4_clicked()
{
    if (this->isMaximized()) {
        this->showNormal();
    } else {
        this->showMaximized();
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    if (window()->isMinimized()) {
        window()->showNormal();
    } else {
        window()->showMinimized();
    }
}


void MainWindow::on_pushButton_5_clicked()
{

    addContactWin = new AddContact() ;
    addContactWin->show();
    connect(addContactWin, SIGNAL(renderConversation()), this, SLOT(renderContactMain()));
}


void MainWindow::on_pushButton_6_clicked()
{
    profWin = new profileWindow();
    profWin->show();
}


void MainWindow::on_pushButton_clicked()
{
    StartNewChatWin = new StartNewChat();
    StartNewChatWin->show();
}



void MainWindow::on_pushButton_7_clicked()
{
    QString textMsg = ui->sendMessageLineEdit->text();
    int DelayHandler ;
    if (textMsg.size() > 30000) {
        messageLongAlertWin = new messageLongAlert() ;
        messageLongAlertWin->show() ;
    }
    else {
        if (!textMsg.isEmpty()) { // check if the text is not empty
            Message *messageTest = new Message(Application::loggedUser->getUserName(), textMsg.toStdString(), Application::currentConversation->getReceiver()->getName(), QDateTime::currentDateTime(), false, false);
            Application::currentConversation->addNewMessage(messageTest);
            QClickableGroupBox* ConversationgroubBoxAddress  = Application::currentConversation->getConversationGroupBoxAddress();
            //        QList<QVBoxLayout*> vlayout= ConversationgroubBoxAddress->findChildren<QVBoxLayout*>("VLayout");
            //        QList<QHBoxLayout*> hlayout = vlayout.front()->findChildren<QHBoxLayout*>("lastMsgBox");
            //        QList<QLabel*> targetLabel= hlayout.front()->findChildren<QLabel*>("textmsg",Qt::FindChildrenRecursively);
            //        if(targetLabel.front() != nullptr){
            //            targetLabel.front()->setText((Application::currentConversation->getMessages().empty())? "Chat now !" : QString::fromStdString(Application::currentConversation->getMessages().back()->getMessageTxt()));
            //        }

            Conversation *receiverConv = Application::getReceiverConversation(Application::currentConversation->getReceiver()->getName());
            if(receiverConv != nullptr)
                receiverConv->addNewMessage(messageTest);

            ui->verticalGroupBox_3->layout()->addWidget(Application::renderMessage(*messageTest));
            ui->sendMessageLineEdit->setText("");
            DelayHandler = messageTest->getMessageTxt().size() ;
            QTimer::singleShot(DelayHandler = DelayHandler <= 300 ? DelayHandler + 100 : 300 , [=](){
                ui->scrollArea_2->verticalScrollBar()->setValue(ui->scrollArea_2->verticalScrollBar()->maximum());
            });
        }

    }
}

void MainWindow::on_addNewStoryBtn_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(1);

    GUI_render::renderStories(this);
}




void MainWindow::on_viewFavMsg_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(2);
}

void MainWindow::on_backSotryBtn_clicked()
{
    MainWindow::BackBtnStoryHandle(this);
}


void MainWindow::on_searchForFav_clicked()
{
    std::string search_keyword = ui->searchFavMsg->text().toStdString();
    std::stack<Conversation *> conv = Application::loggedUser->getConversations();
    while(!conv.empty())
    {
        std::list<Message* > messages = conv.top()->getMessages();
        for(auto msg : messages)
        {
            if(msg->isFavourite() && Application::isSubstringFound(msg->getMessageTxt(), search_keyword))
            {
                qDebug()<<"Found fav message: " + msg->getMessageTxt();
            }
        }
        conv.pop();
    }
}

void MainWindow::on_NextStoryBtn_clicked()
{
    MainWindow::NextBtnStoryHandle(this);
}


void MainWindow::on_pushButton_8_clicked()
{
    addStoryWin = new AddStory() ;
    addStoryWin->show() ;
    connect(addStoryWin, SIGNAL(AddStoryhandler(Story*)), this, SLOT(renderLoggedUserStory(Story*)));
}


void MainWindow::on_pushButton_9_clicked()
{
    this->ui->stackedWidget_2->setCurrentIndex(0);
}
