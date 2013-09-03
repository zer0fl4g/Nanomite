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
#ifndef CLSSTRINGVIEWWORKER_H
#define CLSSTRINGVIEWWORKER_H

#include <Windows.h>

#include <QThread>
#include <QList>

struct StringProcessingData
{
	PTCHAR filePath;
	int processID;
};

struct StringData
{
	QString DataString;
	DWORD64 StringOffset;
	int PID;
};

class clsStringViewWorker : public QThread
{
	Q_OBJECT

public:
	QList<StringData> stringList;
	
	clsStringViewWorker(QList<StringProcessingData> dataForProcessing);
	~clsStringViewWorker();

private:
	QList<StringProcessingData> m_processingData;

protected:
	void run();
};

#endif
