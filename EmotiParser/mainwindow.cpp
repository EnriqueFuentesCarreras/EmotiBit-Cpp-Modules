#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qemotibirparser.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::onLoadFileClicked);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onParseClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString rutaArchivoCaptura;

void MainWindow::onLoadFileClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Seleccionar archivo CSV de EmotiBit",
                                                    QString(),
                                                    "Archivos CSV (*.csv);;Todos los archivos (*)");

    if (!fileName.isEmpty()) {
        rutaArchivoCaptura = fileName;
        ui->labelArchivo->setText("Archivo en bruto EmotiBit: " + fileName);
        ui->textBrowser->append("Archivo cargado: " + fileName);
    }
}

void MainWindow::onParseClicked()
{
    if (rutaArchivoCaptura.isEmpty()) {
        ui->textBrowser->append("Por favor, selecciona primero un archivo.");
        return;
    }

    ui->textBrowser->append("Iniciando parseo del archivo...");
    qemotibirparser parser;
    QStringList archivosGenerados = parser.loadCsvFile(rutaArchivoCaptura);

    if (archivosGenerados.isEmpty()) {
        ui->textBrowser->append("No se generaron archivos. Revisa si el archivo contiene datos válidos.");
    } else {
        ui->textBrowser->append("Archivos generados:");
        for (const QString &nombre : archivosGenerados) {
            ui->textBrowser->append("  → " + QFileInfo(nombre).fileName());
        }
    }

    ui->textBrowser->append("Parseo finalizado.\n");
}





