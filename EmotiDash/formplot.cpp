#include "formplot.h"
#include "ui_formplot.h"

#include <limits>
#include <cmath>
#include <QBoxLayout>
#include <QDateTime>



// ---------------------------------------------------------------
/*
 * Constructor de la clase FormPlot
 * Inicializa la interfaz de usuario, crea los gráficos y configura los temporizadores de refresco.
 *
 *
 *
 * @param parent Puntero al widget padre (por defecto nullptr).
 */
FormPlot::FormPlot(QWidget *parent)
    : QWidget(parent), ui(new Ui::FormPlot)
{
    ui->setupUi(this);

    //------------- tabla de canales -----------------------------
    channelInfos = {
        {"AX","customPlotAXAYAZ", Qt::darkCyan,   "ACC:X"},
        {"AY","customPlotAXAYAZ", Qt::darkGray,   "ACC:Y"},
        {"AZ","customPlotAXAYAZ", Qt::blue,       "ACC:Z"},
        {"GX","customPlotGXGYGZ", Qt::magenta,    "GYRO:X"},
        {"GY","customPlotGXGYGZ", Qt::darkMagenta,"GYRO:Y"},
        {"GZ","customPlotGXGYGZ", Qt::gray,       "GYRO:Z"},
        {"MX","customPlotMXMYMZ", Qt::darkRed,    "MAG:X"},
        {"MY","customPlotMXMYMZ", Qt::darkGray,   "MAG:Y"},
        {"MZ","customPlotMXMYMZ", Qt::blue,       "MAG:Z"},
        {"T1","customPlotT1THT0", Qt::black,      "TEMP1"},
        {"TH","customPlotT1THT0", Qt::darkRed,    "THERM"},
        {"HR","customPlotHR"    , Qt::red,        "HR"},
        //{"SR","customPlotSR"    , Qt::blue,       "SCR:RIS"},
        {"SF","customPlotSF"    , Qt::darkCyan,   "SCR:FREQ"},
         {"EL","customPlotSR"    , Qt::blue,   "EDLevel"},
        {"EA","customPlotEA"    , Qt::darkBlue,   "EDA"},
       // {"SA","customPlotSA"    , Qt::darkMagenta,"SCR:AMP"},
         {"BI","customPlotSA"    , Qt::darkMagenta,"Heart Inter-beat Interval"},
        {"PI","customPlotPI"    , Qt::darkRed,    "PPG:IR"},
        {"PR","customPlotPR"    , Qt::red,        "PPG:RED"},
        {"PG","customPlotPG"    , Qt::darkGreen,  "PPG:GREEN"}
    };

    setupCharts();




    currentTime  = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    smoothDt    = 0.0;
    //   scrollTimer  = new QTimer(this);
    //   connect(scrollTimer, &QTimer::timeout,this,   &FormPlot::advanceTimeWindow);   scrollTimer->start(30);

    // timer para repintar
    replotTimer = new QTimer(this);
    connect(replotTimer, &QTimer::timeout,
            this,        &FormPlot::refreshCharts);
    replotTimer->start(100);              // ms
}


//________________________________________________________
/*
 * Destructor de la clase FormPlot
 * Libera los recursos asociados a la interfaz y temporizadores.
 */
FormPlot::~FormPlot()
{
    if (replotTimer) replotTimer->stop();
    delete ui;
}

// ----------------------------------------------------------------
// ----  crea cada QChart y lo incrusta en el placeholder del .ui
// ----------------------------------------------------------------
/*
 * setupCharts
 * Crea y configura los gráficos (QCharts), ejes y leyendas para cada conjunto de canales.
 * Asocia las series de datos a sus respectivos ejes y contenedores visuales.
 */
void FormPlot::setupCharts(){
    // agrupar canales por plotName
    QMap<QString, QList<ChannelInfo>> plotChannels;
    for (const auto &info : channelInfos)
        plotChannels[info.plotName].append(info);



    for (auto it = plotChannels.cbegin(); it != plotChannels.cend(); ++it)  {
        const QString plotName = it.key();
        const auto   &chs      = it.value();

        QWidget *placeholder = findChild<QWidget *>(plotName);
        if (!placeholder) continue;

        // ---- chart ----
        QChart *chart = new QChart;
        chart->setBackgroundVisible(false);

        chart->legend()->setLayoutDirection(Qt::LeftToRight);
        chart->legend()->setMaximumHeight(100);  // o lo que necesites
        chart->legend()->setContentsMargins(0, 0, 0, 0);

        QLegend *legend = chart->legend();
        legend->setVisible(true);
        legend->setAlignment(Qt::AlignLeft);
        legend->setMarkerShape(QLegend::MarkerShapeFromSeries);
        legend->setFont(QFont("Arial", 6));
        legend->setContentsMargins(0, 0, 0, 0);
        legend->setBackgroundVisible(false);
        legend->setBrush(Qt::NoBrush);

        // eje X (tiempo)
        auto *axX = new QValueAxis;
        axX->setRange(0.0, windowSize);
        axX->setVisible(false);
        chart->addAxis(axX, Qt::AlignBottom);

        // eje Y
        auto *axY = new QValueAxis;
        axY->setTickCount(3);
        axY->setLabelFormat("%.2f");

        axY->setLabelsVisible(true);
        chart->addAxis(axY, Qt::AlignRight);
        chart->setMargins(QMargins(0, 0, 0, 0));  // márgenes
        axY->setLineVisible(true);
        QFont axisFont("Arial", 6);
        axY->setLabelsFont(axisFont);
        axY->setTickCount(3);  // 3 valores: min, medio, max
        plotCache.insert(plotName,   { axX, axY,false, false, 0.0, 0.0,  0.0 });

        // series por canal
        for (const auto &info : chs)
        {
            auto *series = new QLineSeries;
            series->setName(info.channelID);
            series->setColor(info.color);
            chart->addSeries(series);
            series->attachAxis(axX);
            series->attachAxis(axY);
            channel2plot.insert(info.channelID, plotName);
            seriesMap.insert(info.channelID, series);
        }

        // chart view
        auto *view = new QChartView(chart, placeholder);
        view->setRenderHint(QPainter::Antialiasing);
        view->setGeometry(placeholder->rect());
        view->setMinimumHeight(80);
        view->setMaximumHeight(80);
        view->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

        // Establece layout si no existe
        if (!placeholder->layout()) {
            auto *layout = new QVBoxLayout(placeholder);
            layout->setContentsMargins(0, 0, 0, 0);
        } else {
            placeholder->layout()->addWidget(view);
        }
        view->show();
        chartViewMap.insert(plotName, view);
    }
}




//________________________________________________________
/*
 * onNewDataReceived
 * Recibe nuevos datos y actualiza la serie y el buffer del canal correspondiente.
 * También marca la gráfica como "sucia" para recalcular los ejes.
 *
 * @param channelID Identificador del canal (ej. "AX", "HR", etc.)
 * @param t Tiempo en segundos
 * @param y Valor del dato
 */
void FormPlot::onNewDataReceived(const QString &channelID,
                                 double         t,
                                 double         y)
{
    constexpr int MAX_SAMPLES  = 1000;
    constexpr double windowSize = 10.0;       // segundos visibles
    const double  tMin = t - windowSize;

    // --- serie y buffer ---
    auto *series = seriesMap.value(channelID, nullptr);
    if (!series) return;

    auto &buf = buffers[channelID];
    buf.data.append({t, y});
    while (!buf.data.isEmpty() && buf.data.first().x() < tMin)
        buf.data.removeFirst();
    if (buf.data.size() > MAX_SAMPLES)
        buf.data.remove(0, buf.data.size() - MAX_SAMPLES);

    QVector<QPointF> v = buf.data;
    if (!v.isEmpty() && t - v.last().x() > 0.05 * windowSize)
        v.append({t, v.last().y()});          // punto fantasma
    series->replace(v);

    //--- eje X ---
    auto &cache = plotCache[channel2plot.value(channelID)];
    if (cache.axX) cache.axX->setRange(tMin, t);
    cache.dirty = true;                       // recalcular Y

    // --- etiqueta de tiempo (opcional) ---
    if (ui->labelTime)
        ui->labelTime->setText(QString::number(t, 'f', 2));
}




/* ----------------------------------------------------------------
 * repinta charts y re‑escala Y solo cuando es necesario
 * ----------------------------------------------------------------
 * refreshCharts
 * Reescala los ejes Y si hay datos nuevos y repinta las gráficas.
 */
void FormPlot::refreshCharts(){
    for (auto it = chartViewMap.cbegin(); it != chartViewMap.cend(); ++it)    {
        const QString plotName = it.key();
        QChartView   *view     = it.value();
        auto         &cache    = plotCache[plotName];

        if (cache.dirty && cache.axY)        {
            const double xMin = cache.axX->min();
            const double xMax = cache.axX->max();

            double minY =  std::numeric_limits<double>::max();
            double maxY = -std::numeric_limits<double>::max();

            for (auto *s : view->chart()->series())            {
                auto *ls = qobject_cast<QLineSeries*>(s);
                if (!ls) continue;
                for (const QPointF &p : ls->pointsVector())            {
                    if (p.x() < xMin || p.x() > xMax) continue;
                    minY = std::min(minY, p.y());
                    maxY = std::max(maxY, p.y());
                }
            }

            if (minY == std::numeric_limits<double>::max()) {
                minY = -1.0; maxY = 1.0;
            } else if (minY == maxY) {
                minY -= 1.0; maxY += 1.0;
            } else {
                const double margin = 0.10 * (maxY - minY);
                minY -= margin;
                maxY += margin;
            }

            // Suavizado exponencial
            const double alpha = 0.4; // más pequeño = más suavizado

            if (cache.smoothMinY == cache.smoothMaxY) {
                // Primera vez
                cache.smoothMinY = minY;
                cache.smoothMaxY = maxY;
            } else {
                cache.smoothMinY = (1.0 - alpha) * cache.smoothMinY + alpha * minY;
                cache.smoothMaxY = (1.0 - alpha) * cache.smoothMaxY + alpha * maxY;
            }
            cache.axY->setRange(cache.smoothMinY, cache.smoothMaxY);

            // Formato dinámico de etiquetas
            double absMax = std::max(std::fabs(cache.smoothMinY), std::fabs(cache.smoothMaxY));
            int decimals;
            if (absMax < 100.0)
                decimals = 4;
            else if (absMax < 1000.0)
                decimals = 3;
            else if (absMax < 10000.0)
                decimals = 1;
            else
                decimals = 0;
            QString format = QString("%.%1f").arg(decimals);
            cache.axY->setLabelFormat(format);
            cache.dirty = false;
        }
        if (view) view->update();  // repintar vista
    }
}





// ----------------------------------------------------------------
//  limpia todo y reinicia ejes
// ----------------------------------------------------------------
/*
 * resetAllData
 * Limpia todos los datos de las gráficas y reinicia los ejes y temporizadores.
 */
void FormPlot::resetAllData()
{
    // Borrar todas las series
    for (auto *series : std::as_const(seriesMap))
        if (series)
            series->clear();

    // Vaciar los buffers de datos guardados
    for (auto &bufPair : buffers)
        bufPair.data.clear();

    // Reiniciar ejes X/Y y flags
    for (auto &cache : plotCache)
    {
        if (cache.axX) cache.axX->setRange(0.0, windowSize);
        if (cache.axY)
        {
            cache.axY->setRange(-1.0, 1.0);
            cache.smoothMinY = cache.axY->min();
            cache.smoothMaxY = cache.axY->max();
        }
        cache.dirty    = false;
        cache.lastT    = 0.0;
        cache.fracPixPend = 0.0;
    }

    //  Resetear el tiempo de arranque
    currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;

    // Actualizar la etiqueta de tiempo si la usas
    if (ui->labelTime)
        ui->labelTime->setText(QStringLiteral("0.00"));
}
//_______________



//____________________________________________________________________________
/*
 * advanceTimeWindow
 * Avanza suavemente la ventana de tiempo mostrada en el eje X de cada gráfica.
 * Realiza scroll en píxeles y sincroniza los rangos cada cierto tiempo.
 */
void FormPlot::advanceTimeWindow()
{
    //  tiempo actual
    const double now = QDateTime::currentMSecsSinceEpoch()/1000.0;

    //  rawDt
    const double rawDt = now - currentTime;
    currentTime        = now;
    if (rawDt <= 0.0) return;

    //  suavizado exponencial del delta-tiempo
    constexpr double alpha = 0.2;
    if (smoothDt == 0.0)         // primer frame
        smoothDt = rawDt;
    else
        smoothDt = alpha*rawDt + (1.0 - alpha)*smoothDt;
    const double dt = smoothDt;

    // actualiza etiqueta de tiempo (opcional)
    if (ui->labelTime)
        ui->labelTime->setText(QString::number(now, 'f', 2));

    //  desplazamiento en cada gráfico
    // calculamos píxeles/segundo una sola vez por frame
    double wPix = chartViewMap.isEmpty() ? 0.0
                                         : chartViewMap.begin().value()->viewport()->width();
    const double pxPerSec = wPix / windowSize;

    for (auto it = plotCache.begin(); it != plotCache.end(); ++it)
    {
        const QString &plotName = it.key();
        PlotCache     &cache    = it.value();
        QChartView    *view     = chartViewMap.value(plotName, nullptr);
        if (!view || !cache.axX) continue;

        // ) scroll "suave" con fracción acumulada
        double moveAcc = pxPerSec * dt + cache.fracPixPend;
        int    dxPix   = int(moveAcc);
        cache.fracPixPend = moveAcc - dxPix;
        if (dxPix > 0)
            view->chart()->scroll(dxPix, 0);

        //  cada ~0.5 s sincronizamos  el rango X
        if (now - cache.lastT > 0.5) {
            cache.axX->setRange(now - windowSize, now);
            cache.lastT = now;
        }
    }
}

