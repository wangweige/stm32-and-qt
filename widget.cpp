#include "widget.h"
#include "ui_widget.h"
#include<QMovie>
#include<QMessageBox>
#include <QUrl>
#include<QPixmap>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->label_background->setStyleSheet("background-image: url(:/picture/relax.jpg)");
    manager = new QNetworkAccessManager(this);  //新建QNetworkAccessManager对象
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));//关联信号和槽

//    ui->label_gif->setScaledContents(true);
//    QMovie *movie = new QMovie(":/picture/01.gif");
//    ui->label_gif->setMovie(movie);
//    movie->start();
    myserial = new QSerialPort();
    serial_flag = true;
    start_flag = true;
    setupPlot();//图形界面初始化函数

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_Button_Search_clicked()
{
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())//将每一个可用端口名打印出来，在ComboBox上显示
        {
            myserial->setPort(info);//这里相当于自动识别串口号之后添加到了comboBox，如果要手动选择可以用下面列表的方式添加进去
            if(myserial->open(QIODevice::ReadWrite))//打开串口，如果成功返回1
             {
              ui->comboBox->addItem(myserial->portName());//将串口号添加到cmb
              myserial->close();//关闭串口等待人为(打开串口按钮)打开
             }
         }
}

void Widget::on_Button_Open_clicked()
{
    if(serial_flag)//如果串口已经是打开的情况
        {
            ui->comboBox->setDisabled(true); //禁止修改串口
            myserial->setPortName(ui->comboBox->currentText()); //设置串口号
            myserial->setBaudRate(QSerialPort::Baud115200); //设置波特
            myserial->setDataBits(QSerialPort::Data8); //设置数据位数
            myserial->setParity(QSerialPort::NoParity);//设置奇偶校验
            myserial->setStopBits(QSerialPort::OneStop);//设置停止位
            myserial->setFlowControl(QSerialPort::NoFlowControl);//非流控制
            if(myserial->open(QIODevice::ReadWrite))//打开串口,如果成功返回1
            {
                connect(myserial,&QSerialPort::readyRead,this,&Widget::AnalyzeData);
                mystarttime = QDateTime::currentDateTime();//图像横坐标初始值参考点，读取初始时间
                qDebug()<<"串口打开成功";
            }
            else
            {
                qDebug()<<"串口打开失败";
                QMessageBox::warning(this,tr("waring"),tr("串口打开失败"),QMessageBox::Close);
            }
            ui->Button_Open->setText("关闭串口");
            serial_flag = false;//串口标志位置失效
        }
        else
        {
            ui->comboBox->setEnabled(true);//串口号下拉按钮使能工作
            myserial->close();
            ui->Button_Open->setText("打开串口");//按钮显示“打开串口”
            serial_flag = true;//串口标志位置工作
}
}

void Widget::AnalyzeData()
{


    QByteArray arr = myserial->readAll();//定义arr为串口读取的所有数据
    //qDebug() << "arr=" << arr;
    mytemp += arr;
    for (int i=0;i<mytemp.size();i++) {
        //qDebug()<<"arr[i]"<<mytemp[i]<<" i="<<i;
        if (mytemp[i] == '\n') {
            qDebug()<<"mytemp:"<<mytemp;
            QString StrI1=tr(mytemp.mid(mytemp.indexOf("T")+5,2));//自定义了简单协议，通过前面字母读取需要的数据
            QString StrI2=tr(mytemp.mid(mytemp.indexOf("H")+5,2));

            qDebug()<<"StrI1"<<StrI1;
            qDebug()<<"StrI2"<<StrI2;

            ui->line_Temp->setText(StrI1);//显示读取温度值
            ui->line_Humi->setText(StrI2);//显示读取湿度值

            float dataI1=StrI1.toFloat();//将字符串转换成float类型进行数据处理
            float dataI2=StrI2.toFloat();//将字符串转换成float类型进行数据处理
            if(dataI1 > 20)
            {
                ui->label_background->setStyleSheet("background-image: url(:/picture/hot.jpg)");
            }
            else
            {
                ui->label_background->setStyleSheet("background-image: url(:/picture/relax.jpg)");
            }
            mycurrenttime = QDateTime::currentDateTime();//获取系统时间

            double xzb = mystarttime.msecsTo(mycurrenttime)/1000.0;//获取横坐标，相对时间就是从0开始

            ui->widget_plot->graph(0)->addData(xzb,dataI1);//添加数据1到曲线1
            ui->widget_plot->graph(1)->addData(xzb,dataI2);//添加数据1到曲线2

            if(xzb>30)
                   {
                       ui->widget_plot->xAxis->setRange((double)qRound(xzb-30),xzb);//设定x轴的范围
                   }
                   else ui->widget_plot->xAxis->setRange(0,30);//设定x轴的范围
                   ui->widget_plot->replot();//每次画完曲线一定要更新显示

            mytemp = "";//清空，下一次接收
            break;
        }
    }



}

void Widget::setupPlot()
{
    //设置曲线一,温度曲线
        ui->widget_plot->addGraph();//添加一条曲线
        QPen pen;
        pen.setWidth(1);//设置画笔线条宽度
        pen.setColor(Qt::red);
        ui->widget_plot->graph(0)->setPen(pen);//设置画笔颜色
        ui->widget_plot->graph(0)->setBrush(QBrush(QColor(0, 0, 25, 20))); //设置曲线画刷背景
        ui->widget_plot->graph(0)->setName("温度/℃");//设置曲线名字
        ui->widget_plot->graph(0)->setAntialiasedFill(false);
        ui->widget_plot->graph(0)->setLineStyle((QCPGraph::LineStyle)1);//曲线画笔
        ui->widget_plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone,5));//曲线形状


        ui->widget_plot->addGraph();//添加一条曲线，湿度曲线
        pen.setColor(Qt::blue);
        ui->widget_plot->graph(1)->setPen(pen);//设置画笔颜色
        ui->widget_plot->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20))); //设置曲线画刷背景
        ui->widget_plot->graph(1)->setName("湿度/%");//设置曲线名字
        ui->widget_plot->graph(1)->setAntialiasedFill(false);
        ui->widget_plot->graph(1)->setLineStyle((QCPGraph::LineStyle)1);//曲线画笔
        ui->widget_plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone,5));//曲线形状

        //设置图表
        ui->widget_plot->xAxis->setLabel(QStringLiteral("时间/s"));//设置x坐标轴名称
        ui->widget_plot->xAxis->setLabelColor(QColor(20,20,20));//设置x坐标轴名称颜色
        ui->widget_plot->xAxis->setAutoTickStep(false);//设置是否自动分配刻度间距
        ui->widget_plot->xAxis->setTickStep(2);//设置刻度间距5
        ui->widget_plot->xAxis->setRange(0,30);//设定x轴的范围

        ui->widget_plot->yAxis->setLabel(QStringLiteral("TEMP & HUMI"));//设置y坐标轴名称
        ui->widget_plot->yAxis->setLabelColor(QColor(20,20,20));//设置y坐标轴名称颜色
        ui->widget_plot->yAxis->setAutoTickStep(false);//设置是否自动分配刻度间距
        ui->widget_plot->yAxis->setTickStep(10);//设置刻度间距1
        ui->widget_plot->yAxis->setRange(0,100);//设定y轴范围

        ui->widget_plot->axisRect()->setupFullAxesBox(true);//设置缩放，拖拽，设置图表的分类图标显示位置
        ui->widget_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom| QCP::iSelectAxes);// 轴、图例、图表可以被选择，并且是多选的方式
        ui->widget_plot->axisRect()->insetLayout()->setInsetAlignment(0,Qt::AlignTop | Qt::AlignRight);//图例显示位置右上
        ui->widget_plot->legend->setVisible(true);//显示图例

        ui->widget_plot->replot();
}



void Widget::on_Button_Exit_clicked()
{
    this->close();//关闭窗口
}

void Widget::replyFinished(QNetworkReply *reply)  //天气数据处理槽函数
{
    qDebug()<<"recv weather data!!";
    QString all = reply->readAll();
    //ui->textEdit->setText(all); //将接收到的数据显示出来

    QJsonParseError err;
    QJsonDocument json_recv = QJsonDocument::fromJson(all.toUtf8(),&err);//解析json对象
    qDebug() << err.error;
    if(!json_recv.isNull())
    {
        QJsonObject object = json_recv.object();

        if(object.contains("data"))
        {
            QJsonValue value = object.value("data");  // 获取指定 key 对应的 value
            if(value.isObject())
            {
                QJsonObject object_data = value.toObject();
                if(object_data.contains("forecast"))
                {
                    QJsonValue value = object_data.value("forecast");
                    if(value.isArray())
                    {
                        QJsonObject today_weather = value.toArray().at(0).toObject();
                        weather_type = today_weather.value("type").toString();

                        QString low = today_weather.value("low").toString();
                        QString high = today_weather.value("high").toString();
                        wendu = low.mid(low.length()-3,4) +"~"+ high.mid(high.length()-3,4);
                        QString strength = today_weather.value("fengli").toString();
                        strength.remove(0,8);
                        strength.remove(strength.length()-2,2);
                        fengli = today_weather.value("fengxiang").toString() + strength;
                        ui->type->setText(weather_type); //显示天气类型
                        ui->wendu->setText(wendu);   //显示温度
                        ui->fengli->setText(fengli); //显示风力
                    }
                }
            }
        }

    }else
    {
        qDebug()<<"json_recv is NULL or is not a object !!";
    }
    reply->deleteLater(); //销毁请求对象
}

void Widget::on_pushButton_clicked() //点击查询请求天气数据
{
    QString local_city = ui->lineEdit->text().trimmed(); //获得需要查询天气的城市名称
    char quest_array[256]="http://wthrcdn.etouch.cn/weather_mini?city=";
    QNetworkRequest quest;
    sprintf(quest_array,"%s%s",quest_array,local_city.toUtf8().data());
    quest.setUrl(QUrl(quest_array));
    quest.setHeader(QNetworkRequest::UserAgentHeader,"RT-Thread ART");
    /*发送get网络请求*/
    manager->get(quest);
}
