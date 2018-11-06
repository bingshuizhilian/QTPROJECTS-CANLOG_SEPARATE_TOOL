#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>

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
    //������������
    setWindowTitle(tr("CanLogSeparateTool"));

    //״̬������
    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS-FIRMWARE_GENERATOR> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    //�ؼ���ʼ��
    m_pteOutput = new QPlainTextEdit;
    m_pbOpenLog = new QPushButton;
    m_pbOpenLog->setText(tr("open log"));

    //CAN��Ϣ��������
    m_gbCkboxs = new QGroupBox;
    m_gbCkboxs->setTitle(tr("filters"));
    m_gbCkboxs->setLayout(m_gbCkboxsLayout);

    //��Ҫ������ť
    m_gbBtns = new QGroupBox;
    m_gbBtns->setTitle(tr("operations"));
    auto m_gbBtnsLayout = new QHBoxLayout;
    m_gbBtnsLayout->addWidget(m_pbOpenLog);
    m_gbBtns->setLayout(m_gbBtnsLayout);

    //�����CAN��Ϣ���˲���
    auto upLayout = new QHBoxLayout;
    upLayout->addWidget(m_pteOutput);
    upLayout->addWidget(m_gbCkboxs);

    //ȫ�ֲ���
    m_layoutGlobal = new QVBoxLayout;
    m_layoutGlobal->addLayout(upLayout);
    m_layoutGlobal->addWidget(m_gbBtns);

    ui->centralWidget->setLayout(m_layoutGlobal);
}
