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
#ifndef QTDLGOPENNEWFILE_H
#define QTDLGOPENNEWFILE_H

#include <QtGui>
#include <QtCore>
#include <QFileDialog>

/**
* @file qtDLGOpenNewFile.h
* @brief Displaying the openfile widget
*/
class qtDLGOpenNewFile : public QDialog
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the stackview widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	*
	* @return no
	*/
	qtDLGOpenNewFile(QWidget *parent = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGOpenNewFile();

	/**
	* @brief Returns the input from the user
	* @param filePath A QString which contains the selected file path
	* @param commandLine A Qstring which contains the command line
	*
	* @return no
	*/
	void GetFilePathAndCommandLine(QString &filePath, QString &commandLine);

private:
	QFileDialog *m_newFileDialog;
	QGridLayout *m_fileDialogLayout;
	QLineEdit	*m_argumentLine;
	QLabel		*m_argumentLable;

	QString		m_filePath;
	QString		m_commandLine;

private slots:
	/**
	* @brief A Qt slot which get called when the user uses the "Open" button
	* @param filePath A QString which contains the selected file path
	*
	* @return no
	*/
	void fileSelected(const QString & filePath);
};
#endif
