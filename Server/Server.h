#ifndef SERVER_H
#define SERVER_H
#include "usersdb.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
using Socket = QTcpSocket*;

class TestServer : QObject
{
    Q_OBJECT
public:
    TestServer(quint16 pp, bool first_server, QObject *parent = nullptr);

    bool Start();

public slots:
    void onNewConnection();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onReadyRead();

private:
    bool HandleIncomingString(Socket socket, const QString& incoming_str, bool& is_auth_request);
    void SendAnswer(Socket socket, const QString& str);
    void UpdateStats() const;

    QTcpServer          m_server;
    quint16             m_port;
    QMap<Socket, bool>  m_authenticated_by_socket;
    UsersDB             m_users;
    uint32_t            m_add_count = 0;
    uint32_t            m_del_count = 0;
};

#endif // SERVER_H
