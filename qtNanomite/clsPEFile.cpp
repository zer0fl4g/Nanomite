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
#include "clsDebugger\clsDebugger.h"

#include <Psapi.h>

clsPEFile::clsPEFile(QString FileName,bool *bLoaded, int PID, quint64 imageBase) :
	m_fileName(FileName),
	m_fileBuffer(NULL)
{
	if(PID == -1 && imageBase == 0)
		*bLoaded = m_isLoaded = CheckFile(m_fileName);
	else
		*bLoaded = m_isLoaded = LoadFile(m_fileName, PID, imageBase);
}

clsPEFile::~clsPEFile()
{

}

bool clsPEFile::CheckFile(QString FileName)
{
	HANDLE hFile = CreateFileW(FileName.toStdWString().c_str(),GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) return false;
		
	HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,NULL,NULL,NULL);

	m_fileBuffer = MapViewOfFile(hFileMap,FILE_MAP_READ,NULL,NULL,NULL);
	if(m_fileBuffer == NULL)
	{
		CloseHandle(hFile);
		CloseHandle(hFileMap);

		return false;
	}
			
	memcpy_s(&m_IDH, sizeof(IMAGE_DOS_HEADER), m_fileBuffer, sizeof(IMAGE_DOS_HEADER));
	if(m_IDH.e_magic != IMAGE_DOS_SIGNATURE)
	{
		UnmapViewOfFile(m_fileBuffer);
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		
		return false;
	}
	
	memcpy_s(&m_IFH, sizeof(IMAGE_FILE_HEADER), (LPVOID)((DWORD64)m_fileBuffer + m_IDH.e_lfanew + sizeof(DWORD)), sizeof(IMAGE_FILE_HEADER));
	if(m_IFH.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64))
		m_is64Bit = true;
	else if(m_IFH.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32))
		m_is64Bit = false;
	else
	{
		UnmapViewOfFile(m_fileBuffer);
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		
		return false;
	}

	if(m_is64Bit)
	{
		memcpy_s(&m_INH64, sizeof(IMAGE_NT_HEADERS64), (LPVOID)((DWORD64)m_fileBuffer + m_IDH.e_lfanew), sizeof(IMAGE_NT_HEADERS64));
		if(m_INH64.Signature != IMAGE_NT_SIGNATURE)
		{
			UnmapViewOfFile(m_fileBuffer);
			CloseHandle(hFile);
			CloseHandle(hFileMap);
		
			return false;
		}
	}
	else
	{
		memcpy_s(&m_INH32, sizeof(IMAGE_NT_HEADERS32), (LPVOID)((DWORD64)m_fileBuffer + m_IDH.e_lfanew), sizeof(IMAGE_NT_HEADERS32));
		if(m_INH32.Signature != IMAGE_NT_SIGNATURE)
		{
			UnmapViewOfFile(m_fileBuffer);
			CloseHandle(hFile);
			CloseHandle(hFileMap);
		
			return false;
		}
	}

	fileEntropie = loadEntropie(GetFileSize(hFile, NULL));

	UnmapViewOfFile(m_fileBuffer);
	CloseHandle(hFile);
	CloseHandle(hFileMap);
	return true;
}

bool clsPEFile::LoadFile(QString FileName, int PID, quint64 imageBase)
{
	quint64 moduleSize	= 0;
	MEMORY_BASIC_INFORMATION MBI;
	HANDLE processHandle = clsDebugger::GetProcessHandleByPID(PID);
	PTCHAR moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	LPVOID tempBuffer = NULL;

	while(VirtualQueryEx(processHandle, (LPCVOID)(imageBase + moduleSize), &MBI, sizeof(MBI)) != 0)
	{
		if(GetMappedFileName(processHandle, (LPVOID)(imageBase + moduleSize), moduleName, MAX_PATH))
		{
			QStringList file = FileName.split("\\");
			QStringList modName = QString::fromWCharArray(moduleName).split("\\");

			if(file.at(file.length() - 1).compare(modName.at(modName.length() - 1)) == 0)
			{
				DWORD	oldProtection = NULL,
						newProtection = PAGE_READWRITE;
			
				if(VirtualProtectEx(processHandle, (LPVOID)(imageBase + moduleSize), MBI.RegionSize, newProtection, &oldProtection))
				{
					m_fileBuffer = realloc(m_fileBuffer, moduleSize + MBI.RegionSize);
					tempBuffer = malloc(MBI.RegionSize);

					if(!ReadProcessMemory(processHandle, (LPVOID)(imageBase + moduleSize), tempBuffer, MBI.RegionSize, NULL))
					{
						free(m_fileBuffer);
						free(tempBuffer);

						tempBuffer		= NULL;
						m_fileBuffer	= NULL;

						break;
					}
					else
					{
						memcpy((LPVOID)((DWORD64)m_fileBuffer + moduleSize), tempBuffer, MBI.RegionSize);
					}

					VirtualProtectEx(processHandle, (LPVOID)(imageBase + moduleSize), MBI.RegionSize, oldProtection, &newProtection);

					free(tempBuffer);
				}

				moduleSize += MBI.RegionSize;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	clsMemManager::CFree(moduleName);

	if(m_fileBuffer == NULL)
	{
		return false;
	}
			
	memcpy_s(&m_IDH, sizeof(IMAGE_DOS_HEADER), m_fileBuffer, sizeof(IMAGE_DOS_HEADER));
	if(m_IDH.e_magic != IMAGE_DOS_SIGNATURE)
	{
		free(m_fileBuffer);
		
		return false;
	}
	
	memcpy_s(&m_IFH, sizeof(IMAGE_FILE_HEADER), (LPVOID)((DWORD64)m_fileBuffer + m_IDH.e_lfanew + sizeof(DWORD)), sizeof(IMAGE_FILE_HEADER));
	if(m_IFH.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64))
		m_is64Bit = true;
	else if(m_IFH.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32))
		m_is64Bit = false;
	else
	{
		free(m_fileBuffer);
		
		return false;
	}

	if(m_is64Bit)
	{
		memcpy_s(&m_INH64, sizeof(IMAGE_NT_HEADERS64), (LPVOID)((DWORD64)m_fileBuffer + m_IDH.e_lfanew), sizeof(IMAGE_NT_HEADERS64));
		if(m_INH64.Signature != IMAGE_NT_SIGNATURE)
		{
			free(m_fileBuffer);
		
			return false;
		}
	}
	else
	{
		memcpy_s(&m_INH32, sizeof(IMAGE_NT_HEADERS32), (LPVOID)((DWORD64)m_fileBuffer + m_IDH.e_lfanew), sizeof(IMAGE_NT_HEADERS32));
		if(m_INH32.Signature != IMAGE_NT_SIGNATURE)
		{
			free(m_fileBuffer);
					
			return false;
		}
	}

	fileEntropie = NULL; //loadEntropie(moduleSize);
	
	fileSections = loadSections();

	fileExports = loadExports();

	if(m_is64Bit)
	{
		fileImports = loadImports64();
		m_tlsOffset = loadTLSCallbackOffset64();
	}
	else
	{
		fileImports = loadImports32();
		m_tlsOffset = loadTLSCallbackOffset32();
	}

	free(m_fileBuffer);
	return true;
}

QList<APIData> clsPEFile::getImports()
{
	return fileImports;
}

QList<APIData> clsPEFile::getExports()
{
	return fileExports;
}

QList<DWORD64> clsPEFile::getTLSCallbackOffset()
{
	return m_tlsOffset;
}

IMAGE_DOS_HEADER clsPEFile::getDosHeader()
{
	return m_IDH;
}

IMAGE_NT_HEADERS32 clsPEFile::getNTHeader32()
{
	return m_INH32;
}

IMAGE_NT_HEADERS64 clsPEFile::getNTHeader64()
{
	return m_INH64;
}

QList<IMAGE_SECTION_HEADER> clsPEFile::getSections()
{
	return fileSections;
}

QList<IMAGE_SECTION_HEADER> clsPEFile::loadSections()
{
	QList<IMAGE_SECTION_HEADER> sectionsOfFile;
	PIMAGE_SECTION_HEADER pSH = NULL;
	DWORD SectionCount = NULL;

	if(m_is64Bit)
	{
		pSH = (PIMAGE_SECTION_HEADER)((quint64)m_fileBuffer + m_IDH.e_lfanew + sizeof(IMAGE_NT_HEADERS64));
		SectionCount = m_INH64.FileHeader.NumberOfSections;
	}
	else
	{
		pSH = (PIMAGE_SECTION_HEADER)((quint64)m_fileBuffer + m_IDH.e_lfanew + sizeof(IMAGE_NT_HEADERS32));
		SectionCount = m_INH32.FileHeader.NumberOfSections;
	}

	for (DWORD i = 0; i < SectionCount;i++)
	{
		IMAGE_SECTION_HEADER sectionHeader;
		memcpy_s(&sectionHeader, sizeof(IMAGE_SECTION_HEADER), pSH, sizeof(IMAGE_SECTION_HEADER));

		sectionsOfFile.push_back(sectionHeader);

		pSH++;
	}

	return sectionsOfFile;
}

QList<DWORD64> clsPEFile::loadTLSCallbackOffset64()
{
	QList<DWORD64> tlsCallbacks;

	DWORD64 vaOfTLSDir = m_INH64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
	if(vaOfTLSDir == NULL) return tlsCallbacks;

	vaOfTLSDir += (DWORD64)m_fileBuffer;

	PIMAGE_TLS_DIRECTORY64 pTLS = (PIMAGE_TLS_DIRECTORY64)((DWORD64)vaOfTLSDir);
	if(pTLS->AddressOfCallBacks == NULL) return tlsCallbacks;

	DWORD64 rvaTLS = pTLS->AddressOfCallBacks - m_INH64.OptionalHeader.ImageBase;
	DWORD64 callbackOffset = NULL;

	for(int tlsCounter = 0; tlsCounter <= 99; tlsCounter++)
	{
		callbackOffset = (DWORD64)*(PDWORD64)((DWORD64)m_fileBuffer + rvaTLS + (tlsCounter * sizeof(DWORD64)));
		
		if(callbackOffset != 0)
		{
			tlsCallbacks.append(callbackOffset - m_INH64.OptionalHeader.ImageBase);
		}
		else
		{
			break;
		}
	}

	return tlsCallbacks;
}

QList<DWORD64> clsPEFile::loadTLSCallbackOffset32()
{
	QList<DWORD64> tlsCallbacks;

	DWORD vaOfTLSDir = m_INH32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
	if(vaOfTLSDir == NULL) return tlsCallbacks;

	vaOfTLSDir += (DWORD)m_fileBuffer;

	PIMAGE_TLS_DIRECTORY32 pTLS = (PIMAGE_TLS_DIRECTORY32)((DWORD)vaOfTLSDir);
	if(pTLS->AddressOfCallBacks == NULL) return tlsCallbacks;

	DWORD rvaTLS = pTLS->AddressOfCallBacks - m_INH32.OptionalHeader.ImageBase;
	DWORD callbackOffset = NULL;

	for(int tlsCounter = 0; tlsCounter <= 99; tlsCounter++)
	{
		callbackOffset = (DWORD)*(PDWORD)((DWORD)m_fileBuffer + rvaTLS + (tlsCounter * sizeof(DWORD)));
		
		if(callbackOffset != 0)
		{
			tlsCallbacks.append(callbackOffset - m_INH32.OptionalHeader.ImageBase);
		}
		else
		{
			break;
		}
	}

	return tlsCallbacks;
}

QList<APIData> clsPEFile::loadExports()
{
	QList<APIData> exportsOfFile;
	DWORD exportTableOffset = NULL;
	DWORD exportTableVA = NULL;
	DWORD64 imageBase = NULL;

	if(m_is64Bit)
	{
		exportTableVA = m_INH64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		exportTableOffset = exportTableVA + (DWORD64)m_fileBuffer;
		imageBase = m_INH64.OptionalHeader.ImageBase;
	}
	else
	{
		exportTableVA = m_INH32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		exportTableOffset = exportTableVA + (DWORD64)m_fileBuffer;
		imageBase = m_INH32.OptionalHeader.ImageBase;
	}

	if(exportTableOffset != NULL && exportTableVA != NULL)
	{
		PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)(exportTableOffset);

		DWORD* addressOfFunctionsArray = (DWORD*)(((pExportTable->AddressOfFunctions)-exportTableVA) + exportTableOffset);
		DWORD* addressOfNamesArray = (DWORD*)(((pExportTable->AddressOfNames)-exportTableVA) + exportTableOffset);
		WORD* addressOfNameOrdinalsArray = (WORD*)(((pExportTable->AddressOfNameOrdinals)-exportTableVA) + exportTableOffset);

		for (DWORD i = 0; i < pExportTable->NumberOfNames; i++)
		{
			APIData newAPI;
			newAPI.APIOffset = addressOfFunctionsArray[addressOfNameOrdinalsArray[i]] + imageBase;
			newAPI.APIName = (char*)(((addressOfNamesArray[i])-exportTableVA) + exportTableOffset);
			exportsOfFile.push_back(newAPI);
		}
	}
	return exportsOfFile;
}

QList<APIData> clsPEFile::loadImports32()
{
	QList<APIData> importsOfFile;

	DWORD dwVAOfImportSection = m_INH32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD dwImportSectionOffset = dwVAOfImportSection + (DWORD64)m_fileBuffer;

	if(dwImportSectionOffset != NULL && dwVAOfImportSection != NULL)
	{
		PIMAGE_IMPORT_DESCRIPTOR pImportHeader = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)dwImportSectionOffset);
		do
		{
			PIMAGE_THUNK_DATA32 pIAT = NULL;
			PIMAGE_THUNK_DATA32 pIATFirstThunk = NULL;

			if(pImportHeader->OriginalFirstThunk != 0)
			{
				pIAT = (PIMAGE_THUNK_DATA32)((pImportHeader->OriginalFirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
				pIATFirstThunk = (PIMAGE_THUNK_DATA32)((pImportHeader->FirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
			}
			else
			{
				pIAT = pIATFirstThunk = (PIMAGE_THUNK_DATA32)((pImportHeader->FirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
			}

			while (pIAT->u1.Ordinal != 0 && !(pIAT->u1.Ordinal & IMAGE_ORDINAL_FLAG32))
			{
				PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((pIAT->u1.Function - dwVAOfImportSection) + dwImportSectionOffset);
				if(pImportName != NULL && pImportName->Name != NULL)
				{
					APIData newAPI;
					newAPI.APIOffset = pIATFirstThunk->u1.Function;
					newAPI.APIName = QString().fromAscii((const char*)((pImportHeader->Name - dwVAOfImportSection) + dwImportSectionOffset)).append("::").append(QString().fromAscii((const char*)pImportName->Name));			
					importsOfFile.push_back(newAPI);
				}

				pIAT++;pIATFirstThunk++;
			}

			pImportHeader++;
		} while (pImportHeader->Name);
	}

	return importsOfFile;
}

QList<APIData> clsPEFile::loadImports64()
{
	QList<APIData> importsOfFile;

	DWORD64 dwVAOfImportSection = m_INH64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD64 dwImportSectionOffset = dwVAOfImportSection + (DWORD64)m_fileBuffer;

	if(dwImportSectionOffset != NULL && dwVAOfImportSection != NULL)
	{
		PIMAGE_IMPORT_DESCRIPTOR pImportHeader = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)dwImportSectionOffset);
		do
		{
			PIMAGE_THUNK_DATA64 pIAT = NULL;
			PIMAGE_THUNK_DATA64 pIATFirstThunk = NULL;

			if(pImportHeader->OriginalFirstThunk != 0)
			{
				pIAT = (PIMAGE_THUNK_DATA64)((pImportHeader->OriginalFirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
				pIATFirstThunk = (PIMAGE_THUNK_DATA64)((pImportHeader->FirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
			}
			else
			{
				pIAT = pIATFirstThunk = (PIMAGE_THUNK_DATA64)((pImportHeader->FirstThunk - dwVAOfImportSection) + dwImportSectionOffset);
			}

			while (pIAT->u1.Ordinal != 0 && !(pIAT->u1.Ordinal & IMAGE_ORDINAL_FLAG64))
			{
				PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((pIAT->u1.Function - dwVAOfImportSection) + dwImportSectionOffset);
				if(pImportName != NULL && pImportName->Name != NULL)
				{
					APIData newAPI;
					newAPI.APIOffset = pIATFirstThunk->u1.Function;
					newAPI.APIName = QString().fromAscii((const char*)((pImportHeader->Name - dwVAOfImportSection) + dwImportSectionOffset)).append("::").append(QString().fromAscii((const char*)pImportName->Name));			
					importsOfFile.push_back(newAPI);
				}

				pIAT++;pIATFirstThunk++;
			}

			pImportHeader++;
		} while (pImportHeader->Name);
	}

	return importsOfFile;
}

bool clsPEFile::isValidPEFile()
{
	return m_isLoaded;
}

bool clsPEFile::is64Bit()
{
	return m_is64Bit;
}

DWORD64 clsPEFile::VAtoRaw(quint64 Offset)
{
	for(QList<IMAGE_SECTION_HEADER>::const_iterator sectionIT = fileSections.constBegin(); sectionIT != fileSections.constEnd(); ++sectionIT)
	{
		DWORD64 sectionVA = sectionIT->VirtualAddress;
		DWORD64 sectionSize = sectionIT->Misc.VirtualSize;

		if ((sectionVA <= Offset) && (Offset < (sectionVA + sectionSize)))
			return (DWORD64)(sectionIT->PointerToRawData + (Offset - sectionVA));
	}

	return 0;
}

float clsPEFile::loadEntropie(DWORD fileSize)
{
	DWORD64 countBytes[256] = { 0 };
	long double result = 0.0, temp = 0.0;

	for(DWORD i = 0; i < fileSize; i++)
	{
		countBytes[((PBYTE)m_fileBuffer)[i]]++;
	}

	for(int i = 0; i < 256; i++)
	{
		if(countBytes[i] == 0) continue;

		temp = (long double)countBytes[i] / (long double)fileSize;
		result -= temp * log(temp);
	}

	result /= log((long double)256);
	result *= 8;

	return result;
}

float clsPEFile::getEntropie()
{
	return fileEntropie;
}