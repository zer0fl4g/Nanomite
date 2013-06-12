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
#include "clsStringViewWorker.h"
#include "clsMemManager.h"

#include <fstream>
#include <sstream>

using namespace std;

clsStringViewWorker::clsStringViewWorker(QMap<int,PTCHAR> dataForProcessing)
{
	m_processingData = dataForProcessing;
	this->start();
}

clsStringViewWorker::~clsStringViewWorker()
{
	stringList.clear();
}

void clsStringViewWorker::run()
{
	stringList.clear();

	for(QMap<int,PTCHAR>::const_iterator i = m_processingData.constBegin(); i != m_processingData.constEnd(); ++i)
	{
		ifstream inputFile;
	
		inputFile.open(i.value(),ifstream::binary);
		if(!inputFile.is_open())
		{
			MessageBox(NULL,i.value(),L"Error opening File!",MB_OKCANCEL);
			return;
		}

		QString asciiChar;
		CHAR sT = '\0';
		while(inputFile.good())
		{	
			asciiChar.clear();
			inputFile.get(sT);
			while(inputFile.good())
			{
				if(((int)sT >= 0x41 && (int)sT <= 0x5a)		||
					((int)sT >= 0x61 && (int)sT <= 0x7a)	||
					((int)sT >= 0x30 && (int)sT <= 0x39)	|| 
					((int)sT == 0x20)						||
					((int)sT == 0xA))
					asciiChar.append(sT);
				else
				{
					break;
				}

				inputFile.get(sT);
			}

			if((int)sT == 0 && asciiChar.length() > 3)
			{
				StringData newStringData;
				newStringData.DataString = asciiChar;
				newStringData.PID = i.key();
				newStringData.StringOffset = inputFile.tellg();

				stringList.insert(inputFile.tellg(),newStringData);
			}
		}
		inputFile.close();
		
		//wifstream uniInputFile;
		//uniInputFile.open(i.value(),wifstream::binary);
		//QString uniChar;
		//
		//TCHAR uniTempChar[1] = {'\0'};
		//while(uniInputFile.good())
		//{
		//	uniChar.clear();
		//	uniInputFile.read(uniTempChar,2);
		//	while(uniInputFile.good())
		//	{
		//		if(((short)uniTempChar[0] >= 0x0041 && (short)uniTempChar[0] <= 0x005a)		||
		//			((short)uniTempChar[0] >= 0x0061 && (short)uniTempChar[0] <= 0x007a)	||
		//			((short)uniTempChar[0] >= 0x0030 && (short)uniTempChar[0] <= 0x0039)	|| 
		//			((short)uniTempChar[0] == 0x0020)										||
		//			((short)uniTempChar[0] == 0x000A))
		//			uniChar.append(uniTempChar[0]);
		//		else
		//		{
		//			break;
		//		}

		//		uniInputFile.read(uniTempChar,2);
		//	}

		//	if((short)uniTempChar[0] == 0x0000 && uniChar.length() > 4)
		//	{
		//		StringData newStringData;
		//		newStringData.DataString = uniChar;
		//		newStringData.PID = i.key();
		//		newStringData.StringOffset = uniInputFile.tellg();

		//		stringList.insert(uniInputFile.tellg(),newStringData);
		//	}
		//}
		//uniInputFile.close();
	}

	return;
}