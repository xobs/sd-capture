#include <QDebug>
#include <QDateTime>
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
	QDateTime now = QDateTime::currentDateTime();
	QString nowString = now.toString("yyMMddHHmmss");
	QString hostname = ui->hostAddress->text();
	QString logfilename = "log-" + hostname + "-" + nowString;

	this->setVisible(false);
	controllerWindow = new ControllerWindow(this);
	controllerWindow->setVisible(true);

	connect(&networkConnection, SIGNAL(gotCommandLineText(QString&)), controllerWindow, SLOT(appendTextToConsole(QString&)));
	connect(&networkConnection, SIGNAL(sendingCommand(QString&)), controllerWindow, SLOT(appendTextToConsole(QString&)));
	connect(controllerWindow, SIGNAL(runScript(QStringList&)), &networkConnection, SLOT(runScript(QStringList&)));

	networkConnection.setLogfile(logfilename);
	qDebug() << "Connecting to host:" << hostname << "and logfile" << logfilename;
	networkConnection.connectToHost(hostname);
}
