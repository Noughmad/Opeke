/***************************************************************************
 *   Copyright (C) 2008 by Miha Čančula  *
 *   miha@noughmad.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef OPEKE_H
#define OPEKE_H

#include <kxmlguiwindow.h>
#include <kaction.h>

#include "brick.h"
#include "opekeview.h"

#include "ui_prefs_base.h"

class OpekeView;
class OpekeTool;
class KPrinter;
class KToggleAction;
class KUrl;

/**
 * This class serves as the main window for Opeke.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Miha Čančula <miha@noughmad.org>
 * @version 0.4
 */
class Opeke : public KXmlGuiWindow
{
		Q_OBJECT
	public:
		/**
		 * Default Constructor
		 */
		Opeke();

		/**
		 * Default Destructor
		 */
		virtual ~Opeke();
		
		void openFile(const QString &inputFileName);


	private:

		/*void readConfig();
		void saveSettings();*/
		void setupActions();

		Ui::prefs_base ui_prefs_base;
		OpekeView *m_view;
		OpekeTool *m_tool;

		KPrinter   *m_printer;
		KToggleAction *m_toolbarAction;
		KToggleAction *m_statusbarAction;

		QString fileName;
		QList<Brick> Bricks;
		bool m_gridEnabled;
			
		KAction *undoAct, *redoAct, *removeAct;
		
	private slots:
		void fileNew();
		void optionsPreferences();
		void openFile();
		void saveFile();
		void saveFileAs(const QString &outputFileName);
		void saveFileAs();
		void undoEnable(bool empty);
		void redoEnable(bool empty);
		void removeEnable(bool enable);
		void fileModified();

	signals:
		void reload();
		void clear();
		void BricksLoaded(QList<Brick*>);
};

#endif // _opeke_H_
