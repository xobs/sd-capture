#include <QDebug>
#include <QScrollBar>
#include "controllerwindow.h"
#include "ui_controllerwindow.h"
#include "datainput.h"

ControllerWindow::ControllerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ControllerWindow)
{
	ui->setupUi(this);

	connect(ui->resetCardButton, SIGNAL(clicked()), this, SLOT(addResetCommand()));
	connect(ui->setSectorButton, SIGNAL(clicked()), this, SLOT(addSetSectorCommand()));
	connect(ui->readCurrentSectorButton, SIGNAL(clicked()), this, SLOT(addReadSectorCommand()));
	connect(ui->writeSectorButton, SIGNAL(clicked()), this, SLOT(addWriteSectorCommand()));
	connect(ui->stopLoggingButton, SIGNAL(clicked()), this, SLOT(addStopLoggingCommand()));
	connect(ui->resumeLoggingButton, SIGNAL(clicked()), this, SLOT(addResumeLoggingCommand()));

	connect(ui->deleteLineButton, SIGNAL(clicked()), this, SLOT(removeScriptItem()));
	connect(ui->runScriptButton, SIGNAL(clicked()), this, SLOT(doRunScript()));
	connect(ui->addSyncButton, SIGNAL(clicked()), this, SLOT(addSyncCommand()));
	connect(ui->currentScript, SIGNAL(indexesMoved(QModelIndexList)), this, SLOT(indexesMoved(QModelIndexList)));

	connect(ui->readCIDButton, SIGNAL(clicked()), this, SLOT(addReadCIDCommand()));
	connect(ui->readCSDButton, SIGNAL(clicked()), this, SLOT(addReadCSDCommand()));

	dataInput = new DataInput(this);
	connect(dataInput, SIGNAL(cancelData()), this, SLOT(cancelWriteData()));
	connect(dataInput, SIGNAL(saveData(QByteArray)), this, SLOT(saveWriteData(QByteArray)));

	ui->currentScript->clear();
	ui->scriptProgress->setVisible(false);

	NetCommand ib;
	ib.setDisplayString(QString("Reset read offset"));
	ib.addCommand("ib 0");
	addCommand(ib);

	NetCommand rc;
	rc.setDisplayString(QString("Reset card"));
	rc.addCommand("rc");
	addCommand(rc);

	NetCommand rs;
	rs.setDisplayString(QString("Read sector 0"));
	rs.addCommand("rs");
	addCommand(rs);
}

ControllerWindow::~ControllerWindow()
{
	delete ui;
}

void ControllerWindow::appendTextToConsole(const QString &str, int index)
{
	ui->networkConsole->insertPlainText(str);
	ui->networkConsole->verticalScrollBar()->setValue(ui->networkConsole->verticalScrollBar()->maximum());
}

void ControllerWindow::highlightCommand(const QString &command, int position)
{
	ui->scriptProgress->setValue(position);
	ui->currentScript->setCurrentRow(position);
}

void ControllerWindow::networkScriptDone()
{
	ui->scriptProgress->setVisible(false);
	ui->currentScript->setCurrentRow(-1);
}

void ControllerWindow::removeScriptItem()
{
	cmds.removeAt(ui->currentScript->currentRow());
	ui->currentScript->takeItem(ui->currentScript->currentRow());
}

void ControllerWindow::doRunScript()
{
	ui->scriptProgress->setVisible(true);
	ui->scriptProgress->setMaximum(cmds.count());
	ui->scriptProgress->setValue(0);
	emit runScript(cmds);
}

/*--*/
void ControllerWindow::addResetCommand()
{
	NetCommand rc;
	rc.setDisplayString(QString("Reset card"));
	rc.addCommand("rc");
	addCommand(rc);
}

void ControllerWindow::addReadSectorCommand()
{
	NetCommand rs;
	rs.setDisplayString(QString("Read current sector"));
	rs.addCommand("rs");
	addCommand(rs);
}

void ControllerWindow::addIgnoreBytesCommand()
{
	NetCommand cmd;
	quint32 bytes = 0;
	QString command;
	command.sprintf("Set ignore offset to %d", bytes);
	cmd.setDisplayString(command);
	command.sprintf("ib %d", bytes);
	cmd.addCommand(command);
	addCommand(cmd);
}

void ControllerWindow::addSetSectorCommand()
{
	NetCommand so;
	quint32 sector = ui->setSectorText->text().toUInt();
	QString command;
	command.sprintf("Set sector offset to %d", sector);
	so.setDisplayString(command);
	command.sprintf("so %d", sector);
	so.addCommand(command);
	addCommand(so);
}

void ControllerWindow::addSyncCommand()
{
	NetCommand sync;
	sync.setDisplayString(QString("~Sync Point~"));
	sync.addCommand("ib 0");
	addCommand(sync);
}

void ControllerWindow::addReadCIDCommand()
{
	NetCommand cmd;
	cmd.setDisplayString(QString("Read CID"));
	cmd.addCommand("ci");
	addCommand(cmd);
}

void ControllerWindow::addReadCSDCommand()
{
	NetCommand cmd;
	cmd.setDisplayString(QString("Read CSD"));
	cmd.addCommand("cs");
	addCommand(cmd);
}

void ControllerWindow::addStopLoggingCommand()
{
	NetCommand cmd;
	cmd.setDisplayString(QString("Stop logging NAND"));
	cmd.addCommand("ib 2147483648");
	addCommand(cmd);
}

void ControllerWindow::addResumeLoggingCommand()
{
	NetCommand cmd;
	cmd.setDisplayString(QString("Continue logging NAND"));
	cmd.addCommand("ib 0");
	addCommand(cmd);
}

void ControllerWindow::addWriteSectorCommand()
{
	dataInput->show();
}

void ControllerWindow::addCommand(NetCommand &cmd)
{
	cmds.insert(ui->currentScript->currentRow()+1, cmd);
	ui->currentScript->insertItem(ui->currentScript->currentRow()+1, cmd.displayString());
	ui->currentScript->setCurrentRow(ui->currentScript->currentRow()+1);
}

void ControllerWindow::saveWriteData(const QByteArray &data)
{
	NetCommand setSector;
	setSector.setDisplayString("Write current sector");
	int i;
	setSector.addCommand("so 0");
	for (i=0; i<data.size(); i++) {
		QString cmd;
		cmd.sprintf("sb %d", data[i]);
		setSector.addCommand(cmd);
	}
	setSector.addCommand("ws");
	addCommand(setSector);
	cleanupWriteData();
}

void ControllerWindow::cancelWriteData()
{
	cleanupWriteData();
}

void ControllerWindow::cleanupWriteData()
{
	dataInput->close();
}

void ControllerWindow::indexesMoved(const QModelIndexList &indexes)
{
	qDebug() << "Indexes moved: " << indexes;
}
