
#ifndef FILESYSTEM_LIB_H
#define FILESYSTEM_LIB_H

#include <stack>
#include <strings.h>
#include <iostream>
#include <QFile>
#include <QDir>
#include <list>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "logic/user.h"
#include "logic/contact.h"
#include "application/application.h"
#include "logic/message.h"
#include <QDebug>

class fileSystem_lib
{

private :

      // ________create a new message QJson object ready to add for QJsonArray
    static QJsonObject createNewJSONMessage(Message &message){
        QJsonObject JSONmessage;
        JSONmessage["ID"] = message.getID().c_str();
        JSONmessage["msgTxt"] = message.getMessageTxt().c_str();
        JSONmessage["seen"] = message.isSeen();
        JSONmessage["sendDate"] = QJsonValue::fromVariant(message.getSendDate());
        JSONmessage["receiverID"] = message.getReceiverId().c_str();
        return JSONmessage;
    }

    // ________create a new Contact QJson object ready to add for QJsonArray
    static  QJsonObject createNewJSONContact( Contact* contact){
        QJsonObject JSONContact;
        JSONContact["ID"] = QString::fromStdString(contact->getID());
        JSONContact["name"] = QString::fromStdString(contact->getName().c_str());
        JSONContact["imgPath"] = QString::fromStdString(contact->getImgPath().c_str());


        return JSONContact;

    }
    // ________create a new Conversation QJson object ready to add for QJsonArray

    static QJsonObject createNewJSONConversation(Conversation* conversation){
        QJsonObject jsonConversation;

        // adding conversation basic data
        Contact* receiver = conversation->getReceiver();
        jsonConversation["receiver"] = (createNewJSONContact(receiver));
        jsonConversation["name"] = conversation->getName().c_str();
        jsonConversation["isFavourite"] = conversation->getIsFavourite();

        // adding conversation messages
        QJsonArray messages;
        for(auto message : conversation->getMessages()){
            messages.append(createNewJSONMessage(message));
        }
        jsonConversation["messages"] = messages;

        return jsonConversation;
    }

    // _______create new user file if it doesn't exist
    static QJsonObject createNewJSONUser(User* user){

        // preparing user data
        QJsonObject userData;
        if( (&user) == nullptr ||user->getUserID() == "" || user->getUserName() == "" || user->getUserPassword() == ""){
            return userData ;
        }
        userData["ID"] =  user->getUserID().c_str();
        userData["firstName"] =  user->getFirstName().c_str();
        userData["lastName"] =  user->getlastName().c_str();
        userData["username"] =  user->getUserName().c_str();
        userData["password"] =  user->getUserPassword().c_str();
        userData["loggedIn"] = false;
        userData["imgPath"] = user->getIMGpath().c_str();

        // adding user contacts IDs to the QJson array
        QJsonArray contacts;
        for(Contact *contact : user->getUserContacts()){
            contacts.append(createNewJSONContact(contact));
        }
        userData["contacts"] = contacts;

        QJsonArray conversations;

        std::stack<Conversation*> conversationsStack = user->getConversations();
        while(!conversationsStack.empty()){
            conversations.append(createNewJSONConversation(conversationsStack.top()));
            conversationsStack.pop();
        }
        userData["conversations"] = conversations;

        return userData;

    }

    //____ this function creates a new Message object from a QJsonObj
    static  Message createNewMessageObject(const QJsonObject& jsonMessageObj){
        Message message
            (
                jsonMessageObj["ID"].toString().toStdString(),
                jsonMessageObj["msgTxt"].toString().toStdString(),
                jsonMessageObj["receiverID"].toString().toStdString(),
                jsonMessageObj["sendDate"].toVariant().toDateTime(),
                jsonMessageObj["seen"].toBool(),
                jsonMessageObj["Favourite"].toBool()
            );
        return message;
    }

    //____ this function creates a new Contact object from a QJsonObj
    static Contact* createNewContactObject(const QJsonObject& jsonContactObj){
        Contact *contact = new  Contact
            (
                jsonContactObj["ID"].toString().toStdString(),
                jsonContactObj["imgPath"].toString().toStdString(),
                jsonContactObj["name"].toString().toStdString()
            );
        return contact;
    }


    //_____ this function creates new COnversation object from a Conversation QJsonObj
    static Conversation* createNewConversationObject(const QJsonObject& jsonConversationObj){
        Conversation *conversationObj = new Conversation
            (
                createNewContactObject(jsonConversationObj["receiver"].toObject()),
                jsonConversationObj["isFavourite"].toBool(),
                jsonConversationObj["name"].toString().toStdString()
            );

        // adding conversation messages
        QJsonArray jsonMessages = jsonConversationObj["messages"].toArray();
        for(auto message : jsonMessages){
            conversationObj->addNewMessage(createNewMessageObject(message.toObject()));
        }

        return conversationObj;
    }


    //_____ this function creates new user object from a user QJsonObj
    static User* createNewUserObject(QJsonObject& jsonUserObj){
        // adding basic data from the json Filel oaded from the disk
        User *user = new User
            (
                jsonUserObj["ID"].toString().toStdString(),
                jsonUserObj["username"].toString().toStdString(),
                jsonUserObj["firstName"].toString().toStdString(),
                jsonUserObj["lastName"].toString().toStdString(),
                jsonUserObj["password"].toString().toStdString(),
                jsonUserObj["imgPath"].toString().toStdString(),
                jsonUserObj["loggedIn"].toBool()
            );

            // adding lists data from the json file loaded from the disk

            // adding contacts
            QJsonArray jsonContacts = jsonUserObj["contacts"].toArray();
            for(auto contact : jsonContacts){
                Contact *contactJson = createNewContactObject((contact).toObject());
                user->addContact(contactJson);
            }

            // adding conversations
            QJsonArray jsonConversations = jsonUserObj["conversations"].toArray();
            for(auto conversation : jsonConversations){
                Conversation *conversationObj = createNewConversationObject(conversation.toObject());
            user->addNewConversation(conversationObj);
            }

        return user;
    }

    //_______this function constructs the conversation document to be ready to save at the disk

    static QJsonDocument buildConversationsJSONDocument(std::stack<Conversation> conversations){
        QJsonArray JSONConversations;
        while(!conversations.empty()){
            Conversation conversation = conversations.top();
            conversations.pop();
            JSONConversations.append(createNewJSONConversation(&conversation));
        }
        return (QJsonDocument(JSONConversations));
    }


     //_______this function constructs the users document to be ready to save at the disk
    static QJsonDocument buildUsersJSONDocument( std::list<User*> users) {
        QJsonArray JSONUsers;
        for (auto  &user : users) {
                JSONUsers.append(createNewJSONUser(user));
        }



        return QJsonDocument(JSONUsers);
    }



    // types of the JSON document
    enum DocumentType {
        USERS,
        CONVERSATIONS,
        UNKNOWN
    };

    static DocumentType getDocumentType(QJsonDocument &document) {
        if (document.isArray() && !document.array().isEmpty()) {
                const QJsonObject& firstObject = document.array().at(0).toObject();
                if (firstObject.contains("username") && firstObject.contains("password")) {
                    return USERS;
                } else if (firstObject.contains("conversationName") && firstObject.contains("participants")) {
                    return CONVERSATIONS;
                }
        }
        return UNKNOWN;
    }

    //_______ this function saves takes a JSON document and save it to the disk
    static   bool storeJSONDocument(QJsonDocument &document){

        QString PATH = (getDocumentType(document) == USERS)? "users.json" : "conversations.json";
        QFile file(PATH);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            out << document.toJson(QJsonDocument::Indented); // <-- Use QTextStream to write the formatted JSON data to the file
            qDebug()<<document.toJson(QJsonDocument::Indented);
            file.close();
            return true;
        }
        return false;
    }



public:
    fileSystem_lib();
    // sets up the file system if it doesn't exist (can be used on the start of the programm to ensure the file system exists)

    static  bool saveData(){
        // constructing and saving users list in a json file
        std::list<User*> usersList  = Application::users;
        QJsonDocument users = buildUsersJSONDocument(usersList);
        storeJSONDocument(users);

        // constructing and saving conversations stack in a json file
//        QJsonDocument conversations = buildConversationsJSONDocument(Application::conversations);
//        storeJSONDocument(conversations);

        return true;
    }

    static void internalLoadData(DocumentType type){

        QString PATH = (type == USERS)? "users.json" : "conversations.json";
        QFile file(PATH);

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("Failed to open file");
            return;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

        // checking if  the json document is not aan arry of jsons
        if (!jsonDoc.isArray()) {
            qWarning("JSON data is not an Array");
            return;
        }

        // checking if the users list is not empty ehan reading if the function wasused accidentally in a wrong place

        if(type == USERS){
            if (!Application::users.empty()) {
                    std::list<User*>& mutableUsers = const_cast<std::list<User*>&>(Application::users);
                    mutableUsers.clear();
            }

            // putting the read users data into the application::users list
            if (jsonDoc.isArray()) {
                    QJsonArray jsonArray = jsonDoc.array();
                    for (QJsonArray::const_iterator arrayItr = jsonArray.constBegin(); arrayItr != jsonArray.constEnd(); ++arrayItr) {
                        if (arrayItr->isObject()) {
                            QJsonObject jsonUserObj = arrayItr->toObject();
                            User *user = createNewUserObject(jsonUserObj);
                            Application::users.push_back(user);
                        }
                    }
            }
        }


    }


    //______loading application data from the json files
    static void loadData(){
        internalLoadData(USERS);
        internalLoadData(CONVERSATIONS);

    }

};

#endif // FILESYSTEM_LIB_H
