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

clsProjectFile::clsProjectFile(bool isSaveFile) :
	m_pMainWindow(qtDLGNanomite::GetInstance())
{
	if(isSaveFile)
	{
		QString saveFilePath = QFileDialog::getSaveFileName(m_pMainWindow, "Please select a save path!", QDir::currentPath(), "Nanomite Project Files (*.ndb)");

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
		QString loadFilePath = QFileDialog::getOpenFileName(m_pMainWindow, "Please select a save path!", QDir::currentPath(), "Nanomite Project Files (*.ndb)");

		if(loadFilePath.length() <= 0)
		{
			QMessageBox::critical(m_pMainWindow, "Nanomite", "Invalid file selected!", QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		
		if(!ReadDataFromFile(loadFilePath))
		{
			QMessageBox::critical(m_pMainWindow, "Nanomite", "Error while reading the data!", QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QMessageBox::information(m_pMainWindow, "Nanomite", "Data has been loaded!", QMessageBox::Ok, QMessageBox::Ok);
		}
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
		xmlWriter.writeTextElement("bookmarkOffset",	QString("%1").arg(tempBookmarkList.at(i).bookmarkOffset, 16, 16, QChar('0')));
		xmlWriter.writeTextElement("bookmarkComment",	tempBookmarkList.at(i).bookmarkComment);
		xmlWriter.writeTextElement("bookmarkModule",	tempBookmarkList.at(i).bookmarkModule);
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
			xmlWriter.writeTextElement("breakpointSize",		QString("%1").arg(tempBP.at(i).dwSize, 16, 16, QChar('0')));
			xmlWriter.writeTextElement("breakpointTypeFlag",	QString("%1").arg(tempBP.at(i).dwTypeFlag, 16, 16, QChar('0')));
			xmlWriter.writeTextElement("breakpointModuleName",	QString(QString::fromWCharArray(tempBP.at(i).moduleName)));

			switch(bpType) // save only breakpoint specific data
			{
			case SOFTWARE_BP:
				{
					QString tempBuffer;

					for(int bpData = 0; bpData < tempBP.at(i).dwSize; bpData++)
						tempBuffer.append(QString("%1").arg(*((BYTE *)tempBP.at(i).bOrgByte + bpData), 2, 16, QChar('0')));

					xmlWriter.writeTextElement("breakpointData", QString(tempBuffer));
					xmlWriter.writeTextElement("breakpointDataType", QString("%1").arg(tempBP.at(i).dwDataType, 16, 16, QChar('0')));
					break;
				}
			case MEMORY_BP:
				{
					xmlWriter.writeTextElement("breakpointOldProtection", QString("%1").arg(tempBP.at(i).dwOldProtection, 16, 16, QChar('0')));
					break;
				}
			case HARDWARE_BP:
				{
					xmlWriter.writeTextElement("breakpointSlot", QString("%1").arg(tempBP.at(i).dwSlot, 16, 16, QChar('0')));
					break;
				}
			}

			xmlWriter.writeEndElement();
		}
	}
}

bool clsProjectFile::ReadDataFromFile(const QString &loadFilePath)
{
	return true;
}