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
#include "clsMemoryProtector.h"
#include "clsMemManager.h"

clsMemoryProtector::clsMemoryProtector(HANDLE processHandle, DWORD protectionNew, DWORD protectionSize, DWORD64 protectionOffset, bool *pWorked) : 
	m_processHandle(processHandle),
	m_protectionSize(protectionSize),
	m_protectionOffset(protectionOffset),
	m_protectionOld(NULL),
	m_worked(false)
{
	if(VirtualProtectEx(m_processHandle, (LPVOID)m_protectionOffset, protectionSize, protectionNew, &m_protectionOld))
	{
		*pWorked = m_worked = true;

		return;
	}

	*pWorked = m_worked = false;
}

clsMemoryProtector::~clsMemoryProtector()
{
	if(m_worked)
	{
		DWORD protectionTemp = NULL;

		VirtualProtectEx(m_processHandle, (LPVOID)m_protectionOffset, m_protectionSize, m_protectionOld, &protectionTemp);
	}
}
