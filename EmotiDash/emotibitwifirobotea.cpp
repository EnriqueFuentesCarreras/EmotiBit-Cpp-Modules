#include "EmotiBitWiFiRoboTEA.h"
#include "qemotibitpacket.h"
#include <QString>
#include <QVector>
#include <QDebug>
#include <QProcess>
#include <QStringList>
#include <QHostAddress>
#include <cmath> // Para pow()
#include <QVariant>
#include <thread>
#include <chrono>
#include <QMutexLocker>
#include <QByteArray>
#include <QDateTime>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QObject>
#include <QThread>

/**
 * \brief Constructor de la clase EmotiBitWiFiRoboTEA
 *
 * Inicializa los sockets UDP y el servidor TCP para la comunicación con el dispositivo EmotiBit.
 * Establece las conexiones necesarias entre señales y slots para manejar el envío de datagramas
 * y la gestión de nuevas conexiones.
 *
 * \param parent Puntero al objeto padre (por defecto, nullptr)
 */
// Constructor: inicializa con valores predeterminados
EmotiBitWiFiRoboTEA::EmotiBitWiFiRoboTEA(QObject* parent)
    : QObject(parent),
    advertisingCxn(new QUdpSocket(this)),
    dataCxn(new QUdpSocket(this)),
    controlCxn(new QTcpServer(this)),
    connectedClient(nullptr)             // Inicializa a nullptr if (
{
    // Conectar señales para enviar datagramas
    QObject::connect(this, &EmotiBitWiFiRoboTEA::sendDatagram, this, &EmotiBitWiFiRoboTEA::onSendDatagram, Qt::QueuedConnection);


    // Conectar la señal de nuevas conexiones del QTcpServer
    QObject::connect(controlCxn, &QTcpServer::newConnection, this, &EmotiBitWiFiRoboTEA::handleNewConnection);

    // Conectar señales para enviar datos de control
    QObject::connect(this, &EmotiBitWiFiRoboTEA::controlDataToSend,this, &EmotiBitWiFiRoboTEA::writeControlData, Qt::QueuedConnection);  // senddataport



}

/*
 * \brief Destructor de la clase EmotiBitWiFiRoboTEA
 *
 * Libera todos los recursos utilizados:
 * - Detiene los hilos de datos y publicidad
 * - Desconecta y elimina el cliente TCP
 * - Cierra y elimina los sockets UDP y el servidor TCP
 *
 * Garantiza una salida limpia del programa y evita pérdidas de memoria.
 */
EmotiBitWiFiRoboTEA::~EmotiBitWiFiRoboTEA()    {
    // 1. Forzar la salida de los bucles
    stopDataThread = true;
    stopAdvertisingThread = true;

    // 2. Terminar el hilo de datos
    if (dataThread) {
        dataThread->quit();
        dataThread->wait();
        dataThread->deleteLater();
        dataThread = nullptr;
    }

    // 3. Terminar el hilo de publicidad
    if (advertisingThread) {
        advertisingThread->quit();
        advertisingThread->wait();
        advertisingThread->deleteLater();
        advertisingThread = nullptr;
    }

    // 4. Desconectar y eliminar el cliente TCP
    if (connectedClient) {
        connectedClient->disconnectFromHost();
        connectedClient->deleteLater();
        connectedClient = nullptr;
    }

    // 5. Cerrar y eliminar el servidor TCP
    if (controlCxn) {
        controlCxn->close();
        controlCxn->deleteLater();
        controlCxn = nullptr;
    }

    // 6. Cerrar y eliminar los sockets UDP
    if (advertisingCxn) {
        advertisingCxn->close();
        advertisingCxn->deleteLater();
        advertisingCxn = nullptr;
    }

    if (dataCxn) {
        dataCxn->close();
        dataCxn->deleteLater();
        dataCxn = nullptr;
    }

    qDebug() << "Destructor de EmotiBitWiFiRoboTEA ejecutado correctamente.";
}



//____________________________________________________________
//______________________BEGIN_________________________________
//____________________________________________________________
//____________________________________________________________ time
/*!
 * \brief Inicializa la conexión WiFi con el dispositivo EmotiBit.
 *
 * Este método configura los sockets UDP y TCP necesarios para comunicar con el EmotiBit.
 * Además, inicia los hilos de recepción de datos y publicidad, y configura los puertos
 * dinámicamente para evitar conflictos.
 *
 * Pasos principales:
 * - Obtiene redes disponibles y valida su existencia.
 * - Inicializa el socket de publicidad (`advertisingCxn`).
 * - Llama a `_startDataCxn()` para establecer puerto de datos.
 * - Asigna un puerto libre para el control TCP (`controlCxn`).
 * - Crea e inicia hilos `dataThread` y `advertisingThread`.
 * - Configura opciones de los sockets.
 *
 * \return quint8
 *         - SUCCESS si todo se inicializa correctamente.
 *         - FAIL si no hay redes disponibles o no se puede establecer un puerto.
 */
quint8 EmotiBitWiFiRoboTEA::begin() {
    advertisingPort = EmotiBitComms::WIFI_ADVERTISING_PORT;
    getAvailableNetworks();
    if (availableNetworks.size() == 0) {
        qDebug() << "check if network adapters are enabled";
        return FAIL;
    }

    //advertisingCxn.SetNonBlocking(true); no es necesario, ya que en Qt los sockets no son bloqueantes
    //Sockets NO BLOQUEANTES permiten que la aplicación continúe ejecutándose sin detenerse esperando operaciones de red.
    advertisingCxn = new QUdpSocket(this);
    advertisingCxn->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, QVariant(static_cast<int>(std::pow(2, 10))));

    _startDataCxn(EmotiBitComms::WIFI_ADVERTISING_PORT + 1);

    controlPort = _dataPort + 1;

    while (!controlCxn->listen(QHostAddress::Any, controlPort)) {
        //Incrementa el puerto y vuelve a intentar
        controlPort += 2;
        controlCxn->close();
        qDebug() << "Trying control port:" << controlPort;
    }

    qDebug() << "EmotiBit data port: " << _dataPort;
    qDebug() << "EmotiBit control port: " << controlPort;

    advertisingPacketCounter = 0;
    controlPacketCounter = 0;
    connectedEmotibitIp = "";
    _isConnected = false;
    isStartingConnection = false;

    //dataThread = new std::thread(&EmotiBitWiFiRoboTEA::updateDataThread, this);
    //advertisingThread = new std::thread(&EmotiBitWiFiRoboTEA::processAdvertisingThread, this);

    // Crear hilos s
    dataThread = QThread::create([this]() {updateDataThread();});
    advertisingThread = QThread::create([this]() {processAdvertisingThread();});


    // Configurar hilos
    QObject::connect(advertisingThread, &QThread::finished, advertisingThread, &QThread::deleteLater);
    advertisingThread->start();
    QObject::connect(dataThread, &QThread::finished, dataThread, &QThread::deleteLater);
    dataThread->start();

    // Mover los sockets a los hilos correspondientes
    // advertisingCxn->moveToThread(advertisingThread);
    // dataCxn->moveToThread(dataThread);

    // Configurar sockets
    advertisingCxn->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, QVariant(32768));
    dataCxn->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, QVariant(32768));



    qDebug() << "funcion begin() ejecutada con exito";
    return SUCCESS;
}

//____________________________________________________________
//____________________________________________________________


// Método para inicializar la configuración WiFi
void EmotiBitWiFiRoboTEA::parseCommSettings() {
    // En esta implementación, utilizamos los valores predeterminados.

}




/*
 * \brief Devuelve la configuración actual del host WiFi.
 * Método getter para acceder a los valores internos de configuración WiFi del sistema.
 * \return WifiHostSettings Estructura que contiene los parámetros actuales del host WiFi.
 */
// Obtener configuración de WiFi actual
EmotiBitWiFiRoboTEA::WifiHostSettings EmotiBitWiFiRoboTEA::getWifiHostSettings() const {
    return _wifiHostSettings;
}
//_____________________________________________



/*
 * \brief Inicia la conexión UDP para recepción de datos desde EmotiBit.
 * Este método intenta enlazar el socket `dataCxn` a un puerto dado. Si falla,
 * incrementa el puerto y vuelve a intentar hasta 10 veces. Si tiene éxito,
 * configura el buffer de recepción y otras opciones del socket.
 * \param dataPort Puerto inicial que se intenta usar para la conexión de datos.
 * \return quint8
 *         - SUCCESS si la vinculación fue exitosa.
 *         - FAIL si no se pudo enlazar después de varios intentos.
 */
quint8 EmotiBitWiFiRoboTEA::_startDataCxn(quint16 dataPort){
    //Intenta enlazar el socket UDP dataCxn al puerto de datos que se pasa, incrementando el
    //puerto si falla hasta  máximo de 10 intentos, configura el socket
    //si la vinculación es exitosa y devuelve  SUCCESS o FAIL.
    _dataPort = dataPort;


    // Intentar enlazar el socket al puerto especificado
    bool bound = false;
    const int MAX_RETRIES = 10;
    int tries = 0;

    while (!bound && tries < MAX_RETRIES)  {
        if (dataCxn->bind(QHostAddress::Any, _dataPort, QAbstractSocket::DefaultForPlatform)) {
            qDebug() << "Enlazado el socket al puerto:" << _dataPort;

            bound = true;
        }
        else  {
            // Incrementa el puerto y vuelve a intentar
            _dataPort += 2;
            qDebug() << "Trying data port:" << _dataPort;
            tries++;
        }
    }

    if (!bound)  {
        qDebug() << "Failed to bind dataCxn after multiple attempts.";
        return FAIL;
    }

    // Configurar el socket_____________________________________________________________________________________
    //dataCxn->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, QVariant(32768)); // Tamaño del buffer: 2^15 bytes    131072
    dataCxn->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, QVariant(131072));
    dataCxn->setSocketOption(QAbstractSocket::LowDelayOption, true); // No es necesario en UDP, pero se incluye para consistencia
    dataCxn->setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(1)); // Ejemplo de opción adicional para multicast
    //-------------------------------------------------------------------------------------------------------------

    // Obtener información para depuración
    QVariant bufferSizeVar = dataCxn->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);
    int receiveBufferSize = bufferSizeVar.toInt();
    qDebug() << "dataCxn GetReceiveBufferSize:" << receiveBufferSize;
    qDebug() << "dataCxn initialized successfully on port:" << _dataPort;
    return SUCCESS;
}
//____________________________________________


/*
 * \brief Escanea y almacena redes locales disponibles compatibles.
 *
 * Esta función intenta detectar redes locales basándose en las IPs de los adaptadores
 * de red del sistema. Filtra las subredes utilizando listas de inclusión y exclusión
 * definidas en la configuración WiFi.
 *
 * Si detecta nuevas redes válidas, las añade a `availableNetworks` y las muestra por consola.
 */
void EmotiBitWiFiRoboTEA::getAvailableNetworks() {
    QVector<QString> ips;
    QVector<QString> currentAvailableNetworks = availableNetworks;
    const int NUM_TRIES_GET_IP = 10;
    int tries = 0;
    // Intenta obtener local IPs
    while (ips.isEmpty() && tries < NUM_TRIES_GET_IP) {
        ips = getLocalIPs();
        tries++;
    }
    if (!ips.isEmpty()) {
        // Get all available networks
        for (const auto& ip : ips) {
            QStringList ipSplit = ip.split(".");
            if (ipSplit.size() >= 3) {
                QString tempNetwork = ipSplit[0] + "." + ipSplit[1] + "." + ipSplit[2];
                if (!availableNetworks.contains(tempNetwork) && isInNetworkIncludeList(tempNetwork) &&  !isInNetworkExcludeList(tempNetwork)) {
                    availableNetworks.append(tempNetwork);
                    qDebug() << "Network adapters "<<tempNetwork;
                }
            }
        }
    }
    // If new networks are detected, print all networks
    if (availableNetworks.size() != currentAvailableNetworks.size()) {
        QString allAvailableNetworks;
        for (const auto& network : availableNetworks) {
            allAvailableNetworks += "[" + network + ".*] ";
        }
        qDebug() << "All Network(s):" << allAvailableNetworks;
    }
}
//_________




/*
 * \brief Obtiene todas las direcciones IPv4 locales (no loopback) del sistema.
 *
 * Lanza el comando `ipconfig` (Windows) para obtener las IPs asociadas a los adaptadores
 * de red activos. Omite direcciones de loopback (127.*).
 *
 * \return QVector<QString> Lista de direcciones IP detectadas en el sistema.
 */
QVector<QString> EmotiBitWiFiRoboTEA::getLocalIPs() {
    QVector<QString> result;
    QProcess process;
    process.start("ipconfig");
    process.waitForFinished();
    QString commandResult = process.readAllStandardOutput();

    int pos = 0;
    while ((pos = commandResult.indexOf("IPv4", pos)) >= 0) {
        pos = commandResult.indexOf(":", pos) + 2;
        int pos2 = commandResult.indexOf("\n", pos);

        QString ip = commandResult.mid(pos, pos2 - pos).trimmed();
        pos = pos2;

        if (!ip.startsWith("127")) { // Omitir direcciones loopback
            qDebug() << "la funcion getLocalIps() encontro: " << ip;
            result.append(ip);
        }
    }

    return result;
}
//___________________




//____________________Filtro redes _____________________________________________
//_______________________________________________________________________________
/*
 * \brief Convierte un std::vector<QString> a un QList<QString>.
 *
 * Utilidad para adaptar tipos de contenedores de C++ estándar a los de Qt.
 *
 * \param vec Vector estándar de cadenas Qt.
 * \return QList<QString> Lista Qt equivalente.
 */

QList<QString> convertToQList(const std::vector<QString> &vec) {
    QList<QString> list;
    for (const auto &item : vec) {
        list.append(item);
    }
    return list;
}
//_______________


/*
 * \brief Verifica si una IP está en la lista de redes excluidas.
 *
 * Utiliza la configuración de `_wifiHostSettings` para comparar la IP dada con la
 * lista de exclusión.
 *
 * \param ipAddress Dirección IP en formato texto.
 * \return true si la IP está en la lista de exclusión, false en caso contrario.
 */
bool EmotiBitWiFiRoboTEA::isInNetworkExcludeList(const QString &ipAddress) const {
    QList<QString> excludeList = convertToQList(_wifiHostSettings.networkExcludeList);
    return isInNetworkList(ipAddress, excludeList);
}
//________________________________


/*
 * \brief Verifica si una IP está en la lista de redes permitidas.
 *
 * Consulta la lista de inclusión definida en `_wifiHostSettings`.
 *
 * \param ipAddress Dirección IP a comprobar.
 * \return true si la IP está incluida, false si no lo está.
 */
bool EmotiBitWiFiRoboTEA::isInNetworkIncludeList(const QString &ipAddress) const {
    QList<QString> includeList = convertToQList(_wifiHostSettings.networkIncludeList);
    return isInNetworkList(ipAddress, includeList);
}
//________________________


/*
 * \brief Comprueba si una dirección IP pertenece a alguna red de una lista dada.
 *
 * Compara la dirección `ipAddress` con cada entrada de `networkList`, aceptando
 * comodines (`*`) en los segmentos. Devuelve true si hay coincidencia parcial o total.
 *
 * \param ipAddress Dirección IP a verificar.
 * \param networkList Lista de redes en formato de string (ej: "192.168.1").
 * \return true si la IP pertenece a alguna red de la lista, false si no.
 */
bool EmotiBitWiFiRoboTEA::isInNetworkList(const QString &ipAddress, const QList<QString> &networkList) const {
    bool out = false;

    // Dividir la dirección IP en partes
    QStringList ipSplit = ipAddress.split('.');

    // Recorrer la lista de redes
    for (const QString &listIp : networkList) {
        // Dividir la dirección de la lista en partes
        QStringList listIpSplit = listIp.split('.');
        bool partMatch = true;

        // Comparar cada segmento de la dirección IP
        for (int n = 0; n < ipSplit.size() && n < listIpSplit.size(); n++) {
            if (listIpSplit.at(n) == "*" || listIpSplit.at(n) == ipSplit.at(n)) {
                // Coincidencia parcial
                continue;
            } else {
                partMatch = false;
                break;
            }
        }
        if (partMatch) {
            // Se encontró una coincidencia
            out = true;
            break;
        }
    }
    return out;
}
//________________________










//__________________________________________________________________________
//__________________________________________________________________________
//__________________________________________________________________________
//______________________________________________________________________________
//__________________Adversiting, Hilo____________________________________________
//_______________________________________________________________________________
//_______________________________________________________________________________
/*
 * \brief Hilo principal encargado de emitir paquetes de advertising.
 *
 * Ejecuta un bucle que llama a `processAdvertising()` periódicamente para
 * mantener visible la presencia del EmotiBit en la red.
 * El ciclo se interrumpe cuando `stopAdvertisingThread` es true.
 */
void EmotiBitWiFiRoboTEA::processAdvertisingThread(){
    qDebug() << "HILO processAdvertisingThread comenzara a ejecutarse...";

    while (!stopAdvertisingThread) {
        QVector<QString> infoPackets;
        processAdvertising(infoPackets);
        threadSleepFor(_wifiHostSettings.advertisingThreadSleep);
    }
}
//_______________________________________________________________________________






/*
 * \brief Añade una red detectada al listado de redes donde se han encontrado EmotiBits.
 *
 * Extrae los tres primeros octetos de la IP para representar la red. Si la red
 * no está en `emotibitNetworks`, la añade. Imprime las redes si hay alguna nueva.
 *
 * \param ip Dirección IP detectada de un EmotiBit .
 */
void EmotiBitWiFiRoboTEA::updateAdvertisingIpList(const QString &ip) {
    QVector<QString> currentEmotibitNetworks = emotibitNetworks;
    QStringList ipSplit = ip.split(".");

    if (ipSplit.size() < 3) {
        qWarning() << "Invalid IP address format:" << ip;
        return;
    }

    QString networkAddr = ipSplit[0] + "." + ipSplit[1] + "." + ipSplit[2];

    if (emotibitNetworks.isEmpty()) { // Asumimos que todos los EmotiBits están en la misma red
        emotibitNetworks.append(networkAddr);
    }

    // Imprimir todas las direcciones IP de EmotiBit y/o redes cuando se detecten nuevos EmotiBits
    if (emotibitNetworks.size() != currentEmotibitNetworks.size()) {
        QString allEmotibitNetworks;
        for (const auto &network : emotibitNetworks) {
            allEmotibitNetworks += "[" + network + ".*] ";
        }
        qDebug() << "Emotibit Network(s):" << allEmotibitNetworks;
    }
}
//_______________________________________________________________________________


/*
 * \brief Vacía el búfer de recepción del socket de datos `dataCxn`.
 *
 * Utiliza `QMutexLocker` para asegurar el acceso exclusivo al recurso
 * durante la operación de vaciado.
 */
void EmotiBitWiFiRoboTEA::flushData(){
    QMutexLocker locker(&dataCxnMutex);
    dataCxn->flush();
}




//_______________________________________________________________________________
//_______________________________________________________________________________
/*
 * \brief Envía paquetes de publicidad para descubrir dispositivos EmotiBit.
 *
 * Este método administra el envío de paquetes de publicidad tanto por broadcast
 * como por unicast según la configuración. También regula el ritmo de envío
 * para evitar sobrecargar la red.
 */
void EmotiBitWiFiRoboTEA::sendAdvertising() {
    static bool emotibitsFound = false;
    static bool startNewSend = true;
    static bool sendInProgress = true;
    static int unicastNetwork = 0;
    static int broadcastNetwork = 0;
    static int hostId = _wifiHostSettings.unicastIpRange.first;

    static qint64 sendAdvertisingTimer = QDateTime::currentMSecsSinceEpoch();
    qint64 sendAdvertisingTime = QDateTime::currentMSecsSinceEpoch() - sendAdvertisingTimer;
    if (sendAdvertisingTime >= _wifiHostSettings.sendAdvertisingInterval)    {
        // Iniciar un nuevo envío de publicidad periódicamente
        sendAdvertisingTimer = QDateTime::currentMSecsSinceEpoch();
        startNewSend = true;
        sendInProgress = true;
    }

    if (!emotibitNetworks.isEmpty())    {
        // Solo buscar en todas las redes hasta que se encuentre un EmotiBit
        // ToDo: considerar permitir EmotiBits en múltiples redes
        emotibitsFound = true;
    }

    if (!emotibitsFound && startNewSend)  {
        getAvailableNetworks(); // Verificar si apareció una nueva red después de abrir la aplicación
    }

    // **** Manejar envíos de publicidad ****
    // Manejar publicidad por broadcast
    if (_wifiHostSettings.enableBroadcast && startNewSend) {
        QString broadcastIp;

        if (emotibitsFound)    {
            broadcastIp = emotibitNetworks.at(0) + "." + QString::number(255);
        }
        else   {
            broadcastIp = availableNetworks.at(broadcastNetwork) + "." + QString::number(255);
        }
        //qDebug() << "Sending advertising broadcast:" << sendAdvertisingTime;
        // qDebug() << broadcastIp;
        startNewSend = false;

        QString packet = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::HELLO_EMOTIBIT,advertisingPacketCounter++,"", 0);


        // Habilitar broadcast
        QHostAddress broadcastAddress(broadcastIp);
        QByteArray data = packet.toUtf8();

        // advertisingCxn->writeDatagram(data, broadcastAddress, advertisingPort);//:
        emit sendDatagram(data, broadcastAddress, advertisingPort, "advertisingCxn");

        if (!emotibitsFound)   {
            broadcastNetwork++;
            if (broadcastNetwork >= availableNetworks.size())     {
                broadcastNetwork = 0;
            }
        }

        // Omitir unicast cuando se envía broadcast para evitar spam en la red
        return;
    }
    // Manejar publicidad por unicast
    if (_wifiHostSettings.enableUnicast && sendInProgress)    {
        static qint64 unicastLoopTimer = QDateTime::currentMSecsSinceEpoch();
        qint64 unicastLoopTime = QDateTime::currentMSecsSinceEpoch() - unicastLoopTimer;
        // Limitar la tasa de envío unicast
        if (unicastLoopTime >= _wifiHostSettings.unicastMinLoopDelay)  {
            unicastLoopTimer = QDateTime::currentMSecsSinceEpoch();
            //qDebug() << "Sending advertising unicast:" << unicastLoopTime;  //dispositivo

            for (qint32 i = 0; i < _wifiHostSettings.nUnicastIpsPerLoop; i++)  {
                QString unicastIp;

                if (emotibitsFound)   {
                    unicastIp = emotibitNetworks.at(0) + "." + QString::number(hostId);
                }
                else   {
                    unicastIp = availableNetworks.at(unicastNetwork) + "." + QString::number(hostId);
                }

                if (_wifiHostSettings.enableUnicast && sendInProgress){
                    //qDebug() << unicastIp;
                    QString packet = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::HELLO_EMOTIBIT,advertisingPacketCounter++, "",  0); //el ultimo dato(0) es tamaño
                    // Deshabilitar broadcast
                    //advertisingCxn->setSocketOption(QAbstractSocket::BroadcastOption, false);
                    //advertisingCxn->setSocketOption(QAbstractSocket::BroadcastOption, 0);  en Qt no es necesaro habilitar/desabilitar broadcast
                    QHostAddress unicastAddress(unicastIp);
                    QByteArray data = packet.toUtf8();
                    //advertisingCxn->writeDatagram(data, unicastAddress, advertisingPort);//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                    emit sendDatagram(data, unicastAddress, advertisingPort, "advertisingCxn");
                }

                // Iterar dirección IP
                if (hostId < _wifiHostSettings.unicastIpRange.second)                {
                    hostId++;
                }
                else   {
                    // Se alcanzó el final de unicastIpRange
                    hostId = _wifiHostSettings.unicastIpRange.first; // Reiniciar hostId al inicio del rango
                    if (emotibitsFound) {
                        // Se completó el envío a todas las IPs
                        sendInProgress = false;
                        break;
                    }
                    else  {
                        unicastNetwork++;
                        if (unicastNetwork >= availableNetworks.size())  {
                            // Se alcanzó el final de unicastIpRange para la última red conocida en la lista
                            sendInProgress = false;
                            unicastNetwork = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
}
//__________________________________________________________________________________________________________




/*
 * \brief Procesa los paquetes de advertising recibidos de los dispositivos EmotiBit.
 *
 * Este método se encarga de:
 *  - Llamar a `sendAdvertising()`.
 *  - Escuchar mensajes HELLO_HOST y PONG.
 *  - Detectar dispositivos disponibles y actualizar su estado.
 *  - Gestionar intentos de conexión y mantener viva la conexión con PING.
 *
 * \param infoPackets Vector donde se almacenarán paquetes no reconocidos que pueden contener datos útiles.
 * \return SUCCESS si el proceso se ejecuta correctamente.
 */
qint8 EmotiBitWiFiRoboTEA::processAdvertising(QVector<QString> &infoPackets){
    const int maxSize = 32768;
    sendAdvertising();
    static qint64 checkAdvertisingTimer = QDateTime::currentMSecsSinceEpoch();
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 checkAdvertisingTime = currentTime - checkAdvertisingTimer;

    if (checkAdvertisingTime >= _wifiHostSettings.checkAdvertisingInterval) {
        checkAdvertisingTimer = currentTime;
        //qDebug() << "checkAdvertising:" << checkAdvertisingTime;

        // Receive advertising messages
        QByteArray udpMessage;
        udpMessage.resize(maxSize);

        QHostAddress senderIp;
        quint16 senderPort;



        if (advertisingCxn->state() == QUdpSocket::BoundState) {
            int size = advertisingCxn->pendingDatagramSize();
            udpMessage.resize(size);
            dataCxn->readDatagram(udpMessage.data(), udpMessage.size());   //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: moveToThread
        } else {
            qWarning() << "Socket no está en estado BoundState. Estado actual:" << dataCxn->state();
        }


        qint64 msgSize = advertisingCxn->readDatagram(udpMessage.data(), udpMessage.size(), &senderIp, &senderPort);
        //emit processIncomingData(udpMessage.left(msgSize), senderIp, senderPort, "advertisingCxn");

        if (msgSize > 0)  {
            QString message = QString::fromUtf8(udpMessage.left(msgSize));
            //qDebug() << "Received:" << message;


            QStringList packets = message.split(qEmotiBitPacket::PACKET_DELIMITER_CSV);

            for (const QString &packet : packets)      {
                qEmotiBitPacket::Header header;
                qint16 dataStartChar = qEmotiBitPacket::getHeader(packet, header);
                if (dataStartChar != 0)    {
                    //_________________________________HELLO_HOST
                    //__________________________________________
                    if (header.typeTag == qEmotiBitPacket::TypeTag::HELLO_HOST){
                        //qDebug() << "HELLO_HOST recibido";
                        QString value;
                        QString emotibitDeviceId = "";
                        qint16 valuePos = qEmotiBitPacket::getPacketKeyedValue(packet, qEmotiBitPacket::PayloadLabel::DATA_PORT, value, dataStartChar);
                        //qDebug() << "HELLO_HOST recibido, DataPort: "<<value;
                        if (valuePos > -1)   {
                            updateAdvertisingIpList(senderIp.toString());
                            //qDebug() << "EmotiBit IP:" << senderIp.toString() << ":" << senderPort;
                            qint16 deviceIdPos = -1;
                            deviceIdPos = qEmotiBitPacket::getPacketKeyedValue(packet, qEmotiBitPacket::PayloadLabel::DEVICE_ID, emotibitDeviceId, dataStartChar);
                            if (deviceIdPos > -1)  {
                                // se encontro el identificador en el mensaje HELLO_HOST
                                // qDebug() << "EmotiBit DeviceId:" << emotibitDeviceId;
                            }
                            else {
                                emotibitDeviceId = senderIp.toString();
                                //qDebug() << "EmotiBit DeviceId no esta disponible, se usara  IP address como identificador";
                            }

                            QMutexLocker locker(&discoveredEmotibitsMutex);
                            std::string emotibitDeviceIdStd = emotibitDeviceId.toStdString();
                            //qDebug() << "___Se va ha a establecer Available"<< emotibitDeviceId;
                            qint64 tiempo=currentTime;
                            auto result = _discoveredEmotibits.emplace(emotibitDeviceIdStd,  qEmotiBitPacket::EmotibitInfo(senderIp.toString(), value.toInt() == EmotiBitComms::EMOTIBIT_AVAILABLE,tiempo));
                            //qDebug() << "___Se va ha a establecido con EXITO Available"<< emotibitDeviceId;
                            if(!result.second){
                                // if it's not a new IP address, update the status
                                result.first->second=qEmotiBitPacket::EmotibitInfo(senderIp.toString(), value.toInt() == EmotiBitComms::EMOTIBIT_AVAILABLE,tiempo);
                                //qDebug() << " Establecido como EMOTIBIT_AVAILABLE________ " << EmotiBitComms::EMOTIBIT_AVAILABLE ;
                            }
                        }
                    }
                    //____________________________FIN__HELLO_HOST
                    //__________________________________________

                    //--------------------PONG-----------------------------------------------
                    //-----------------------------------------------------------------------
                    else if (header.typeTag == qEmotiBitPacket::TypeTag::PONG)  {
                        // PONG
                        if (senderIp.toString() == QString::fromStdString(connectedEmotibitIp)) {
                            //____________________________________BUSCA DE DISPOSITIVO
                            // Buscar el dispositivo en la lista
                            auto it = _discoveredEmotibits.find(senderIp.toString().toStdString());
                            if (it != _discoveredEmotibits.end()) {
                                // Actualizar la marca de tiempo de la última vez visto
                                it->second.lastSeen = QDateTime::currentMSecsSinceEpoch();
                                it->second.isAvailable = true;  // Asegurar que esté marcado como disponible
                                //qDebug() << "Actualizado lastSeen para" << senderIp.toString() << "a" << it->second.lastSeen;
                            } else {
                                //qDebug() << "Dispositivo no encontrado en _discoveredEmotibits:" << senderIp.toString();
                            }
                            //___________________________________FIN BUSQUEDA

                            //_____VERIFICA PUERTO, establece o mantiene el estado de conexión.
                            QString value;
                            qint16 valuePos = qEmotiBitPacket::getPacketKeyedValue(packet, qEmotiBitPacket::PayloadLabel::DATA_PORT, value, dataStartChar);
                            if (valuePos > -1 && value.toInt() == _dataPort)  {
                                if (isStartingConnection)   {
                                    flushData();
                                    _isConnected = true;
                                    isStartingConnection = false;
                                    qDebug() << " Cambiando estado: ****_isConnected: true; ****isStartingConnection = false; **** ";
                                }
                                if (_isConnected)   {
                                    connectionTimer = QDateTime::currentMSecsSinceEpoch();
                                }
                            }
                            //_____FIN VERIFICA
                        }
                    }
                    //--------------FIN----PONG-----------------------------------------------
                    //-----------------------------------------------------------------------
                    else {
                        infoPackets.append(packet);
                    }
                }
            }
        }

        //__________________________________________________________________________________________________________________________
        //_____________Envía periódicamente un paquete PING al EmotiBit conectado para mantener o verificar el estado de la conexión.
        //qDebug() << " Se comprobara si esta conectado (_isconnected). " ;
        if (_isConnected)   {
            //qDebug() << " Esta conectado --> _isConnected=true; se mandara PING si se cumplen intervalos de tiempo. " ;
            // Si estamos conectados, enviar PING periódicamente
            static qint64 pingTimer = QDateTime::currentMSecsSinceEpoch();
            if (QDateTime::currentMSecsSinceEpoch() - pingTimer > pingInterval)   {
                pingTimer = QDateTime::currentMSecsSinceEpoch();
                QVector<QString> payload;
                payload.append(qEmotiBitPacket::PayloadLabel::DATA_PORT);
                payload.append(QString::number(_dataPort));
                QString packet = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::PING,advertisingPacketCounter++,payload,0);
                //qDebug() << "Sent: " << packet;
                // Enviar PING a la IP conectada
                QHostAddress address(QString::fromStdString(connectedEmotibitIp));
                QByteArray data = packet.toUtf8();
                //advertisingCxn->writeDatagram(data, address, advertisingPort);//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                emit sendDatagram(data, address, advertisingPort, "advertisingCxn");
            }
        }



        // **** Manejar Conexión en Progreso ****
        if (isStartingConnection) {
            // Enviar mensajes de conexión periódicamente
            static qint64 startCxnTimer = QDateTime::currentMSecsSinceEpoch();
            //qDebug() << " Esta intentando conectar. Se comprobaran intervalos de tiempo " ;
            if (QDateTime::currentMSecsSinceEpoch() - startCxnTimer > startCxnInterval)            {
                startCxnTimer = QDateTime::currentMSecsSinceEpoch();

                // Enviar un mensaje de conexión al EmotiBit seleccionado
                qDebug() << "enviando mensaje de conexión al EmotiBit seleccionado";
                QVector<QString> payload;
                payload.append(qEmotiBitPacket::PayloadLabel::CONTROL_PORT);
                payload.append(QString::number(controlPort));
                payload.append(qEmotiBitPacket::PayloadLabel::DATA_PORT);
                payload.append(QString::number(_dataPort));
                QString packet = qEmotiBitPacket::createPacket(
                    qEmotiBitPacket::TypeTag::EMOTIBIT_CONNECT, advertisingPacketCounter++, payload,0);
                //qDebug() << "Sent: " << packet;

                // Enviar EMOTIBIT_CONNECT a la IP conectada
                QHostAddress connectedAddress(QString::fromStdString(connectedEmotibitIp));
                QByteArray data = packet.toUtf8();
                //advertisingCxn->writeDatagram(data, connectedAddress, advertisingPort); //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                emit sendDatagram(data, connectedAddress, advertisingPort, "advertisingCxn");
            }

            // Timeout starting connection if no response is received
            if (QDateTime::currentMSecsSinceEpoch() - startCxnAbortTimer > startCxnTimeout)            {
                isStartingConnection = false;
                connectedEmotibitIp = "";
                connectedEmotibitIdentifier = "";
            }
        }

        // **** Verificar si la conexión ha expirado ****
        if (_isConnected)  {
            if (QDateTime::currentMSecsSinceEpoch() - connectionTimer > connectionTimeout)  {
                qDebug() << " Ha expirado la conexion , disconnect()" ;
                //disconnect();  //_______________llamar desde aqui a disconect genera error porque la conexion se esta ejecutando en otro hilo
                //Se desconecta la app, pero quizas la desconexion de la pulser deba ser la responsabilidad de la pulsera,
                //para que la pulsera desconecte se deberia implementar mediante ranuras y señales para que no se "enreden" los hilos
            }
        }
        // **** Verificar si la disponibilidad de EmotiBit está obsoleta o necesita purgarse ****
        discoveredEmotibitsMutex.lock();
        //qDebug() << "Se va a iterar sobre todos los dispositivos detectados";
        for (auto it = _discoveredEmotibits.begin(); it != _discoveredEmotibits.end();) {
            //qDebug() << "Dispositivo" <<  it->second.ip << "./Estado Avaiable: " << it->second.isAvailable << "./LastSeen: "<< it->second.lastSeen ;                      {
            if (QDateTime::currentMSecsSinceEpoch() - it->second.lastSeen > availabilityTimeout)            {
                it->second.isAvailable = false;
                //qDebug() << "Dispositivo " << QString::fromStdString(it->first)  << " marcado como no disponible.";
                //qDebug() << "Tiempo actual:" << QDateTime::currentMSecsSinceEpoch();
                //qDebug() << "Última vez visto:" << it->second.lastSeen;
                // qDebug() << "Tiempo transcurrido:"  << QDateTime::currentMSecsSinceEpoch() - it->second.lastSeen;
                ++it;
            }
            else  {
                ++it;  // Avanzar solo si no se elimina
                //qDebug() << "E_Tiempo actual:" << QDateTime::currentMSecsSinceEpoch();
                //qDebug() << "E_Última vez visto:" << it->second.lastSeen;
                //qDebug() << "E_Tiempo transcurrido:"  << QDateTime::currentMSecsSinceEpoch() - it->second.lastSeen;
            }
        }
        discoveredEmotibitsMutex.unlock();
        return SUCCESS;
    }
    return SUCCESS;
}
//_____________________________________________






//________________________________________________________________________________________
//________________________________________________________________________________________
//_____________________________________________HILO_____________________________________
//_______________________________________________________________________________________
//_______________________________________________________________________________________
//______________________________________updateDataThread, hilo___________________________
//_______________________________________________________________________________________

/*
 * @brief Hilo principal de procesamiento de datos entrantes.
 *
 * Esta función se ejecuta continuamente en un hilo separado mientras no se indique su detención.
 * Llama periódicamente a `updateData()` para procesar los datos recibidos y duerme el hilo
 * según el valor configurado en `dataThreadSleep`.
 *
 * En caso de producirse una excepción durante el procesamiento, se captura y muestra un aviso.
 *
 * No recibe parámetros.
 * No retorna ningún valor.
 */
void EmotiBitWiFiRoboTEA::updateDataThread() {
    qDebug() << "HILO updateDataThread comenzara a ejecutarse...";

    while (!stopDataThread) {
        try {
            updateData();  // Procesar datos
        } catch (const std::exception &e) {
            qWarning() << "Exception in updateData:" << e.what();
        }
        threadSleepFor(_wifiHostSettings.dataThreadSleep);
    }
    qDebug() << "updateDataThread has stopped.";
}

//_____________________________________________________________________________

/*
 * @brief Controla la ejecución del hilo actual.
 *
 * - Si sleepMicros > 0: El hilo se detiene durante la cantidad especificada de microsegundos.
 * - Si sleepMicros == 0: El hilo cede su ejecución, permitiendo que otros hilos se ejecuten.
 * - Si sleepMicros < 0: No realiza ninguna acción y emite una advertencia.
 *
 * @param sleepMicros Tiempo en microsegundos para dormir o acciones a realizar.
 */
void EmotiBitWiFiRoboTEA::threadSleepFor(int sleepMicros){
    if (sleepMicros < 0)    {
        // Loguear advertencia sobre valor inválido
        qWarning() << "threadSleepFor recibió un valor negativo:" << sleepMicros << ". No se realizará ninguna acción.";
    }
    else if (sleepMicros == 0)    {
        std::this_thread::yield();  // Ceder ejecución del hilo actual
    }
    else    {
        std::this_thread::sleep_for(std::chrono::microseconds(sleepMicros));
    }
}




//_____________________________________________



//________________________________________________________________________________________
//________________________________UPDATEDATA______________________________________________
//________________________________________________________________________________________
//_______________________FUNCION IMPORTANTE SE EJECUTA EN  HILO DATATHREAD________________
//________________________________________________________________________________________ getLocalIps ping REQUEST
/*
 * @brief Procesa todos los datagramas UDP pendientes recibidos en el socket de datos.
 *
 * Esta función se ejecuta en el hilo de recepción de datos. Lee cada datagrama entrante,
 * separa los paquetes utilizando el delimitador CSV definido y analiza su cabecera.
 * Si el paquete contiene una solicitud de datos (`REQUEST_DATA`), se procesa mediante `processRequestData()`.
 * También se encarga de evitar duplicados y sincronizar el puerto de envío si es necesario.
 *
 * @note Usa `dataCxnMutex` para proteger el acceso al socket UDP.
 */
void EmotiBitWiFiRoboTEA::updateData() {
    while (dataCxn->hasPendingDatagrams()) {
        QByteArray message;
        QHostAddress remoteAddress;
        quint16 remotePort;
        dataCxnMutex.lock();
        message.resize(dataCxn->pendingDatagramSize());

        //dataCxn->readDatagram(message.data(), message.size());  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        dataCxn->readDatagram(message.data(), message.size(), &remoteAddress, &remotePort);

        dataCxnMutex.unlock();

        if (!_isConnected)    {
            //qDebug() << "_isConnected =FALSE_____________________Sale de la funcion updateData";
            return;
        }

        //if (message.size() > 0)  { //_______________________________________________________________probar esta opcion
        if (!message.isEmpty()){
            QString packet;
            qEmotiBitPacket::Header header;
            size_t startChar = 0;
            size_t endChar;
            //qDebug() <<"El MENSAJE completo es______________________"<<message.data();

            char delimiter = qEmotiBitPacket::PACKET_DELIMITER_CSV.toLatin1();
            // endChar = message.find_first_of(delimiter, startChar);

            do  {
                // Buscar posición del delimitador a partir de posición actual (startChar)
                endChar = message.indexOf(delimiter, startChar);

                // Verifica si se encontró el delimitador en el mensaje
                if (endChar == string::npos)    {
                    qDebug() << "**** MENSAJE MALFORMADO **** : no se encontró el delimitador del paquete";
                } else        {
                    if (endChar == startChar) {
                        qDebug() << "**** MENSAJE VACÍO **** ";
                    }  else {

                        packet = message.mid(startChar, endChar - startChar);	// extract packet processRequestData

                        qint16 dataStartChar = qEmotiBitPacket::getHeader(packet, header);	// Obtiene, analiza la cabecera del paquete
                        if (dataStartChar == qEmotiBitPacket::MALFORMED_HEADER)  {
                            qDebug()  << "**** MENSAJE MALFORMADO **** : no header data found";
                        }  else  { // La cabecera del paquete estará bin formada
                            if (startChar == 0)  { // Este es el primer paquete del mensaje_______
                                if (_isConnected)  {
                                    // Conecta un canal para manejar la sincronización de tiempo
                                    dataCxnMutex.lock();  // Bloquear el mutex para asegurar acceso exclusivo
                                    //QHostAddress remoteAddress;
                                    //quint16 remotePort;
                                    // Obtene la dirección y puerto (host remoto)
                                    // remoteAddress=dataCxn->peerAddress();
                                    // remotePort =dataCxn->peerPort();
                                    if (remotePort != sendDataPort)     {
                                        if (remotePort == 0) {         //qWarning() << "El puerto remoto de datos no es válido:" << remotePort;
                                        }else{
                                            qDebug()  << "El puerto donde esta conectado el puerto de datos es " << sendDataPort ;
                                            qDebug()  << "El puerto remoto de datos es " << remotePort ;
                                            sendDataPort = remotePort;
                                            dataCxn->connectToHost(remoteAddress, remotePort);
                                            advertisingCxn->setSocketOption(QAbstractSocket::MulticastTtlOption, false);
                                            qDebug()  << "___Actualizado y conectado puerto datos__";

                                        }
                                    }
                                    dataCxnMutex.unlock();
                                }
                                if (header.packetNumber == receivedDataPacketNumber)    {
                                    // Saltar paquetes duplicados
                                }
                                else {
                                    // Actualizar el número de paquete recibido para rastrear futuros duplicados
                                    receivedDataPacketNumber = header.packetNumber;
                                }
                            }
                            //qDebug() << "TIPETAG_HEADER_____________"<<header.typeTag;
                            if (header.typeTag.compare(qEmotiBitPacket::TypeTag::REQUEST_DATA) == 0)   {  // Process data requests
                                processRequestData(packet, dataStartChar);
                                //qDebug()  << "Se ha rearizado una___SOLICITUD DE DATOS_____";
                            }
                            dataPackets.push_back(packet);
                            emit newDataPacket(packet);
                        }
                    }
                }
                startChar = endChar + 1;
            }while (endChar != string::npos && startChar < message.size());	// Hasta que se han procesado todos los delimitadores de paquete
        }
    }
}
//_____________________________________________



//-------------------------------------------------------------------------------------------
//________SOLCITUD DE PROCESAMIENTO DE DATOS_________________________________________________
//________Las peticiones de tiempo local realizadas por el dispositivo EmotiBit
//________sirven para la alineación de Tiempos entre Dispositivos y Host:
//________garantizar que EmotiBit y la aplicación estén utilizando una referencia de tiempo común.
//___________________________________________________________________________________________  Error al enviar el paquete RECORD_BEGIN.
/*
 * @brief Procesa una solicitud de datos recibida desde un EmotiBit.
 *
 * Analiza los elementos solicitados dentro del paquete, como `TIMESTAMP_LOCAL`,
 * y genera las respuestas correspondientes para sincronización de tiempo.
 * Finalmente, se genera un paquete `ACK` para confirmar la recepción de la solicitud.
 *
 * @param packet Cadena con el paquete recibido.
 * @param dataStartChar Índice donde empiezan los datos dentro del paquete.
 */
void EmotiBitWiFiRoboTEA::processRequestData(const QString &packet, qint16 dataStartChar){
    // Esta función procesa solicitudes de datos contenidas en un paquete recibido.
    // Parámetros:
    // - packet: El paquete de datos recibido .
    // - dataStartChar: posición en el paquete donde comienzan los datos de verdad.
    QString element;
    QString outPacket;
    do   {
        // Parsear los elementos solicitados en el paquete.
        // getPacketElement extrae siguiente elemento solicitado y actualiza posición inicio.
        dataStartChar = qEmotiBitPacket::getPacketElement(packet, element, dataStartChar);
            // Verificar si el elemento solicitado es TIMESTAMP_LOCAL.
        if (element.compare(qEmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0)       {
            QString timestampStr = getTimestampString(qEmotiBitPacket::TIMESTAMP_STRING_FORMAT); // Obtiene cadena de tiempo local en formato especificado.
            //Con el siguiente paquete el dispositivo emotibit sincroniza su reloj

            //outPacket = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::TIMESTAMP_LOCAL,controlPacketCounter++,timestampStr,0);// Crear un paquete de respuesta con el TIMESTAMP_LOCAL.________________________________
            outPacket = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::TIMESTAMP_LOCAL,controlPacketCounter++,timestampStr,1);

            //qDebug() << "___________________________________SE ENVIO REQUESTDATA____________________________________________________";
        }
        // Verificar si el elemento solicitado es TIMESTAMP_UTC.
        if (element.compare(qEmotiBitPacket::TypeTag::TIMESTAMP_UTC) == 0)     {
            // No implementado
        }
    } while (dataStartChar > 0); // Continuar procesando mientras haya más elementos en el paquete.

    // Después de procesar todos los elementos solicitados envía una confirmación (ACK).

    qEmotiBitPacket::Header header;
    qEmotiBitPacket::getHeader(packet, header);
    QVector<QString> payload;
    payload.push_back(QString::number(header.packetNumber));
    payload.push_back(header.typeTag);
    outPacket = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::ACK, dataPacketCounter++, payload);
    //qDebug()  << "Se va a enviar paquete de respuesta" << outPacket;
      QString remoteIp = QString::fromStdString(connectedEmotibitIp);
     QHostAddress remoteAddress(remoteIp);
    emit sendDatagram(outPacket.toUtf8(), remoteAddress, sendDataPort,"dataCxn");
}
//________________________




//__________________________________________________________________________
//__________________________________________________________________________
/*
 * @brief Envía un paquete de datos al EmotiBit conectado por UDP.
 * @param packet Cadena que representa el paquete a enviar.
 * @return SUCCESS si se envía correctamente, FAIL si no hay conexión.
 */
quint8 EmotiBitWiFiRoboTEA::sendData(const QString &packet) {
    if (_isConnected) {
        QMutexLocker locker(&dataCxnMutex); // Bloquea el mutex para evitar condiciones de carrera
        QByteArray data = packet.toUtf8();  // Convertir QString a QByteArray
        // Convierte connectedEmotibitIp de std::string a QString     // estas conversiones intentar eliminarlas adaptando todo a QString
        QString remoteIp = QString::fromStdString(connectedEmotibitIp);
        // Crea un objeto QHostAddress a partir de QString
        QHostAddress remoteAddress(remoteIp);
        // Envia el datagrama
        //qint64 bytesSent = dataCxn->writeDatagram(data, remoteAddress, sendDataPort);
        emit sendDatagram(data, remoteAddress, sendDataPort,"dataCxn");
        //if (bytesSent == -1) {
        //    qWarning() << "Failed to send data:" << dataCxn->errorString();
        //    return FAIL;
        //}
        return SUCCESS;
    } else {
        return FAIL;
    }
}
//________________

/*
 * @brief Genera una cadena de timestamp con microsegundos.
 * @param timestampFormat Formato del timestamp.
 * @return Cadena con el timestamp formateado.
 */
QString EmotiBitWiFiRoboTEA::getTimestampString(const QString &timestampFormat){

    // Obtener la hora actual
    auto now = QDateTime::currentDateTime();
    qint64 microseconds = (QDateTime::currentMSecsSinceEpoch() % 1000) * 1000;

    // Reemplazar %i y %f con valores personalizados
    QString tmpTimestampFormat = timestampFormat;
    tmpTimestampFormat.replace("%i", QString("%1").arg(microseconds / 1000, 3, 10, QChar('0')));
    tmpTimestampFormat.replace("%f", QString("%1").arg(microseconds, 6, 10, QChar('0')));

    // Generar el timestamp
    QString timestamp = now.toString(tmpTimestampFormat);

    qDebug() << "TimeStampFormateado_________________________________________________________:" << timestamp;
    return timestamp;

    //return QDateTime::currentDateTime().toString(format);
}
//__________________________



//__________________________________________________________________________
//__________________________________________________________________________
//________________________CONTROL___________________________________________
//__________________________________________________________________________
//__________________________________________________________________________




/*
 * @brief Envía un paquete de control por TCP al cliente conectado.
 * @param packet Cadena con el paquete a enviar.
 * @return SUCCESS si se envía correctamente, FAIL si ocurre un error.
 */
quint8 EmotiBitWiFiRoboTEA::sendControl(const QString &packet) {
    QMutexLocker locker(&controlCxnMutex); // Bloquea el mutex para evitar condiciones de carrera
    qDebug() << "Entrando en <sendControl>.";

    if (!connectedClient || !controlCxn->isListening()) {
        qWarning() << "La conexión de control no está establecida o el servidor no está escuchando.";
        return FAIL;
    }

    // Preparar los datos para enviar
    QByteArray data = packet.toUtf8();
    QString expectedClientIp = QString::fromStdString(connectedEmotibitIp);

    // Verificar si la IP del cliente conectado coincide con la esperada
    if (connectedClient->peerAddress().toString() != expectedClientIp) {
        qWarning() << "IP del cliente conectado no coincide con la IP esperada. Esperada:"
                   << expectedClientIp << "Actual:" << connectedClient->peerAddress().toString();
        return FAIL;
    }

    // Verificar el estado de la conexión
    if (connectedClient->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "El cliente no está en estado conectado.";
        return FAIL;
    }

    // Enviar los datos
    qint64 bytesWritten = connectedClient->write(data);
    if (bytesWritten == -1) {
        qWarning() << "Error al enviar datos al cliente:" << connectedClient->errorString();
        return FAIL;
    } else {
        qDebug() << "Paquete enviado a" << expectedClientIp << ":" << bytesWritten << "bytes.";
        connectedClient->flush();
        return SUCCESS;
    }
}
//__________________________________________________________________________





/*
 * @brief Convierte y devuelve los paquetes almacenados como std::string.
 * @param packets Vector donde se almacenarán los paquetes convertidos.
 */
void EmotiBitWiFiRoboTEA::readData(std::vector<std::string> &packets) {
    //_________________________________________________________________________________
    // MArgen de mejora, comencé utilizando  el tipo std::string pero para aprobechar...
    //...las ventajas de Qt terminé utilizando QStrin para muchas variables...
    //...deberia intentar dejar de utilizar std::string en todo el codigo y ahorrar en conversiones
    //---------------------------------------------------------------------------------
    std::vector<QString> temp;   //
    dataPackets.get(temp); // Obtén los datos como QString

    packets.clear(); // Limpia el vector de salida
    packets.reserve(temp.size());
    for (const auto &qstr : temp) {
        packets.push_back(qstr.toStdString()); // Convierte QString a std::string //_______________________Esto deberia simplificarlo
    }
}


//_____________________________________________________________________________
//_____________________________________________________________________________
// Método para obtener una copia de los EmotiBits descubiertos de manera segura
/**
 * @brief Devuelve una copia del mapa de dispositivos EmotiBit detectados.
 * @return Mapa con identificadores y estructuras de información de EmotiBit.
 */
unordered_map<string, qEmotiBitPacket::EmotibitInfo> EmotiBitWiFiRoboTEA::getdiscoveredEmotibits(){
    // locker que bloquea automáticamente el mutex al crearse y lo desbloquea al destruirse
    discoveredEmotibitsMutex.lock();

    //copia del mapa de EmotiBits descubiertos
    auto output = _discoveredEmotibits;

    discoveredEmotibitsMutex.unlock();

    return output;
}
//________________



//______________________________CONNECT()___________________________________
//__________________________________________________________________________
/**
 * @brief Inicia el proceso de conexión a un EmotiBit si está disponible.
 * @param deviceId Identificador del dispositivo a conectar.
 * @return SUCCESS si se inicia la conexión, FAIL si no se encuentra o no está disponible.
 */
qint8 EmotiBitWiFiRoboTEA::connect(const QString &deviceId){
    //qDebug()<< "**************Entrando en Connetc******************************()";
    if (!isStartingConnection && !_isConnected)   {
        //qDebug()<< "no esta conectado, comenzando la conexion";
        QMutexLocker locker(&discoveredEmotibitsMutex);
        auto it = _discoveredEmotibits.find(deviceId.toStdString());
        if (it == _discoveredEmotibits.end())    {
            locker.unlock();
            qWarning() << "EmotiBit" << deviceId.toStdString() << "not found";
            //qDebug()<< "No encontrado para la conexion"<< (deviceId.toStdString());
            return FAIL;
        }


        // Obtiene  IP y  estado de disponibilidad
        QString ip = it->second.ip;
        bool isAvailable = it->second.isAvailable;

        locker.unlock();

        if (!ip.isEmpty() && isAvailable)    {
            connectedEmotibitIp = ip.toStdString();
            connectedEmotibitIdentifier = deviceId;
            isStartingConnection = true;
            startCxnAbortTimer = QDateTime::currentMSecsSinceEpoch();
            //qDebug() << "Iniciando conexión con EmotiBit:" << deviceId << "IP:" << ip;

            //connectToControlPort();
        }
        else   {
            qWarning() << "EmotiBit" << deviceId << "is not available or IP is empty";
            return FAIL;
        }
    }
    return SUCCESS;
}
//______________________________




/**
 * @brief Finaliza la conexión activa con un EmotiBit.
 * @return SUCCESS si se desconecta correctamente, FAIL si no hay conexión activa.
 */
signed char EmotiBitWiFiRoboTEA::disconnect(){
    if (_isConnected && connectedClient) {
        QMutexLocker controlLocker(&controlCxnMutex);

        // Crear paquete de desconexión
        //QString packet = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::EMOTIBIT_DISCONNECT, controlPacketCounter++, "", 0);

        // Enviar paquete de desconexión al cliente conectado
        //QByteArray data = packet.toUtf8();
        //qint64 bytesWritten = connectedClient->write(data);
        //if (bytesWritten != -1) {
        //    connectedClient->flush();
        //    qDebug() << "Paquete de desconexión enviado.";
        //} else {
        //    qWarning() << "Error al enviar paquete de desconexión:" << connectedClient->errorString();
        //}

        // Cerrar la conexión
        connectedClient->disconnectFromHost();
        if (connectedClient->state() != QAbstractSocket::UnconnectedState) {
            connectedClient->waitForDisconnected();
        }

        connectedClient->deleteLater();
        connectedClient = nullptr;
        connectedEmotibitIp = "";
        connectedEmotibitIdentifier = "";
        _isConnected = false;
        isStartingConnection = false;

        qDebug() << "Desconectado del dispositivo EmotiBit.";
        return SUCCESS;
    }
    return FAIL;
}
//______________________________________



/**
 * @brief Slot para enviar datagramas desde el socket correspondiente.
 * @param data Datos a enviar.
 * @param address Dirección IP destino.
 * @param port Puerto destino.
 * @param socketType Tipo de socket: "dataCxn" o "advertisingCxn".
 */
void EmotiBitWiFiRoboTEA::onSendDatagram(const QByteArray &data, const QHostAddress &address, quint16 port, QString socketType) {
    QUdpSocket *socket = (socketType == "dataCxn") ? dataCxn : advertisingCxn;
    if (socket) {
        socket->writeDatagram(data, address, port);
    } else {
        qWarning() << "Socket no inicializado para el tipo:" << socketType;
    }
}
//______________________________________






/**
 * @brief Envia una orden al EmotiBit conectado para iniciar la grabación.
 * @return true si el paquete RECORD_BEGIN se envió correctamente.
 */
bool EmotiBitWiFiRoboTEA::startRecording() {
    // Crear un paquete de inicio de grabación
    QString timestampStr = getTimestampString(qEmotiBitPacket::TIMESTAMP_STRING_FORMAT);
    QString packet = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::RECORD_BEGIN, controlPacketCounter++,  timestampStr, 1);

    // Enviar el paquete por el canal de control
    if (sendControl(packet) == SUCCESS) {
        qDebug() << "Paquete RECORD_BEGIN enviado exitosamente para iniciar la grabación en el instante:" <<timestampStr;
        return true;
    } else {
        qDebug() << "Error al enviar el paquete RECORD_BEGIN.";
        return false;
    }
}
//______________________________________





// Detiene la grabación en el dispositivo EmotiBit
// Devuelve true si el paquete se envió con éxito, false en caso contrario
bool EmotiBitWiFiRoboTEA::stopRecording() {
    // Crear un paquete de finalización de grabación
    QString timestampStr = getTimestampString(qEmotiBitPacket::TIMESTAMP_STRING_FORMAT);
    QString packet = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::RECORD_END, controlPacketCounter++, timestampStr, 0);

    // Enviar el paquete por el canal de control
    if (sendControl(packet) == SUCCESS) {
        qDebug() << "Paquete RECORD_END enviado exitosamente para detener la grabación.";
        return true;
    } else {
        qWarning() << "Error al enviar el paquete RECORD_END.";
        return false;
    }
}
//______________________________________






// Envía datos por el canal de control solo si la IP coincide y el cliente está conectado
// @param data: datos a enviar
// @param expectedClientIp: IP esperada del cliente
void EmotiBitWiFiRoboTEA::writeControlData(const QByteArray &data, const QString &expectedClientIp) {
    QMutexLocker locker(&controlCxnMutex); // Bloquea el mutex para evitar condiciones de carrera

    if (!connectedClient) {
        qWarning() << "No hay un cliente conectado para enviar datos.";
        return;
    }

    // Verificar si la IP del cliente conectado coincide con la esperada
    QString clientIp = connectedClient->peerAddress().toString();
    if (clientIp != expectedClientIp) {
        qWarning() << "IP del cliente conectado no coincide con la IP esperada. Esperada:"
                   << expectedClientIp << "Actual:" << clientIp;
        return;
    }

    // Verificar el estado de la conexión
    if (connectedClient->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "El cliente no está en estado conectado.";
        return;
    }

    // Enviar los datos
    qint64 bytesWritten = connectedClient->write(data);
    if (bytesWritten == -1) {
        qWarning() << "Error al enviar datos al cliente:" << connectedClient->errorString();
    } else {
        qDebug() << "Paquete enviado a" << clientIp << ":" << bytesWritten << "bytes.";
        connectedClient->flush();
    }
}
//______________________________________








// Maneja nuevas conexiones entrantes del cliente EmotiBit
void EmotiBitWiFiRoboTEA::handleNewConnection() {
    while (controlCxn->hasPendingConnections()) {
        QTcpSocket* clientSocket = controlCxn->nextPendingConnection();
        if (clientSocket) {
            // Verificar si ya hay una conexión establecida
            if (connectedClient) {
                qWarning() << "Ya hay un cliente conectado. Cerrando la nueva conexión desde:"
                           << clientSocket->peerAddress().toString()
                           << ":" << clientSocket->peerPort();
                clientSocket->disconnectFromHost();
                clientSocket->deleteLater();
                continue;
            }

            // Asignar la nueva conexión
            connectedClient = clientSocket;
            qDebug() << "Nuevo cliente conectado desde:" << clientSocket->peerAddress().toString()
                     << ":" << clientSocket->peerPort();

            // Almacenar la IP del EmotiBit conectado
            connectedEmotibitIp = clientSocket->peerAddress().toString().toStdString();
            connectedEmotibitIdentifier = ""; // Asigna según tu lógica

            _isConnected = true;
            isStartingConnection = false;


        }
    }

}
//______________________________________



// Maneja la desconexión del cliente y limpia los recursos relacionados
void EmotiBitWiFiRoboTEA::handleClientDisconnected() {
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket && clientSocket == connectedClient) {
        qDebug() << "Cliente desconectado:" << clientSocket->peerAddress().toString()
                 << ":" << clientSocket->peerPort();
        connectedClient->deleteLater();
        connectedClient = nullptr;
        _isConnected = false;
        isStartingConnection = false;
        connectedEmotibitIp = "";
        connectedEmotibitIdentifier = "";
    }
}

//______________________________________




/** Envía una nota de usuario con marca de tiempo al dispositivo EmotiBit
// @param nota: texto de la nota
/ @return true si se envió correctamente, false si falló */
bool EmotiBitWiFiRoboTEA::sendNota(const QString &nota) {
    // Crear un vector para almacenar los datos

    QString timestampStr = getTimestampString(qEmotiBitPacket::TIMESTAMP_STRING_FORMAT);

    QList<QString> pp;
    pp.append(nota);
    pp.append(timestampStr);
    //QVector<QString> payload;

    // payload.append(nota);          // Agregar la nota
    // payload.append(timestampStr);     // Agregar el segundo dato (extraData)
    //payload.

    // Crear el paquete con el payload

    QString packet = qEmotiBitPacket::createPacket(qEmotiBitPacket::TypeTag::USER_NOTE, controlPacketCounter++, pp);

    // Enviar el paquete por el canal de control
    if (sendControl(packet) == SUCCESS) {
        qDebug() << "Paquete USER_NOTE enviado exitosamente para añadir Nota.";
        return true;
    } else {
        qDebug() << "Error al enviar el paquete USER_NOTE."<<packet;
        return false;
    }
}




// Detiene los hilos internos de datos y publicidad
void EmotiBitWiFiRoboTEA::stopThreads() {
    stopAdvertisingThread = true;
    stopDataThread = true;
}

// Devuelve una lista con los IDs de EmotiBits descubiertos
// @return QStringList con los IDs encontrados
QStringList EmotiBitWiFiRoboTEA::getDiscoveredEmotibitIds() const {
    QStringList deviceIds;
    for (const auto &[id, info] : _discoveredEmotibits) {
        deviceIds.append(QString::fromStdString(id)); // Solo agrega el ID
    }
    return deviceIds;
}













