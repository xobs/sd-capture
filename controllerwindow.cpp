#include <QDebug>
#include "controllerwindow.h"
#include "ui_controllerwindow.h"

ControllerWindow::ControllerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ControllerWindow)
{
	ui->setupUi(this);
	connect(ui->currentScript, SIGNAL(currentRowChanged(int)), this, SLOT(changeScriptSelection(int)));
	connect(ui->deleteLineButton, SIGNAL(clicked()), this, SLOT(removeScriptItem()));
	connect(ui->resetCardButton, SIGNAL(clicked()), this, SLOT(addResetCommand()));
	connect(ui->readCurrentSectorButton, SIGNAL(clicked()), this, SLOT(addReadSectorCommand()));

	connect(ui->runScriptButton, SIGNAL(clicked()), this, SLOT(doRunScript()));
}

ControllerWindow::~ControllerWindow()
{
	delete ui;
}

void ControllerWindow::appendTextToConsole(QString &str)
{
	ui->networkConsole->insertPlainText(str);
}

void ControllerWindow::removeScriptItem()
{
	ui->currentScript->takeItem(ui->currentScript->currentRow());
}

void ControllerWindow::changeScriptSelection(int newItem)
{
}

void ControllerWindow::doRunScript()
{
	QStringList script;
	int i;
	for (i=0; i<ui->currentScript->count(); i++)
		script.append(ui->currentScript->item(i)->text());

	emit runScript(script);
}

/*--*/
void ControllerWindow::addResetCommand()
{
	ui->currentScript->insertItem(ui->currentScript->currentRow()+1, "rc");
}

void ControllerWindow::addReadSectorCommand()
{
	ui->currentScript->insertItem(ui->currentScript->currentRow()+1, "rs");
}
