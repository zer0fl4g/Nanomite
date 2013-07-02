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
#ifndef CLSUPDATER_H
#define CLSUPDATER_H

#include <QObject>
#include <QProcess>

class clsUpdater : public QObject
{
	Q_OBJECT
public:
	clsUpdater(const QString &fileName, QObject *parent = 0);

	void launchUpdater();

private:
	void init();

	private Q_SLOTS:
		void slot_checkExitCode(int exitCode);

private:
	QProcess m_process;
	QString m_fileName;
};

#endif
