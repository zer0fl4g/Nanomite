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
#include "qtDLGOpenNewFile.h"

#include "clsMemManager.h"

qtDLGOpenNewFile::qtDLGOpenNewFile(QWidget *parent)
	: QDialog(parent),
	m_filePath(""),
	m_commandLine("")
{
	m_newFileDialog = new QFileDialog(this,"Please select a Target",QDir::currentPath(),"Executables (*.exe)");
	m_fileDialogLayout = static_cast<QGridLayout*>(m_newFileDialog->layout());
	m_argumentLine = new QLineEdit("");
	m_argumentLable = new QLabel("Arguments:");

	m_fileDialogLayout->addWidget(m_argumentLable, 4, 0, 1, 1);
	m_fileDialogLayout->addWidget(m_argumentLine, 4, 1, 2, 1);

	setLayout(m_fileDialogLayout);
	setWindowTitle("Please select a Target");

	connect(m_newFileDialog,SIGNAL(fileSelected(const QString &)),this,SLOT(fileSelected(const QString &)));
	connect(m_newFileDialog,SIGNAL(rejected()),this,SLOT(close()));
}

qtDLGOpenNewFile::~qtDLGOpenNewFile()
{
	delete m_newFileDialog;
	delete m_fileDialogLayout;
	delete m_argumentLine;
	delete m_argumentLable;
}

void qtDLGOpenNewFile::fileSelected(const QString & filePath)
{
	m_filePath = filePath;
	m_commandLine = m_argumentLine->text();

	close();
}

void qtDLGOpenNewFile::GetFilePathAndCommandLine(QString &filePath, QString &commandLine)
{
	filePath = m_filePath;
	commandLine = m_commandLine;
}