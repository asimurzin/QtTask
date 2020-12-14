#include "Server.h"

#include <iostream>

static const QString s_good_answer("200");
static const QString s_bad_answer("500");
static const QString s_bad_auth_answer("401");
static const QString s_login_command("login");
static const QString s_add_command("add");
static const QString s_del_command("del");

TestServer::TestServer(quint16 pp, bool first_server, QObject *parent)
    : QObject(parent)
    , m_server(this)
    , m_port(pp)
    , m_users(first_server)
{
}
bool TestServer::Start()
{
    connect(&m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    return m_server.listen(QHostAddress::Any, m_port);
}

void TestServer::onNewConnection()
{
    Socket Socket = m_server.nextPendingConnection();
    connect(Socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(Socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    m_authenticated_by_socket.insert(Socket, false);
    UpdateStats();
}

void TestServer::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::SocketState::UnconnectedState)
    {
        auto socket_it = m_authenticated_by_socket.find(static_cast<Socket>(QObject::sender()));
        m_authenticated_by_socket.erase(socket_it);
        UpdateStats();
    }
}

void TestServer::onReadyRead()
{
    Socket sender = static_cast<Socket>(QObject::sender());

    if (m_authenticated_by_socket.find(sender) == m_authenticated_by_socket.end())
    {
        qDebug() << "ERROR: message from not connected user";
        //exit(-1);
        return;
    }
    QByteArray incoming_data = sender->readAll();

    bool is_auth_request = false;
    if (HandleIncomingString(sender, incoming_data, is_auth_request))
    {
        SendAnswer(sender, s_good_answer);
        return;
    }

    if (is_auth_request)
        SendAnswer(sender, s_bad_auth_answer);
    else
        SendAnswer(sender, s_bad_answer);
}

bool TestServer::HandleIncomingString(Socket socket,const QString &incoming_str, bool& is_auth_request)
{
    //qDebug() << "HandleIncomingString::" << incoming_str;
    QStringList incoming_str_parts = incoming_str.split(",");
    if (incoming_str_parts.empty())
        return false;

    if (incoming_str_parts[0] == s_login_command && incoming_str_parts.size() ==3 )
    {
        if(m_authenticated_by_socket[socket])
            return false;

        is_auth_request = true;
        if ( m_users.CheckUserPasswords(incoming_str_parts[1].toStdString(), incoming_str_parts[2].toStdString()))
        {
            m_authenticated_by_socket[socket] = true;
            return true;
        }

        return false;
    }
    if (!m_authenticated_by_socket[socket])
        return false;

    bool result = false;
    if (incoming_str_parts[0] == s_add_command && incoming_str_parts.size() ==3 &&
            m_users.AddUser(incoming_str_parts[1].toStdString(), incoming_str_parts[2].toStdString()))
    {
        ++m_add_count;
        result =true;
    }
    else if (incoming_str_parts[0] == s_del_command && incoming_str_parts.size() ==2 &&
        m_users.DeleteUser(incoming_str_parts[1].toStdString()))
    {
        ++m_del_count;
        result = true;
    }

    return result;
}

void TestServer::SendAnswer(Socket socket, const QString &str)
{
    //qDebug() << "Send answer " << str;
    const std::string sent_string = str.toStdString();
    socket->write(sent_string.c_str(), sent_string.size());
}

void TestServer::UpdateStats() const
{
    std::cout << "Connections:" << m_authenticated_by_socket.size() << ", adds:" << m_add_count << ", dels:" << m_del_count << "\t\r" << std::flush;
}

