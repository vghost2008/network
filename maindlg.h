_Pragma("once")
#include <QtWidgets>
#include <QtCore>
#include <wio/tcpclient.h>
#include <wio/tcpserver.h>
#include <wio/udpsocket.h>
enum NetWorkType
{
	NWT_TCP_CLIENT      ,   
	NWT_TCP_SERVER      ,   
	NWT_UDP_UNICAST     ,   
	NWT_UDP_MULTICAST   ,   
	NWT_UDP_BROADCAST   ,   
	NWT_NR              ,   
};
class MainDlg:public QDialog
{
	Q_OBJECT
	public:
		MainDlg();
	private:
		QLabel      *lb_info_;
		QCheckBox   *cb_hex_send_;
		QCheckBox   *cb_hex_recv_;
		QLineEdit   *le_local_address_;
		QLineEdit   *le_remote_address_;
		QLineEdit   *le_multicast_address_;
		QLineEdit   *le_local_port_;
		QLineEdit   *le_remote_port_;
		QLineEdit   *le_send_;
		QLineEdit   *le_timer_interval_;
		QTextEdit   *te_message_;
		QPushButton *btn_connect_;
		int          timer_id_             = -1;
	private:
		qint64      got_data_length_;
		qint64      send_data_length_;
		WTCPClient  tcp_client_;
		WBuffer     read_buffer_;
		WBuffer     write_buffer_;
		WTCPServer  tcp_server_;
		WUDPSocket  udp_;
		NetWorkType type_;
	private:
		bool eventFilter(QObject* obj,QEvent* event);
		void sendMsg(const QString& te_message_);
		void displayInfo(const QString& text,bool binit=false);
		void updateInfo();
		void timerEvent(QTimerEvent* event)override;
		void getArgument();
		WAbstractSocket* currentIODevice();
	public slots:
		void slotSendTextChanged(const QString& text);
		void slotReadyRead();
		void slotConnect();
		void slotSend();
		void slotClean();
		void slotNetworkTypeChanged(int type);
		void slotAutoSend(bool ischecked);
};
