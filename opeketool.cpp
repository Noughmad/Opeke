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

#include "settings.h"
#include "opeketool.h"

OpekeTool::OpekeTool ( QWidget * )
{
	ui_opeketool_base.setupUi ( this );
	ui_opeketool_base.sizeX->setLabel ( i18n ( "Length" ) );
	ui_opeketool_base.sizeY->setLabel ( i18n ( "Width" ) );
	ui_opeketool_base.sizeZ->setLabel ( i18n ( "Height" ) );
	QList<QColor> emptyList;
	ui_opeketool_base.color->setColors (emptyList);
	ui_opeketool_base.color->setColor (Qt::red);
	
	ui_opeketool_base.planeZ->setLabel ( i18n ( "Plane height" ) );
	changeType(Brick::Block);

	setupActions();
	
	ui_opeketool_base.sizeX->setValue(6);
	ui_opeketool_base.sizeY->setValue(4);
	ui_opeketool_base.sizeZ->setValue(3);
	
}

OpekeTool::~OpekeTool()
{

}

void OpekeTool::setupActions()
{
	//	connect(ui_opeketool_base.buttonRefresh, SIGNAL(clicked()), m_view, SLOT(updateGL()));

	connect ( this, SIGNAL ( changeSizeX ( int ) ), ui_opeketool_base.sizeX, SLOT ( setValue ( int ) ) );
	connect ( this, SIGNAL ( changeSizeY ( int ) ), ui_opeketool_base.sizeY, SLOT ( setValue ( int ) ) );
	connect ( this, SIGNAL ( changeSizeZ ( int ) ), ui_opeketool_base.sizeZ, SLOT ( setValue ( int ) ) );

	//	connect(ui_opeketool_base.color, SIGNAL(highlighted(QColor)), m_view, SLOT(setColor(QColor)));
	//	connect(ui_opeketool_base.color, SIGNAL(activated(QColor)), m_view, SLOT(setColor(QColor)));
	
	connect (ui_opeketool_base.b_rotateX, SIGNAL(clicked()), this, SLOT(rotateX()));
	connect (ui_opeketool_base.b_rotateY, SIGNAL(clicked()), this, SLOT(rotateY()));
	connect (ui_opeketool_base.b_rotateZ, SIGNAL(clicked()), this, SLOT(rotateZ()));
	
	connect (ui_opeketool_base.b_orientation, SIGNAL(activated(int)), this, SIGNAL(setOrientation(int)));
	connect (ui_opeketool_base.b_orientation_cyl, SIGNAL(activated(int)), this, SIGNAL(setCylOrientation(int)));
}

void OpekeTool::refresh()
{
	emit tool_refresh();
}

void OpekeTool::rotateX()
{
	int temp = ui_opeketool_base.sizeY->value();
	ui_opeketool_base.sizeY->setValue(ui_opeketool_base.sizeZ->value());
	ui_opeketool_base.sizeZ->setValue(temp);
	emit rotX();
	emit tool_refresh();
	emit getCylOrientation();
}

void OpekeTool::rotateY()
{
	int temp = ui_opeketool_base.sizeZ->value();
	ui_opeketool_base.sizeZ->setValue(ui_opeketool_base.sizeX->value());
	ui_opeketool_base.sizeX->setValue(temp);
	emit rotY();
	emit tool_refresh();
	emit getCylOrientation();
}

void OpekeTool::rotateZ()
{
	int temp = ui_opeketool_base.sizeX->value();
	ui_opeketool_base.sizeX->setValue(ui_opeketool_base.sizeY->value());
	ui_opeketool_base.sizeY->setValue(temp);
	emit rotZ();
	emit tool_refresh();
	emit getCylOrientation();
}

void OpekeTool::changeType(int changedType)
{
	if (changedType == Brick::Roof)
	{
		ui_opeketool_base.b_orientation->show();
		ui_opeketool_base.b_orientation_cyl->hide();
	}
	else if (changedType == Brick::Cylinder || changedType == Brick::InvertedCylinder)
	{
		ui_opeketool_base.b_orientation->hide();
		ui_opeketool_base.b_orientation_cyl->show();
	}
	else
	{
		ui_opeketool_base.b_orientation->hide();
		ui_opeketool_base.b_orientation_cyl->hide();
	}
}

void OpekeTool::changeTypeBlock()
{
	changeType(Brick::Block);
}

void OpekeTool::changeTypeRoof()
{
	changeType(Brick::Roof);
	emit setOrientation(ui_opeketool_base.b_orientation->currentIndex());
}

void OpekeTool::changeTypeCylinder()
{
	changeType(Brick::Cylinder);
	emit setCylOrientation(ui_opeketool_base.b_orientation_cyl->currentIndex());
}

void OpekeTool::changeTypeInvCyl()
{
	changeType(Brick::InvertedCylinder);
	emit setCylOrientation(ui_opeketool_base.b_orientation_cyl->currentIndex());
}

void OpekeTool::changeTypeSphere()
{
	changeType(Brick::Sphere);
}

void OpekeTool::changeCylOrientation(int changedOrientation)
{
	ui_opeketool_base.b_orientation_cyl->setCurrentIndex(changedOrientation);
}

#include "opeketool.moc"
