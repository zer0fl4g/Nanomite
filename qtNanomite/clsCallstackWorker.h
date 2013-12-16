/*
 * 	This file is part of Nanomite.
 *
 *    Nanomite is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nanomite is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Nanomite.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CLSCALLSTACKWORKER_H
#define CLSCALLSTACKWORKER_H

#include <QObject>
#include <QRunnable>
#include <QString>
#include <QList>

#include <Windows.h>

struct callstackData
{
	HANDLE	processHandle;
	DWORD	threadID;
	bool	isWOW64;
	CONTEXT	processContext;
	WOW64_CONTEXT wowProcessContext;
};

struct callstackDisplay
{
	quint64 stackAddress;
	quint64 returnOffset;
	QString returnFunctionName;
	QString returnModuleName;
	quint64 currentOffset;
	QString currentFunctionName;
	QString currentModuleName;
	QString sourceFilePath;
	int sourceLineNumber;
};

class clsCallstackWorker : public QObject, public QRunnable
{
	Q_OBJECT

public:
	clsCallstackWorker();
	~clsCallstackWorker();

	void setCallstackData(callstackData ctProcessingData);

signals:
	void OnCallstackFinished(QList<callstackDisplay> callstackDisplayData);

private:
	callstackData m_processingData;

protected:
	void run();
};

#endif