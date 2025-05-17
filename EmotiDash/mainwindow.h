/**
 * @file mainwindow.h
 * @brief Ventana principal que contiene e integra todas las vistas de la aplicación.
 *
 * La ventana principal sirve como contenedor principal para la interfaz gráfica,
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "formvistaemotibit.h"


/**
 * @class MainWindow
 * @brief Ventana principal que aloja la interfaz gráfica principal.
 *
 * Esta clase inicializa y aloja `FormVistaEmotiBit`, proporcionando la estructura
 * general de navegación y gestión de las interfaces de usuario de la aplicación.
 *
 * @see FormVistaEmotiBit
 * @see EmotiBitController
 */
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;  
    FormVistaEmotiBit *formVistaEmotiBit;
};

#endif

