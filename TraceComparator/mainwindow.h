#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include <QtCharts/QChartView>
#include "ChannelFrequencies.h"
#include <QPushButton>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

//QT_CHARTS_USE_NAMESPACE

    class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSelectSdFile();
    void onSelectPcFile();
    void onPlot();


private:
      Ui::MainWindow *ui;
    // Función auxiliar para obtener el timestamp inicial del archivo PC.
    qint64 getEarliestTimestamp(const QString &filePath);
    void onCalculateCoincidence();


    // Botones
    QPushButton *fileSdBtn;
    QPushButton *filePcBtn;
    QPushButton *plotBtn;
    QPushButton *calcCoincidenceBtn;

    // Combo de canales
    QComboBox *channelCombo;

    // Etiquetas
    QLabel *labelChannel;
    QLabel *labelCombined;
    QLabel *labelPc;
    QLabel *labelSd;


    QString fileSdPath;  // Archivo capturado en la SD
    QString filePcPath;  // Archivo capturado en el PC


    // Tres vistas para las gráficas:
    QChartView *chartViewCombined; // Gráfica combinada
    QChartView *chartViewPc;       // Gráfica PC sola
    QChartView *chartViewSd;       // Gráfica SD sola

    ChannelFrequencies channelFreq;  // Instancia con las frecuencias definidas

    qint64 pcEarliestTimestamp = -1;   // Se fijará al seleccionar el archivo PC

};

#endif
