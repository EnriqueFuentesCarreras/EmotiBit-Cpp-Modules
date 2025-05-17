#ifndef EMOTIBITEMULATOR_H
#define EMOTIBITEMULATOR_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QHostAddress>
#include <QElapsedTimer>

class EmotiBitEmulator : public QObject
{
    Q_OBJECT

public:
    explicit EmotiBitEmulator(QObject *parent = nullptr);
    ~EmotiBitEmulator();

    void setCsvFile(const QString &filePath);
    void startEmulation();
    void stopEmulation();

signals:
    void messageLogged(const QString &message);

private slots:
    void readAdvertisingMessage();
    void readDataMessage();
    void sendHelloHostMessage(const QHostAddress &sender, quint16 senderPort);
    void sendPongMessage();
    void sendData();

private:
    bool setupAdvertisingSocket();
    QString getLocalIpAddress();
    void connectHost(quint16 controlPort, quint16 dataPort);
    QString formatPacket(const QString &payload);

    // Sockets para comunicación
    QUdpSocket *advertisingSocket;
    QUdpSocket *dataSocket;
    QTcpSocket *controlClientSocket;
    QTimer *dataTimer;

    qint64 hostTimestamp = -1;  // Timestamp más reciente recibido del host
    qint64 localTimestampOffset = 0; // Diferencia entre el tiempo local y el del host


    // Variables para el archivo CSV
    QFile csvFile;
    QTextStream csvStream;

    // Estado de conexión y configuración
    bool isConnected;
    QString deviceId;
    quint16 advertisingPort;
    quint16 dataPort;    // Se establecerá luego del EMOTIBIT_CONNECT
    quint16 controlPort; // Se establecerá luego del EMOTIBIT_CONNECT
    quint16 packetNumber;

    QHostAddress hostIp;
    QHostAddress hostAddress;

    QHostAddress senderAddress; // Dirección IP del Oscilloscope o Host
    quint16 senderPort;         // Puerto desde el cual el Oscilloscope envía mensajes

    QElapsedTimer elapsedTimer; // Agregado para medir el tiempo transcurrido desde el inicio
};

#endif
