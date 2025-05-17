/****************************************************************************
 * EmotiBitController.cpp
 *
 * Descripción: Controlador para el dispositivo EmotiBit. Permite la
 * comunicación con el dispositivo, la grabación de datos localmente, y la
 * gestión de dispositivos EmotiBit a través de Wi-Fi.
 *
 * Autor: Enrique Fuentes Carreras
 * Fecha: 2025-04-24
 *
 * Dependencias:
 * - EmotiBitWiFiRoboTEA
 * - ChannelFrequencies
 * - qEmotiBitPacket
 ****************************************************************************/

#include "EmotiBitController.h"
#include <QDebug>
#include <QEmotiBitPacket.h>

// Define la frecuencia de los canales
#include "ChannelFrequencies.h"
extern ChannelFrequencies channelFrequencies;

EmotiBitController::EmotiBitController(QObject *parent)
    : QObject(parent) {
    // Conecta la señal de nuevos paquetes de datos
    connect(&wifiHost, &EmotiBitWiFiRoboTEA::newDataPacket,
            this, &EmotiBitController::onNewPacketReceived);
}

EmotiBitController::~EmotiBitController(){
    stop();  // Detiene hilos y desconecta
}

void EmotiBitController::begin(){
    reiniciarTiempo();
    wifiHost.parseCommSettings();
    wifiHost.begin();
}

void EmotiBitController::stop(){
    // Desconecta y detiene hilos si es necesario
    if (wifiHost._isConnected) {
        wifiHost.disconnect();
    }
    wifiHost.stopThreads();
}

void EmotiBitController::discoverDevices(){
    // Inicia la búsqueda de dispositivos, consulta después
}

QStringList EmotiBitController::getAvailableDeviceIds() const {
    return wifiHost.getDiscoveredEmotibitIds();
}

std::unordered_map<std::string, qEmotiBitPacket::EmotibitInfo> EmotiBitController::getDiscoveredDevices() {
    // Retorna el mapa de dispositivos encontrados
    return wifiHost.getdiscoveredEmotibits();
}

/**
 * Conecta al dispositivo EmotiBit con el ID proporcionado.
 *
 * @param deviceId El ID del dispositivo al que conectar.
 * @return true si la conexión fue exitosa, false en caso contrario.
 */
bool EmotiBitController::connectToDevice(const QString &deviceId){
    reiniciarTiempo();
    auto result = wifiHost.connect(deviceId);
    if (result == EmotiBitWiFiRoboTEA::SUCCESS) {
        emit newMessage(QString("Conectado con éxito a: %1").arg(deviceId));
        return true;
    } else {
        emit newMessage(QString("Error al conectar con: %1").arg(deviceId));
        return false;
    }
}

/**
 * Desconecta del dispositivo EmotiBit.
 *
 * @return true si se desconectó correctamente, false en caso contrario.
 */
bool EmotiBitController::disconnectFromDevice(){
    auto result = wifiHost.disconnect();
    if (result == EmotiBitWiFiRoboTEA::SUCCESS) {
        emit newMessage("Dispositivo desconectado.");
        return true;
    } else {
        emit newMessage("Error al desconectar.");
        return false;
    }
}

// -------------------------------------------------------------------
//      Grabación local en el PC
// -------------------------------------------------------------------

/**
 * Inicia la grabación local de datos en un archivo.
 *
 * @param filePath Ruta donde se guardará el archivo.
 * @return true si la grabación se inicia correctamente, false en caso contrario.
 */
bool EmotiBitController::startLocalRecording(const QString &filePath){
    if (m_isRecordingLocally) {
        emit newMessage("Ya grabando.");
        return false;
    }
    m_localOutputFile.setFileName(filePath);
    if (!m_localOutputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit newMessage("Error al abrir archivo.");
        return false;
    }

    m_localOutputStream.setDevice(&m_localOutputFile);
    m_isRecordingLocally = true;
    sendNota("INICIA_GRABACION");

    emit newMessage("Grabación iniciada: " + filePath);

    m_localOutputStream << "timestamp,channelID,sampleTime,value\n";
    return true;
}

/**
 * Inicia la grabación local con una ruta de archivo generada automáticamente.
 *
 * @return true si la grabación se inicia correctamente, false en caso contrario.
 */
bool EmotiBitController::startLocalRecording(){
    QString filePath = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss") + ".csv";
    return startLocalRecording(filePath);
}

/**
 * Detiene la grabación local si está en curso.
 *
 * @return true si se detuvo correctamente, false en caso contrario.
 */
bool EmotiBitController::stopLocalRecording(){
    if (!m_isRecordingLocally) {
        emit newMessage("No se está grabando.");
        return false;
    }

    if (m_localOutputFile.isOpen()) {
        m_localOutputStream.flush();
        m_localOutputFile.close();
    }
    m_isRecordingLocally = false;
    emit newMessage("Grabación detenida.");
    return true;
}

// -------------------------------------------------------------------
//      Grabación local en SD EmotiBit
// ------------------------------------------------------------------

/**
 * Inicia la grabación en la tarjeta SD del dispositivo EmotiBit.
 *
 * @return true si la grabación se inicia correctamente, false en caso contrario.
 */
bool EmotiBitController::startRecordingOnSD(){
    if (wifiHost.startRecording()) {
        emit newMessage("Grabación iniciada en tarjeta SD.");
        wifiHost.sendNota("Orden de grabación");
        return true;
    } else {
        emit newMessage("Error al iniciar grabación.");
        return false;
    }
}

/**
 * Detiene la grabación en la tarjeta SD del dispositivo EmotiBit.
 *
 * @return true si la grabación se detuvo correctamente, false en caso contrario.
 */
bool EmotiBitController::stopRecordingOnSD(){
    if (wifiHost.stopRecording()) {
        emit newMessage("Grabación detenida en tarjeta SD.");
        return true;
    } else {
        emit newMessage("Error al detener grabación.");
        return false;
    }
}

// -------------------- PARSEO DE PAQUETES --------------------

/**
 * Procesa un paquete de datos recibido y realiza las acciones correspondientes.
 *
 * @param packet El paquete de datos recibido.
 */
void EmotiBitController::onNewPacketReceived(const QString &packet)
{
    QString trimmedPacket = packet.trimmed();
    if (trimmedPacket.isEmpty()) {
        emit newMessage("Paquete vacío.");
        return;
    }

    QStringList fields = trimmedPacket.split(',');
    if (fields.size() < 7) {
        emit newMessage("Paquete con formato incorrecto.");
        return;
    }

    // Graba localmente si está en modo grabación
    if (m_isRecordingLocally && m_localOutputFile.isOpen()) {
        m_localOutputStream << packet << "\n";
    }

    QString channelID = fields[3];
    int numSamples = fields[2].toInt();

    // Procesa paquetes de estado, batería, y otros
    if (channelID == "EM") {
        processDeviceState(fields);
        emit newMessage(packet);
        return;
    }

    if (channelID == "B%") {
        processBatteryPacket(fields);
        emit newMessage(packet);
        return;
    }

    // Muestra paquete si no es relevante para el gráfico
    if (!channelFrequencies.contains(channelID) || channelID == "UN") {
        emit newMessage(packet);
        return;
    }

    // Procesa datos de sensores
    qint64 timestamp = fields[0].toLongLong();
    QStringList dataFields = fields.mid(6);
    double frequency = channelFrequencies.getFrequency(channelID);
    double dt = 1.0 / frequency;

    // Control de timestamp inicial
    if (!firstTimestampFound) {
        initialTimestamp = timestamp;
        firstTimestampFound = true;
    }

    processSensorData(channelID, timestamp, dataFields, numSamples, dt);
}

/**
 * Procesa el estado de grabación del dispositivo.
 *
 * @param fields El conjunto de campos del paquete que contiene el estado.
 */
void EmotiBitController::processDeviceState(const QStringList &fields)
{
    if (fields.size() < 9) return; // Validación

    QString recordStatus = fields[7];
    if (recordStatus == "RB") {
        // Inicia grabación
        QString fileName = fields[8];
        emit recordingStateUpdated(true, fileName);
        if (fields.size() > 9 && fields[9] == "PS") {
            if (fields.size() > 10) {
                QString mode = fields[10];
                emit deviceModeUpdated(mode);
            }
        }
    } else if (recordStatus == "RE") {
        // Detiene grabación
        emit recordingStateUpdated(false, QString());
        if (fields.size() > 8 && fields[8] == "PS") {
            if (fields.size() > 9) {
                QString mode = fields[9];
                emit deviceModeUpdated(mode);
            }
        }
    }
}

/**
 * Procesa el paquete de datos de la batería.
 *
 * @param fields El conjunto de campos del paquete que contiene el nivel de batería.
 */
void EmotiBitController::processBatteryPacket(const QStringList &fields)
{
    bool ok;
    int batteryLevel = fields[6].toInt(&ok);
    if (ok) {
        emit batteryLevelUpdated(batteryLevel);
    }
}

/**
 * Procesa los datos de los sensores y emite los valores.
 *
 * @param channelID El ID del canal de datos.
 * @param timestamp El timestamp del paquete.
 * @param dataFields Los datos del paquete.
 * @param numSamples El número de muestras en el paquete.
 * @param dt El intervalo de tiempo entre muestras.
 */
void EmotiBitController::processSensorData(const QString &channelID, qint64 timestamp, const QStringList &dataFields, int numSamples, double dt){
    if (dataFields.size() < numSamples) {
        emit newMessage("Datos insuficientes en paquete.");
        return;
    }

    double relativeTime = (timestamp - initialTimestamp) / 1000.0;

    for (int i = 0; i < numSamples; ++i) {
        bool ok;
        double value = dataFields[i].toDouble(&ok);
        if (ok) {
            double sampleTime = relativeTime - (numSamples - 1 - i) * dt;
            emit sensorDataReceived(channelID, sampleTime, value);
        } else {
            emit newMessage(QString("Dato inválido en índice %1").arg(i));
        }
    }
}

/**
 * Envía una nota al dispositivo EmotiBit.
 *
 * @param nota La nota que se enviará.
 */
void EmotiBitController::sendNota(QString nota){
    wifiHost.sendNota(nota);
}

/**
 * Reinicia el tiempo de la grabación.
 */
void EmotiBitController::reiniciarTiempo(){
    initialTimestamp = 0;
    firstTimestampFound = false;
}



