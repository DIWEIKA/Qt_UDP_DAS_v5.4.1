#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isStart = false;

    isASCII = false;

    isHEX = false;

    isSave = false;

    AcqMode = 1;

    //set Style Sheet
    setLocalStyleSheet();

    //set Local Message
    setLocalMsg();

    udp_recv = new UDP_Recv(this);

    udp_recv->start();

    //Counting 60s
    udpTimer = new QTimer();
    udpTimer->setTimerType(Qt::PreciseTimer);

    writeToFiles = new WriteToFiles(udp_recv);

    wave_Widget = new wave_widget();

    demodu = new Demodulation(udp_recv->CHdata2);

    com_send = new COM_Send(this);

    //clear window
    if(ui->textEdit_Msg->isFullScreen())
        ui->textEdit_Msg->clear();

    connect(ui->comboBox_Mode, QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MainWindow::on_comboBox_Mode_currentIndexChangedSlot);

    connect(udp_recv,&QThread::finished,this,&MainWindow::FinishUDP_RecvThread);

    connect(udpTimer,&QTimer::timeout,this,&MainWindow::OpenWriteToFilesThread);

    connect(writeToFiles,&QThread::finished,this,&MainWindow::FinishWriteToFilesThread);

    connect(udp_recv,&UDP_Recv::SendtoWidget,wave_Widget,&wave_widget::FlashWave3,Qt::BlockingQueuedConnection);

    connect(demodu,&Demodulation::sendPhToWrite,writeToFiles,&WriteToFiles::recvPhSlot,Qt::BlockingQueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setLocalStyleSheet()
{
    //set Style Sheet
    QFile file("./my.qss"); //放在build directory下

    file.open(QIODevice::ReadOnly);

    QString styleSheet = QLatin1String(file.readAll());

    qApp->setStyleSheet(styleSheet);

}

void MainWindow::setLocalMsg()
{
    //获取本机的计算机名
    QString localHostName = QHostInfo:: localHostName();
    qDebug() <<"LocalHostName: "<<localHostName<<endl;

    ui->textEdit_Msg->insertPlainText("LocalHostName: "+localHostName+"\n");

    //获取本机IP
    QHostInfo info = QHostInfo::fromName(localHostName);
    QList<QHostAddress> strIpAddress  = info.addresses();
    QHostAddress IpAddress =  strIpAddress.back();
    qDebug() << "IpAddress: " << IpAddress<<endl;
    qDebug()<<"--------------------------"<<endl;

    ui->textEdit_Msg->insertPlainText("IpAddress: "+IpAddress.toString()+" Port: 7000 \n");

    //设置窗口的标题
    QString title = QString("Server IP: %1, Port: 7000").arg(IpAddress.toString());
    setWindowTitle(title);

    ui->textEdit_Msg->insertPlainText("Setting Acquisition Card Mode I ! \n");
}

void MainWindow::FinishUDP_RecvThread()
{
    //quit Thread
    udp_recv->quit();

    udp_recv->wait();

}

void MainWindow::OpenWriteToFilesThread()
{
    isSave = ui->checkBox_Save->isChecked();

    if(isSave && isStart){

        writeToFiles->saveFlag = true;

        writeToFiles->start();

        writeToFiles->demoFlag = ui->checkBox_Demo->isChecked();
    }

}

void MainWindow::FinishWriteToFilesThread()
{
    //quit Thread
    writeToFiles->quit();

    writeToFiles->wait();

    ui->textEdit_Msg->insertPlainText(" Files have been saved in " + writeToFiles->saveFilenameAll+"\n");

}

void MainWindow::on_pushButton_Start_clicked()
{
    isStart = true;

    ui->textEdit_Msg->insertPlainText("Started ! \n");
}

void MainWindow::on_pushButton_Stop_clicked()
{
    isStart = false;
    isSave = false;

    ui->checkBox_ASCII->setChecked(isStart);
    ui->checkBox_Hex->setChecked(isStart);
    ui->checkBox_Save->setChecked(isSave);

    //clear CHdata
    udp_recv->clearCHdata();

    ui->textEdit_Msg->insertPlainText("Stopped ! \n");
}

void MainWindow::on_pushButton_Clear_clicked()
{

    ui->textEdit_Msg->clear();

}

void MainWindow::on_checkBox_Save_clicked()
{

    udpTimer->start(1000);

}

void MainWindow::on_checkBox_ASCII_clicked()
{
    isASCII = true;
}

void MainWindow::on_checkBox_Hex_clicked()
{
    isHEX = true;
}

void MainWindow::on_pushButton_Display_clicked()
{
    wave_Widget->show();
}

void MainWindow::on_checkBox_Demo_clicked()
{
    demodu->start();
}

void MainWindow::on_comboBox_Mode_currentIndexChangedSlot()
{
    //采集卡模式选择
    if(ui->comboBox_Mode->currentIndex() == 0){
        ui->textEdit_Msg->insertPlainText("Setting Acquisition Card Mode I ! \n");

        AcqMode = 1;
    }

    if(ui->comboBox_Mode->currentIndex() == 1){
        ui->textEdit_Msg->insertPlainText("Setting Acquisition Card Mode II ! \n");

        AcqMode = 2;
    }
}

void MainWindow::on_pushButton_Send_clicked()
{
    com_send->start();

     ui->textEdit_Msg->insertPlainText("Sendding peak.txt ! \n");
}

