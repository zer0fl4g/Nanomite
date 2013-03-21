#include "clsPEManager.h"

using namespace std;

clsPEManager* clsPEManager::pThis = NULL;

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
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName)
		{
			PEFiles[i].PID = PID;
			return;
		}
	}

	//File not found so open it (child proc)
	OpenFile(FileName,PID);
}

bool clsPEManager::OpenFile(std::wstring FileName,int PID,bool is64Bit)
{
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName || PEFiles[i].PID == PID)
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
		return false;
	return true;
}

bool clsPEManager::isValidPEFile(std::wstring FileName,int PID)
{
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName || PEFiles[i].PID == PID)
			return PEFiles[i].PEFile->isValidPEFile();
	}
	return false;
}

void clsPEManager::CloseFile(std::wstring FileName,int PID)
{
	if(PEFiles.size() <= 0) return;

	for(vector<PEManager>::const_iterator it = PEFiles.cbegin(); it != PEFiles.cend(); ++it)
	{
		if(it->FileName == FileName || it->PID == PID)
		{
			delete it->PEFile;
			PEFiles.erase(it);
			return;
		}
	}
}

QStringList clsPEManager::getImportsFromFile(std::wstring FileName)
{
	return clsPEManager::pThis->getImports(FileName,0);
}

QStringList clsPEManager::getImports(std::wstring FileName,int PID)
{
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName || PEFiles[i].PID == PID)
			return PEFiles[i].PEFile->getImports();
	}

	// no imports or invalid file
	return QStringList();
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
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName || PEFiles[i].PID == PID)
			return PEFiles[i].PEFile->getDosHeader();
	}
}

PIMAGE_NT_HEADERS32 clsPEManager::getNTHeader32(std::wstring FileName,int PID)
{
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName || PEFiles[i].PID == PID)
			return PEFiles[i].PEFile->getNTHeader32();
	}
}

PIMAGE_NT_HEADERS64 clsPEManager::getNTHeader64(std::wstring FileName,int PID)
{
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName || PEFiles[i].PID == PID)
			return PEFiles[i].PEFile->getNTHeader64();
	}
}

bool clsPEManager::is64BitFile(std::wstring FileName,int PID)
{
	for(size_t i = 0; i < PEFiles.size(); i++)
	{
		if(PEFiles[i].FileName == FileName || PEFiles[i].PID == PID)
			return PEFiles[i].is64Bit;
	}
}