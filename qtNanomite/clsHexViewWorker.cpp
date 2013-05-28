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
#include "clsHexViewWorker.h"
#include "clsMemManager.h"

clsHexViewWorker::clsHexViewWorker(int PID, HANDLE hProc, DWORD64 startOffset, DWORD64 dataLength)
{
	m_hProc = hProc;
	m_PID = PID;
	m_startOffset = startOffset;
	m_dataLength = dataLength;

	this->start();
}

clsHexViewWorker::~clsHexViewWorker()
{
	dataList.clear();
}

void clsHexViewWorker::run()
{
	dataList.clear();

	SIZE_T dwBytesRead = NULL;
	DWORD dwCounter = NULL,
	//	dwProtection = NULL,
		dwStepSize = 0x10;
	DWORD64	dwBaseOffset = m_startOffset;
	LPVOID pBuffer = malloc(m_dataLength);
	TCHAR *tcTempBuffer = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR)),
		*tcAsciiHexTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

	if(m_hProc == NULL || m_hProc == INVALID_HANDLE_VALUE)
	{
		clsMemManager::CFree(pBuffer);
		clsMemManager::CFree(tcAsciiHexTemp);
		clsMemManager::CFree(tcTempBuffer);
		return;
	}

	//if(!VirtualProtectEx(hProcess,(LPVOID)StartOffset,Size,PAGE_EXECUTE_READWRITE,&dwProtection))
	//{
	//	clsMemManager::CFree(pBuffer);
	//	clsMemManager::CFree(tcAsciiHexTemp);
	//	clsMemManager::CFree(tcTempBuffer);
	//	return;
	//}

	if(!ReadProcessMemory(m_hProc,(LPVOID)m_startOffset,(LPVOID)pBuffer,m_dataLength,&dwBytesRead))
	{
		clsMemManager::CFree(pBuffer);
		clsMemManager::CFree(tcAsciiHexTemp);
		clsMemManager::CFree(tcTempBuffer);
		return;
	}

	for(int iLineStep = 0; iLineStep < (m_dataLength / dwStepSize); iLineStep++)
	{
		if(m_startOffset > (dwBaseOffset + m_dataLength))
			break;

		HexData newHexData;
		newHexData.PID = m_PID;
		newHexData.hexOffset = m_startOffset;

		// Hexiss
		memset(tcTempBuffer,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		for(size_t i = 0;i < dwStepSize;i++)
		{
			wsprintf(tcAsciiHexTemp,L"%02X ",*(LPBYTE)((DWORD)pBuffer + dwCounter + i));
			wcscat_s(tcTempBuffer,MAX_PATH,tcAsciiHexTemp);
		}
		newHexData.hexString = QString::fromWCharArray(tcTempBuffer);

		//Acsii
		memset(tcTempBuffer,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		memset(tcAsciiHexTemp,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		for(size_t i = 0;i < dwStepSize;i++)
		{
			wsprintf(tcAsciiHexTemp,L"%c ",*(PCHAR)((DWORD)pBuffer + dwCounter + i));
			if(wcsstr(tcAsciiHexTemp,L"  ") != NULL)
				wcscat_s(tcTempBuffer,MAX_PATH,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\r") != NULL)
				wcscat_s(tcTempBuffer,MAX_PATH,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\t") != NULL)
				wcscat_s(tcTempBuffer,MAX_PATH,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\n") != NULL)
				wcscat_s(tcTempBuffer,MAX_PATH,L". ");
			else if(*(PCHAR)((DWORD)pBuffer + dwCounter + i) ==  0x00)
				wcscat_s(tcTempBuffer,MAX_PATH,L". ");
			else
				wcscat_s(tcTempBuffer,MAX_PATH,tcAsciiHexTemp);
		}
		newHexData.asciiData = QString::fromWCharArray(tcTempBuffer);

		dataList.insert(m_startOffset,newHexData);

		m_startOffset += dwStepSize;
		dwCounter += dwStepSize;
	}

	//VirtualProtectEx(hProcess,(LPVOID)StartOffset,Size,dwProtection,NULL);

	clsMemManager::CFree(pBuffer);
	clsMemManager::CFree(tcAsciiHexTemp);
	clsMemManager::CFree(tcTempBuffer);

	return;
}