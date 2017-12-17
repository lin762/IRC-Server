/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:

**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include "dialog.h"
#include "IRCSocket.h"
#include <time.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sstream>
using namespace std;

char * host = "localhost";
char * user;
char * password;
char * sport;
int port = 22222;

#define MAX_MESSAGES 100
#define MAX_MESSAGE_LEN 300
#define MAX_RESPONSE (20 * 1024)

int lastMessage = 0;

int open_client_socket(char * host, int port) {
        // Initialize socket address structure
        struct  sockaddr_in socketAddress;

        // Clear sockaddr structure
        memset((char *)&socketAddress,0,sizeof(socketAddress));

        // Set family to Internet
        socketAddress.sin_family = AF_INET;

        // Set port
        socketAddress.sin_port = htons((u_short)port);

        // Get host table entry for this host
        struct  hostent  *ptrh = gethostbyname(host);
        if ( ptrh == NULL ) {
                perror("gethostbyname");
                exit(1);
        }

        // Copy the host ip address to socket address structure
        memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

        // Get TCP transport protocol entry
        struct  protoent *ptrp = getprotobyname("tcp");
        if ( ptrp == NULL ) {
                perror("getprotobyname");
                exit(1);
        }
        // Create a tcp socket
                int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
                if (sock < 0) {
                        perror("socket");
                        exit(1);
                }

                // Connect the socket to the specified server
                if (connect(sock, (struct sockaddr *)&socketAddress,
                            sizeof(socketAddress)) < 0) {
                        perror("connect");
                        exit(1);
                }

        return sock;
}

int sendCommand(char * host, int port, char * command, char * user,
                 char * password, char * args, char * response) {
          int sock = open_client_socket( host, port);

          // Send command
          write(sock, command, strlen(command));
          write(sock, " ", 1);
          write(sock, user, strlen(user));
          write(sock, " ", 1);
          write(sock, password, strlen(password));
          write(sock, " ", 1);
          write(sock, args, strlen(args));
          write(sock, "\r\n",2);

          // Keep reading until connection is closed or MAX_REPONSE
          int n = 0;
          int len = 0;
          while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
                  len += n;
          }

         //printf("response:%s\n", response);
          response[len] = '\0';
          close(sock);
}


char* userInput;
char* passInput;
char* roomInput;
char* currentRoomName = "";

void Dialog::sendAction()
{
    //printf("Send Button\n");
    QString msg = inputMessage->toPlainText();
    QByteArray arr1 = msg.toLocal8Bit();
    char* message = strdup(arr1.data());
    std::string str(message);
    std::string str2(currentRoomName);
    std::string messageStr = str2+ " " + str;
    char* mstr = strdup(messageStr.c_str());
    char *res = (char*)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host,port,"SEND-MESSAGE",userInput, passInput,mstr,res);
    printf("%s",res);
    sendCommand(host,port,"GET-MESSAGES",userInput,passInput,strdup((to_string(0) + " " + currentRoomName).c_str()),res);
    printf("%s",res);
    messageCount++;

    QString respQstr = QString::fromStdString(res);
    QStringList mssgs = respQstr.split("\r\n", QString::SkipEmptyParts);
    allMessages->clear();
    for (int i = 0; i < mssgs.size(); i++)
        allMessages->append(mssgs.at(i));

}



void Dialog::enterRoomAction()
{
    if(roomsList->currentItem() == NULL)
        return;
    QString roomItem = roomsList->currentItem()->text();
    QByteArray arr1 = roomItem.toLocal8Bit();
    roomInput = strdup(arr1.data());

    char *res = (char*)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host,port,"ENTER-ROOM",userInput,passInput,roomInput,res);
    if(res[0] == 'O' && res[1] == 'K'){
        currentRoomName = strdup(roomInput);
    }

    std::string str2 = " has entered ";
    std::string str3(currentRoomName);
    std::string message = str2+str3;

    std::string messageStr = str3+ " " + message;
    char* mstr = strdup(messageStr.c_str());
    sendCommand(host,port,"SEND-MESSAGE",userInput, passInput,mstr,res);
    printf("%s",res);

    //QString msg = QString::fromStdString(message);
    currentRoomName = strdup(roomInput);

}

void Dialog::leaveRoomAction(){
    if(!strcmp(currentRoomName, "")){
        QDialog *failedLog = new QDialog();
        QLabel *error = new QLabel("Not in existing room.");
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(error);
        failedLog->setLayout(layout);
        failedLog->show();

    }

    std::string str2 = " has left ";
    std::string str3(currentRoomName);
    std::string message = str2+str3;

    char *res = (char*)malloc(MAX_RESPONSE*sizeof(char));
    std::string messageStr = str3+ " " + message;
    char* mstr = strdup(messageStr.c_str());
    sendCommand(host,port,"SEND-MESSAGE",userInput, passInput,mstr,res);
    printf("%s",res);
    sendCommand(host,port,"LEAVE-ROOM",userInput,passInput,currentRoomName,res);
    currentRoomName = "";


    usersList->clear();
    allMessages->clear();

}

void Dialog::loginAction()
{
    bool ok;
    QString userQ = QInputDialog::getText(this, "Username", "Enter username",QLineEdit::Normal, NULL, &ok);

    bool ok2;
    QString passQ = QInputDialog::getText(this, "Password", "Enter password",QLineEdit::Normal, NULL, &ok2);

    QByteArray arr1 = userQ.toLocal8Bit();
    userInput = strdup(arr1.data());
    QByteArray arr2 = passQ.toLocal8Bit();
    passInput = strdup(arr2.data());

    char *res = (char*)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host,port,"CHECK-AUTH",userInput,passInput,"",res);

    if(res[0] == 'O' && res[1] == 'K'){
        QDialog *successLog = new QDialog();
        QLabel *success = new QLabel("Login successful");
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(success);
        successLog->setLayout(layout);
        successLog->show();
    }else{
        QDialog *failedLog = new QDialog();
        QLabel *error = new QLabel("Please create account");
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(error);
        failedLog->setLayout(layout);
        failedLog->show();
    }
}

void Dialog::sendNewUserAction(){
    //printf("%s---%s\n",userInput, passInput);
    char nuser[MAX_RESPONSE];
    sendCommand(host,port,"ADD-USER",userInput,passInput,"",nuser);

}

void Dialog::newUserAction()
{
    bool ok;
    QString userQ = QInputDialog::getText(this, "Username", "Enter username:",QLineEdit::Normal, NULL, &ok);

    bool ok2;
    QString passQ = QInputDialog::getText(this, "Password", "Enter password:",QLineEdit::Normal, NULL, &ok2);

    QByteArray arr1 = userQ.toLocal8Bit();
    userInput = strdup(arr1.data());
    QByteArray arr2 = passQ.toLocal8Bit();
    passInput = strdup(arr2.data());

    if(ok && ok2){
        sendNewUserAction();
    }

}

void Dialog::sendCreateRoomAction()
{
    char *nroom = (char*)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host,port,"CREATE-ROOM",userInput,passInput,roomInput,nroom);
    if (nroom[0] == 'O' && nroom[1] == 'K'){
        char s[50];
        sprintf(s, "%s", roomInput);
        roomsList->addItem(s);
    }
}

void Dialog::createRoomAction()
{

    bool ok;
    QString roomQ = QInputDialog::getText(this, "Create Room", "Enter room name",QLineEdit::Normal, NULL, &ok);

    QByteArray arr1 = roomQ.toLocal8Bit();
    roomInput = strdup(arr1.data());

    if(ok){
        sendCreateRoomAction();
    }
}

void Dialog::updateMessage()
{
    if(!strcmp(currentRoomName,"")){

    }else{
        char * res = (char*)malloc(MAX_RESPONSE*sizeof(char));
        sendCommand(host,port,"GET-MESSAGES",userInput,passInput,strdup((to_string(0) + " " + currentRoomName).c_str()),res);
        QString respQstr = QString::fromStdString(res);
        QStringList mssgs = respQstr.split("\r\n", QString::SkipEmptyParts);
        allMessages->clear();
        for (int i = 0; i < mssgs.size(); i++)
            allMessages->append(mssgs.at(i));
    }
}

void Dialog::updateRoom()
{
    char * res = (char*)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host,port, "LIST-ROOMS", userInput, passInput, "", res);

    QString respQstr = QString::fromStdString(res);
    QStringList rooms = respQstr.split("\r\n", QString::SkipEmptyParts);
    roomsList->clear();
    for(int i = 0; i < rooms.size(); i++)
        roomsList->addItem(rooms.at(i));

}

void Dialog::updateUsersInRoom()
{
    char * res = (char*)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host,port, "GET-USERS-IN-ROOM", userInput, passInput, currentRoomName, res);

    QString resQ = QString::fromStdString(res);
    QStringList lusers = resQ.split("\r\n", QString::SkipEmptyParts);
    usersList->clear();
    for(int i = 0; i < lusers.size(); i++)
        usersList->addItem(lusers.at(i));
}

void Dialog::timerAction()
{


    char message[50];
//    sprintf(message,"Timer Refresh New message %d",messageCount);
//    allMessages->append(message);
    if(currentRoomName == ""){
        updateRoom();
        return;
    }
    updateRoom();
    updateUsersInRoom();
    updateMessage();
    if(!strcmp(currentRoomName,"")){
        usersList->clear();
        allMessages->clear();
    }

}

Dialog::Dialog()
{
    createMenu();

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Rooms List
    QVBoxLayout * roomsLayout = new QVBoxLayout();
    QLabel * roomsLabel = new QLabel("Rooms");
    roomsList = new QListWidget();
    userInput = "admin";
    passInput = "admin";
    roomsLayout->addWidget(roomsLabel);
    roomsLayout->addWidget(roomsList);
    char *res = (char *)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host,port,"ADD-USER","admin","admin","",res);
    updateRoom();

    // Users ListcurrentRoomName
    QVBoxLayout * usersLayout = new QVBoxLayout();
    QLabel * usersLabel = new QLabel("Users");
    usersList = new QListWidget();
    usersLayout->addWidget(usersLabel);
    usersLayout->addWidget(usersList);

    // Layout for rooms and users
    QHBoxLayout *layoutRoomsUsers = new QHBoxLayout;
    layoutRoomsUsers->addLayout(roomsLayout);
    layoutRoomsUsers->addLayout(usersLayout);

    // Textbox for all messages
    QVBoxLayout * allMessagesLayout = new QVBoxLayout();
    QLabel * allMessagesLabel = new QLabel("Messages");
    allMessages = new QTextEdit;
    allMessagesLayout->addWidget(allMessagesLabel);
    allMessagesLayout->addWidget(allMessages);

    // Textbox for input message
    QVBoxLayout * inputMessagesLayout = new QVBoxLayout();
    QLabel * inputMessagesLabel = new QLabel("Type your message:");
    inputMessage = new QTextEdit;
    inputMessagesLayout->addWidget(inputMessagesLabel);
    inputMessagesLayout->addWidget(inputMessage);

    // Send and new account buttons
    QHBoxLayout *layoutButtons = new QHBoxLayout;
    QPushButton * sendButton = new QPushButton("Send");
    QPushButton * newUserButton = new QPushButton("New Account");
    QPushButton * createRoomButton = new QPushButton("Create Room");
    QPushButton * loginButton = new QPushButton("Login");
    QPushButton * joinRoomButton = new QPushButton("Enter Room");
    QPushButton * leaveRoomButton = new QPushButton("Leave Room");

    layoutButtons->addWidget(sendButton);
    layoutButtons->addWidget(newUserButton);
    layoutButtons->addWidget(createRoomButton);
    layoutButtons->addWidget(loginButton);
    layoutButtons->addWidget(joinRoomButton);
    layoutButtons->addWidget(leaveRoomButton);

    // Setup actions for buttons
    connect(sendButton, SIGNAL (released()), this, SLOT (sendAction()));
    connect(newUserButton, SIGNAL (released()), this, SLOT (newUserAction()));
    connect(createRoomButton, SIGNAL (released()), this, SLOT (createRoomAction()));
    connect(loginButton, SIGNAL (released()), this, SLOT (loginAction()));
    connect(joinRoomButton, SIGNAL (released()), this, SLOT (enterRoomAction()));
    connect(leaveRoomButton, SIGNAL (released()), this, SLOT (leaveRoomAction()));

    // Add all widgets to window
    mainLayout->addLayout(layoutRoomsUsers);
    mainLayout->addLayout(allMessagesLayout);
    mainLayout->addLayout(inputMessagesLayout);
    mainLayout->addLayout(layoutButtons);

    // Add layout to main window
    setLayout(mainLayout);

    setWindowTitle(tr("CS240 IRC Client"));
    //timer->setInterval(5000);

    messageCount = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL (timeout()), this, SLOT (timerAction()));
    timer->start(5000);
}


void Dialog::createMenu()

{
    menuBar = new QMenuBar;
    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);

    connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}
