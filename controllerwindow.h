#ifndef CONTROLLERWINDOW_H
#define CONTROLLERWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>
#include "netcommand.h"

namespace Ui {
class ControllerWindow;
}

class DataInput;

class ControllerWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit ControllerWindow(QWidget *parent = 0);
	~ControllerWindow();
	
public slots:
	void appendTextToConsole(const QString &str, int index);
	void removeScriptItem();
	void doRunScript();

	void addResetCommand();
	void addReadSectorCommand();
	void addIgnoreBytesCommand();
	void addSetSectorCommand();
	void addSyncCommand();
	void addWriteSectorCommand();
	void addReadCSDCommand();
	void addReadCIDCommand();
	void addStopLoggingCommand();
	void addResumeLoggingCommand();

	void saveWriteData(const QByteArray &data);
	void cancelWriteData();

	void indexesMoved(const QModelIndexList &indexes);

	void highlightCommand(const QString &command, int position);
	void networkScriptDone();

signals:
	void runScript(QList<NetCommand> &strings);

private:
	Ui::ControllerWindow *ui;
	QList<NetCommand> cmds;
	DataInput *dataInput;

	void cleanupWriteData();
	void addCommand(NetCommand &cmd);
};

#endif // CONTROLLERWINDOW_H
