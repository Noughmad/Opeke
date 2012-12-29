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

#include <QtGui/QWidget>
#include <QtGui/QResizeEvent>
#include <QtGui/QButtonGroup>
#include <QtGui/QFormLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "KColorCombo"
#include "KComboBox"
#include "KNumInput"
#include "KPushButton"
#include "brick.h"

class QPainter;
class KUrl;

class OpekeTool : public QWidget
{
		Q_OBJECT

	public:
		OpekeTool ( QWidget * parent );
		virtual ~OpekeTool();
	private:
		QBoxLayout *MainLayout;
		QVBoxLayout *SizeLayout;
		KIntNumInput *sizeX;
		KIntNumInput *sizeZ;
		KIntNumInput *sizeY;
		QVBoxLayout *ColorLayout;
		KColorCombo *color;
		KIntNumInput *planeZ;
		QVBoxLayout *RotateLayout;
		KPushButton *b_rotateX;
		KPushButton *b_rotateY;
		KPushButton *b_rotateZ;

		void setupUi();
		void setupActions();
		
	protected:
		void resizeEvent(QResizeEvent *event);

	private slots:
		void rotateX();
		void rotateY();
		void rotateZ();
		
		void changePlaneZ(int);
	
	signals:
		void changeTool ( const int toolID );
		
		void rotX();
		void rotY();
		void rotZ();
		
		void colorChanged(QColor);
		void colorViewed(QColor);
		void planeChanged(int);
		void sizeXChanged(int);
		void sizeYChanged(int);
		void sizeZChanged(int);
};

#endif //opekeTOOL_H

