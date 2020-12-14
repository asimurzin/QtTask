#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QTimer>

class Client : public QObject
{
    Q_OBJECT

public slots:
    void onReadyRead();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void timerEnd();

signals:
    void connected();
    void disconnected();

public:
    Client(const QString& host, quint16 port, const QString& credentials,  QObject* parent = nullptr );

    void Connect();
    bool Send(const QString& message);
    void Close();

    bool IsConnected() const;
    bool IsAuthenticated() const {return m_authenticated;}

private:
    QTcpSocket    m_connection;
    const QString m_host;
    const quint16 m_port;
    const QString m_credentials; // <login>,<password>
    bool          m_authenticated = false;
    QTimer        m_timer;
};

#endif // CLIENT_H
