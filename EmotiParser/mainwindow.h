#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadFileClicked();   // Slot para el botón "Cargar Archivo"
    void onParseClicked();      // Slot para el botón "Parser"

private:
    Ui::MainWindow *ui;
    QString rutaArchivoCaptura;  // Ruta del archivo CSV de entrada
};

#endif

