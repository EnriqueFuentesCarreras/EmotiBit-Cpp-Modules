/*
 * @class FormVistaEmotiBit
 * @brief Interfaz gráfica principal para la interacción con un dispositivo EmotiBit.
 *
 * Esta clase proporciona un formulario Qt que permite:
 * - Detectar dispositivos EmotiBit disponibles en la red.
 * - Conectar y desconectar un dispositivo.
 * - Iniciar y detener la grabación de datos en el PC o en la tarjeta SD del EmotiBit.
 * - Visualizar en tiempo real los datos de sensores utilizando un widget `FormPlot`.
 * - Monitorear estado de batería, modo operativo y grabación del EmotiBit.
 * - Enviar notas o anotaciones al dispositivo.
 *
 * La clase está diseñada para actuar como puente entre el usuario y el controlador `EmotiBitController`,
 * manejando las señales y actualizaciones de estado, y proporcionando retroalimentación visual.
 *
 * @author Enrique
 * @date Abril 2025
 */


#include "FormVistaEmotiBit.h"
#include "ui_formvistaemotibit.h"
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QPushButton>
#include<QEmotiBitPacket.h>

#include "ChannelFrequencies.h"
extern ChannelFrequencies channelFrequencies;


/**
 * @brief Constructor de la clase FormVistaEmotiBit.
 *
 * Inicializa la interfaz de usuario, enlaza el widget de gráficas `FormPlot`,
 * inicializa el controlador de EmotiBit y conecta las señales y botones.
 *
 * @param parent Puntero al widget padre (por defecto nullptr).
 */
FormVistaEmotiBit::FormVistaEmotiBit(QWidget *parent) : QWidget(parent), ui(new Ui::FormVistaEmotiBit){
    ui->setupUi(this);

    // Enlazamos el widget del plot (FormPlot) si está en el .ui
    formPlot = qobject_cast<FormPlot *>(ui->widget);
    if (!formPlot) {
        qDebug() << "Error: ui->widget no es un FormPlot válido.";
    }
    ui->widget->setStyleSheet("background-color: white;");

    // Iniciamos el controlador
    controller.begin();

    // Conectar señales del UI a slots locales
    connect(ui->pushButtonDetectarPulsera, &QPushButton::clicked, this, &FormVistaEmotiBit::detectarPulsera);
    connect(ui->pushButtonConectar, &QPushButton::clicked,  this, &FormVistaEmotiBit::conectarPulsera);
    connect(ui->pushButtonDesconectar, &QPushButton::clicked,this, &FormVistaEmotiBit::desconectarPulsera);

    // Botones de grabación en archivo local
    connect(ui->pushButtonGrabar, &QPushButton::clicked, this, [=]() {
        QString filePath = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss") + ".csv";
        startRecording(filePath);
    });
    //connect(ui->pushButtonGrabar, &QPushButton::clicked, this, &FormVistaEmotiBit::startRecording);
    connect(ui->pushButtonStop, &QPushButton::clicked, this, &FormVistaEmotiBit::stopRecording);
    // Conectar señales del controlador a estos slots
    connect(&controller, &EmotiBitController::newMessage,this, &FormVistaEmotiBit::displayPacket); // Para mostrar mensajes varios   recordingLocalStateUpdated
    connect(&controller, &EmotiBitController::recordingStateUpdated,this, &FormVistaEmotiBit::updateDeviceState);
    connect(&controller, &EmotiBitController::batteryLevelUpdated, this, &FormVistaEmotiBit::updateBatteryLevel);
    connect(&controller, &EmotiBitController::deviceModeUpdated,this, &FormVistaEmotiBit::updateDeviceMode);
    connect(&controller, &EmotiBitController::sensorDataReceived,this, &FormVistaEmotiBit::onNewSensorData);
    ui->pushButtonConectar->setEnabled(false);
    ui->pushButtonDesconectar->setEnabled(false);
}
//_______________________







/**
 * @brief Destructor de la clase FormVistaEmotiBit.
 *
 * Se asegura de detener grabaciones activas, parar hilos del controlador,
 * y limpiar recursos de la interfaz de usuario.
 */
FormVistaEmotiBit::~FormVistaEmotiBit()
{
    // Si está grabando, detenemos
    if (m_isRecording) {
        stopRecording();
    }
    // Pedimos al controlador que pare hilos y desconecte si hace falta
    controller.stop();


    delete ui;
    qDebug() << "FormVistaEmotiBit destruido correctamente.";
}//_______________






// -----------------------------------------------------------
//     GRABACIÓN EN ARCHIVO LOCAL (PC)
// -----------------------------------------------------------
/**
 * @brief Inicia la grabación de datos en la tarjeta SD del EmotiBit y/o en el disco local.
 *
 * Este método se utiliza cuando no se especifica un nombre de archivo manualmente.
 */
void FormVistaEmotiBit::startRecording(){
    // Si queremos grabar en la SD
    if (ui->checkBoxGrabarEnSD->isChecked()) {
        controller.startRecordingOnSD();
        ui->textBrowserMensajes->append("Iniciada Grabacion en la SD de Emotibit");
        emit signalLogEmotiBit("Iniciada Grabacion en la SD de Emotibit");
    }
    // Llamamos al método del controller para grabación local  
    bool success = controller.startLocalRecording();
    if (success) {
        // Ajusta botones
        ui->pushButtonStop->setDisabled(false);
        ui->pushButtonGrabar->setDisabled(true);
        ui->textBrowserMensajes->append("<span style='color:green;'> Record: </span> Iniciada Grabacion en el Disco Local");
        emit signalLogEmotiBit("<span style='color:green;'> Record: </span> Iniciada Grabacion en el Disco Local");

    }
}




/**
 * @brief Inicia la grabación de datos en el disco local utilizando el nombre de archivo especificado.
 *
 * @param filePath Ruta o nombre del archivo CSV donde se guardarán los datos.
 */
void FormVistaEmotiBit::startRecording(const QString &filePath){
    // Si queremos grabar en la SD
    if (ui->checkBoxGrabarEnSD->isChecked()) {
        controller.startRecordingOnSD();
        ui->textBrowserMensajes->append("Iniciada Grabacion en la SD de Emotibit");
        emit signalLogEmotiBit("Iniciada Grabacion en la SD de Emotibit");


    }
    // Llamamos al método del controller para grabación local
    bool success = controller.startLocalRecording(filePath);
    if (success) {
        // Ajusta botones
        ui->pushButtonStop->setDisabled(false);
        ui->pushButtonGrabar->setDisabled(true);
        ui->textBrowserMensajes->append("<span style='color:green;'> Record: </span> Iniciada Grabacion en el Disco Local");
        emit signalLogEmotiBit("<span style='color:green;'> Record: </span> Iniciada Grabacion en el Disco Local");
    }
}//___________________





/**
 * @brief Detiene cualquier grabación activa, tanto en la SD del EmotiBit como en el disco local.
 */
void FormVistaEmotiBit::stopRecording(){
    // Detener grabación local
    controller.stopLocalRecording();
    ui->textBrowserMensajes->append("<span style='color:red;'> Stop:</span> Detenida la Grabacion en el Disco Local");
    emit signalLogEmotiBit("<span style='color:red;'> Stop:</span> Detenida la Grabacion en el Disco Local");
    // Detener grabación en SD
    if (ui->checkBoxGrabarEnSD->isChecked()) {
        controller.stopRecordingOnSD();
        ui->textBrowserMensajes->append("Detenida la Grabacion en la SD de Emotibit");
        emit signalLogEmotiBit("Detenida la Grabacion en la SD de Emotibit");
    }

    ui->pushButtonStop->setDisabled(true);
    ui->pushButtonGrabar->setDisabled(false);
}//____________





// -----------------------------------------------------------
//     DETECTAR / CONECTAR / DESCONECTAR PULSERA
// -----------------------------------------------------------
/**
 * @brief Inicia la búsqueda de dispositivos EmotiBit disponibles en la red.
 *
 * Actualiza el ComboBox con los dispositivos detectados y su disponibilidad.
 */
void FormVistaEmotiBit::detectarPulsera(){
    ui->textBrowserMensajes->append("Buscando dispositivos EmotiBit...");
    ui->comboBoxDispositivos->clear();;

    // Esperamos 2 seg y luego consultamos
    QTimer::singleShot(2000, this, [this]() {
       auto devices = controller.getDiscoveredDevices();

        if (devices.empty()) {
            ui->textBrowserMensajes->append("No se encontraron dispositivos EmotiBit.");
            ui->pushButtonConectar->setEnabled(false);
        } else {
            for (const auto &[id, info] : devices) {
                QString qId = QString::fromStdString(id);
                if (info.isAvailable) {
                    QString deviceInfo = QString("%1 (%2)").arg(qId).arg(info.ip);
                    ui->comboBoxDispositivos->addItem(deviceInfo);
                    ui->textBrowserMensajes->append(
                        QString("Dispositivo detectado: <span style='color:blue;'>%1</span>, IP: %2, Estado <span style='color:green;'>Disponible</span>")
                            .arg(qId).arg(info.ip)
                        );
                    ui->pushButtonConectar->setEnabled(true);
                } else {
                    ui->textBrowserMensajes->append(
                        QString("Dispositivo detectado:<span style='color:grey;'> %1</span>, IP: %2,<span style='color:red;'> NO Disponible</span>")
                            .arg(qId).arg(info.ip)  // std
                        );
                }
            }
        }
    });
}//_______________



/**
 * @brief Intenta conectar al dispositivo EmotiBit seleccionado en el ComboBox.
 *
 * Actualiza el estado de los botones y emite señales de conexión.
 */
void FormVistaEmotiBit::conectarPulsera(){
    ui->textBrowserMensajes->append("***************** Intentando conectar al dispositivo EmotiBit...");
    int selectedIndex = ui->comboBoxDispositivos->currentIndex();
    if (selectedIndex == -1) {
        ui->textBrowserMensajes->append("<span style='color:red;'>Error: No se ha seleccionado ningún dispositivo.</span>");
        return;
    }
    // Extraemos el ID verdadero, asumiendo que en el combobox está "ID (x.x.x.x)"
    QString selectedText = ui->comboBoxDispositivos->currentText();
    QString selectedDeviceId = selectedText.split(' ').first();
    // (Si deseas parsear IP u otra cosa, ajusta aquí)

    bool success = controller.connectToDevice(selectedDeviceId);
    if (success) {
        ui->textBrowserMensajes->append("Nombre del dispositivo conectado: " + selectedDeviceId);
        emit signalLogEmotiBit("Nombre del dispositivo conectado: " + selectedDeviceId);
        ui->textBrowserMensajes->append("<span style='color:green;'>Connect:</span> conexión establecida con éxito.");
        emit signalLogEmotiBit("<span style='color:green;'>Connect:</span> conexión establecida con éxito.");
        ui->pushButtonDesconectar->setEnabled(true);
        ui->pushButtonConectar->setEnabled(false);
        emit signalIDEmotiBit(selectedDeviceId);
        // Reiniciamos gráfica
        if (formPlot) {
            formPlot->resetAllData();
        }

    } else {
        ui->textBrowserMensajes->append("<span style='color:red;'>Error al conectar con el dispositivo.</span>");
        emit signalLogEmotiBit("<span style='color:red;'>Error al conectar con el dispositivo.</span>");
        ui->pushButtonDesconectar->setEnabled(false);
    }
    emit signalOnlineEmotiBit(success);
}//_______________________



/**
 * @brief Desconecta el dispositivo EmotiBit actualmente conectado.
 *
 * Actualiza el estado de la interfaz tras la desconexión.
 */
void FormVistaEmotiBit::desconectarPulsera(){

    ui->textBrowserMensajes->append("<span style='color:red;'> Disconect: </span> Desconectando del dispositivo EmotiBit...");
    emit signalLogEmotiBit("<span style='color:red;'> Disconect: </span> Desconectando del dispositivo EmotiBit...");
    bool success = controller.disconnectFromDevice();
    if (success) {
        ui->pushButtonDesconectar->setEnabled(false);
        ui->pushButtonConectar->setEnabled(true);
    }
}




// -----------------------------------------------------------
//     SLOTS LLAMADOS POR SEÑALES DEL CONTROLLER
// -----------------------------------------------------------

/**
 * @brief Muestra un paquete de datos recibido en el TextBrowser correspondiente si está activado.
 *
 * @param packet Contenido textual del paquete recibido.
 */
void FormVistaEmotiBit::displayPacket(const QString &packet){
    // Muestra el texto en textBrowserMensajes
    // o en textBrowserPaquetes (depende de tu preferencia)
    if (ui->checkBoxPaquetesRecibidos->isChecked()) {
        ui->textBrowserPaquetes->append(packet);
    }
}
//________________________




/**
 * @brief Actualiza el estado de grabación en la interfaz (grabando o no grabando en SD).
 *
 * @param isRecording Indica si la grabación en SD está activa.
 * @param fileName Nombre del archivo en la SD (si aplica).
 */
void FormVistaEmotiBit::updateDeviceState(bool isRecording, const QString &fileName){
    if (isRecording) {
        ui->label_Grabar->setText("Grabando en la tarjeta SD");
        ui->label_nombreArchivo->setText(fileName);
    } else {
        ui->label_Grabar->setText("NO grabando en la tarjeta SD");
        ui->label_nombreArchivo->clear();
    }
}


/**
 * @brief Actualiza el nivel de batería del EmotiBit mostrado en la interfaz.
 *
 * @param batteryLevel Nivel de batería en porcentaje (0-100).
 */
void FormVistaEmotiBit::updateBatteryLevel(int batteryLevel){
    ui->progressBarBatery->setValue(batteryLevel);
}//________________




/**
 * @brief Actualiza el modo operativo del dispositivo (Normal, Low Power, Hibernate, etc.).
 *
 * @param mode Código del modo actual del EmotiBit (por ejemplo: "MN", "ML", "MH").
 */
void FormVistaEmotiBit::updateDeviceMode(const QString &mode){
    // Traducir la abreviatura a algo legible
    QString modeText;
    if      (mode == "MN") modeText = "Normal Mode";
    else if (mode == "ML") modeText = "Low Power";
    else if (mode == "MM") modeText = "Medium Power";
    else if (mode == "MO") modeText = "Wireless Off";
    else if (mode == "MH") modeText = "Hibernate";
    else                   modeText = "Modo desconocido";

    ui->label_Modo->setText(modeText);
}
//____________________________


/**
 * @brief Slot que recibe nuevas muestras de sensores y las envía al FormPlot para su graficado.
 *
 * @param channelID Identificador del canal de datos (ej. "AX", "HR", "EA").
 * @param sampleTime Tiempo relativo de la muestra en segundos.
 * @param value Valor de la muestra.
 */

void FormVistaEmotiBit::onNewSensorData(const QString &channelID, double sampleTime, double value){
    // Aquí actualizamos la gráfica
    if (formPlot) {
        formPlot->onNewDataReceived(channelID, sampleTime, value);
    }
}//__________________________ reset




/**
 * @brief Envía una nota de usuario al EmotiBit con el texto introducido en la interfaz.
 */
void FormVistaEmotiBit::on_pushButtonNota_clicked(){
    // Envia una nota al dispositivo
    controller.sendNota(ui->lineEditNota->text());
    // controller.sendNoteToDevice(ui->lineEditNota->text());
}


