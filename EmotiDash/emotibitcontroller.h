/**
 * @file emotibitcontroller.h
 * @brief Controlador para la gestión de dispositivos EmotiBit mediante WiFi.
 *

 * Esta clase maneja todas las funciones relacionadas con la comunicación, descubrimiento, conexión,
 * recepción de datos y grabación con dispositivos EmotiBit. Proporciona interfaces simples para
 * gestionar conexiones y procesar datos recibidos.
 *
* @author Enrique
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
#ifndef EMOTIBITCONTROLLER_H
#define EMOTIBITCONTROLLER_H

#define WIN32_LEAN_AND_MEAN     // Evita incluir gran parte de headers Windows
#define NOGDI                   // Evita GDI, si no lo necesitas
#include <QFile>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QTimer>
#include <utility>
#include<channelfrequencies.h>
#include<QEmotiBitPacket.h>
#include "EmotiBitWiFiRoboTEA.h"


/**
 * @class EmotiBitController
 * @brief Gestiona dispositivos EmotiBit a través de WiFi.
 *
 * Proporciona métodos para inicializar la conexión, descubrir dispositivos disponibles,
 * manejar conexiones, controlar grabaciones locales y remotas, y procesar datos recibidos.
 *
 * @see EmotiBitWiFiRoboTEA
 * @see FormVistaEmotiBit
 * @see FormPlot
 */
class EmotiBitController : public QObject
{
    Q_OBJECT

public:
    explicit EmotiBitController(QObject *parent = nullptr);
    ~EmotiBitController();


     ChannelFrequencies channelFrequencies; // Asegúrate de que esta clase esté definida
    // Métodos para interacción con la pulsera:
    void begin();  // Inicializa wifiHost
    void stop();   // Detiene hilos, etc.

    // Descubrir dispositivos
    void discoverDevices();  // Lanza búsqueda de EmotiBits
    QStringList getAvailableDeviceIds() const; // Devuelve IDs disponibles
    std::unordered_map<std::string, qEmotiBitPacket::EmotibitInfo> getDiscoveredDevices() ; // Opción con info

    // Conectar / Desconectar
    bool connectToDevice(const QString &deviceId);
    bool disconnectFromDevice();

    // Grabación en la SD del EmotiBit
    bool startRecordingOnSD();
    bool stopRecordingOnSD();
    void sendNota(QString nota);
    bool startLocalRecording(const QString &filePath);
    bool stopLocalRecording();
    bool startLocalRecording();
    void reiniciarTiempo( );

signals:
    // Emite un mensaje genérico (por ejemplo, texto para mostrar en la interfaz).
    void newMessage(const QString &message);

    // Señales de actualización de estado del dispositivo
    void recordingStateUpdated(bool isRecording, const QString &fileName);
    void batteryLevelUpdated(int batteryLevel);
    void deviceModeUpdated(const QString &mode);

    // Señal con datos de sensores (para graficar)
    void sensorDataReceived(const QString &channelID, double sampleTime, double value);

    // (Opcional) señal cuando se descubren dispositivos
    void devicesDiscovered(const QStringList &deviceIds);


//public slots:
    // Slot intermedios que se conectan directamente desde otras clases vía señales
//    void onRequestStartRecording(const QString &filePath);
//    void onRequestStopRecording();

private slots:
    // Slot que recibe paquetes en bruto desde wifiHost.
    void onNewPacketReceived(const QString &packet);

private:
    // Procesa la parte de estado del dispositivo (EM,...).
    void processDeviceState(const QStringList &fields);

    // Procesa datos de batería u otros especiales.
    void processBatteryPacket(const QStringList &fields);

    // Procesa los datos de sensor y emite la señal sensorDataReceived(...) por cada muestra.
    void processSensorData(const QString &channelID, qint64 timestamp,
                           const QStringList &dataFields, int numSamples, double dt);



private:
    EmotiBitWiFiRoboTEA wifiHost;
    bool firstTimestampFound = false;
    qint64 initialTimestamp = 0;

    //control grabacion
    bool m_isRecordingLocally = false;
    QFile m_localOutputFile;
    QTextStream m_localOutputStream;

};

#endif // EMOTIBITCONTROLLER_H
