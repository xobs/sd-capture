#ifndef NETCOMMAND_H
#define NETCOMMAND_H

#include <QObject>

class NetCommand : public QObject
{
	Q_OBJECT
public:
	explicit NetCommand(QObject *parent = 0);
	NetCommand(const NetCommand &other);
	NetCommand &operator =(const NetCommand &other);

	const QList<QString> &commands() const;
	const QString & displayString() const;
	void addCommand(QString command);
	void setDisplayString(QString str);

private:
	QList<QString> _cmds;
	QString _display;

signals:
	
public slots:
	
};

#endif // NETCOMMAND_H
