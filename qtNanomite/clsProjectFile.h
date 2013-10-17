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
#ifndef CLSPROJECTFILE_H
#define CLSPROJECTFILE_H

#include "qtDLGNanomite.h"

#include <QString>

class clsProjectFile
{
public:
	clsProjectFile(bool isSaveFile, bool *pStartDebugging = NULL);
	~clsProjectFile();

private:
	qtDLGNanomite *m_pMainWindow;

	bool WriteDataToFile(const QString &saveFilePath);
	bool ReadDataFromFile(const QString &loadFilePath);

	void WritePatchDataToFile(QXmlStreamWriter &xmlWriter);
	void WriteBookmarkDataToFile(QXmlStreamWriter &xmlWriter);
	void WriteDebugDataToFile(QXmlStreamWriter &xmlWriter);
	void WriteBreakpointDataToFile(QXmlStreamWriter &xmlWriter);
	void WriteBreakpointListToFile(QList<BPStruct> &tempBP, int bpType, QXmlStreamWriter &xmlWriter);

	bool ReadDebugDataFromFile(QXmlStreamReader &xmlReader);

	void ReadBookmarkDataFromFile(QXmlStreamReader &xmlReader);
	void ReadBreakpointDataFromFile(QXmlStreamReader &xmlReader);
	void ReadPatchDataFromFile(QXmlStreamReader &xmlReader);
};

#endif