#include "Server.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("Test server");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption  port_option(QStringList() << "p" << "port",
                                    QCoreApplication::translate("main", "Run server on <port>."),
                                    QCoreApplication::translate("main", "port"));

    QCommandLineOption firt_server_option(QStringList() << "f" << "first", QCoreApplication::translate("main", "Is first server"));

    parser.addOption(firt_server_option);
    parser.addOption(port_option);
    parser.process(a);

    QString portOption = parser.value(port_option);

    if (!portOption.length())
    {
        qDebug() << parser.helpText();
        a.exit(-1);
    }
    uint port = portOption.toUInt();

    bool first_server = parser.isSet(firt_server_option);

    TestServer server(static_cast<quint16>(port), first_server);
    server.Start();

    return a.exec();
}

