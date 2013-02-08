#ifndef CONTROLLERWINDOW_H
#define CONTROLLERWINDOW_H

#include <QMainWindow>

namespace Ui {
class ControllerWindow;
}

class ControllerWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit ControllerWindow(QWidget *parent = 0);
	~ControllerWindow();
	
public slots:
	void appendTextToConsole(QString &str);
	void removeScriptItem();
	void doRunScript();
	void changeScriptSelection(int newItem);

	void addResetCommand();
	void addReadSectorCommand();

signals:
	void runScript(QStringList &strings);

private:
	Ui::ControllerWindow *ui;
};

#endif // CONTROLLERWINDOW_H
