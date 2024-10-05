#include <QtGui>
#include <QFont>
#include <QtCore>
#include <wlog.h>
#include "maindlg.h"
#include <toolkit.h>
using namespace WSpace;

  void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	fprintf(stdout, "%s\n", localMsg.constData());
}

int main (int argc, char *argv[])
{
																																										       qInstallMessageHandler(myMessageOutput); 
	QApplication app(argc, argv);

	app.setApplicationName("NetWorkKit");
	const char filecode[]="UTF-8";
	QTextCodec::setCodecForLocale(QTextCodec::codecForName(filecode)); 

	WLogConfig config{{LL_DEBUG,LL_DEBUG}};
	WLog::init(config);
	MainDlg dlg;
	dlg.show();
	return app.exec();
}