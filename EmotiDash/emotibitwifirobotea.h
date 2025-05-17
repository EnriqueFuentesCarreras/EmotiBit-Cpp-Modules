/*
MIT License

Copyright (c) 2018 Connected Future Labs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

--------------------------------------------------------------------------------
Este archivo ha sido modificado por Enrique Fuentes en el contexto de su Trabajo Fin de Grado.
El código se ha adaptado para integrarse en la herramienta RoboTEA, utilizando el
entorno Qt, extendiendo su funcionalidad y reorganizando su estructura.
*/



/**
 * @file emotibitwifirobotea.h
 * @author Enrique Fuentes
 * @date 2024-2025
 * @brief Clase para gestionar la comunicación WiFi con dispositivos EmotiBit integrada en RoboTEA.
 *
 * Esta clase proporciona métodos y estructuras para establecer y manejar conexiones WiFi
 * con dispositivos EmotiBit, encargándose de aspectos como publicidad en la red,
 * intercambio de datos, control de grabación, sincronización de tiempos y gestión de conexiones.
 *
 * @details
 * - Gestiona sockets UDP y TCP para datos y control.
 * - Implementa mecanismos para publicidad y descubrimiento automático de dispositivos.
 * - Ofrece funcionalidades para el envío y recepción de paquetes de datos y control.
 * - Utiliza multihilo para el procesamiento asíncrono y simultáneo de datos y publicidad.
 * - Proporciona interfaces para integración en aplicaciones Qt.
 *
 * @note Este código ha sido adaptado y refactorizado por Enrique Fuentes para integrarse en RoboTEA.
 *       Originalmente basado en el proyecto EmotiBit WiFi Host desarrollado por Connected Future Labs.
 *
* @author Enrique Fuentes
* @date Mayo 2025
*
* @details
* Proyecto desarrollado en C++ con Qt 6.7.2 para la UNED.
*
* - Framework: Qt (módulos: Network, Widgets, Core, etc.)
* - Compilador: MSVC / MinGW (según configuración)
* - Entorno: Windows 10/11
* - Herramientas de documentación: Doxygen, Graphviz
* - Licencia: MIT (si corresponde)
 */

#ifndef EMOTIBIT_WIFI_ROBOTEA_H
#define EMOTIBIT_WIFI_ROBOTEA_H

//#include <string>
#include <vector>
#include <utility> // para std::pair
#include "emotiBitComms.h"
#include "QEmotiBitPacket.h"
#include <QString>
#include <QVector>
#include <QMutexLocker>
#include <QUdpSocket>
#include <QTcpServer>
#include <thread>
#include <QMutex>



#include "DoubleBuffer.h" // Incluye la definición de DoubleBuffer
#include <unordered_map>
#include <QObject>


using namespace std;



class EmotiBitWiFiRoboTEA : public QObject       {
    Q_OBJECT // Necesario para usar señales y ranuras
public:
    // Estructura de configuración WiFi con valores predeterminados
    struct WifiHostSettings {
        int sendAdvertisingInterval = 1000; // Intervalo entre envíos de publicidad (ms)    //advertisingPort success	availabilityTimeout _discoveredEmotibits _discoveredEmotibits sending pingInterval EMOTIBITINFO onData i
        int checkAdvertisingInterval = 100; // Intervalo entre revisiones (ms)
        int advertisingThreadSleep = 0;     // Tiempo de suspensión en el hilo de publicidad (μs)
        int dataThreadSleep = 0;            // Tiempo de suspensión en el hilo de datos (μs)

        bool enableBroadcast = true;        // Habilitar transmisión por broadcast
        bool enableUnicast = true;          // Habilitar transmisión por unicast
        pair<int, int> unicastIpRange = {2, 254}; // Rango de direcciones IP para unicast
        int nUnicastIpsPerLoop = 1;         // Número de IPs unicast por ciclo
        int unicastMinLoopDelay = 3;        // Tiempo mínimo entre ciclos de unicast (ms)

        vector<QString> networkIncludeList = {"*.*.*.*"}; // Redes incluidas por defecto
        vector<QString> networkExcludeList = {""};        // Redes excluidas por defecto
    };

    QVector<QString> availableNetworks; // redes disponibles
    QVector<QString> emotibitNetworks;  // Redes que contienen dispositivos EmotiBit

    quint16 startCxnInterval = 100;

    quint16 advertisingPort;
    quint16 _dataPort;
    quint16 sendDataPort;
    quint16 controlPort;

    QUdpSocket* advertisingCxn;
    QUdpSocket* dataCxn;
    QTcpServer* controlCxn;
    QTcpSocket* connectedClient;


    QMutex controlCxnMutex;      // Mutex para proteger el acceso a controlCxn
    QMutex dataCxnMutex;         // Mutex para proteger el acceso a dataCxn
    QMutex discoveredEmotibitsMutex;


    quint16 advertisingPacketCounter = 0;
    quint16 controlPacketCounter = 0;
    quint16 dataPacketCounter = 0;


    qint16 pingInterval = 500;
    qint64 connectionTimer;
    qint16 connectionTimeout = 10000;
    qint16 availabilityTimeout = 1000;
    qint16 ipPurgeTimeout = 15000;

    QString connectedEmotibitIdentifier;

    DoubleBuffer<QString> dataPackets;

    bool _isConnected;
    bool isStartingConnection;
    quint16 startCxnTimeout = 5000;	// milliseconds
    quint64 startCxnAbortTimer;


    static const quint8 SUCCESS = 0;
    static const quint8 FAIL = -1;

    string connectedEmotibitIp;

    std::unordered_map<string, qEmotiBitPacket::EmotibitInfo> _discoveredEmotibits;

    unordered_map<string, qEmotiBitPacket::EmotibitInfo> getdiscoveredEmotibits();

    QStringList getDiscoveredEmotibitIds() const;

    bool sendNota(const QString &nota);

    void updateAdvertisingIpList(const QString &ip);
    void flushData();
    void sendAdvertising();
    qint8 processAdvertising(QVector<QString> &infoPackets);

    bool  stopRecording();
    bool  startRecording();


    void stopThreads();



    //SEÑAL
    // void dataPacketReceived(const QString &packet);

    // Constructor
    //EmotiBitWiFiRoboTEA();
    //EmotiBitWiFiRoboTEA(QObject* parent = nullptr);
    explicit EmotiBitWiFiRoboTEA(QObject *parent = nullptr);
    ~EmotiBitWiFiRoboTEA();

    quint8 begin();
    void getAvailableNetworks();
    QVector<QString> getLocalIPs();

    bool isInNetworkExcludeList(const QString &ipAddress) const;
    bool isInNetworkIncludeList(const QString &ipAddress) const;
    bool isInNetworkList(const QString &ipAddress, const QList<QString> &networkList) const;

    quint8 _startDataCxn(uint16_t dataPort);

    // Método para inicializar la configuración WiFi (antes setWifiHostSettings)
    void parseCommSettings();

    // Obtener configuración de WiFi actual
    WifiHostSettings getWifiHostSettings() const;

    atomic_bool stopDataThread = {false};
    atomic_bool stopAdvertisingThread = { false };

    quint16 receivedDataPacketNumber = 60000;	// Tracks packet numbers (for multi-send). inicializa con un numero arbitrario largo

    void updateDataThread();
    void processAdvertisingThread();
    void threadSleepFor(int sleepMicros);

    void updateData();

    void processRequestData(const QString &packet, qint16 dataStartChar);

    QString getTimestampString(const QString &format);

    quint8 sendControl(const QString &packet);
    quint8 sendData(const QString &packet);
    void readData(vector<string> &packets);

    qint8 connect(const QString &deviceId);
    //qint8 connect(qint8 i);          //esta declarado pero no esta desarrollado en el cpp
    qint8 disconnect();

public slots:
    void writeControlData(const QByteArray &data, const QString &expectedClientIp);
    //void  sendToControlPort(const QString &data);
signals:
    void newDataPacket(const QString &packet); // Señal para los paquetes nuevos.
    void sendDatagram(const QByteArray &data, const QHostAddress &address, quint16 port, QString socketType);
    void processIncomingData(const QByteArray &data, const QHostAddress &address, quint16 port, QString socketType);
    void controlDataToSend(const QByteArray &data, const QString &expectedClientIp);

private:
    void handleNewConnection();
    void handleClientDisconnected();
    WifiHostSettings _wifiHostSettings; // Configuración WiFi actual
    //std::thread* dataThread = nullptr;
    //std::thread* advertisingThread = nullptr;
    QThread* dataThread= nullptr;
    QThread* advertisingThread = nullptr;
    //void handleSocketReadyRead(QUdpSocket *socket, const QString &socketType);
    void onSendDatagram(const QByteArray &data, const QHostAddress &address, quint16 port, QString socketType);
   // void connectToControlPort();
};




#endif // EMOTIBIT_WIFI_ROBOTEA_H
