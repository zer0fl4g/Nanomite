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
#ifndef CLSBREAKPOINTMANAGER_H
#define CLSBREAKPOINTMANAGER_H

#include "clsBreakpointHardware.h"
#include "clsBreakpointMemory.h"
#include "clsBreakpointSoftware.h"

#include <Windows.h>
#include <QObject>
#include <QList>
	 
struct BPStruct
{
	DWORD dwSize;
	DWORD dwSlot;
	DWORD dwTypeFlag;	/* see BP_BREAKON		*/
	DWORD dwHandle;		/* see BREAKPOINT_TYPE	*/
	DWORD dwOldProtection;
	quint64 dwOffset;
	quint64 dwBaseOffset;
	quint64 dwOldOffset;
	int dwPID;
	BYTE bOrgByte;
	bool bRestoreBP;
	PTCHAR moduleName;
};

class clsBreakpointManager : public QObject
{
	Q_OBJECT

public:
	QList<BPStruct> SoftwareBPs;
	QList<BPStruct> MemoryBPs;
	QList<BPStruct> HardwareBPs;

	clsBreakpointManager();
	~clsBreakpointManager();

	bool BreakpointRemove(DWORD64 breakpointOffset, DWORD breakpointType);
	bool BreakpointClear();
	bool BreakpointAdd(DWORD breakpointType, DWORD typeFlag, DWORD processID, DWORD64 breakpointOffset, DWORD breakpointHandleType);
	bool BreakpointInit(DWORD processID, bool isThread = false);
	bool BreakpointFind(DWORD64 breakpointOffset, int breakpointType, DWORD processID, bool takeAll, BPStruct** pBreakpointSearched);

	void BreakpointCleanup();
	void BreakpointUpdateOffsets();

	static bool IsOffsetAnBP(quint64 Offset);
	static bool BreakpointInsert(DWORD breakpointType, DWORD typeFlag, DWORD processID, DWORD64 breakpointOffset, DWORD breakpointHandleType);
	static bool BreakpointDelete(DWORD64 breakpointOffset, DWORD breakpointType);

	static void RemoveSBPFromMemory(bool isDisable, DWORD processID);

signals:
	void OnBreakpointAdded(BPStruct newBreakpoint, int breakpointType);
	void OnBreakpointDeleted(quint64 bpOffset);

private: 
	static clsBreakpointManager *pThis;
};

#endif