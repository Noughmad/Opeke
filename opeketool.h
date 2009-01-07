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

#ifndef OPEKE_TOOL_H
#define OPEKE_TOOL_H

#include "opeke.h"

#include <QtGui/QDockWidget>

#include "ui_opeketool_base.h"

class QPainter;
class KUrl;

class OpekeTool : public QDockWidget, public Ui::opeketool_base
{
		Q_OBJECT

	public:
		OpekeTool ( QWidget * parent );
		virtual ~OpekeTool();

		Ui::opeketool_base ui_opeketool_base;

	private:

		void setupActions();

	private slots:
		void refresh();
		void rotateX();
		void rotateY();
		void rotateZ();
		void changeType(int changedType);
		void changeTypeBlock();
		void changeTypeRoof();
		void changeTypeCylinder();
		void changeTypeInvCyl();
		void changeTypeSphere();
		
	//	void changeOrientation(int changedOrientation);
		void changeCylOrientation(int changedOrientation);
				
	signals:
		void changeTool ( const int toolID );
		void tool_refresh();

		void changeSizeX ( int );
		void changeSizeY ( int );
		void changeSizeZ ( int );
		
		void setOrientation(int);
		void setCylOrientation(int);
	//	void getOrientation();
		void getCylOrientation();
		
		void rotX();
		void rotY();
		void rotZ();
};

#endif //opekeTOOL_H

