#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>

QSerialPort *serial;
QString comport="";
char buffer[2];


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

        ui->pushButton->setDisabled(true);

        //ui->customPlot = new QCustomPlot(ui->centralWidget);
        //QCustomPlot *customPlot;
        ui->customPlot->addGraph(); // blue line
        ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
        ui->customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
        ui->customPlot->graph(0)->setAntialiasedFill(false);
        ui->customPlot->addGraph(); // red line
        ui->customPlot->graph(1)->setPen(QPen(Qt::red));
        ui->customPlot->graph(0)->setChannelFillGraph(ui->customPlot->graph(1));

        ui->customPlot->addGraph(); // blue dot
        ui->customPlot->graph(2)->setPen(QPen(Qt::blue));
        ui->customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
        ui->customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
        ui->customPlot->addGraph(); // red dot
        ui->customPlot->graph(3)->setPen(QPen(Qt::red));
        ui->customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
        ui->customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

        ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
        ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
        ui->customPlot->xAxis->setAutoTickStep(false);
        ui->customPlot->xAxis->setTickStep(2);
        ui->customPlot->axisRect()->setupFullAxesBox();

        // make left and bottom axes transfer their ranges to right and top axes:
        connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
        connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

        // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
        connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
        dataTimer.start(0); // Interval 0 means to refresh as fast as possible

}

MainWindow::~MainWindow()
{
    serial->close();
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString angle = ui->textEdit_2->toPlainText();

    serial->write(angle.toLatin1());
    //serial->write("9");
    char a;
    serial->read(&a,1);
    QString b= NULL;
    b=a;
    //ui->label->setText(angle);
    ui->label->clear();
    ui->textEdit_2->clear();


}

void MainWindow::Receivedata(){

    QByteArray ba;
    ba=serial->readAll();
    ui->label->setText(ba);

}

void MainWindow::realtimeDataSlot()
{
  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
  static double lastPointKey = 0;
  if (key-lastPointKey > 0.01) // at most add point every 10 ms
  {
    double value0 = tan(key);  //qSin(key); //sin(key*1.6+cos(key*1.7)*2)*10 + sin(key*1.2+0.56)*20 + 26;
    double value1 = -tan(key); //qCos(key); //sin(key*1.3+cos(key*1.2)*1.2)*7 + sin(key*0.9+0.26)*24 + 26;
    // add data to lines:
    ui->customPlot->graph(0)->addData(key, value0);
    ui->customPlot->graph(1)->addData(key, value1);
    // set data of dots:
    ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(2)->addData(key, value0);
    ui->customPlot->graph(3)->clearData();
    ui->customPlot->graph(3)->addData(key, value1);
    // remove data of lines that's outside visible range:
    ui->customPlot->graph(0)->removeDataBefore(key-8);
    ui->customPlot->graph(1)->removeDataBefore(key-8);
    // rescale value (vertical) axis to fit the current data:
    ui->customPlot->graph(0)->rescaleValueAxis();
    ui->customPlot->graph(1)->rescaleValueAxis(true);
    lastPointKey = key;
  }
  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
  ui->customPlot->replot();

  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->customPlot->graph(0)->data()->count()+ui->customPlot->graph(1)->data()->count())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}

void MainWindow::setupPlayground(QCustomPlot *customPlot)
{
  Q_UNUSED(customPlot)
}

void MainWindow::on_pushButton_2_clicked()
{
    serial = new QSerialPort(this);
    comport = ui->textEdit->toPlainText();
    serial->setPortName(comport);
    serial->open(QIODevice::ReadWrite);
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    connect(serial,SIGNAL(readyRead()),this,SLOT(Receivedata()));
    ui->pushButton_2->setDisabled(true);
    ui->pushButton->setDisabled(false);
}

