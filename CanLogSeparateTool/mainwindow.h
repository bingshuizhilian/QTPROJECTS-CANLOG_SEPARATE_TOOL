#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLayout>
#include <QGroupBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private:
    QPlainTextEdit *m_pteOutput;
    QPushButton *m_pbOpenLog;

private:
    QVBoxLayout* m_layoutGlobal;
    QGridLayout* m_gbCkboxsLayout;
    QGroupBox* m_gbBtns;
    QGroupBox* m_gbCkboxs;

private:
    void initialization();
};

#endif // MAINWINDOW_H
