#ifndef CLSPEMANAGER_H
#define CLSPEMANAGER_H

#include <QtCore>

#include "clsPEFile.h"

class clsPEManager: public QObject
{
	Q_OBJECT
public:
	static QList<APIData> getImportsFromFile(std::wstring FileName);
	static clsPEManager* GetInstance();

	clsPEManager();
	~clsPEManager();

	bool OpenFile(std::wstring FileName,int PID = -1,bool is64Bit = false);
	bool isValidPEFile(std::wstring FileName,int PID = 0);
	bool is64BitFile(std::wstring FileName,int PID = 0);

	QList<APIData> getImports(std::wstring FileName,int PID = -1);
	QList<APIData> getExports(std::wstring FileName,int PID = -1);

	PIMAGE_DOS_HEADER getDosHeader(std::wstring FileName,int PID = -1);
	PIMAGE_NT_HEADERS32 getNTHeader32(std::wstring FileName,int PID = -1);
	PIMAGE_NT_HEADERS64 getNTHeader64(std::wstring FileName,int PID = -1);

	std::wstring getFilenameFromPID(int PID);

	public slots:
		void InsertPIDForFile(std::wstring,int);
		void CloseFile(std::wstring,int);
		void CleanPEManager();

private:
	static clsPEManager *pThis;

	struct PEManager
	{
		clsPEFile *PEFile;
		int PID;
		bool is64Bit;
		std::wstring FileName;
	};

	std::vector<PEManager> PEFiles;
};

#endif