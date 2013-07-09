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
#ifndef QTDLGDETINFO_H
#define QTDLGDETINFO_H

#include "ui_qtDLGDetailInfo.h"

#include <Windows.h>
#include <string>

/**
* @file qtDLGDetailInfo.h
* @brief Displaying the detailinfo widget
*/
class qtDLGDetailInfo : public QWidget, public Ui_qtDLGDetailInfoClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the detailinfo widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGDetailInfo(QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGDetailInfo();

signals:
	/**
	* @brief A Qt signal which is send when the user wants to show a offset in the disassembler
	* @param Offset The offset to be displayed
	*
	* @return no
	*/
	void ShowInDisassembler(quint64 Offset);
	/**
	* @brief A Qt signal which is send when the user wants to show the file in the PEEditor
	* @param FileName The file name which should be opened
	* @param processID The process id which contains the file
	*
	* @return no
	*/
	void OpenFileInPEManager(std::wstring FileName,int processID);

public slots:
	/**
	* @brief A Qt slot which is called when the clsDebugger instance catched create/exit thread event
	* @param processID The process id of the thread
	* @param threadID The thread id of the thread
	* @param entrypointOffset The entrypoint of the thread
	* @param bSuspended Is this thread suspended or running
	* @param exitCode The exitcode of the thread
	* @param bFound Does this thread already exists ( true = exit thread event)
	*
	* @return no
	*/
	void OnThread(DWORD processID, DWORD threadID, quint64 entrypointOffset, bool bSuspended, DWORD exitCode, bool bFound);
	/**
	* @brief A Qt slot which is called when the clsDebugger instance catched create/exit process event
	* @param processID The process id
	* @param sFile The file which is running
	* @param exitCode The exitcode of the process
	* @param entrypointOffset The entrypoint of the process
	* @param bFound Does this process already exists ( true = exit process event)
	*
	* @return no
	*/
	void OnPID(DWORD processID, std::wstring sFile, DWORD exitCode, quint64 entrypointOffset, bool bFound);
	/**
	* @brief A Qt slot which is called when the clsDebugger instance catched exception
	* @param functionName A string containing the function name which created the exception
	* @param moduleName A string containing the module name in which the function is located
	* @param dwOffset The offset of the exception
	* @param exceptionCode The exception code
	* @param processID The process id which throwed the exception
	* @param threadID The thread id which throwed the exception
	*
	* @return no
	*/
	void OnException(std::wstring functionName, std::wstring moduleName, quint64 exceptionOffset, quint64 exceptionCode, DWORD processID, DWORD threadID);
	/**
	* @brief A Qt slot which is called when the clsDebugger instance catched load/unload dll event
	* @param sDLLPath The path of the dll
	* @param processID The process id which (un)loaded the dll
	* @param entrypointOffset The entrypoint of the dll
	* @param bLoaded Is this dll already loaded
	*
	* @return no
	*/
	void OnDll(std::wstring sDLLPath, DWORD processID, quint64 entrypointOffset, bool bLoaded);

private:
	int m_selectedRow; /* contains the selected row when user opens the context menu */

	quint64 m_selectedOffset; /* contains the selected offset when user opens the context menu */

	/**
	* @brief Set a thread priority
	* @param threadID ID of the thread
	* @param threadPrio the whiched priority
	*
	* @return bool true = success, false = fail
	*/
	bool SetThreadPriorityByTid(DWORD threadID, int threadPrio);
	/**
	* @brief Set a process priority
	* @param processID ID of the process
	* @param processPrio the whiched priority
	*
	* @return bool true = success, false = fail
	*/
	bool SetProcessPriorityByPid(DWORD processID, int processPrio);

	/**
	* @brief Get a thread priority
	* @param threadID ID of the thread from which the priority will be recived
	*
	* @return int The thread priority
	*/
	int GetThreadPriorityByTid(DWORD threadID);
	/**
	* @brief Get a process priority
	* @param processID ID of the process from which the priority will be recived
	*
	* @return int The process priority
	*/
	int GetProcessPriorityByPid(DWORD processID);

private slots:
	/**
	* @brief A Qt slot which is called when the user clicked on a element of the
	* context menu
	* @param pAction The selected element
	*
	* @return no
	*/
	void MenuCallback(QAction* pAction);
	/**
	* @brief A Qt slot which is called when the user clicked on a element of the
	* context menu for the PID Table
	* @param pAction The selected element
	*
	* @return no
	*/
	void PIDMenuCallback(QAction* pAction);
	/**
	* @brief A Qt slot which is called when the user opened the context menu of
	* the thread table
	* @param qPoint The position where the user clicked
	*
	* @return no
	*/
	void OnCustomTIDContextMenu(QPoint qPoint);
	/**
	* @brief A Qt slot which is called when the user opened the context menu of
	* the process table
	* @param qPoint The position where the user clicked
	*
	* @return no
	*/
	void OnCustomPIDContextMenu(QPoint qPoint);
	/**
	* @brief A Qt slot which is called when the user opened the context menu of
	* the exception table
	* @param qPoint The position where the user clicked
	*
	* @return no
	*/
	void OnCustomExceptionContextMenu(QPoint qPoint);
	/**
	* @brief A Qt slot which is called when the user opened the context menu of
	* the module table
	* @param qPoint The position where the user clicked
	*
	* @return no
	*/
	void OnCustomModuleContextMenu(QPoint qPoint);
};

#endif
