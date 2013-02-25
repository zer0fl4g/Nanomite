#ifndef CLSPEFILE_H
#define CLSPEFILE_H

#include <Windows.h>
#include <string>
#include <vector>
#include <QStringlist.h>

class clsPEFile
{
public:
	clsPEFile(std::wstring FileName);
	~clsPEFile();

	bool isValidPEFile();

	QStringList clsPEFile::getImports();
	QStringList clsPEFile::getImports32();
	QStringList clsPEFile::getImports64();

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

	bool LoadFile(std::wstring);
	bool CheckPEType(LPVOID pBuffer);

	DWORD dwCalculateTableOffset32(int iTableEntryNr,PIMAGE_NT_HEADERS32 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer);
	DWORD64 dwCalculateTableOffset64(int iTableEntryNr,PIMAGE_NT_HEADERS64 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer);
};

#endif