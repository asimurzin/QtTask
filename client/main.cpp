#include "Client.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Test client");
    parser.addHelpOption();

    QCommandLineOption  host_option(QStringList() << "a" << "address",
                                    QCoreApplication::translate("main", "Connect to server <address>."),
                                    QCoreApplication::translate("main", "address"));

    QCommandLineOption  port_option(QStringList() << "p" << "port",
                                    QCoreApplication::translate("main", "Connect to local server on <port>."),
                                    QCoreApplication::translate("main", "port"));

    QCommandLineOption  credential_option(QStringList() << "cr" << "credentials",
                                          QCoreApplication::translate("main", "Connect to local server with <user>,<password>."),
                                          QCoreApplication::translate("main", "<user>,<password>"));

    QCommandLineOption  command_option(QStringList() << "c" << "command",
                                       QCoreApplication::translate("main", "Command to send to server: add,<name>,<password> or del,<name>"),
                                       QCoreApplication::translate("main", "<command>,<user>,<password>"));


    parser.addOption(host_option);
    parser.addOption(port_option);
    parser.addOption(credential_option);
    parser.addOption(command_option);
    parser.process(a);

    const QStringList options = parser.optionNames();

    if ( options.size() < 3)
    {
        qDebug() << parser.helpText();
        a.exit(-1);
        return -1;
    }

    QString host = parser.value(host_option);
    if (!host.length())
    {
        qDebug() << "Empty host";
        qDebug() << parser.helpText();
        a.exit(-1);
        return -1;
    }

    QString port_str = parser.value(port_option);

    uint port = port_str.toUInt();
    if (port == 0 )
    {
        qDebug() << "Wrong Port";
        qDebug() << parser.helpText();
        a.exit(-1);
        return -1;
    }

    QString credentials = parser.value(credential_option);
    QStringList credentials_list = credentials.split(",");
    if (credentials_list.size() != 2)
    {
        qDebug() << "Wrong credentials format";
        qDebug() << parser.helpText();
        a.exit(-1);
        return -1;
    }

    QString message = parser.value(command_option);

    Client* client = new Client(host, static_cast<quint16>(port), credentials);
    QTimer* timer = new QTimer();
    QTimer::connect(timer,&QTimer::timeout, [&](){client->Close();});
    QObject::connect(client,&Client::disconnected, [&](){a.exit();});
    QObject::connect(client,&Client::connected, [&]()
                                                {
                                                    if (message.length())
                                                        client->Send(message);

                                                    timer->start(5000);
                                                });

    client->Connect();
    a.exec();

    return 1;
}
