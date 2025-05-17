#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , emulator(new EmotiBitEmulator(this))

{
    ui->setupUi(this);

    // Inicializar botones
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);

    // Conectar señales y slots
    connect(emulator, &EmotiBitEmulator::messageLogged, this, &MainWindow::displayMessage);

    ui->pushButtonStart->setVisible(false);
    ui->pushButtonStop->setVisible(false);
    ui->pushButtonLoadCsv->setVisible(false);
}





MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonLoadCsv_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Archivo CSV",QDir::homePath(), "Archivos CSV (*.csv)");
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Advertencia", "No se seleccionó ningún archivo");
        return;
    }

    emulator->setCsvFile(fileName);
    ui->pushButtonStart->setEnabled(true);
    displayMessage("Archivo CSV cargado: " + fileName);
}

void MainWindow::on_pushButtonStart_clicked(){
    emulator->startEmulation();
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonStop->setEnabled(true);
    displayMessage("Emulación iniciada_.");
}

void MainWindow::on_pushButtonStop_clicked()
{
    emulator->stopEmulation();
    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonStop->setEnabled(false);
    displayMessage("Emulación detenida.");
}

void MainWindow::displayMessage(const QString &message)
{
    ui->textBrowserMensajes->append(message);
}



