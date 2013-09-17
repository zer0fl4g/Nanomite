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
#ifndef CLSBREAKPOINTHARDWARE_H
#define CLSBREAKPOINTHARDWARE_H

#include <Windows.h>

class clsBreakpointHardware
{
public:
	static bool wHardwareBP(DWORD processID, DWORD64 breakpointOffset, DWORD breakpointSize, DWORD breakpointSlot, DWORD typeFlag);
	static bool dHardwareBP(DWORD processID, DWORD64 breakpointOffset, DWORD breakpointSlot);
};

#endif