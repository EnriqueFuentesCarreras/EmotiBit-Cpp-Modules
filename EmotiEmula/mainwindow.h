#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "EmotiBitEmulator.h"

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
    void on_pushButtonLoadCsv_clicked();
    void on_pushButtonStart_clicked();
    void on_pushButtonStop_clicked();

    void displayMessage(const QString &message);

private:
    Ui::MainWindow *ui;
    EmotiBitEmulator *emulator;

};

#endif
