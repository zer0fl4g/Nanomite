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
#ifndef CLSPEFILE_H
#define CLSPEFILE_H

#include <Windows.h>
#include <string>
#include <vector>
#include <QStringlist.h>

struct APIData
{
	QString APIName;
	quint64 APIOffset;
};

struct PESectionData
{
	QString SectionName;
	quint64 VirtualSize;
	quint64 VirtualAddress;
	quint64 SizeOfRawData;
	quint64 PointerToRawData;
	quint64 Characteristics;
};

class clsPEFile
{
public:
	clsPEFile(std::wstring FileName,bool *bLoaded);
	~clsPEFile();

	bool isValidPEFile();

	QList<APIData> getImports();
	QList<APIData> getExports();
	QList<PESectionData> getSections();

	PIMAGE_DOS_HEADER getDosHeader();
	PIMAGE_NT_HEADERS32 getNTHeader32();
	PIMAGE_NT_HEADERS64 getNTHeader64();

	bool is64Bit();

private:
	std::wstring _FileName;

	LPVOID _lpBuffer;

	bool _is64Bit;

	PIMAGE_DOS_HEADER _pIDH;
	PIMAGE_NT_HEADERS64 _pINH64;
	PIMAGE_NT_HEADERS32 _pINH32;

	QList<APIData> getImports32();
	QList<APIData> getImports64();

	bool LoadFile(std::wstring);
	bool CheckPEType(LPVOID pBuffer);

	DWORD dwCalculateTableOffset32(int iTableEntryNr,PIMAGE_NT_HEADERS32 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer);
	DWORD64 dwCalculateTableOffset64(int iTableEntryNr,PIMAGE_NT_HEADERS64 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer);
};

#endif