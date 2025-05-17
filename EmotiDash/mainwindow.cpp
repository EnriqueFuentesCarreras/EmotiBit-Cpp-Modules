/****************************************************************************
 * mainwindow.cpp
 *
 * @brief Implementación de la ventana principal de la aplicación.
 *
 * Este archivo configura la interfaz principal, integrando el formulario
 * de vista EmotiBit (`FormVistaEmotiBit`) como el contenido central
 * de la ventana principal (`QMainWindow`).
 *
 * Funcionalidades:
 * - Inicialización de la UI.
 * - Inserción de la vista EmotiBit como widget principal.
 *
 * @author Enrique
 * @date 2025-04-29
 ****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formvistaemotibit.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    formVistaEmotiBit(new FormVistaEmotiBit(this))
{
    ui->setupUi(this);  

    setCentralWidget(formVistaEmotiBit);



}

MainWindow::~MainWindow()
{
    delete ui;
}
