/**
 * @file FormVistaEmotiBit.h
 * @brief Form que contiene e integra todas las vistas de la aplicación.
 *
 * El Form sirve como contenedor principal para la interfaz gráfica,
 * integrando la vista especializada para visualizar y gestionar dispositivos EmotiBit.
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

#ifndef FORMVISTAEMOTIBIT_H
#define FORMVISTAEMOTIBIT_H

#include <QWidget>

#include <QTextStream>
#include "EmotiBitController.h"
#include "FormPlot.h"

namespace Ui {
class FormVistaEmotiBit;
}



/**
 * @class FormVistaEmotiBit
 * @brief Clase principal para gestionar la interfaz gráfica del Emotibit.
 *
 * Esta clase utiliza directamente las clases:
 * - EmotiBitWiFiRoboTEA (gestión de comunicación)
 * - FormPlot (gráficas de visualización)
 *
 * @see EmotiBitWiFiRoboTEA
 * @see FormPlot
 * @see EmotiBitController
 */
class FormVistaEmotiBit : public QWidget
{
    Q_OBJECT



public:
    /**
     * @brief Constructor de VistaEmotibit.
     * @param parent Widget padre.
     * @param emotiBit Puntero al objeto EmotiBitWiFiRoboTEA.
     * @param plotter Puntero al objeto FormPlot.
     */
    explicit FormVistaEmotiBit(QWidget *parent = nullptr);
    ~FormVistaEmotiBit();

       EmotiBitController* getController() { return &controller; }



   public slots:
    /**
     * @brief Inicia la grabación en el dispositivo.
     *
     * @see startRecording(const QString &)
     */
       void startRecording();
     /**
     * @brief Detiene la grabación activa.
     */
       void stopRecording();
     /**
     * @brief Inicia la grabación especificando una ruta de almacenamiento.
     * @param filePath Ruta del archivo de destino.
     *
     * @see startRecording()
     */
       void startRecording(const QString &filePath);


private slots:
       /**
     * @brief Inicia la detección de dispositivos EmotiBit en la red.
     */
    void detectarPulsera();

    /**
     * @brief Conecta con el EmotiBit seleccionado.
     */
    void conectarPulsera();
    /**
     * @brief Desconecta del EmotiBit actualmente conectado.
     */
    void desconectarPulsera();

    //void startRecording(const QString &filePath);


    // Slots llamados desde las señales del controlador
    void displayPacket(const QString &packet);                   // newMessage
    void updateDeviceState(bool isRecording, const QString &fileName);
    void updateBatteryLevel(int batteryLevel);
    void updateDeviceMode(const QString &mode);
    void onNewSensorData(const QString &channelID, double sampleTime, double value);

    // Slot para enviar una nota
    void on_pushButtonNota_clicked();


private:
    Ui::FormVistaEmotiBit *ui;
    FormPlot *formPlot = nullptr;

    EmotiBitController controller;  // Instancia del controlador

    // Variables para grabación en archivo local
    bool m_isRecording = false;  
    // Tiempos para etiquetar los datos, si deseas
    bool firstTimestampFound = false;
    qint64 initialTimestamp = 0;
signals:
    void signalOnlineEmotiBit(bool online);
    void signalIDEmotiBit(QString id);
    void signalLogEmotiBit(QString log);
};

#endif
