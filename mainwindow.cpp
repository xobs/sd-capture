#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QErrorMessage>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
	ui(new Ui::MainWindow),
	networkConnection(this)
{
	ui->setupUi(this);
	connect(ui->hostAddress, SIGNAL(returnPressed()), ui->connectButton, SLOT(click()));
	connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(doConnect()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::doConnect()
{
	QString hostname = ui->hostAddress->text();



	this->setVisible(false);
	controllerWindow = new ControllerWindow(this);
	controllerWindow->setVisible(true);

	connect(controllerWindow, SIGNAL(runScript(QList<NetCommand>&)),
			&networkConnection, SLOT(runScript(QList<NetCommand>&)));
	connect(&networkConnection, SIGNAL(sendingCommand(QString,int)),
			controllerWindow, SLOT(appendTextToConsole(QString,int)));
	connect(&networkConnection, SIGNAL(sendingCommand(QString,int)),
			controllerWindow, SLOT(highlightCommand(QString,int)));
	connect(&networkConnection, SIGNAL(scriptComplete()),
			controllerWindow, SLOT(networkScriptDone()));

	networkConnection.connectToHost(hostname);
}

void MainWindow::networkConnectionError(QAbstractSocket::SocketError error)
{
	Q_UNUSED(error);
	QErrorMessage dialog;
	dialog.showMessage(QString("Unable to connect to server: %1").arg(networkConnection.socketErrorString()));
	dialog.exec();
	qApp->quit();
}

void MainWindow::networkConnected()
{
	QString logfilename;

	QFileDialog selectFile(controllerWindow->centralWidget());
	selectFile.setFileMode(QFileDialog::AnyFile);
	selectFile.setAcceptMode(QFileDialog::AcceptSave);
	selectFile.setNameFilter("Raw Tap Board capture files (*.tbraw)");
	selectFile.selectFile(QString("log-%1.tbraw").arg(QDateTime::currentDateTime().toString("yyMMdd-HHmm")));
	if (!selectFile.exec()) {
		qDebug() << "No file selected";
		qApp->quit();
	}
	logfilename = selectFile.selectedFiles()[0];
	networkConnection.setLogfile(logfilename);
}
