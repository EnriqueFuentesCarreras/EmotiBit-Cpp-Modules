#include "MainWindow.h"
#include <QPushButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QChart>
#include "EmotiBitParser.h"
#include <cmath>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow){

    ui->setupUi(this);

    // Conectar botones
    connect(ui-> fileSdBtn, &QPushButton::clicked, this, &MainWindow::onSelectSdFile);
    connect(ui->filePcBtn, &QPushButton::clicked, this, &MainWindow::onSelectPcFile);
    connect(ui->plotBtn, &QPushButton::clicked, this, &MainWindow::onPlot);
    connect(ui->calcCoincidenceBtn, &QPushButton::clicked, this, &MainWindow::onCalculateCoincidence);
}

qint64 MainWindow::getEarliestTimestamp(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "No se pudo abrir para leer timestamp:" << filePath;
        return -1;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        QStringList fields = line.split(',');
        if (fields.size() < 1)
            continue;
        qint64 t = fields[0].toLongLong();
        file.close();
        return t;
    }
    file.close();
    return -1;
}

void MainWindow::onSelectPcFile() {
    QString f = QFileDialog::getOpenFileName(this, "Seleccionar archivo PC", "", "CSV Files (*.csv)");
    if (!f.isEmpty()) {
        filePcPath = f;
        qDebug() << "Archivo PC:" << filePcPath;
        pcEarliestTimestamp = getEarliestTimestamp(filePcPath);
        if (pcEarliestTimestamp < 0)
            qWarning() << "No se pudo determinar el timestamp inicial del archivo PC.";
        else
            qDebug() << "pcEarliestTimestamp =" << pcEarliestTimestamp;
    }
}

void MainWindow::onSelectSdFile() {
    QString f = QFileDialog::getOpenFileName(this, "Seleccionar archivo SD", "", "CSV Files (*.csv)");
    if (!f.isEmpty()) {
        fileSdPath = f;
        qDebug() << "Archivo SD:" << fileSdPath;
    }
}


void MainWindow::onPlot() {
    if (fileSdPath.isEmpty() || filePcPath.isEmpty()) {
        QMessageBox::warning(this, "Faltan archivos", "Por favor selecciona ambos archivos.");
        return;
    }
    if (pcEarliestTimestamp < 0) {
        QMessageBox::warning(this, "Error de timestamp", "No se pudo determinar el timestamp inicial del archivo PC.");
        return;
    }

    QString chan = ui->channelCombo->currentText();
    EmotiBitParser parser(channelFreq);

    QVector<Sample> pcSamples = parser.parseFile(filePcPath, chan, pcEarliestTimestamp);
    QVector<Sample> sdSamples = parser.parseFile(fileSdPath, chan, pcEarliestTimestamp);

    if (pcSamples.isEmpty() && sdSamples.isEmpty()) {
        QMessageBox::information(this, "Sin datos", QString("No se encontraron datos para el canal %1.").arg(chan));
        return;
    }

    // --- Gráfica combinada ---
    auto *pcUpperSeries = new QLineSeries;
    auto *pcBaseline = new QLineSeries;
    for (const Sample &s : pcSamples) {
        pcUpperSeries->append(s.time, s.value);
        pcBaseline->append(s.time, 0.0);
    }
    auto *pcAreaSeries = new QAreaSeries(pcUpperSeries, pcBaseline);
    pcAreaSeries->setName("PC");
    QBrush areaBrush(pcUpperSeries->color().lighter(150));
    pcAreaSeries->setBrush(areaBrush);

    auto *sdSeriesCombined = new QLineSeries;
    for (const Sample &s : sdSamples)
        sdSeriesCombined->append(s.time, s.value);
    sdSeriesCombined->setName("SD");

    auto *chartCombined = new QChart;
    chartCombined->setTitle(QString("Gráfica combinada canal %1").arg(chan));
    chartCombined->addSeries(pcAreaSeries);
    chartCombined->addSeries(sdSeriesCombined);
    chartCombined->createDefaultAxes();
    chartCombined->legend()->setAlignment(Qt::AlignBottom);
    ui->chartViewCombined->setChart(chartCombined);

    // --- Gráfica PC sola ---
    auto *pcSeriesAlone = new QLineSeries;
    pcSeriesAlone->setName("PC");
    for (const Sample &s : pcSamples)
        pcSeriesAlone->append(s.time, s.value);
    auto *chartPc = new QChart;
    chartPc->setTitle(QString("Gráfica PC canal %1").arg(chan));
    chartPc->addSeries(pcSeriesAlone);
    chartPc->createDefaultAxes();
    chartPc->legend()->setAlignment(Qt::AlignBottom);
    ui->chartViewPc->setChart(chartPc);

    // --- Gráfica SD sola ---
    auto *sdSeriesAlone = new QLineSeries;
    sdSeriesAlone->setName("SD");
    for (const Sample &s : sdSamples)
        sdSeriesAlone->append(s.time, s.value);
    auto *chartSd = new QChart;
    chartSd->setTitle(QString("Gráfica SD canal %1").arg(chan));
    chartSd->addSeries(sdSeriesAlone);
    chartSd->createDefaultAxes();
    chartSd->legend()->setAlignment(Qt::AlignBottom);
    ui->chartViewSd->setChart(chartSd);
}





void MainWindow::onCalculateCoincidence() {
    ui->textBrowserCalculo->clear();  // Limpia antes de mostrar nuevos resultados

    if (fileSdPath.isEmpty() || filePcPath.isEmpty()) {
        ui->textBrowserCalculo->append("⚠️ Por favor selecciona ambos archivos.\n");
        return;
    }

    if (pcEarliestTimestamp < 0) {
        ui->textBrowserCalculo->append("⚠️ No se pudo determinar el timestamp inicial del archivo PC.\n");
        return;
    }

    QString chan = ui->channelCombo->currentText();
    EmotiBitParser parser(channelFreq);

    QVector<Sample> pcSamples = parser.parseFile(filePcPath, chan, pcEarliestTimestamp);
    QVector<Sample> sdSamples = parser.parseFile(fileSdPath, chan, pcEarliestTimestamp);

    QFileInfo infoPc(filePcPath);
    QFileInfo infoSd(fileSdPath);

    ui->textBrowserCalculo->append("🗂️ <b>Archivo PC:</b> " + infoPc.fileName());
    ui->textBrowserCalculo->append("🗂️ <b>Archivo SD:</b> " + infoSd.fileName());
    ui->textBrowserCalculo->append("📊 <b>Canal analizado:</b> " + chan);
    ui->textBrowserCalculo->append("");

    if (pcSamples.isEmpty() || sdSamples.isEmpty()) {
        ui->textBrowserCalculo->append("⚠️ No hay datos suficientes en uno de los archivos para realizar el cálculo.");
        return;
    }

    double tStart = std::max(pcSamples.first().time, sdSamples.first().time);
    double tEnd = std::min(pcSamples.last().time, sdSamples.last().time);
    double effectiveStart = tStart + 1.0;
    double effectiveEnd = tEnd - 1.0;

    if (effectiveEnd <= effectiveStart) {
        ui->textBrowserCalculo->append("⚠️ El intervalo efectivo es demasiado corto para realizar el cálculo.");
        return;
    }

    int countPc = std::count_if(pcSamples.begin(), pcSamples.end(),
                                [&](const Sample &s) { return s.time >= effectiveStart && s.time <= effectiveEnd; });

    int countSd = std::count_if(sdSamples.begin(), sdSamples.end(),
                                [&](const Sample &s) { return s.time >= effectiveStart && s.time <= effectiveEnd; });

    if (countPc == 0 || countSd == 0) {
        ui->textBrowserCalculo->append("⚠️ No se encontraron muestras en el intervalo efectivo.");
        return;
    }

    int diff = std::abs(countPc - countSd);
    double percentage = (countPc > countSd)
                            ? (double(diff) / countSd) * 100.0
                            : (double(diff) / countPc) * 100.0;

    ui->textBrowserCalculo->append(QString("✅ <b>Muestras en intervalo común:</b>"));
    ui->textBrowserCalculo->append(QString("• Archivo PC: <b>%1</b>").arg(countPc));
    ui->textBrowserCalculo->append(QString("• Archivo SD: <b>%1</b>").arg(countSd));
    ui->textBrowserCalculo->append("");

    ui->textBrowserCalculo->append(QString("📈 <b>Diferencia:</b> %1 muestras (%2%)")
                                       .arg(diff)
                                       .arg(percentage, 0, 'f', 2));
}



