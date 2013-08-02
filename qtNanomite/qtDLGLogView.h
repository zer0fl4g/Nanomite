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
#ifndef QTDLGLOGVIEW_H
#define QTDLGLOGVIEW_H

#include <QDockWidget>
#include <QString>

#include <string>

#include "ui_qtDLGLogView.h"

/**
* @file qtDLGLogView.h
* @brief Displaying the logview widget
*/
class qtDLGLogView : public QDockWidget, public Ui_qtDLGLogView
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the regedit dialog
	* @param parent Takes the a QWidget pointer of the calling QObject
	*
	* @return no
	*/
	qtDLGLogView(QWidget *parent = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGLogView();

	/**
	* @brief Is called from the OnLog Slot or if some message directly needs to be displayed
	* @param logString The line which will be saved to gui
	* 
	* @return no
	*/
	void OnLog(QString logString);

public slots:
	/**
	* @brief A Qt slot which is called when the user does a right click to open the
	* context menu
	* @param qPoint The point where the user right clicked
	*
	* @return no
	*/
	void OnContextMenu(QPoint qPoint);
	/**
	* @brief A Qt slot which is called when the user clicked the close button
	* @param pAction The selected action from the context menu
	*
	* @return no
	*/
	void MenuCallback(QAction* pAction);
	/**
	* @brief A Qt slot which is called when the clsDebugger instance wants to place
	* a log line in the gui
	* @param logString The line which will be saved to gui
	* 
	* @return no
	*/
	void OnLog(std::wstring logString);

private:
	int m_selectedRow; /* contains the selected row if the context menu was activated */
};

#endif // QTDLGLOGVIEW_H
