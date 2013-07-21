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
#include "clsPEFile.h"
#include "clsHelperClass.h"
#include "clsMemManager.h"

using namespace std;

clsPEFile::clsPEFile(wstring FileName,bool *bLoaded)
{
	_FileName = clsHelperClass::replaceAll(FileName,'\\','/');
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

	if(GetFileSize(hFile,NULL) < 0x2000)
		_lpBuffer = MapViewOfFile(hFileMap,FILE_MAP_READ,NULL,NULL,NULL);
	else
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
		if(_pINH64 != NULL && _pINH64->Signature != IMAGE_NT_SIGNATURE)
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
		if(_pINH32 != NULL && _pINH32->Signature != IMAGE_NT_SIGNATURE)
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

QList<APIData> clsPEFile::getImports()
{
	QList<APIData> importsOfFile;
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

QList<APIData> clsPEFile::getImports32()
{
	QList<APIData> importsOfFile;

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

			while (pIAT->u1.Ordinal != 0 && !(pIAT->u1.Ordinal & IMAGE_ORDINAL_FLAG32))
			{
				PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((pIAT->u1.Function - dwVAOfImportSection) + dwImportSectionOffset);
				if(pImportName != NULL && pImportName->Name != NULL)
				{
					APIData newAPI;
					newAPI.APIOffset = 0;
					newAPI.APIName = QString().fromAscii((const char*)((pImportHeader->Name - dwVAOfImportSection) + dwImportSectionOffset)).append("::").append(QString().fromAscii((const char*)pImportName->Name));			
					importsOfFile.push_back(newAPI);
				}

				pIAT++;
			}

			pImportHeader++;
		} while (pImportHeader->Name);
	}

	return importsOfFile;
}

QList<APIData> clsPEFile::getImports64()
{
	QList<APIData> importsOfFile;

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

			while (pIAT->u1.Ordinal != 0 && !(pIAT->u1.Ordinal & IMAGE_ORDINAL_FLAG64))
			{
				PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((pIAT->u1.Function - dwVAOfImportSection) + dwImportSectionOffset);
				if(pImportName != NULL && pImportName->Name != NULL)
				{
					APIData newAPI;
					newAPI.APIOffset = 0;
					newAPI.APIName = QString().fromAscii((const char*)((pImportHeader->Name - dwVAOfImportSection) + dwImportSectionOffset)).append("::").append(QString().fromAscii((const char*)pImportName->Name));			
					importsOfFile.push_back(newAPI);
				}

				pIAT++;
			}

			pImportHeader++;
		} while (pImportHeader->Name);
	}

	return importsOfFile;
}

QList<APIData> clsPEFile::getExports()
{
	QList<APIData> exportsOfFile;
	HANDLE hFile = CreateFileW(_FileName.c_str(),GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) return exportsOfFile;

	HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,NULL,NULL,NULL);
	LPVOID lpOrgBuffer = _lpBuffer;

	_lpBuffer = MapViewOfFile(hFileMap,FILE_MAP_READ,NULL,NULL,NULL);
	if(_lpBuffer == NULL)
	{
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		UnmapViewOfFile(_lpBuffer);
		_lpBuffer = lpOrgBuffer;
		return exportsOfFile;
	}

	DWORD exportTableOffset = NULL;
	DWORD exportTableVA = NULL;

	if(_is64Bit)
	{
		exportTableVA = _pINH64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		exportTableOffset = dwCalculateTableOffset64(IMAGE_DIRECTORY_ENTRY_EXPORT,_pINH64,_pIDH,(LPBYTE)_lpBuffer);
	}
	else
	{
		exportTableVA = _pINH32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		exportTableOffset = dwCalculateTableOffset32(IMAGE_DIRECTORY_ENTRY_EXPORT,_pINH32,_pIDH,(LPBYTE)_lpBuffer);
	}

	if(exportTableOffset != 0)
	{
		PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)(exportTableOffset);

		DWORD* addressOfFunctionsArray = (DWORD*)(((pExportTable->AddressOfFunctions)-exportTableVA) + exportTableOffset);
		DWORD* addressOfNamesArray = (DWORD*)(((pExportTable->AddressOfNames)-exportTableVA) + exportTableOffset);
		WORD* addressOfNameOrdinalsArray = (WORD*)(((pExportTable->AddressOfNameOrdinals)-exportTableVA) + exportTableOffset);

		for (DWORD i = 0; i < pExportTable->NumberOfNames; i++)
		{
			APIData newAPI;
			newAPI.APIOffset = addressOfFunctionsArray[addressOfNameOrdinalsArray[i]];// + _pINH64->OptionalHeader.ImageBase;
			newAPI.APIName = (char*)(((addressOfNamesArray[i])-exportTableVA) + exportTableOffset);
			exportsOfFile.push_back(newAPI);
		}
	}

	UnmapViewOfFile(_lpBuffer);
	_lpBuffer = lpOrgBuffer;
	CloseHandle(hFile);
	CloseHandle(hFileMap);
	return exportsOfFile;
}

DWORD64 clsPEFile::VAtoRaw64(quint64 Offset)
{
	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD64)_lpBuffer + _pIDH->e_lfanew + sizeof(IMAGE_NT_HEADERS64));

	for (WORD i = 0; i < _pINH64->FileHeader.NumberOfSections; i++)
	{
	  DWORD64 sectionVA = pSectionHeader->VirtualAddress;
	  DWORD64 sectionSize = pSectionHeader->Misc.VirtualSize;

	  if ((sectionVA <= Offset) && (Offset < (sectionVA + sectionSize)))
		  return (DWORD64)(pSectionHeader->PointerToRawData + (Offset - sectionVA));

	   pSectionHeader++;
	}
	return 0;
}

DWORD clsPEFile::VAtoRaw32(quint64 Offset)
{
	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)_lpBuffer + _pIDH->e_lfanew + sizeof(IMAGE_NT_HEADERS32));

	for (WORD i = 0; i < _pINH32->FileHeader.NumberOfSections; i++)
	{
	  DWORD sectionVA = pSectionHeader->VirtualAddress;
	  DWORD sectionSize = pSectionHeader->Misc.VirtualSize;

	  if ((sectionVA <= Offset) && (Offset < (sectionVA + sectionSize)))
		  return (DWORD)(pSectionHeader->PointerToRawData + (Offset - sectionVA));

	   pSectionHeader++;
	}
	return 0;
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
		return (DWORD64)(pBuffer + pSectionHeader->PointerToRawData + (tableVA-sectionVA));

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
		   return (DWORD)(pBuffer + pSectionHeader->PointerToRawData + (tableVA-sectionVA));
	  pSectionHeader++;
	}
	return 0;
}

DWORD64 clsPEFile::getTLSCallbackOffset64()
{
	if(_pINH64 == NULL) return 0;

	DWORD64 vaOfTLSDir = dwCalculateTableOffset64(IMAGE_DIRECTORY_ENTRY_TLS,_pINH64,_pIDH,(PBYTE)_lpBuffer);
	if(vaOfTLSDir == NULL) return 0;

	PIMAGE_TLS_DIRECTORY64 pTLS = (PIMAGE_TLS_DIRECTORY64)((DWORD64)vaOfTLSDir);

	if(pTLS->AddressOfCallBacks == NULL) return 0;

	DWORD64 rvaTLS = VAtoRaw64(pTLS->AddressOfCallBacks - _pINH64->OptionalHeader.ImageBase);
	DWORD64 callbackOffset = (DWORD64)*(PDWORD64)((DWORD64)_lpBuffer + rvaTLS);
	return callbackOffset - _pINH64->OptionalHeader.ImageBase;
}

DWORD clsPEFile::getTLSCallbackOffset32()
{
	if(_pINH32 == NULL) return 0;

	DWORD vaOfTLSDir = dwCalculateTableOffset32(IMAGE_DIRECTORY_ENTRY_TLS,_pINH32,_pIDH,(PBYTE)_lpBuffer);
	if(vaOfTLSDir == NULL) return 0;

	PIMAGE_TLS_DIRECTORY32 pTLS = (PIMAGE_TLS_DIRECTORY32)((DWORD)vaOfTLSDir);
	
	if(pTLS->AddressOfCallBacks == NULL) return 0;

	DWORD rvaTLS = VAtoRaw32(pTLS->AddressOfCallBacks - _pINH32->OptionalHeader.ImageBase);
	DWORD callbackOffset = (DWORD)*(PDWORD)((DWORD)_lpBuffer + rvaTLS);
	return callbackOffset - _pINH32->OptionalHeader.ImageBase;
}

DWORD64 clsPEFile::getTLSCallbackOffset()
{
	HANDLE hFile = CreateFileW(_FileName.c_str(),GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) return 0;

	HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,NULL,NULL,NULL);
	LPVOID lpOrgBuffer = _lpBuffer;

	_lpBuffer = MapViewOfFile(hFileMap,FILE_MAP_READ,NULL,NULL,NULL);
	if(_lpBuffer == NULL)
	{
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		UnmapViewOfFile(_lpBuffer);
		_lpBuffer = lpOrgBuffer;
		return 0;
	}

	DWORD tlsValue = NULL;
	if(_is64Bit)
		tlsValue = getTLSCallbackOffset64();
	else
		tlsValue = getTLSCallbackOffset32();

	UnmapViewOfFile(_lpBuffer);
	_lpBuffer = lpOrgBuffer;
	CloseHandle(hFile);
	CloseHandle(hFileMap);

	return tlsValue;
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

QList<PESectionData> clsPEFile::getSections()
{
	QList<PESectionData> sectionsOfFile;
	HANDLE hFile = CreateFileW(_FileName.c_str(),GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) return sectionsOfFile;

	HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,NULL,NULL,NULL);
	LPVOID lpOrgBuffer = _lpBuffer;

	_lpBuffer = MapViewOfFile(hFileMap,FILE_MAP_READ,NULL,NULL,NULL);
	if(_lpBuffer == NULL)
	{
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		UnmapViewOfFile(_lpBuffer);
		_lpBuffer = lpOrgBuffer;
		return sectionsOfFile;
	}

	PIMAGE_SECTION_HEADER pSH = NULL;
	DWORD SectionCount = NULL;

	if(_is64Bit)
	{
		pSH = (PIMAGE_SECTION_HEADER)((quint64)_lpBuffer + _pIDH->e_lfanew + sizeof(IMAGE_NT_HEADERS64));
		SectionCount = _pINH64->FileHeader.NumberOfSections;
	}
	else
	{
		pSH = (PIMAGE_SECTION_HEADER)((quint64)_lpBuffer + _pIDH->e_lfanew + sizeof(IMAGE_NT_HEADERS32));
		SectionCount = _pINH32->FileHeader.NumberOfSections;
	}

	for (DWORD i = 0; i < SectionCount;i++)
	{
		PESectionData newSection;
		newSection.Characteristics = pSH->Characteristics;
		newSection.PointerToRawData = pSH->PointerToRawData;
		newSection.SectionName = (char*)pSH->Name;
		newSection.SizeOfRawData = pSH->SizeOfRawData;
		newSection.VirtualAddress = pSH->VirtualAddress;
		newSection.VirtualSize = pSH->Misc.VirtualSize;
		sectionsOfFile.push_back(newSection);

		pSH++;
	}

	UnmapViewOfFile(_lpBuffer);
	_lpBuffer = lpOrgBuffer;
	CloseHandle(hFile);
	CloseHandle(hFileMap);
	return sectionsOfFile;
}

/*

//IMAGE_DATA_DIRS
printf("\r\nIMAGE_DATA_DIRECTORY:\r\n");
for(int i = 0; i < pINH->OptionalHeader.NumberOfRvaAndSizes;i++)
{
printf("\t Nr.%02d: VA: 0x%08X Size: 0x%08X\r\n",
i,
pINH->OptionalHeader.DataDirectory[i].VirtualAddress,
pINH->OptionalHeader.DataDirectory[i].Size);
}
*/