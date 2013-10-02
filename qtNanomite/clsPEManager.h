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
#ifndef CLSPEMANAGER_H
#define CLSPEMANAGER_H

#include <QtCore>

#include "clsPEFile.h"

class clsPEManager: public QObject
{
	Q_OBJECT
public:
	static QList<APIData> getImportsFromFile(QString FileName);

	static clsPEManager* GetInstance();

	static QList<DWORD64> getTLSCallbackOffset(QString FileName,int PID);

	clsPEManager();
	~clsPEManager();

	bool OpenFile(QString FileName,int PID = -1);
	bool isValidPEFile(QString FileName,int PID = 0);
	bool is64BitFile(QString FileName,int PID = 0);

	DWORD64 VAtoRaw(QString FileName,int PID, DWORD64 RVAOffset);

	QList<APIData> getImports(QString FileName,int PID = -1);
	QList<APIData> getExports(QString FileName,int PID = -1);
	
	QList<IMAGE_SECTION_HEADER> getSections(QString FileName,int PID = -1);

	IMAGE_DOS_HEADER getDosHeader(QString FileName,int PID = -1);
	IMAGE_NT_HEADERS32 getNTHeader32(QString FileName,int PID = -1);
	IMAGE_NT_HEADERS64 getNTHeader64(QString FileName,int PID = -1);

	float getEntropie(QString FileName, int PID = -1);

	QString getFilenameFromPID(int PID);

	public slots:
		void InsertPIDForFile(QString,int);
		void CloseFile(QString,int);
		void CleanPEManager();

private:
	static clsPEManager *pThis;

	struct PEManager
	{
		clsPEFile *PEFile;
		int PID;
		bool is64Bit;
		QString FileName;
	};

	QList<PEManager> PEFiles;
};

#endif