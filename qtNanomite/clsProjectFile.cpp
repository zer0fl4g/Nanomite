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
#include "clsProjectFile.h"
#include "clsMemManager.h"

#include <QFileDialog>
#include <QMessageBox>

clsProjectFile::clsProjectFile(bool isSaveFile, bool *pStartDebugging) :
	m_pMainWindow(qtDLGNanomite::GetInstance())
{
	if(isSaveFile)
	{
		QString saveFilePath = QFileDialog::getSaveFileName(m_pMainWindow, "Please select a save path", QDir::currentPath(), "Nanomite Project Files (*.ndb)");

		if(saveFilePath.length() <= 0)
		{
			QMessageBox::critical(m_pMainWindow, "Nanomite", "Invalid file selected!", QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		
		if(!WriteDataToFile(saveFilePath))
		{
			QMessageBox::critical(m_pMainWindow, "Nanomite", "Error while saving the data!", QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QMessageBox::information(m_pMainWindow, "Nanomite", "Data has been saved!", QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else
	{
		QString loadFilePath = QFileDialog::getOpenFileName(m_pMainWindow, "Please select a file to load", QDir::currentPath(), "Nanomite Project Files (*.ndb)");

		if(loadFilePath.length() <= 0)
		{
			QMessageBox::critical(m_pMainWindow, "Nanomite", "Invalid file selected!", QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		
		if(!ReadDataFromFile(loadFilePath))
		{
			QMessageBox::critical(m_pMainWindow, "Nanomite", "Error while reading the data!", QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		else
		{
			QMessageBox::information(m_pMainWindow, "Nanomite", "Data has been loaded!", QMessageBox::Ok, QMessageBox::Ok);
		}

		*pStartDebugging = true;
	}
}

clsProjectFile::~clsProjectFile()
{
	m_pMainWindow = NULL;
}

bool clsProjectFile::WriteDataToFile(const QString &saveFilePath)
{
	QFile saveFile(saveFilePath);
	if(!saveFile.open(QIODevice::WriteOnly))
		return false;

	QXmlStreamWriter xmlWriter(&saveFile);
	xmlWriter.setAutoFormatting(true);

	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("NANOMITE-DATA");

	WriteDebugDataToFile(xmlWriter);
	WriteBookmarkDataToFile(xmlWriter);
	WritePatchDataToFile(xmlWriter);
	WriteBreakpointDataToFile(xmlWriter);

	xmlWriter.writeEndElement();
	xmlWriter.writeEndDocument();

	saveFile.close();
	return true;
}

void clsProjectFile::WritePatchDataToFile(QXmlStreamWriter &xmlWriter)
{
	QList<PatchData> tempPatchList = qtDLGPatchManager::GetPatchList();
	QString tempPatchData;

	for(int i = 0; i < tempPatchList.size(); i++)
	{
		xmlWriter.writeStartElement(QString("PATCH_%1").arg(i));
		xmlWriter.writeTextElement("patchOffset",	QString("%1").arg(tempPatchList.at(i).Offset - tempPatchList.at(i).BaseOffset, 16, 16, QChar('0')));
		xmlWriter.writeTextElement("patchSize",		QString("%1").arg(tempPatchList.at(i).PatchSize));
		xmlWriter.writeTextElement("patchModule",	QString::fromWCharArray(tempPatchList.at(i).ModuleName));
		xmlWriter.writeTextElement("patchPMod",		tempPatchList.at(i).processModule);
	
		tempPatchData.clear();
		for(int patchData = 0; patchData < tempPatchList.at(i).PatchSize; patchData++)
			tempPatchData.append(QString("%1").arg(*((BYTE *)tempPatchList.at(i).newData + patchData), 2, 16, QChar('0')));

		xmlWriter.writeTextElement("patchNewData",	tempPatchData);

		tempPatchData.clear();
		for(int patchData = 0; patchData < tempPatchList.at(i).PatchSize; patchData++)
			tempPatchData.append(QString("%1").arg(*((BYTE *)tempPatchList.at(i).orgData + patchData), 2, 16, QChar('0')));

		xmlWriter.writeTextElement("patchOrgData",	tempPatchData);
		xmlWriter.writeEndElement();
	}
}

void clsProjectFile::WriteBookmarkDataToFile(QXmlStreamWriter &xmlWriter)
{
	QList<BookmarkData> tempBookmarkList = qtDLGBookmark::BookmarkGetList();

	for(int i = 0; i < tempBookmarkList.size(); i++)
	{
		xmlWriter.writeStartElement(QString("BOOKMARK_%1").arg(i));
		xmlWriter.writeTextElement("bookmarkOffset",	QString("%1").arg(tempBookmarkList.at(i).bookmarkOffset - tempBookmarkList.at(i).bookmarkBaseOffset, 16, 16, QChar('0')));
		xmlWriter.writeTextElement("bookmarkComment",	tempBookmarkList.at(i).bookmarkComment);
		xmlWriter.writeTextElement("bookmarkModule",	tempBookmarkList.at(i).bookmarkModule);
		xmlWriter.writeTextElement("bookmarkPMod",		tempBookmarkList.at(i).bookmarkProcessModule);
		xmlWriter.writeEndElement();
	}
}

void clsProjectFile::WriteDebugDataToFile(QXmlStreamWriter &xmlWriter)
{
	xmlWriter.writeStartElement("TARGET");
	xmlWriter.writeTextElement("FilePath", m_pMainWindow->coreDebugger->GetTarget());
	xmlWriter.writeTextElement("CommandLine", m_pMainWindow->coreDebugger->GetCMDLine());
	xmlWriter.writeEndElement();
}

void clsProjectFile::WriteBreakpointDataToFile(QXmlStreamWriter &xmlWriter)
{
	WriteBreakpointListToFile(m_pMainWindow->coreBPManager->SoftwareBPs, SOFTWARE_BP, xmlWriter);
	WriteBreakpointListToFile(m_pMainWindow->coreBPManager->MemoryBPs, MEMORY_BP, xmlWriter);
	WriteBreakpointListToFile(m_pMainWindow->coreBPManager->HardwareBPs, HARDWARE_BP, xmlWriter);
}

void clsProjectFile::WriteBreakpointListToFile(QList<BPStruct> &tempBP, int bpType, QXmlStreamWriter &xmlWriter)
{
	QString bpTypeString;

	switch(bpType)
	{
	case SOFTWARE_BP:	bpTypeString = "SW_BP"; break;
	case MEMORY_BP:		bpTypeString = "MEM_BP"; break;
	case HARDWARE_BP:	bpTypeString = "HW_BP"; break;
	default: return;
	}

	for(int i = 0; i < tempBP.size(); i++)
	{
		if(tempBP.at(i).dwHandle == BP_KEEP)
		{
			xmlWriter.writeStartElement(QString("BREAKPOINT_%1_%2").arg(bpTypeString).arg(i));

			xmlWriter.writeTextElement("breakpointOffset",		QString("%1").arg(tempBP.at(i).dwOffset - tempBP.at(i).dwBaseOffset, 16, 16, QChar('0')));
			xmlWriter.writeTextElement("breakpointSize",		QString("%1").arg(tempBP.at(i).dwSize, 8, 16, QChar('0')));
			xmlWriter.writeTextElement("breakpointTypeFlag",	QString("%1").arg(tempBP.at(i).dwTypeFlag, 8, 16, QChar('0')));
			xmlWriter.writeTextElement("breakpointModuleName",	QString(QString::fromWCharArray(tempBP.at(i).moduleName)));

			switch(bpType) // save only breakpoint specific data
			{
			case SOFTWARE_BP:
				{
					QString tempBuffer;

					for(unsigned int bpData = 0; bpData < tempBP.at(i).dwSize; bpData++)
						tempBuffer.append(QString("%1").arg(*((BYTE *)tempBP.at(i).bOrgByte + bpData), 2, 16, QChar('0')));

					xmlWriter.writeTextElement("breakpointData", tempBuffer);
					xmlWriter.writeTextElement("breakpointDataType", QString("%1").arg(tempBP.at(i).dwDataType, 8, 16, QChar('0')));
					break;
				}
			case MEMORY_BP:
				{
					xmlWriter.writeTextElement("breakpointOldProtection", QString("%1").arg(tempBP.at(i).dwOldProtection, 8, 16, QChar('0')));
					break;
				}
			case HARDWARE_BP:
				{
					xmlWriter.writeTextElement("breakpointSlot", QString("%1").arg(tempBP.at(i).dwSlot, 8, 16, QChar('0')));
					break;
				}
			}

			xmlWriter.writeEndElement();
		}
	}
}

bool clsProjectFile::ReadDataFromFile(const QString &loadFilePath)
{
	QFile loadFile(loadFilePath);
	if(!loadFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	m_pMainWindow->ClearDebugData(true);

	QXmlStreamReader xmlReader(&loadFile);
	while(!xmlReader.atEnd() && !xmlReader.hasError())
	{
		QXmlStreamReader::TokenType token = xmlReader.readNext();
		if(token == QXmlStreamReader::StartDocument)
		{
			continue;
		}
		else if(token == QXmlStreamReader::StartElement)
		{
			if(xmlReader.name() == "NANOMITE_DATA")
			{
				continue;
			}
			else if(xmlReader.name() == "TARGET")
			{
				if(!ReadDebugDataFromFile(xmlReader))
				{
					loadFile.close();
					return false;
				}
			}
			else if(xmlReader.name().contains("BOOKMARK_"))
			{
				ReadBookmarkDataFromFile(xmlReader);
			}
			else if(xmlReader.name().contains("PATCH_"))
			{
				ReadPatchDataFromFile(xmlReader);
			}
			//else if(xmlReader.name().contains("BREAKPOINT_"))
			//{
			//	ReadBreakpointDataFromFile(xmlReader);
			//}
		}
	}

	loadFile.close();
	return true;
}

bool clsProjectFile::ReadDebugDataFromFile(QXmlStreamReader &xmlReader)
{
	QString filePath, commandLine;

	xmlReader.readNext();

	while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "TARGET"))
	{
		if(xmlReader.tokenType() == QXmlStreamReader::StartElement)
		{
			if(xmlReader.name() == "FilePath")
			{
				xmlReader.readNext();
				filePath = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "CommandLine")
			{
				xmlReader.readNext();
				commandLine = xmlReader.text().toString();
			}
		}

		xmlReader.readNext();
	}

	if(filePath.length() <= 0)
	{
		return false;
	}
	else
	{
		m_pMainWindow->coreDebugger->SetTarget(filePath);
		m_pMainWindow->coreDebugger->SetCommandLine(commandLine);
	}

	return true;
}

void clsProjectFile::ReadBookmarkDataFromFile(QXmlStreamReader &xmlReader)
{
	QString bmOffset, bmComment, bmModule, bmPModule;

	xmlReader.readNext();

	while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name().contains("BOOKMARK_")))
	{
		if(xmlReader.tokenType() == QXmlStreamReader::StartElement)
		{
			if(xmlReader.name() == "bookmarkOffset")
			{
				xmlReader.readNext();
				bmOffset = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "bookmarkComment")
			{
				xmlReader.readNext();
				bmComment = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "bookmarkModule")
			{
				xmlReader.readNext();
				bmModule = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "bookmarkPMod")
			{
				xmlReader.readNext();
				bmPModule = xmlReader.text().toString();
			}
		}

		xmlReader.readNext();
	}

	if(bmOffset.length() > 0 && bmComment.length() > 0 && bmModule.length() > 0 && bmPModule.length() > 0)
	{
		BookmarkData newBookmark = { 0 };
		newBookmark.bookmarkComment = bmComment;
		newBookmark.bookmarkModule = bmModule;
		newBookmark.bookmarkProcessModule = bmPModule;
		newBookmark.bookmarkOffset = bmOffset.toULongLong(0,16);

		m_pMainWindow->dlgBookmark->BookmarkInsertFromProjectFile(newBookmark);
	}
}

void clsProjectFile::ReadBreakpointDataFromFile(QXmlStreamReader &xmlReader)
{

}

void clsProjectFile::ReadPatchDataFromFile(QXmlStreamReader &xmlReader)
{
	QString patchOffset, patchSize, patchModule, patchPMod, patchNewData, patchOrgData;

	xmlReader.readNext();

	while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name().contains("PATCH_")))
	{
		if(xmlReader.tokenType() == QXmlStreamReader::StartElement)
		{
			if(xmlReader.name() == "patchOffset")
			{
				xmlReader.readNext();
				patchOffset = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "patchSize")
			{
				xmlReader.readNext();
				patchSize = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "patchModule")
			{
				xmlReader.readNext();
				patchModule = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "patchPMod")
			{
				xmlReader.readNext();
				patchPMod = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "patchNewData")
			{
				xmlReader.readNext();
				patchNewData = xmlReader.text().toString();
			}
			else if(xmlReader.name() == "patchOrgData")
			{
				xmlReader.readNext();
				patchOrgData = xmlReader.text().toString();
			}
		}

		xmlReader.readNext();
	}

	if(patchOffset.length() > 0 && patchSize.length() > 0 && patchModule.length() > 0 && patchPMod.length() > 0 && patchNewData.length() > 0 && patchOrgData.length() > 0)
	{
		PatchData newPatch = { 0 };
		newPatch.Offset = patchOffset.toULongLong(0,16);
		newPatch.PatchSize = patchSize.toInt();
		newPatch.processModule = patchPMod;
		newPatch.newData = clsMemManager::CAlloc(newPatch.PatchSize);
		newPatch.orgData = clsMemManager::CAlloc(newPatch.PatchSize);
		newPatch.ModuleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

		ZeroMemory(newPatch.ModuleName, MAX_PATH * sizeof(TCHAR));
		patchModule.toWCharArray(newPatch.ModuleName);

		BYTE tempNewData = NULL, tempOrgData = NULL;
		for(int i = 0, d = 0; i < newPatch.PatchSize; i++ , d += 2)
		{
			tempNewData = patchNewData.mid(d, 2).toInt(0, 16);
			tempOrgData = patchOrgData.mid(d, 2).toInt(0, 16);

			memcpy((LPVOID)((DWORD)newPatch.newData + i), (LPVOID)&tempNewData, 1);
			memcpy((LPVOID)((DWORD)newPatch.orgData + i), (LPVOID)&tempOrgData, 1);
		}
		
		qtDLGPatchManager::InsertPatchFromProjectFile(newPatch);
	}
}