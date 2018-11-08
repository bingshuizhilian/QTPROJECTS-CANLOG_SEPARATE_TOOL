#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLayout>
#include <QGroupBox>
#include <QStringList>
#include <QSet>
#include <QLineEdit>

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
    enum FileInfoType
    {
        ABSOLUTE_FILE_PATH,
        ABSOLUTE_PATH,
        FILE_NAME
    };

    const QString busmaster = "<Time><Tx/Rx><Channel><CAN ID><Type><DLC><DataBytes>";

private:
    QStringList m_slFileInfo;
    QStringList m_slOriginalLog;
    QSet<QString> m_strsetAllCanMessages;
    QStringList m_slSeparators;
    QStringList m_slStatistics;

private:
    QPlainTextEdit *m_pteOutput;
    QPushButton *m_pbOpenLog;
    QPushButton *m_pbShowStatistics;
    QPushButton *m_pbSaveScreen;
    QPushButton *m_pbClearScreen;
    QLineEdit *m_leSeparator;
    QComboBox *m_cbLogStyle;
    QPushButton *m_pbSeparateLog;

private:
    QVBoxLayout* m_layoutGlobal;
    QGroupBox* m_gbBtns;
    QGroupBox* m_gbFilterSettings;

private:
    void initialization();

private slots:
    void onOpenLogButtonClicked();
    void onShowStatisticsButtonClicked();
    void onSaveScreenButtonClicked();
    void onSeparateLogButtonClicked();
};

#endif // MAINWINDOW_H
