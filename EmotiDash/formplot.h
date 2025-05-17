/**
 * @file FormPlot.h
 * @brief Clase que mediante un Form que contiene e integra todas las vistas de la aplicación.
 *
 * El codigo esta encapsulado de forma que unicamente se necesita recibir señales para que se grafiquen los datos
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

#pragma once
#include <QWidget>
#include <QTimer>
#include <QMap>
#include <QColor>
#include <QQueue>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

QT_BEGIN_NAMESPACE
namespace Ui { class FormPlot; }
QT_END_NAMESPACE


/**
 * @brief Constructor de la clase FormPlot.Esta clase escucha señales de FormVistaEmotiBit.
 * @param parent Puntero al widget padre.
 *
 * @see FormVistaEmotiBit
 */
class FormPlot : public QWidget
{
    Q_OBJECT
public:
    explicit FormPlot(QWidget *parent = nullptr);
    ~FormPlot();

    /**
     * @brief Recibe nuevos datos para visualizarlos en los gráficos.
     * @param channelID Identificador del canal de datos.
     * @param t Marca de tiempo del dato recibido.
     * @param y Valor numérico recibido.
     */
    void onNewDataReceived(const QString &channelID,
                           double         t,
                           double         y);

    /**
     * @brief Limpia y reinicia todos los gráficos y datos.
     */
    void resetAllData();


private:
    /* ----- tipos auxiliares ---------------------------------------- */
    struct ChannelInfo {
        QString channelID;
        QString plotName;   // widget placeholder en el .ui
        QColor  color;
        QString label;
    };


    QTimer *scrollTimer {nullptr};   //  +  un timer dedicado
     double _t0 = -1.0;
    /*  en PlotCache añadimos la fracción de píxel pendiente */



    struct PlotCache
    {
        QValueAxis *axX{}, *axY{};
        bool   dirty{false};
        bool   xInit{false};          // NUEVO  ← ¿ya se fijó el rango?
        double lastT{0.0};
        double smoothMinY{0.0}, smoothMaxY{0.0};
        double fracPixPend{0.0};
    };

    struct PlotBuffer {
        /* cola de puntos para no crecer sin límite */
        QQueue<QPointF> data;
    };


    /* ----- helpers ------------------------------------------------- */
    void setupCharts();        // crea charts al arrancar
    void refreshCharts();      // timer → repinta + re‑escala Y

    /* ----- miembros ------------------------------------------------ */
    Ui::FormPlot *ui{};
    QTimer *replotTimer{nullptr};
    double currentTime = 0.0;
    double   smoothDt    = 0.0;
    QVector<ChannelInfo> channelInfos;

    void advanceTimeWindow();

    /* mapas de acceso O(1) */
    QMap<QString, QChartView*>  chartViewMap; // plotName → view
    QMap<QString, QLineSeries*> seriesMap;    // channelID → serie
    QMap<QString, QString>                channel2plot; // canal → plotName
    QMap<QString, PlotCache>              plotCache;    // plotName → cache
    QMap<QString, PlotBuffer>             buffers;      // canal → buffer

    double windowSize{10.0};   // segundos mostrados en X
};
//scrollTimer
