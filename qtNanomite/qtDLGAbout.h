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
#ifndef QTDLGABOUT_H
#define QTDLGABOUT_H

#include "ui_qtDLGAbout.h"

/**
* @file qtDLGAbout.h
* @brief Displaying the about dialog
*/
class qtDLGAbout : public QDialog
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the about dialog
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGAbout(QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGAbout();
	
private:
	Ui::qtDLGAboutUI ui; /* a private variable holding the gui instance  */
};

#endif
