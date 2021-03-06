#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>

#include<json-c/json.h>
int pitch = 0;


MainWindow *handle;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->count = 50;
    this->time = 0;
    this->setWindowTitle("EE513 Assignment 2");
    this->ui->customPlot->addGraph();
    this->ui->customPlot->yAxis->setLabel("Pitch (degrees)");
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    this->ui->customPlot->xAxis->setTicker(timeTicker);
    this->ui->customPlot->yAxis->setRange(-180,180);
    this->ui->customPlot->replot();
    QObject::connect(this, SIGNAL(messageSignal(QString)),
                     this, SLOT(on_MQTTmessage(QString)));
    ::handle = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    // For more help on real-time plots, see: http://www.qcustomplot.com/index.php/demos/realtimedatademo
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    ui->customPlot->graph(0)->addData(key,count);
    ui->customPlot->graph(0)->rescaleKeyAxis(true);
    this->ui->customPlot->graph(0)->addData(key, pitch);

    ui->customPlot->replot();
    QString text = QString("Value added is %1").arg(this->count);
    //ui->outputEdit->setText(text);
}

void MainWindow::on_connectButton_clicked()
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)==0){
        ui->ConOutput->appendPlainText(QString("Callbacks set correctly"));
    }
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        ui->ConOutput->appendPlainText(QString("Failed to connect, return code %1").arg(rc));
    }
    ui->ConOutput->appendPlainText(QString("Subscribing to topic " TOPIC " for client " CLIENTID));
    int x = MQTTClient_subscribe(client, TOPIC, QOS);
    ui->ConOutput->appendPlainText(QString("Result of subscribe is %1 (0=success)").arg(x));
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    (void)context;
    // Please don't modify the Window UI from here
    qDebug() << "Message delivery confirmed";
    handle->deliveredtoken = dt;
}

/* This is a callback function and is essentially another thread. Do not modify the
 * main window UI from here as it will cause problems. Please see the Slot method that
 * is directly below this function. To ensure that this method is thread safe I had to
 * get it to emit a signal which is received by the slot method below */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    (void)context; (void)topicLen;
    qDebug() << "Message arrived (topic is " << topicName << ")";
    qDebug() << "Message payload length is " << message->payloadlen;
    QString payload;
    payload.sprintf("%s", (char *) message->payload).truncate(message->payloadlen);
    emit handle->messageSignal(payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/** This is the slot method. Do all of your message received work here. It is also safe
 * to call other methods on the object from this point in the code */
void MainWindow::on_MQTTmessage(QString payload){
    ui->ConOutput->appendPlainText(payload);
    ui->ConOutput->ensureCursorVisible();

    //ADD YOUR CODE HERE
    QByteArray QTpay = payload.toLocal8Bit();
    const char *new_payload = QTpay.data();

    struct json_object *parsed_json;
    struct json_object *CPUt;
    struct json_object *piTime;
    struct json_object *parsedX;
    struct json_object *parsedY;
    struct json_object *parsedZ;

    parsed_json = json_tokener_parse(new_payload);
    json_object_object_get_ex(parsed_json, "CPUTemp", &CPUt);
    json_object_object_get_ex(parsed_json, "Time(at publish)", &piTime);
    json_object_object_get_ex(parsed_json, "X", &parsedX);
    json_object_object_get_ex(parsed_json, "Y", &parsedY);
    json_object_object_get_ex(parsed_json, "Z", &parsedZ);

    signed int X = json_object_get_int(parsedX);
    signed int Y = json_object_get_int(parsedY);
    signed int Z = json_object_get_int(parsedZ);

    ui->ConOutput->appendPlainText(QString("Time at publish: ") + json_object_get_string(piTime));
    ui->ConOutput->appendPlainText(QString("CPU Temperature: ") + json_object_get_string(CPUt) + QString(" degrees"));
    ui->ConOutput->appendPlainText(QString("X Co-ordinate: ") + json_object_get_string(parsedX));
    ui->ConOutput->appendPlainText(QString("Y Co-ordinate: ") + json_object_get_string(parsedY));
    ui->ConOutput->appendPlainText(QString("Z Co-ordinate: ") + json_object_get_string(parsedZ));

    this->getPitch(X,Y,Z);
    this->update();

}

void connlost(void *context, char *cause) {
    (void)context; (void)*cause;
    // Please don't modify the Window UI from here
    qDebug() << "Connection Lost" << endl;
}

void MainWindow::on_disconnectButton_clicked()
{
    qDebug() << "Disconnecting from the broker" << endl;
    MQTTClient_disconnect(client, 10000);
    //MQTTClient_destroy(&client);
}

void MainWindow::getPitch(int X,int Y,int Z){
    float gravity_range=4.0f;
    float resolution = 1024.0f;
    float factor = gravity_range/resolution*1000;

    signed int accelerationX = (signed int)(X * factor);
    signed int accelerationY = (signed int)(Y * factor);
    signed int accelerationZ = (signed int)(Z * factor);

    signed int pitch = 180 * atan (accelerationX/sqrt(accelerationY*accelerationY
                                                      + accelerationZ*accelerationZ))/M_PI;

}
