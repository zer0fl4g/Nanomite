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

clsPEManager* clsPEManager::pThis = NULL;

clsPEManager* clsPEManager::GetInstance()
{
	return pThis;
}

clsPEManager::clsPEManager()
{
	pThis = this;
}

QString clsPEManager::getFilenameFromPID(int PID)
{
	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].PID == PID)
		{
			return PEFiles[i].FileName;
		}
	}
	return QString("");
}

void clsPEManager::InsertPIDForFile(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0)
		{
			PEFiles[i].PID = PID;
			return;
		}
	}

	//File not found so open it (child proc)
	OpenFile(FileName,PID);
}

bool clsPEManager::OpenFile(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
			return false;
	}

	PEManager newPEFile;
	bool bLoaded = false;

	newPEFile.PEFile = new clsPEFile(FileName,&bLoaded);
	newPEFile.FileName = FileName;
	newPEFile.is64Bit = newPEFile.PEFile->is64Bit();
	newPEFile.PID = PID;

	if(bLoaded)
		PEFiles.push_back(newPEFile);
	else
	{
		delete newPEFile.PEFile;
		return false;
	}

	return true;
}

bool clsPEManager::isValidPEFile(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->isValidPEFile();
	}
	return false;
}

void clsPEManager::CloseFile(QString FileName,int PID)
{
	if(PEFiles.size() <= 0) return;

	FileName.replace('/','\\');
	for(QList<PEManager>::iterator it = PEFiles.begin(); it != PEFiles.end(); ++it)
	{
		if(it->FileName.compare(FileName) == 0 || it->PID == PID) 
		{
			delete it->PEFile;
			PEFiles.erase(it);
			return;
		}
	}
}

QList<APIData> clsPEManager::getImportsFromFile(QString FileName)
{
	return clsPEManager::pThis->getImports(FileName,0);
}

QList<APIData> clsPEManager::getImports(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getImports();
	}

	return QList<APIData>();
}

QList<APIData> clsPEManager::getExports(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID*/)
			return PEFiles[i].PEFile->getExports();
	}

	return QList<APIData>();
}

QList<IMAGE_SECTION_HEADER> clsPEManager::getSections(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getSections();
	}

	return QList<IMAGE_SECTION_HEADER>();
}

clsPEManager::~clsPEManager()
{
	CleanPEManager();
}

void clsPEManager::CleanPEManager()
{
	for(int i = 0; i < PEFiles.size(); i++)
		delete PEFiles[i].PEFile;
	PEFiles.clear();
}

IMAGE_DOS_HEADER clsPEManager::getDosHeader(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getDosHeader();
	}

	IMAGE_DOS_HEADER failed;
	return failed;
}

IMAGE_NT_HEADERS32 clsPEManager::getNTHeader32(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getNTHeader32();
	}

	IMAGE_NT_HEADERS32 failed;
	return failed;
}

IMAGE_NT_HEADERS64 clsPEManager::getNTHeader64(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
			return PEFiles[i].PEFile->getNTHeader64();
	}

	IMAGE_NT_HEADERS64 failed;
	return failed;
}

bool clsPEManager::is64BitFile(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 || PEFiles[i].PID == PID)
			return PEFiles[i].is64Bit;
	}
	return false;
}

DWORD64 clsPEManager::VAtoRaw(QString FileName,int PID, DWORD64 RVAOffset)
{
	FileName.replace('/','\\');

	for(int i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
		{
			return PEFiles[i].PEFile->VAtoRaw(RVAOffset);
		}
	}
	return NULL;
}

QList<DWORD64> clsPEManager::getTLSCallbackOffset(QString FileName,int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < pThis->PEFiles.size(); i++)
	{
		if(pThis->PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
		{
			return pThis->PEFiles[i].PEFile->getTLSCallbackOffset();
		}
	}

	return QList<DWORD64>();
}

float clsPEManager::getEntropie(QString FileName, int PID)
{
	FileName.replace('/','\\');

	for(int i = 0; i < pThis->PEFiles.size(); i++)
	{
		if(pThis->PEFiles[i].FileName.compare(FileName) == 0 /* || PEFiles[i].PID == PID */)
		{
			return pThis->PEFiles[i].PEFile->getEntropie();
		}
	}

	return 0.0;
}