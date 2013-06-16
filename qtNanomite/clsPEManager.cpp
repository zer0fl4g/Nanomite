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
#include "clsPEManager.h"
#include "clsMemManager.h"
#include "clsHelperClass.h"

using namespace std;

clsPEManager* clsPEManager::pThis = NULL;

clsPEManager* clsPEManager::GetInstance()
{
	return pThis;
}

clsPEManager::clsPEManager()
{
	pThis = this;
}

wstring clsPEManager::getFilenameFromPID(int PID)
{
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].PID == PID)
		{
			return PEFiles[i].FileName;
		}
	}
	return L"";
}

void clsPEManager::InsertPIDForFile(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0)
		{
			PEFiles[i].PID = PID;
			return;
		}
	}

	//File not found so open it (child proc)
	OpenFile(newFileName,PID);
}

bool clsPEManager::OpenFile(std::wstring FileName,int PID,bool is64Bit)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
			return false;
	}

	PEManager newPEFile;
	bool bLoaded = false;

	newPEFile.PEFile = new clsPEFile(newFileName,&bLoaded);
	newPEFile.FileName = newFileName;
	newPEFile.is64Bit = newPEFile.PEFile->is64Bit();
	newPEFile.PID = PID;

	if(bLoaded)
		PEFiles.push_back(newPEFile);
	else
		return false;
	return true;
}

bool clsPEManager::isValidPEFile(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->isValidPEFile();
	}
	return false;
}

void clsPEManager::CloseFile(std::wstring FileName,int PID)
{
	if(PEFiles.size() <= 0) return;

	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');
	for(vector<PEManager>::const_iterator it = PEFiles.cbegin(); it != PEFiles.cend(); ++it)
	{
		if(it->FileName.compare(newFileName) == 0  /* || PEFiles[i].PID == PID */)
		{
			delete it->PEFile;
			PEFiles.erase(it);
			return;
		}
	}
}

QList<APIData> clsPEManager::getImportsFromFile(std::wstring FileName)
{
	return clsPEManager::pThis->getImports(FileName,0);
}

QList<APIData> clsPEManager::getImports(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getImports();
	}

	return QList<APIData>();
}

QList<APIData> clsPEManager::getExports(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID*/)
			return PEFiles[i].PEFile->getExports();
	}

	return QList<APIData>();
}

QList<PESectionData> clsPEManager::getSections(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getSections();
	}

	return QList<PESectionData>();
}

clsPEManager::~clsPEManager()
{
	CleanPEManager();
}

void clsPEManager::CleanPEManager()
{
	for(size_t i = 0; i < PEFiles.size(); i++)
		delete PEFiles[i].PEFile;
	PEFiles.clear();
}

PIMAGE_DOS_HEADER clsPEManager::getDosHeader(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getDosHeader();
	}
	return NULL;
}

PIMAGE_NT_HEADERS32 clsPEManager::getNTHeader32(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getNTHeader32();
	}
	return NULL;
}

PIMAGE_NT_HEADERS64 clsPEManager::getNTHeader64(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getNTHeader64();
	}
	return NULL;
}

bool clsPEManager::is64BitFile(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 || PEFiles[i].PID == PID)
			return PEFiles[i].is64Bit;
	}
	return false;
}

DWORD64 clsPEManager::VAtoRaw(std::wstring FileName,int PID, DWORD64 RVAOffset)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
		{
			if(PEFiles[i].is64Bit)
			{
				return PEFiles[i].PEFile->VAtoRaw64(RVAOffset);
			}
			else
			{
				return PEFiles[i].PEFile->VAtoRaw32(RVAOffset);
			}
		}
	}
	return NULL;
}

DWORD64 clsPEManager::getTLSCallbackOffset(std::wstring FileName,int PID)
{
	wstring newFileName = clsHelperClass::replaceAll(FileName,'\\','/');

	for(size_t i = 0; i < pThis->PEFiles.size(); i++)
	{
		if(pThis->PEFiles[i].FileName.compare(newFileName) == 0 /* || PEFiles[i].PID == PID */)
		{
			return pThis->PEFiles[i].PEFile->getTLSCallbackOffset();
		}
	}
	return NULL;
}