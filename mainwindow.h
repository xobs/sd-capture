#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "networkconnection.h"
#include "controllerwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void doConnect();
	void networkConnected();
	void networkConnectionError(QAbstractSocket::SocketError);

private:
	Ui::MainWindow *ui;
	NetworkConnection networkConnection;
	ControllerWindow *controllerWindow;
};

#endif // MAINWINDOW_H
