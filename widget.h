#include <QtSerialPort/QSerialPort>//串口
#include <QtSerialPort/QSerialPortInfo>//串口
#include <QDebug>//用于在控制台输出调试信息
#include <QTime>//定时器
#include <QPainter>//坐标系绘图

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
signals:
    void readyAirData(QString data);//解析完成后发出信息
private slots:
    void on_Button_Search_clicked();
    void AnalyzeData();//数据读取
    void setupPlot();//初始化

    void on_Button_Open_clicked();

    void on_Button_Exit_clicked();

    void replyFinished(QNetworkReply *);
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QSerialPort *myserial;//声明串口类，myserial是QSerialPort的实例
    bool serial_flag,start_flag;//定义两个标志位
    QByteArray alldata;//接收串口数据
    //绘图函数
    QDateTime mycurrenttime;//系统当前时间
    QDateTime mystarttime;//系统开始时间
    QByteArray mytemp; //接收数据

    QNetworkAccessManager *manager;
    QString fengli;       //风力
    QString wendu;        //温度
    QString weather_type;  //天气类型
};

#endif // WIDGET_H
