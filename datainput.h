#ifndef DATAINPUT_H
#define DATAINPUT_H

#include <QMainWindow>
#include <QBuffer>

namespace Ui {
class DataInput;
}

class DataInput : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit DataInput(QWidget *parent = 0);
	~DataInput();
	void setSize(quint32 size);
	void clear();
	
private:
	Ui::DataInput *ui;
	QByteArray data;

public slots:
	void doSaveData();
	void doCancelData();

signals:
	void saveData(const QByteArray data);
	void cancelData();
};

#endif // DATAINPUT_H
