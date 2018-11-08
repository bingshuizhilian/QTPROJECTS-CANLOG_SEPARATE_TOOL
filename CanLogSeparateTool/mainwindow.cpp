#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initialization();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialization()
{
    //窗体名称设置
    setWindowTitle(tr("CanLogSeparateTool"));

    //状态栏设置
    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS-CANLOG_SEPARATE_TOOL> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    //控件初始化
    m_pteOutput = new QPlainTextEdit;
    m_leSeparator = new QLineEdit;
    m_leSeparator->setToolTip(tr("input format(case insensitive): 0xhhh;0xhhhh"));
    QRegExp hexCodeRegex("^(0[xX][a-fA-F\\d]{3,4};)+$");
    auto validator = new QRegExpValidator(hexCodeRegex, m_leSeparator);
    m_leSeparator->setValidator(validator);
    m_cbLogStyle = new QComboBox;
    m_cbLogStyle->setStatusTip(tr("select a log style"));
    m_cbLogStyle->addItem(tr("time stamp"));
    m_cbLogStyle->addItem(tr("line number"));
    m_cbLogStyle->addItem(tr("time & line"));
    m_cbLogStyle->addItem(tr("none"));
    m_pbOpenLog = new QPushButton;
    m_pbOpenLog->setText(tr("open log"));
    connect(m_pbOpenLog, &m_pbOpenLog->clicked, this, &onOpenLogButtonClicked);
    m_pbShowStatistics = new QPushButton;
    m_pbShowStatistics->setText(tr("log statistics"));
    connect(m_pbShowStatistics, &m_pbShowStatistics->clicked, this, &onShowStatisticsButtonClicked);
    m_pbSaveScreen = new QPushButton;
    m_pbSaveScreen->setText(tr("save screen"));
    connect(m_pbSaveScreen, &m_pbSaveScreen->clicked, this, &onSaveScreenButtonClicked);
    m_pbClearScreen = new QPushButton;
    m_pbClearScreen->setText(tr("clear screen"));
    connect(m_pbClearScreen, &m_pbClearScreen->clicked, m_pteOutput, &m_pteOutput->clear);
    m_pbSeparateLog = new QPushButton;
    m_pbSeparateLog->setText(tr("separate log"));
    connect(m_pbSeparateLog, &m_pbSeparateLog->clicked, this, &onSeparateLogButtonClicked);

    //CAN消息过滤设置
    m_gbFilterSettings = new QGroupBox;
    m_gbFilterSettings->setTitle(tr("filters"));
    auto m_gbFilterSettingsLayout = new QHBoxLayout;
    m_gbFilterSettingsLayout->addWidget(m_leSeparator);
    m_gbFilterSettingsLayout->addWidget(m_cbLogStyle);
    m_gbFilterSettingsLayout->addWidget(m_pbSeparateLog);
    m_gbFilterSettings->setLayout(m_gbFilterSettingsLayout);

    //主要操作按钮
    m_gbBtns = new QGroupBox;
    m_gbBtns->setTitle(tr("operations"));
    auto m_gbBtnsLayout = new QHBoxLayout;
    m_gbBtnsLayout->addWidget(m_pbOpenLog);
    m_gbBtnsLayout->addWidget(m_pbShowStatistics);
    m_gbBtnsLayout->addWidget(m_pbSaveScreen);
    m_gbBtnsLayout->addWidget(m_pbClearScreen);
    m_gbBtns->setLayout(m_gbBtnsLayout);

    //全局布局
    m_layoutGlobal = new QVBoxLayout;
    m_layoutGlobal->addWidget(m_pteOutput);
    m_layoutGlobal->addWidget(m_gbFilterSettings);
    m_layoutGlobal->addWidget(m_gbBtns);

    ui->centralWidget->setLayout(m_layoutGlobal);
    this->setMinimumSize(480, 400);
}

void MainWindow::onOpenLogButtonClicked()
{
    //定义文件对话框类
    QFileDialog* fileDialog = new QFileDialog(this);
    //定义文件对话框标题
    fileDialog->setWindowTitle(tr("choose log file"));
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器
    fileDialog->setNameFilter(tr("*.log *.txt"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFile
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    QStringList fileNames;

    if(fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();

        m_slFileInfo.clear();
        m_slFileInfo.push_back(fileNames.first());
        m_slFileInfo.push_back(QFileInfo(fileNames.first()).absolutePath());
        m_slFileInfo.push_back(QFileInfo(fileNames.first()).fileName());
    }

    if(fileNames.isEmpty())
        return;

    if(m_slFileInfo.isEmpty())
        return;

    for(auto tmp:m_slFileInfo)
        qDebug()<<tmp<<endl;

    //清理旧信息
    m_pteOutput->clear();
    m_slOriginalLog.clear();
    m_strsetAllCanMessages.clear();

    //获取log原文件
    QString fileName = m_slFileInfo.at(ABSOLUTE_FILE_PATH);
    QFile logFile(fileName);
    if(!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + fileName, QMessageBox::Yes);
        return;
    }
    QTextStream logFileIn(&logFile);
    while(!logFileIn.atEnd())
    {
        QString readStr = logFileIn.readLine();

        //busmaster log format
        if(!readStr.isEmpty() && !readStr.startsWith('*') && !readStr.startsWith('"'))
        {
            m_slOriginalLog.push_back(readStr);

            if(readStr.contains("0x", Qt::CaseInsensitive))
            {
                int index1 = 0, index2 = 0;
                QString tmpStr;

                index1 = readStr.indexOf("0x", Qt::CaseInsensitive);

                if(-1 != index1)
                    index2 = readStr.indexOf(' ', index1, Qt::CaseInsensitive);

                if(-1 != index2)
                    tmpStr = readStr.mid(index1, index2 - index1);

                if(!tmpStr.isEmpty())
                    m_strsetAllCanMessages.insert(tmpStr.toLower());
            }
        }
    }
    logFile.close();

    m_pbOpenLog->setStatusTip("current log file: " + m_slFileInfo.at(FILE_NAME));

    //统计信息
    m_slStatistics.clear();
    m_pteOutput->appendPlainText("current loaded log file: " + m_slFileInfo.at(ABSOLUTE_FILE_PATH) + '\n');

    m_slStatistics << "***all detected can message list***";
    foreach(const QString &value, m_strsetAllCanMessages)
        m_slStatistics << value;

    m_slStatistics << "\n***statistics infos***";
    m_slStatistics << "Total effective lines: " + QString::number(m_slOriginalLog.size());
    m_slStatistics << "Total different can messages: " + QString::number(m_strsetAllCanMessages.size());

    foreach(const QString &value, m_slStatistics)
        m_pteOutput->appendPlainText(value);
}

void MainWindow::onShowStatisticsButtonClicked()
{
    m_pteOutput->clear();

    if(!m_slFileInfo.isEmpty())
        m_pteOutput->appendPlainText("current loaded log file: " + m_slFileInfo.at(ABSOLUTE_FILE_PATH) + '\n');

    if(!m_slStatistics.isEmpty())
    {
        foreach(const QString &value, m_slStatistics)
            m_pteOutput->appendPlainText(value);
    }
}

void MainWindow::onSaveScreenButtonClicked()
{
    if(m_pteOutput->toPlainText().isEmpty())
        return;

    //存储屏幕内容
    QString tmpFileName = m_slFileInfo.at(FILE_NAME);
    QString tmpFileClassId = tmpFileName.right(4);
    QString timeInfo = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    tmpFileName = tmpFileName.left(tmpFileName.size() - 4);
    tmpFileName += "_SeparatedLog_" + timeInfo + tmpFileClassId;

    qDebug()<<tmpFileName<<endl;

    QString folderName = "/separatedLogs/";
    QString dirPath = m_slFileInfo.at(ABSOLUTE_PATH) + folderName;
    QDir dir(dirPath);
    if(!dir.exists())
        dir.mkdir(dirPath);

    qDebug()<<dirPath<<endl;

    QString newFilePathName = dirPath + tmpFileName;
    QFile newFile(newFilePathName);
    if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + newFilePathName, QMessageBox::Yes);
        return;
    }

    QTextStream out(&newFile);

    if(!m_slFileInfo.isEmpty())
        out << "original log file: " + m_slFileInfo.at(ABSOLUTE_FILE_PATH) + '\n';
    out << m_pteOutput->toPlainText();

    newFile.close();

    qDebug()<<newFilePathName<<endl;

    //WINDOWS环境下，选中该文件
#ifdef WIN32
    QProcess process;
    QString openFileName = newFilePathName;

    openFileName.replace("/", "\\");    //***这句windows下必要***
    process.startDetached("explorer /select," + openFileName);
#endif
}

void MainWindow::onSeparateLogButtonClicked()
{
    QString separator = m_leSeparator->text();
    if(separator.isEmpty() || m_strsetAllCanMessages.isEmpty())
        return;

    QStringList tmpSeparators;
    tmpSeparators = separator.split(';', QString::SkipEmptyParts, Qt::CaseInsensitive);

    m_slSeparators.clear();
    foreach(const QString &value, tmpSeparators)
    {
        //can id format: 0x3af 0x5edd
        if(value.size() != 5 && value.size() != 6)
        {
            QMessageBox::warning(this, "Warnning", "please check input", QMessageBox::Yes);
            return;
        }

        if(!m_strsetAllCanMessages.contains(value.toLower()))
        {
            QMessageBox::warning(this, "Warnning", "none " + value + " record founded", QMessageBox::Yes);
            continue;
        }

        m_slSeparators << value.toLower();
    }

    if(m_slSeparators.isEmpty())
        return;

    m_pteOutput->clear();
    QString bmStr;
    switch(m_cbLogStyle->currentIndex())
    {
    case 0:
        bmStr = busmaster;
        break;
    case 1:
        bmStr = busmaster.right(busmaster.size() - busmaster.indexOf("<CAN ID>", Qt::CaseInsensitive));
        bmStr += "<Line Number>";
        break;
    case 2:
        bmStr = busmaster + "<Line Number>";
        break;
    case 3:
        bmStr = busmaster.right(busmaster.size() - busmaster.indexOf("<CAN ID>", Qt::CaseInsensitive));
        break;
    default:
        break;
    }
    m_pteOutput->appendPlainText(bmStr);

    unsigned int lineNumber = 1;
    foreach(const QString &value1, m_slOriginalLog)
    {
        foreach(const QString &value2, m_slSeparators)
        {
            if(value1.contains(value2, Qt::CaseInsensitive))
            {
                QString tmpStr;
                switch(m_cbLogStyle->currentIndex())
                {
                case 0:
                    tmpStr = value1;
                    break;
                case 1:
                    tmpStr = value1.right(value1.size() - value1.indexOf("0x", Qt::CaseInsensitive));
                    tmpStr += "  (line: " + QString::number(lineNumber++) + ")";
                    break;
                case 2:
                    tmpStr = value1 + "  (line: " + QString::number(lineNumber++) + ")";
                    break;
                case 3:
                    tmpStr = value1.right(value1.size() - value1.indexOf("0x", Qt::CaseInsensitive));
                    break;
                default:
                    tmpStr = value1;
                    break;
                }

                m_pteOutput->appendPlainText(tmpStr);
            }
        }
    }

    QTextCursor cursor = m_pteOutput->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_pteOutput->setTextCursor(cursor);
}
