#include "Client.h"

#include <QHostAddress>
#include <QTime>
#include <QCoreApplication>
#include <QThread>

static const int     s_connection_timeout = 2000;
static const QString s_login_command("login");
static const QString s_add_command("add");
static const QString s_del_command("del");
static const QString s_good_answer("200");
static const QString s_bad_answer("500");
static const QString s_bad_auth_answer("401");

Client::Client(const QString &host, quint16 port, const QString &credentials, QObject* parent)
    : QObject(parent)
    , m_connection(this)
    , m_host(host)
    , m_port(port)
    , m_credentials(credentials)
    , m_timer(this)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerEnd()));
}

void Client::onReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray incoming_data = sender->readAll();
    qDebug()<< "Answer:" << incoming_data;
    if (IsConnected() && !m_authenticated)
    {
        m_timer.stop();
        if (QString(incoming_data) == s_good_answer)
        {
            m_authenticated = true;
            emit connected();
        }
        else
            m_connection.close();
    }
 }

void Client::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::SocketState::UnconnectedState)
    {
        qDebug() << "Disconnected";
        emit disconnected();
    }
    else if (socketState == QAbstractSocket::SocketState::ConnectedState)
    {
        qDebug() << "Connected";
        m_timer.stop();
        Send(s_login_command + "," + m_credentials);
        m_timer.start();
    }
}

void Client::timerEnd()
{
    if (m_connection.state() == QAbstractSocket::SocketState::UnconnectedState)
        qDebug() << "Could not connect";
    else if (m_connection.state() == QAbstractSocket::SocketState::ConnectedState)
        qDebug() << "Wrong credentials";

    m_timer.stop();
}

void Client::Connect()
{
    qDebug() << "Try to connect to " << m_host << ":" << m_port;
    m_connection.connectToHost(QHostAddress(m_host), m_port);
    connect(&m_connection, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&m_connection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)) );
    m_timer.start(s_connection_timeout);
}

bool Client::IsConnected() const
{
    return m_connection.state() == QTcpSocket::ConnectedState;
}

bool Client::Send(const QString& message)
{
    qDebug() << "Send:" << message;
    const std::string mess = message.toStdString();
    m_connection.write(mess.c_str(), mess.size());

    return true;
}

void Client::Close()
{
    m_connection.close();
}
