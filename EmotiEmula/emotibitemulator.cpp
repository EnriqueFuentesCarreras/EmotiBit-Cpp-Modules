#include "EmotiBitEmulator.h"
#include <QDateTime>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>

EmotiBitEmulator::EmotiBitEmulator(QObject *parent) :
    QObject(parent),
    advertisingSocket(new QUdpSocket(this)),
    dataSocket(new QUdpSocket(this)),
    controlClientSocket(nullptr),
    dataTimer(new QTimer(this)),
    isConnected(false),
    deviceId("Simulador_EmotiBit"),
    advertisingPort(3131),
    dataPort(0),    // Se establecerá luego del EMOTIBIT_CONNECT
    controlPort(0), // Se establecerá luego del EMOTIBIT_CONNECT
    packetNumber(0),
    senderPort(0)
{
    if (!setupAdvertisingSocket()) {
        emit messageLogged("Error al configurar los puertos de advertising. Emulador no iniciado.");   //pong Añadido asegur stard
        return;
    }

    // Conectar señales
    connect(advertisingSocket, &QUdpSocket::readyRead, this, &EmotiBitEmulator::readAdvertisingMessage);
    if(!csvFile.isOpen()) csvFile.open(QIODevice::ReadOnly | QIODevice::Text);
    connect(dataTimer, &QTimer::timeout, this, &EmotiBitEmulator::sendData);
}




EmotiBitEmulator::~EmotiBitEmulator()
{
    // Detener el temporizador si está activo
    if (dataTimer->isActive()) {
        dataTimer->stop();
    }

    // Cerrar el archivo CSV si está abierto
    if (csvFile.isOpen()) {
        csvFile.close();
        qDebug() << "Archivo CSV cerrado en el destructor.";
        emit messageLogged("Archivo CSV cerrado en el destructor.");
    }

    // Desconectar y destruir el socket de control si está activo
    if (controlClientSocket) {
        if (controlClientSocket->isOpen()) {
            controlClientSocket->disconnectFromHost();
            controlClientSocket->waitForDisconnected(1000); // Espera breve opcional
        }
        controlClientSocket->deleteLater();
        controlClientSocket = nullptr;
    }

    // Cerrar sockets UDP si están abiertos
    if (advertisingSocket && advertisingSocket->isOpen()) {
        advertisingSocket->close();
    }

    if (dataSocket && dataSocket->isOpen()) {
        dataSocket->close();
    }

    qDebug() << "EmotiBitEmulator destruido correctamente.";
    emit messageLogged("EmotiBitEmulator destruido correctamente.");
}









bool EmotiBitEmulator::setupAdvertisingSocket()
{
    if (!advertisingSocket->bind(advertisingPort, QUdpSocket::ShareAddress)) {
        emit messageLogged(QString("Error al vincular el socket de advertising al puerto %1: %2")
                               .arg(advertisingPort)
                               .arg(advertisingSocket->errorString()));
        return false;
    }
    emit messageLogged(QString("Socket de advertising vinculado al puerto %1").arg(advertisingPort));
    return true;
}

void EmotiBitEmulator::setCsvFile(const QString &filePath)
{
    csvFile.setFileName(filePath);
}

void EmotiBitEmulator::startEmulation(){

        if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit messageLogged("Error al abrir el archivo CSV.");
            qDebug() << "Error al abrir el archivo CSV.";
            return;
        }
        csvStream.setDevice(&csvFile);
        emit messageLogged("_________________________Emulación iniciada.");
        qDebug() << "_________________________Emulación iniciada.";

        // Iniciar el temporizador para medir el tiempo transcurrido
        elapsedTimer.start();

        // Log tamaño del archivo
        qDebug() << "CSV file opened. Size:" << csvFile.size();
        emit messageLogged(QString("CSV file opened. Size: %1 bytes").arg(csvFile.size()));

        // Leer y loguear las primeras líneas para verificar contenido
        if (csvFile.size() > 0) {
            for(int i = 0; i < 5; ++i) { // Leer las primeras 5 líneas
                QString line = csvStream.readLine();
                if (line.isNull()) break;
                qDebug() << "Line " << i+1 << ": " << line;
                emit messageLogged(QString("Line %1: %2").arg(i+1).arg(line));
            }
            // Resetear el puntero de lectura al inicio
            csvStream.seek(0);
            qDebug() << "csvStream position after seek: " << csvStream.pos();
        }
    }



void EmotiBitEmulator::stopEmulation()
{
    dataTimer->stop();
    //if (csvFile.isOpen()) {
    //    csvFile.close();
    //}
    emit messageLogged("_________________________Emulación detenida.");
    qDebug() << "_________________________Emulación detenida.";
}

void EmotiBitEmulator::readAdvertisingMessage()
{
    qDebug() << "readAdvertisingMessage()";

    while (advertisingSocket->hasPendingDatagrams()) {
        qDebug() << "Hay paquetes de publicidad pendientes de lectura";
        QByteArray datagram;
        datagram.resize(int(advertisingSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 port;

        advertisingSocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);

        QString message = QString::fromUtf8(datagram);
        emit messageLogged("Recibido: " + message);
        qDebug() << "Recibido: " << message;

        QStringList packetFields = message.split(',');
        if (packetFields.size() < 4) {
            emit messageLogged("Formato del mensaje no válido.");
            qDebug() << "Formato del mensaje no válido.";
            continue;
        }

        QString typeTag = packetFields.at(3);
        emit messageLogged("Tipo de mensaje recibido: " + typeTag);
        qDebug() << "Tipo de mensaje recibido: " << typeTag;

        if (typeTag == "HE") { // HELLO_EMOTIBIT
            senderAddress = sender;
            senderPort = port;

            sendHelloHostMessage(senderAddress, senderPort);

            hostIp = senderAddress;
            hostAddress = senderAddress;

            // No enviar PONG aún. Esperaremos a recibir EMOTIBIT_CONNECT (EC)

        } else if (typeTag == "EC") { // EMOTIBIT_CONNECT
            emit messageLogged("EMOTIBIT_CONNECT entrando");
            qDebug() << "Tipo de mensaje recibido: " << typeTag;

            // Se espera el formato:
            // timestamp,packetNumber,0,EC,1,100,CP,<controlPort>,DP,<dataPort>;
            if (packetFields.size() >= 10) {
                QString cp = packetFields.at(6);
                QString dp = packetFields.at(8);
                if (cp == "CP" && dp == "DP") {
                    emit messageLogged("EMOTIBIT_CONNECT contiene etiquetas CP y DP, se creará conexión");
                    qDebug() << "EMOTIBIT_CONNECT contiene etiquetas CP y DP, se creará conexión.";

                    // Obtener puertos
                    controlPort = packetFields.at(7).toUShort();
                    dataPort = packetFields.at(9).toUShort();

                    qDebug() << "ControlPort establecido a " << controlPort;
                    qDebug() << "DataPort establecido a " << dataPort;

                    // Conectar al host con los puertos recibidos
                    connectHost(controlPort, dataPort);

                    // Establecer el archvo CSV después de conectar
                    //setCsvFile("D://emuladorEmotiBit8//2024-11-10_13-18-28-299019.csv");

                    setCsvFile("muestras.csv");

                    // Iniciar la emulación
                    startEmulation();
                } else {
                    emit messageLogged("EMOTIBIT_CONNECT mensaje mal formado. No se encontraron CP, DP en las posiciones esperadas.");
                    qDebug() << "EMOTIBIT_CONNECT mensaje mal formado. CP y DP no encontrados correctamente.";
                }
            } else {
                emit messageLogged("EMOTIBIT_CONNECT mensaje mal formado. Campos insuficientes.");
                qDebug() << "EMOTIBIT_CONNECT mensaje mal formado. Campos insuficientes.";
            }

        } else if (typeTag == "PN") { // PING
            emit messageLogged("Recibido PING (PN).");
            qDebug() << "Recibido PING (PN).";
            // Enviar PONG en respuesta
            sendPongMessage();
        } else {
            emit messageLogged("Mensaje no reconocido: " + typeTag);
            qDebug() << "Mensaje no reconocido: " << typeTag;
        }
    }
}





void EmotiBitEmulator::connectHost(quint16 controlPort, quint16 dataPort)
{
    qDebug() << "connectHost(controlPort, dataPort): " << controlPort << ", " << dataPort;

    // Verificar que dataPort sea válido
    if (dataPort <= 0 || dataPort > 65535) {
        emit messageLogged("Error: dataPort inválido: " + QString::number(dataPort));
        qDebug() << "Error: dataPort inválido: " << dataPort;
        return;
    }

    // Cerrar y limpiar cualquier conexión anterior
    if (controlClientSocket) {
        controlClientSocket->disconnectFromHost();
        controlClientSocket->deleteLater();
        controlClientSocket = nullptr;
    }

    // Crear el socket TCP para el puerto de control
    controlClientSocket = new QTcpSocket(this);

    // Conectar señales del socket TCP
    connect(controlClientSocket, &QTcpSocket::connected, this, [this]() {
        emit messageLogged("Conexión de control establecida con el Host.");
        qDebug() << " Conexión de control establecida con el Host.";
        isConnected = true;

        // Enviar PONG después de establecer la conexión
        sendPongMessage();

        // Iniciar el envío de datos
        dataTimer->start(5);     //________________________________________________________________________________________
    });

    connect(controlClientSocket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = controlClientSocket->readAll();
        emit messageLogged("Datos recibidos en el canal de control: " + QString::fromUtf8(data));
        qDebug() << "Datos recibidos en el canal de control:" << QString::fromUtf8(data);


    });

    connect(controlClientSocket, &QTcpSocket::disconnected, this, [this]() {
        emit messageLogged("Conexión de control cerrada por el Oscilloscope.");
        qDebug() << "Conexión de control cerrada por el Oscilloscope.";
        isConnected = false;
        dataTimer->stop();
    });

    // Conectar al puerto de control usando TCP
    controlClientSocket->connectToHost(senderAddress, controlPort);
    qDebug() << "Intentando conectar al puerto de control:" << controlPort;

    if (!controlClientSocket->waitForConnected(5000)) {
        emit messageLogged("Error al conectar al puerto de control: " + controlClientSocket->errorString());
        qDebug() << "Error al conectar al puerto de control: " << controlClientSocket->errorString();
        isConnected = false;
        controlClientSocket->deleteLater();
        controlClientSocket = nullptr;
        return;
    }


    // Configurar el socket de datos para escuchar en el puerto especificado
    if (!dataSocket->bind(dataPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit messageLogged(QString("Error al vincular el socket de datos al puerto %1: %2")
                               .arg(dataPort)
                               .arg(dataSocket->errorString()));
        qDebug() << "Error al vincular el socket de datos al puerto:" << dataPort;
        return;
    }

    // Conectar la señal readyRead al método que procesará los mensajes en el puerto de datos
    connect(dataSocket, &QUdpSocket::readyRead, this, &EmotiBitEmulator::readDataMessage);

    emit messageLogged("Preparado para enviar y recibir datos en el puerto de datos: " + QString::number(dataPort));
    qDebug() << "Preparado para enviar y recibir datos en el puerto de datos:" << dataPort;

    this->dataPort = dataPort;
}


QString EmotiBitEmulator::formatPacket(const QString &payload)
{
    // Dividir el payload en campos separados por comas
    QStringList fields = payload.split(',');

    if (fields.size() < 7) {
        emit messageLogged("Formato del payload no válido para formatear el paquete.");
        qDebug() << "Formato del payload no válido para formatear el paquete.";
        return "";
    }

    fields[0] = QString::number(QDateTime::currentMSecsSinceEpoch()); //______________________
    // Reensamblar el paquete con el nuevo timestamp
    QString formattedPacket = fields.join(',').trimmed();

    // Asegurarse de que el paquete termine con ';'
    if (!formattedPacket.endsWith('\n')) {
        formattedPacket += '\n';
    }

    return formattedPacket;
}




void EmotiBitEmulator::sendHelloHostMessage(const QHostAddress &sender, quint16 senderPort){
    QString localIp = getLocalIpAddress();

    packetNumber++;

    // Enviar HELLO_HOST con DP,-1
    QString response = QString("%1,%2,0,HH,1,100,DP,-1,DI,%3\n")
                           .arg(QDateTime::currentMSecsSinceEpoch()) // Puedes mantener este timestamp absoluto si es necesario
                           .arg(packetNumber)
                           .arg(deviceId);

    emit messageLogged("LocalIP: " + localIp);
    qDebug() << "LocalIP: " << localIp;

    qint64 bytesSent = advertisingSocket->writeDatagram(response.toUtf8(), sender, senderPort);
    if (bytesSent == -1) {
        emit messageLogged("Error al enviar HELLO_HOST: " + advertisingSocket->errorString());
        qDebug() << "Error al enviar HELLO_HOST:" << advertisingSocket->errorString();
    } else {
        emit messageLogged(QString("Enviado HELLO_HOST: %1 al Host: %2 Puerto: %3")
                               .arg(response.trimmed())
                               .arg(sender.toString())
                               .arg(senderPort));
        qDebug() << QString("Enviado HELLO_HOST: %1 al Host: %2 Puerto: %3")
                        .arg(response.trimmed())
                        .arg(sender.toString())
                        .arg(senderPort);
    }
}




QString EmotiBitEmulator::getLocalIpAddress()
{
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : ipAddressesList) {
        if (address != QHostAddress::LocalHost && address.protocol() == QAbstractSocket::IPv4Protocol) {
            return address.toString();
        }
    }
    return "0.0.0.0";
}

void EmotiBitEmulator::sendPongMessage()
{
    // Generar un timestamp relativo en milisegundos desde el inicio de la emulación
    uint32_t relativeTimestamp = static_cast<uint32_t>(elapsedTimer.elapsed());

    packetNumber++;

    // Crear mensaje PONG
    QString pongMessage = QString("%1,%2,0,PO,1,100,DP,%3\n")
                              .arg(relativeTimestamp)
                              .arg(packetNumber)
                              .arg(dataPort);

    qint64 bytesSent = advertisingSocket->writeDatagram(pongMessage.toUtf8(), senderAddress, senderPort);
    if (bytesSent == -1) {
        emit messageLogged("Error al enviar PONG: " + advertisingSocket->errorString());
        qDebug() << "Error al enviar PONG:" << advertisingSocket->errorString();
    } else {
        emit messageLogged(QString("Enviado PONG: %1 al Host: %2 Puerto: %3")
                               .arg(pongMessage.trimmed())
                               .arg(senderAddress.toString())
                               .arg(senderPort));
        qDebug() << QString("Enviado PONG: %1 al Host: %2 Puerto: %3")
                        .arg(pongMessage.trimmed())
                        .arg(senderAddress.toString())
                        .arg(senderPort);
    }
}






void EmotiBitEmulator::sendData()
{
    if (!csvStream.atEnd()) {
        QString line = csvStream.readLine().trimmed();

        if (line.isEmpty()) {
            emit messageLogged("Línea vacía en el archivo CSV.");
            qDebug() << "Línea vacía en el archivo CSV.";
            return;
        }

        // Calcular el timestamp sincronizado
        qint64 currentLocalTime = QDateTime::currentMSecsSinceEpoch();
        qint64 syncedTimestamp = currentLocalTime - localTimestampOffset;

        // Formatear el paquete reemplazando el timestamp con el sincronizado
        QStringList fields = line.split(',');
        if (!fields.isEmpty()) {
            fields[0] = QString::number(syncedTimestamp);
        }

        QString formattedPacket = fields.join(',').trimmed();
        if (!formattedPacket.endsWith('\n')) {
            formattedPacket += '\n';
        }

        QByteArray dataPacket = formattedPacket.toUtf8();

        // Verificar que senderAddress y dataPort están correctamente establecidos
        if (senderAddress.isNull() || dataPort <= 0 || dataPort > 65535) {
            emit messageLogged("Error al enviar datos: senderAddress o dataPort no válidos.");
            qDebug() << "Error al enviar datos: senderAddress o dataPort no válidos.";
            return;
        }

        // Enviar los datos
        qint64 bytesSent = dataSocket->writeDatagram(dataPacket, senderAddress, dataPort);
        if (bytesSent == -1) {
            emit messageLogged("Error al enviar datos: " + dataSocket->errorString());
            qDebug() << "Error al enviar datos:" << dataSocket->errorString();
        } else {
            emit messageLogged("Paquete de datos enviado: " + formattedPacket);
            qDebug() << "Paquete de datos enviado:" << formattedPacket;
        }
    } else {
        dataTimer->stop();
        if (csvFile.isOpen()) {
            csvFile.close();
        }
        emit messageLogged("Transmisión de datos completada.");
        qDebug() << "Transmisión de datos completada.";
    }
}




void EmotiBitEmulator::readDataMessage()
{
    while (dataSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(dataSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;

        dataSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString message = QString::fromUtf8(datagram);
        emit messageLogged("Mensaje recibido en el puerto de datos: " + message);
        qDebug() << "Mensaje recibido en el puerto de datos:" << message;

        // Extraer el timestamp del mensaje (asumiendo que está en el primer campo)
        QStringList fields = message.split(',');
        if (!fields.isEmpty()) {
            bool ok;
            qint64 receivedTimestamp = fields[0].toLongLong(&ok); // Timestamp en el mensaje del host
            if (ok) {
                hostTimestamp = receivedTimestamp; // Actualizar la variable global
                localTimestampOffset = QDateTime::currentMSecsSinceEpoch() - hostTimestamp;
                emit messageLogged(QString("Timestamp sincronizado con el host: %1").arg(hostTimestamp));
                qDebug() << "Timestamp sincronizado con el host:" << hostTimestamp;
            }
        }
    }
}



