#include "netcommand.h"

NetCommand::NetCommand(QObject *parent) :
    QObject(parent)
{
}

NetCommand::NetCommand(const NetCommand &other) :
	QObject(0)
{
	this->_cmds = other._cmds;
	this->_display = other._display;
}

NetCommand & NetCommand::operator =(const NetCommand &other)
{
	this->_cmds = other._cmds;
	this->_display = other._display;
	return *this;
}

const QList<QString> &NetCommand::commands() const
{
	return _cmds;
}

const QString & NetCommand::displayString() const
{
	return _display;
}

void NetCommand::addCommand(QString command)
{
	_cmds.append(command);
}

void NetCommand::setDisplayString(QString str)
{
	_display = str;
}
