#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
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
	QString logfilename;

	QFileDialog selectFile(ui->centralWidget);
	selectFile.setFileMode(QFileDialog::AnyFile);
	selectFile.setAcceptMode(QFileDialog::AcceptSave);
	selectFile.setNameFilter("Raw Tap Board capture files (*.tbraw)");
	selectFile.selectFile(QString("log-%1.tbraw").arg(QDateTime::currentDateTime().toString("yyMMddHHmm")));
	if (!selectFile.exec()) {
		qDebug() << "No file selected";
		return;
	}
	logfilename = selectFile.selectedFiles()[0];



	this->setVisible(false);
	controllerWindow = new ControllerWindow(this);
	controllerWindow->setVisible(true);

	connect(&networkConnection, SIGNAL(sendingCommand(QString&)), controllerWindow, SLOT(appendTextToConsole(QString&)));
	connect(controllerWindow, SIGNAL(runScript(QList<NetCommand>&)), &networkConnection, SLOT(runScript(QList<NetCommand>&)));

	networkConnection.setLogfile(logfilename);
	qDebug() << "Connecting to host:" << hostname << "and logfile" << logfilename;
	networkConnection.connectToHost(hostname);
}
