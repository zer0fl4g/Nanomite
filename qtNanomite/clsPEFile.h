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

class clsPEFile
{
public:
	clsPEFile(std::wstring FileName,bool *bLoaded);
	~clsPEFile();

	bool isValidPEFile();

	QList<APIData> getImports();
	QList<APIData> getExports();

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
	QList<APIData> getExports32();
	QList<APIData> getExports64();

	bool LoadFile(std::wstring);
	bool CheckPEType(LPVOID pBuffer);

	DWORD dwCalculateTableOffset32(int iTableEntryNr,PIMAGE_NT_HEADERS32 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer);
	DWORD64 dwCalculateTableOffset64(int iTableEntryNr,PIMAGE_NT_HEADERS64 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer);
};

#endif