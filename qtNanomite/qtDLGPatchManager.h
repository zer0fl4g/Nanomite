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
#ifndef QTDLGPATCHMANAGER_H
#define QTDLGPATCHMANAGER_H

#include "ui_qtDLGPatchManager.h"

#include <Windows.h>

#include <QList>

struct PatchData
{
	quint64 Offset;
	quint64 BaseOffset;
	quint64 OldBaseOffset;
	int PID;
	int PatchSize;
	HANDLE hProc;
	PTCHAR ModuleName;
	LPVOID orgData;
	LPVOID newData;
	bool bWritten;
	bool bSaved;
};

class qtDLGPatchManager : public QWidget, public Ui_qtDLGPatchManagerClass
{
	Q_OBJECT

public:
	qtDLGPatchManager(QWidget *parent, Qt::WFlags flags);
	~qtDLGPatchManager();

	static bool AddNewPatch(int PID, HANDLE hProc, quint64 Offset, int PatchSize, LPVOID newData, bool bRewrite = false);
	static bool RemovePatch(int PID, quint64 Offset);
	static bool DeletePatch(int PID, quint64 Offset);
	static void ClearAllPatches();

signals:
	void OnReloadDebugger();
	void OnShowInDisassembler(quint64 Offset);

private:
	int _iSelectedRow;

	QList<PatchData> patches;
	
	static qtDLGPatchManager *pThis;

	bool WritePatchToProc(HANDLE hProc, quint64 Offset, int PatchSize, LPVOID DataToWrite, LPVOID OriginalData, bool bRemove = false);
	
	void UpdatePatchTable();

	private slots:
		void MenuCallback(QAction*);
		void UpdateOffsetPatch(HANDLE newOffset, int newPID);
		void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif
