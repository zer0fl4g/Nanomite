#include "clsPEFile.h"

using namespace std;

clsPEFile::clsPEFile(wstring FileName,bool *bLoaded)
{
	_FileName = FileName;
	*bLoaded = LoadFile(_FileName);
}

clsPEFile::~clsPEFile()
{
	UnmapViewOfFile(_lpBuffer);
	_FileName.erase();
	//free(_lpBuffer);
}

bool clsPEFile::LoadFile(wstring FileName)
{
	HANDLE hFile = CreateFileW(FileName.c_str(),GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) return false;
		
	HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,NULL,NULL,NULL);
	_lpBuffer = MapViewOfFile(hFileMap,FILE_MAP_READ,NULL,NULL,0x2000);
	if(_lpBuffer == NULL)
	{
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		return false;
	}
			
	_pIDH = (PIMAGE_DOS_HEADER)_lpBuffer;
	if(_pIDH->e_magic != IMAGE_DOS_SIGNATURE)
	{
		UnmapViewOfFile(_lpBuffer);
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		return false;
	}
	
	_is64Bit = CheckPEType(_lpBuffer);
	if(_is64Bit)
	{
		_pINH64 = (PIMAGE_NT_HEADERS64)((DWORD)_pIDH + _pIDH->e_lfanew);
		if((_pINH64 != NULL && _pINH64->Signature != IMAGE_NT_SIGNATURE) || _pINH64->FileHeader.Characteristics & IMAGE_FILE_DLL)
		{
			UnmapViewOfFile(_lpBuffer);
			CloseHandle(hFile);
			CloseHandle(hFileMap);
			return false;
		}
	}
	else
	{
		_pINH32 = (PIMAGE_NT_HEADERS32)((DWORD)_pIDH + _pIDH->e_lfanew);
		if((_pINH32 != NULL && _pINH32->Signature != IMAGE_NT_SIGNATURE) || _pINH32->FileHeader.Characteristics & IMAGE_FILE_DLL)
		{
			UnmapViewOfFile(_lpBuffer);
			CloseHandle(hFile);
			CloseHandle(hFileMap);
			return false;
		}
	}

	CloseHandle(hFile);
	CloseHandle(hFileMap);
	return true;
}

bool clsPEFile::isValidPEFile()
{
	if(_is64Bit)
	{
		if(_pINH64 == NULL || _pINH64->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) 
		{
			return false;
		}
	}
	else
	{
		if(_pINH32 == NULL || _pINH32->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) 
		{
			return false;
		}

	}
	return true;
}

QStringList clsPEFile::getImports()
{
	QStringList importsOfFile;
	HANDLE hFile = CreateFileW(_FileName.c_str(),GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) return importsOfFile;

	HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,NULL,NULL,NULL);
	LPVOID lpOrgBuffer = _lpBuffer;

	_lpBuffer = MapViewOfFile(hFileMap,FILE_MAP_READ,NULL,NULL,NULL);
	if(_lpBuffer == NULL)
	{
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		UnmapViewOfFile(_lpBuffer);
		_lpBuffer = lpOrgBuffer;
		return importsOfFile;
	}

	if(_is64Bit)
		importsOfFile = getImports64();
	else
		importsOfFile = getImports32();

	UnmapViewOfFile(_lpBuffer);
	_lpBuffer = lpOrgBuffer;
	CloseHandle(hFile);
	CloseHandle(hFileMap);
	return importsOfFile;
}

QStringList clsPEFile::getImports32()
{
	QStringList importsOfFile;

	DWORD dwVAOfImportSection = _pINH32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD dwImportSectionOffset = dwCalculateTableOffset32(IMAGE_DIRECTORY_ENTRY_IMPORT,_pINH32,_pIDH,(PBYTE)_lpBuffer);
	
	if(dwVAOfImportSection != 0)
	{
		PIMAGE_IMPORT_DESCRIPTOR pImportHeader = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)dwImportSectionOffset);
		do
		{
			PIMAGE_THUNK_DATA32 pIAT = NULL;

			if(pImportHeader->OriginalFirstThunk != 0)
				pIAT = (PIMAGE_THUNK_DATA32)((pImportHeader->OriginalFirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
            else
                pIAT = (PIMAGE_THUNK_DATA32)((pImportHeader->FirstThunk - dwVAOfImportSection) + dwImportSectionOffset);

			while (pIAT->u1.Ordinal != 0 && !(pIAT->u1.Ordinal & IMAGE_ORDINAL_FLAG))
			{
				PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((pIAT->u1.Function - dwVAOfImportSection) + dwImportSectionOffset);
				if(pImportName != NULL && pImportName->Hint != NULL && pImportName->Name != NULL)
					importsOfFile.push_back(QString().fromAscii((const char*)((pImportHeader->Name - dwVAOfImportSection) + dwImportSectionOffset)).append("::").append(QString().fromAscii((const char*)pImportName->Name)));			

				pIAT++;
			}

			pImportHeader++;
		} while (pImportHeader->Name);
	}

	return importsOfFile;
}

QStringList clsPEFile::getImports64()
{
	QStringList importsOfFile;

	DWORD64 dwVAOfImportSection = _pINH64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD64 dwImportSectionOffset = dwCalculateTableOffset64(IMAGE_DIRECTORY_ENTRY_IMPORT,_pINH64,_pIDH,(PBYTE)_lpBuffer);

	if(dwVAOfImportSection != 0)
	{
		PIMAGE_IMPORT_DESCRIPTOR pImportHeader = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)dwImportSectionOffset);
		do
		{
			PIMAGE_THUNK_DATA64 pIAT = NULL;

			if(pImportHeader->OriginalFirstThunk != 0)
				pIAT = (PIMAGE_THUNK_DATA64)((pImportHeader->OriginalFirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
            else
                pIAT = (PIMAGE_THUNK_DATA64)((pImportHeader->FirstThunk - dwVAOfImportSection) + dwImportSectionOffset);

			while (pIAT->u1.Ordinal != 0 && !(pIAT->u1.Ordinal & IMAGE_ORDINAL_FLAG))
			{
				PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((pIAT->u1.Function - dwVAOfImportSection) + dwImportSectionOffset);
				if(pImportName != NULL && pImportName->Hint != NULL && pImportName->Name != NULL)
					importsOfFile.push_back(QString().fromAscii((const char*)((pImportHeader->Name - dwVAOfImportSection) + dwImportSectionOffset)).append("::").append(QString().fromAscii((const char*)pImportName->Name)));			

				pIAT++;
			}

			pImportHeader++;
		} while (pImportHeader->Name);
	}

	return importsOfFile;
}

DWORD64 clsPEFile::dwCalculateTableOffset64(int iTableEntryNr,PIMAGE_NT_HEADERS64 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer)
{
	DWORD64 tableVA = pINH->OptionalHeader.DataDirectory[iTableEntryNr].VirtualAddress;
	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)(pBuffer + pIDH->e_lfanew + sizeof(IMAGE_NT_HEADERS64));

	for (WORD i = 0; i < pINH->FileHeader.NumberOfSections; i++)
	{
	  DWORD64 sectionVA = pSectionHeader->VirtualAddress;
	  DWORD64 sectionSize = pSectionHeader->Misc.VirtualSize;

	  if ((sectionVA <= tableVA) && (tableVA < (sectionVA + sectionSize)))
	  {
		   return (DWORD64)(pBuffer + pSectionHeader->PointerToRawData + (tableVA-sectionVA));
		   break;
	  }
	  pSectionHeader++;
	}
	return 0;
}

DWORD clsPEFile::dwCalculateTableOffset32(int iTableEntryNr,PIMAGE_NT_HEADERS32 pINH,PIMAGE_DOS_HEADER pIDH,PBYTE pBuffer)
{
	DWORD tableVA = pINH->OptionalHeader.DataDirectory[iTableEntryNr].VirtualAddress;
	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)(pBuffer + pIDH->e_lfanew + sizeof(IMAGE_NT_HEADERS32));

	for (WORD i = 0; i < pINH->FileHeader.NumberOfSections; i++)
	{
	  DWORD sectionVA = pSectionHeader->VirtualAddress;
	  DWORD sectionSize = pSectionHeader->Misc.VirtualSize;

	  if ((sectionVA <= tableVA) && (tableVA < (sectionVA + sectionSize)))
	  {
		   return (DWORD)(pBuffer + pSectionHeader->PointerToRawData + (tableVA-sectionVA));
		   break;
	  }
	  pSectionHeader++;
	}
	return 0;
}

bool clsPEFile::CheckPEType(LPVOID pBuffer)
{
	PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)pBuffer;

	PIMAGE_NT_HEADERS32 pINH32 = (PIMAGE_NT_HEADERS32)((DWORD)pIDH + pIDH->e_lfanew);
	if(pINH32 != NULL && pINH32->Signature == IMAGE_NT_SIGNATURE)
	{
		if(pINH32->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
			return false;
#ifdef _AMD64_
		else if(pINH32->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
			return true;
#endif
	}
	// do something incase its invalid...
	return false;
}

PIMAGE_DOS_HEADER clsPEFile::getDosHeader()
{
	return _pIDH;
}

PIMAGE_NT_HEADERS32 clsPEFile::getNTHeader32()
{
	if(!_is64Bit)
		return _pINH32;
	return 0;
}

PIMAGE_NT_HEADERS64 clsPEFile::getNTHeader64()
{
	if(_is64Bit)
		return _pINH64;
	return 0;
}

bool clsPEFile::is64Bit()
{
	if(_is64Bit)
		return true;
	else 
		return false;
}