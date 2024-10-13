#include "maindlg.h"
#include <QHostAddress>
#include <toolkit.h>
#include <QtDebug>
#include <iostream>
#include "trace_data.h"
#include "radar_sim_data.h"
using namespace std;
const QString kConnect    = QString::fromUtf8("连接");
const QString kDisconnect = QString::fromUtf8("断开连接");

MainDlg::MainDlg()
:read_buffer_(1024*1024)
,write_buffer_(1024*1024)
,got_data_length_(0)
,send_data_length_(0)
,tcp_client_(NWT_TCP_CLIENT)
,tcp_server_(NWT_TCP_SERVER)
,udp_(NWT_UDP_UNICAST)
,type_(NWT_TCP_SERVER)
{
	int            row             = 0;
	QVBoxLayout   *layout          = new QVBoxLayout(this);
	QGridLayout   *glayout0        = new QGridLayout;
	QHBoxLayout   *hlayout2        = new QHBoxLayout;
	QHBoxLayout   *hlayout3        = new QHBoxLayout;
	QPushButton   *btn_send        = new QPushButton("发送");
	QPushButton   *btn_clean       = new QPushButton("清空");
	QComboBox     *cb_network_type = new QComboBox;
	QCheckBox     *cb_auto_send    = new QCheckBox("自动发送");
	const QString  network_type[]  = {"TCP服务端","TCP客户端","UDP单播","UDP组播","UDP广播"};
	const QString  radar_type[]  = {"单通道","双通道","多通道"};
	QRegExp rx("[0-2]{0,1}\\d{0,2}(\\.[0-2]{0,1}\\d{1,2}){3}");

	for(const QString& type:network_type) {
		cb_network_type->addItem(type);
	}
	for(const QString& type:radar_type) {
		cb_radar_type_->addItem(type);
	}

	static_assert(ARRAY_SIZE(network_type)==NWT_NR,"Error network type number");

    le_local_address_       =   new QLineEdit("192.168.0.30");
    le_remote_address_      =   new QLineEdit("192.168.0.30");
    le_multicast_address_   =   new QLineEdit("224.100.1.1");
    le_local_port_          =   new QLineEdit("2000");
    le_remote_port_         =   new QLineEdit("2000");
	cb_hex_send_            =   new QCheckBox("十六进制发送");
	cb_hex_recv_            =   new QCheckBox("十六进制接收");
	btn_connect_            =   new QPushButton(kConnect);
	le_timer_interval_      =   new QLineEdit("1");
	le_send_                =   new QLineEdit;
	te_message_             =   new QTextEdit;
	lb_info_                =   new QLabel;

	le_local_address_->setValidator(new QRegExpValidator(rx,le_local_address_));
	le_remote_address_->setValidator(new QRegExpValidator(rx,le_remote_address_));
	le_multicast_address_->setValidator(new QRegExpValidator(rx,le_multicast_address_));
	le_local_port_->setValidator(new QIntValidator(le_local_port_));
	le_remote_port_->setValidator(new QIntValidator(le_remote_port_));
	le_timer_interval_->setValidator(new QIntValidator(0,0x0FFFFFFF,le_timer_interval_));
	le_timer_interval_->setMaximumWidth(100);
	lb_info_->setMinimumWidth(150);


	glayout0->addWidget(new QLabel("本地地址:"),row,0);
	glayout0->addWidget(le_local_address_,row,1);
	glayout0->addWidget(new QLabel("远程地址:"),row,2);
	glayout0->addWidget(le_remote_address_,row,3);
	glayout0->addWidget(new QLabel("组播地址:"),row,4);
	glayout0->addWidget(le_multicast_address_,row,5);

	++row;

	glayout0->addWidget(new QLabel("网络类型:"),row,0);
	glayout0->addWidget(cb_network_type,row,1);
	glayout0->addWidget(new QLabel("本地端口:"),row,2);
	glayout0->addWidget(le_local_port_,row,3);
	glayout0->addWidget(new QLabel("远程端口:"),row,4);
	glayout0->addWidget(le_remote_port_,row,5);

	++row;
	glayout0->addWidget(new QLabel("雷达型号:"),row,0);
	glayout0->addWidget(cb_radar_type_,row,1);

	hlayout2->addWidget(cb_hex_recv_);
	hlayout2->addWidget(cb_hex_send_);
	hlayout2->addWidget(btn_connect_);
	hlayout2->addWidget(btn_clean);
	hlayout2->addWidget(btn_send);
	hlayout3->addWidget(lb_info_);
	hlayout3->addStretch(0);
	hlayout3->addWidget(cb_auto_send);
	hlayout3->addWidget(le_timer_interval_);

	layout->addLayout(glayout0);
	layout->addWidget(te_message_);
	layout->addWidget(le_send_);
	layout->addLayout(hlayout3);
	layout->addLayout(hlayout2);

	qApp->installEventFilter(this);
	setWindowTitle("NetWorkKit");
	resize(900,520);
	slotClean();

	connect(btn_connect_,SIGNAL(clicked()),this,SLOT(slotConnect()));
	connect(le_send_,SIGNAL(textChanged(const QString&)),this,SLOT(slotSendTextChanged(const QString&)));
	connect(btn_send,SIGNAL(clicked()),this,SLOT(slotSend()));
	connect(btn_clean,SIGNAL(clicked()),this,SLOT(slotClean()));
	connect(cb_network_type,SIGNAL(currentIndexChanged(int)),this,SLOT(slotNetworkTypeChanged(int)));
	connect(cb_auto_send,SIGNAL(clicked(bool)),this,SLOT(slotAutoSend(bool)));
	connect(&tcp_client_,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));
	connect(&tcp_server_,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));
	connect(&udp_,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));
	le_send_->setText("BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAE");
    cb_hex_send_->setChecked(true);
    cb_hex_recv_->setChecked(true);

	slotConnect();
}
void MainDlg::slotSendTextChanged(const QString& text)
{
	if(text.right(1)=="\n") sendMsg(text.left(text.size()-1));
}
void MainDlg::slotConnect()
{
	WAbstractSocket* device = currentIODevice();
	if(NULL == device) return;
	if(btn_connect_->text() == kConnect) {
		getArgument();
		if(true == device->open()) 
			btn_connect_->setText(kDisconnect);
		else
			ERROR_LOG("Connect faild");
	} else {
		if(false == device->close()) {
			btn_connect_->setText(kConnect);
		} else {
			ERROR_LOG("Close faild");
		}
	}
}
void MainDlg::displayInfo(const QString& text,bool binit)
{
	QString str;
	te_message_->toPlainText();
	if(binit||te_message_->toPlainText().size()>16384)
		str=text;
	else
		str=te_message_->toPlainText()+"\n"+text;
	te_message_->setText(str);
	QScrollBar *scrollbar = te_message_->verticalScrollBar();
	scrollbar->setSliderPosition(scrollbar->maximum());
}
void MainDlg::sendMsg(const QString& message)
{
	WAbstractSocket* device = currentIODevice();
	if(NULL == device || !device->isValid()) return;
	QByteArray ba=message.toUtf8();
	if(cb_hex_send_->isChecked()) {
		ba=ba.replace(' ',"");
		int                     newsize  = ba.size()/2;
		unsigned short          ts;
		char                   *pdata    = ba.data();
		char                   *tempdata = new char[newsize+1];
		bool                    bok;
		for(int i=0;i<newsize;++i) {
			ts = QByteArray(pdata+i*2,2).toShort(&bok,16);
			memcpy(tempdata+i,(char*)(&ts),2);
		}
		write_buffer_.setData(tempdata,newsize);
	} else {
		write_buffer_.setData(ba.data(),ba.size());
	}
	send_data_length_ += write_buffer_.size();
	device->write(&write_buffer_);
	write_buffer_.clear();

	updateInfo();
}
void MainDlg::slotSend()
{
	const QString message = le_send_->text();
	sendMsg(message);
}
bool MainDlg::eventFilter(QObject* obj,QEvent* event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if(keyEvent->key()==Qt::Key_Enter||keyEvent->key()==Qt::Key_Return) {
			slotSend();
			return true;
		}

	} 
	return QObject::eventFilter(obj, event);
}
void MainDlg::slotClean()
{
	got_data_length_    =   0;
	send_data_length_   =   0;
	lb_info_->setText("RX:0, TX:0");
	te_message_->setText("");
}
void MainDlg::updateInfo()
{
	lb_info_->setText(QString("RX:%1, TX:%2, Delta=%3").arg(got_data_length_).arg(send_data_length_).arg(got_data_length_-send_data_length_));
}
void MainDlg::slotNetworkTypeChanged(int type)
{
	switch(type_) {
		case NWT_TCP_CLIENT:
			tcp_client_.close();
			break;
		case NWT_TCP_SERVER:
			tcp_server_.close();
			break;
		case NWT_UDP_UNICAST:
		case NWT_UDP_MULTICAST:
		case NWT_UDP_BROADCAST:
			udp_.close();
			break;
		default:
			SIMPLE_LOG(LL_ERROR,"Error type:%d",type_);
			break;
	}
	type_               =   static_cast<NetWorkType>(type);
	got_data_length_    =   0;
	send_data_length_   =   0;
	btn_connect_->setText(kConnect);
	getArgument();
}
void MainDlg::getArgument()
{
	WAbstractSocket *io     = currentIODevice();

	if(NULL == io) return;

	io->setLocalAddrAndPort(le_local_address_->text().toUtf8().data(),le_local_port_->text().toUInt());
	io->setRemoteAddrAndPort(le_remote_address_->text().toUtf8().data(),le_remote_port_->text().toUInt());
	io->setMulticastAddr(le_multicast_address_->text().toUtf8().data());

	switch(type_) {
		case NWT_TCP_CLIENT:
			break;
		case NWT_TCP_SERVER:
			break;
		case NWT_UDP_UNICAST:
			udp_.setUDPMode(UM_UNICAST);
			break;
		case NWT_UDP_MULTICAST:
			udp_.setUDPMode(UM_MULTICAST);
			break;
		case NWT_UDP_BROADCAST:
			udp_.setUDPMode(UM_BROADCAST);
			break;
		default:
			SIMPLE_LOG(LL_ERROR,"Error type:%d",type_);
			break;
	}
}
WAbstractSocket* MainDlg::currentIODevice()
{
	WAbstractSocket* device = NULL;
	switch(type_) {
		case NWT_TCP_CLIENT:
			device = &tcp_client_;
			break;
		case NWT_TCP_SERVER:
			device = &tcp_server_;
			break;
		case NWT_UDP_UNICAST:
		case NWT_UDP_MULTICAST:
		case NWT_UDP_BROADCAST:
			device = &udp_;
			break;
		default:
			SIMPLE_LOG(LL_ERROR,"Error type:%d",type_);
			break;
	}
	return device;
}
void MainDlg::slotReadyRead()
{
	WAbstractSocket* device = currentIODevice();
	if(NULL == device) return;
	device->read(&read_buffer_);
	if(read_buffer_.size() == 0) return;

	got_data_length_ += read_buffer_.size();
	QByteArray ba;
	if(cb_hex_recv_->isChecked()) {
		const QString   text = WSpace::toHexString((unsigned char*)(read_buffer_.data()),read_buffer_.size()).c_str();
		displayInfo(QTime::currentTime().toString("hh:mm:ss:zzz:")+QString("Get From %1, size=%2:\n").arg(device->id()).arg(read_buffer_.size())+text);
	} else {
		displayInfo(QTime::currentTime().toString("hh:mm:ss:zzz:")+QString("Get From %1, size=%2:\n").arg(device->id()).arg(read_buffer_.size())+QByteArray(read_buffer_.data(),read_buffer_.size()));
	}
	getData(read_buffer_);
	read_buffer_.clear();
	updateInfo();
}
void MainDlg::slotAutoSend(bool ischecked)
{
	if(timer_id_ > 0) {
		killTimer(timer_id_);
		timer_id_ = -1;
	}
	if(!ischecked) return;
	unsigned timer_interval = le_timer_interval_->text().toInt();
	timer_id_ = startTimer(timer_interval);
}
void MainDlg::timerEvent(QTimerEvent* event)
{
	if(event->timerId() == timer_id_) {
		//slotSend();
		if(auto_send_trace_)
			sendTrace();
		update();
	}
}
void MainDlg::sendTrace()
{
	auto device = currentIODevice();
	if(NULL == device) return;

	if(508== pts_) {
		WBuffer buffer((void*)onech_data,sizeof(onech_data));
		qDebug()<<"RTrace: pts="<<pts_<<", size="<<buffer.size()<<","<<sizeof(onech_data);
		device->write(&buffer);
		++counter_;
		return;
	}

	TraceData data;
	data.counter = counter_;

	WBuffer buffer = {data};
	auto A = 30000.0;
	for(auto i=0; i<pts_; ++i) {
		short v = A*sin((i+counter_)*M_PI/(50+i))*exp(-i/512.0);
		buffer.append(v);
	}

	qDebug()<<"Trace: pts="<<pts_<<", size="<<buffer.size();
	device->write(&buffer);
	++counter_;
}
void MainDlg::getData(WBuffer& data)
{
	if(data.size()<2) {
		qDebug()<<"Drop error data";
		data.dump();
		return;
	}
	auto cmd = data.castRef<uint16_t>();
	qDebug()<<"Get command"<<QString("%1").arg(cmd,4,16);
	auto addr = cmd&0xFF;
	switch(addr) {
		case 0x01:
			{
				auto value = 256*(1<<(cmd>>8))-4;
				qDebug()<<"set pts to "<<value;
				pts_ = value;
				data.right(data.size()-2);
				getData(data);
			}
			break;
	}
	switch(cmd) {
		case 0x0000:
			{
				if(timer_id_ > 0) {
					killTimer(timer_id_);
					timer_id_ = -1;
				}
				auto_send_trace_ = false;
				data.right(data.size()-2);
				getData(data);
			}
			break;
		case 0x0F00:
			{
				if(data.size()<4)
					return;
				auto cmd1 = data.castRef<uint16_t>(2);
				if(cmd1 == 0x0F00) 
					autoSendTrace();
				data.right(data.size()-4);
				getData(data);
			}
			break;
		case 0xAA00:
			{
				auto cmd1 = data.castRef<uint16_t>(2);
				if(cmd1 == 0x0000) 
					sendTrace();
				data.right(data.size()-4);
				getData(data);
			}
			break;
		case 0x88FA:
			sendRadarInfo();
			data.right(data.size()-2);
			getData(data);
			break;
		default:
			qDebug()<<"Unknow command"<<QString("%1").arg(cmd,4,16);
			data.right(data.size()-2);
			getData(data);
			break;
	}
}
void MainDlg::autoSendTrace()
{
	if(timer_id_ > 0) {
		killTimer(timer_id_);
		timer_id_ = -1;
	}
	qDebug()<<"Begin auto send trace.";
	unsigned timer_interval = le_timer_interval_->text().toInt();
	timer_id_ = startTimer(timer_interval);
	auto_send_trace_ = true;
}
void MainDlg::sendRadarInfo()
{
	auto device = currentIODevice();
	if(NULL == device) return;

	auto type = cb_radar_type_->currentIndex();
	WBuffer buffer;
	switch(type) {
		case 0:
		{
			unsigned char d[] = {0x53, 0x53, 0x49, 0x44, 0x3D, 0x53, 0x68, 0x69, 0x65, 0x6C, 0x64, 0x65, 0x64, 0x5F, 0x53, 0x31, 0x30, 0x30, 0x4D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};
			//buffer.append("SSID=Shielded_S800M;");
			cout<<d<<endl;
			buffer.append((const void*)d,sizeof(d));
		}
			break;
		case 1:
			buffer.append("SSID=Shielded_MC2;");
			break;
		case 2:
			buffer.append("SSID=MC8;LINK=NET; PORT=8; Designer=ZW; Version=20211017; PRF=300K; READY=1; PORTNAME1=S0200,5.12G; PORTNAME2=S0200,5.12G; PORTNAME3=S0200,5.12G; PORTNAME4=S0200,5.12G; PORTNAME5=S0200,5.12G; PORTNAME6=S0200,5.12G; PORTNAME7=S0200,5.12G; PORTNAME8=S0200,5.12G;");
			break;
	}
	device->write(&buffer);
}
