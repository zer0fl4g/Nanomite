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
#include "clsUpdater.h"

#ifdef _DEBUG
#include <QDebug>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
clsUpdater::clsUpdater(const QString &fileName, QObject *parent)
	: QObject(parent), m_fileName(fileName)
{
	init();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void clsUpdater::launchUpdater()
{
	m_process.start(m_fileName);
	m_process.waitForFinished();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void clsUpdater::init()
{
	connect(&m_process, SIGNAL(finished(int)),
		this,       SLOT(slot_checkExitCode(int)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void clsUpdater::slot_checkExitCode(int exitCode)
{
#ifdef _DEBUG
	qDebug() << exitCode;
#endif

	// 0 - return if all is OK
	// 1 - if there is some error
	// 2 - if need to update updater.exe ( this case is handle inner of updater.exe )

	// 3 - means that this is update of QtNanomite.exe and for install QtNanomite.exe need to close process of QtNanomite.exe.
	if (exitCode == 3) {
		m_process.start(m_fileName, QStringList() << "startUpdate");
		exit(0);
	}
}
