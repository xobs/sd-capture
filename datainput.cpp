#include "datainput.h"
#include "ui_datainput.h"
#include "qhexedit.h"

DataInput::DataInput(QWidget *parent) :
    QMainWindow(parent),
	ui(new Ui::DataInput),
	data(512, 0)
{
	ui->setupUi(this);
	ui->hexEdit->setData(data);
	ui->hexEdit->setReadOnly(false);
	ui->hexEdit->setOverwriteMode(true);

	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(doCancelData()));
	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(doSaveData()));
}

DataInput::~DataInput()
{
	delete ui;
}

void DataInput::doCancelData()
{
	emit cancelData();
}

void DataInput::doSaveData()
{
	data = ui->hexEdit->data();
	emit saveData(data);
}

void DataInput::setSize(quint32 size)
{
	data.resize(size);
}

void DataInput::clear()
{
	memset(data.data(), 0, data.size());
}
