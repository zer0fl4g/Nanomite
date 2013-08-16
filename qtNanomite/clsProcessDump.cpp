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

#include "clsProcessDump.h"
#include "clsMemManager.h"

#include "dbghelp.h"

clsProcessDump::clsProcessDump(HANDLE processHandle, DWORD processID, QString filePath, QWidget *pParent)
{
	QString dumpFilePath = QFileDialog::getSaveFileName(pParent,
		"Please select a path to save the dump",
		QString("DUMP_%1_%2.dmp").arg(filePath).arg(processID,6,10,QChar('0')),
		"Dump files (*.dmp)",
		NULL,
		QFileDialog::DontUseNativeDialog);

	if(dumpFilePath.length() <= 0) return;

	HANDLE dumpFile = CreateFileW(dumpFilePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 

	if(dumpFile != INVALID_HANDLE_VALUE) 
	{
		MINIDUMP_TYPE dumpTyp   = (MINIDUMP_TYPE)(	MiniDumpWithFullMemoryInfo				| 
													MiniDumpWithIndirectlyReferencedMemory	|
													MiniDumpWithFullMemory					|
													MiniDumpWithHandleData					|	 
													MiniDumpWithProcessThreadData			|
													MiniDumpWithThreadInfo					| 
													MiniDumpWithUnloadedModules	); 

		if(MiniDumpWriteDump(processHandle, processID, dumpFile, dumpTyp, NULL, NULL, NULL))
			QMessageBox::information(pParent,"Nanomite","Process dump was successfull!",QMessageBox::Ok,QMessageBox::Ok);
	
		CloseHandle(dumpFile); 
		return;
	}

	QMessageBox::critical(pParent,"Nanomite","Process dump has failed!",QMessageBox::Ok,QMessageBox::Ok);
}

clsProcessDump::~clsProcessDump()
{

}