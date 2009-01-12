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

#include "KLocale"

#include "settings.h"
#include "opeketool.h"

OpekeTool::OpekeTool ( QWidget * parent)
	: QWidget(parent)
{
	setupUi ();
	
	sizeX->setLabel ( i18n ( "Length" ) );
	sizeY->setLabel ( i18n ( "Width" ) );
	sizeZ->setLabel ( i18n ( "Height" ) );
	QList<QColor> emptyList;
	color->setColors (emptyList);
	color->setColor (Qt::red);
	
	planeZ->setLabel ( i18n ( "Plane height" ) );
	changeType(Brick::Block);

	setupActions();
	
	sizeX->setValue(24);
	sizeY->setValue(16);
	sizeZ->setValue(12);
	

}

OpekeTool::~OpekeTool()
{

}

void OpekeTool::setupActions()
{
	//	connect(buttonRefresh, SIGNAL(clicked()), m_view, SLOT(updateGL()));

	connect ( this, SIGNAL ( changeSizeX ( int ) ), sizeX, SLOT ( setValue ( int ) ) );
	connect ( this, SIGNAL ( changeSizeY ( int ) ), sizeY, SLOT ( setValue ( int ) ) );
	connect ( this, SIGNAL ( changeSizeZ ( int ) ), sizeZ, SLOT ( setValue ( int ) ) );

	//	connect(color, SIGNAL(highlighted(QColor)), m_view, SLOT(setColor(QColor)));
	//	connect(color, SIGNAL(activated(QColor)), m_view, SLOT(setColor(QColor)));
	
	connect (b_rotateX, SIGNAL(clicked()), this, SLOT(rotateX()));
	connect (b_rotateY, SIGNAL(clicked()), this, SLOT(rotateY()));
	connect (b_rotateZ, SIGNAL(clicked()), this, SLOT(rotateZ()));
	
	connect (b_orientation, SIGNAL(activated(int)), this, SIGNAL(setOrientation(int)));
	connect (b_orientation_cyl, SIGNAL(activated(int)), this, SIGNAL(setCylOrientation(int)));
	
	connect ( color, SIGNAL ( highlighted ( QColor ) ), this, SIGNAL ( colorChanged( QColor ) ) );
	connect ( color, SIGNAL ( activated ( QColor ) ), this, SIGNAL ( colorViewed ( QColor ) ) );

	connect ( planeZ, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( planeChanged ( int ) ) );

	connect ( sizeX, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( SizeXChanged ( int ) ) );
	connect ( sizeY, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( SizeYChanged ( int ) ) );
	connect ( sizeZ, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( SizeZChanged ( int ) ) );
}

void OpekeTool::refresh()
{
	emit tool_refresh();
}

void OpekeTool::rotateX()
{
	int temp = sizeY->value();
	sizeY->setValue(sizeZ->value());
	sizeZ->setValue(temp);
	emit rotX();
	emit tool_refresh();
	emit getCylOrientation();
}

void OpekeTool::rotateY()
{
	int temp = sizeZ->value();
	sizeZ->setValue(sizeX->value());
	sizeX->setValue(temp);
	emit rotY();
	emit tool_refresh();
	emit getCylOrientation();
}

void OpekeTool::rotateZ()
{
	int temp = sizeX->value();
	sizeX->setValue(sizeY->value());
	sizeY->setValue(temp);
	emit rotZ();
	emit tool_refresh();
	emit getCylOrientation();
}

void OpekeTool::changeType(int changedType)
{
	if (changedType == Brick::Roof)
	{
		b_orientation->show();
		b_orientation_cyl->hide();
	}
	else if (changedType == Brick::Cylinder || changedType == Brick::InvertedCylinder)
	{
		b_orientation->hide();
		b_orientation_cyl->show();
	}
	else
	{
		b_orientation->hide();
		b_orientation_cyl->hide();
	}
}

void OpekeTool::changeTypeBlock()
{
	changeType(Brick::Block);
}

void OpekeTool::changeTypeRoof()
{
	changeType(Brick::Roof);
	emit setOrientation(b_orientation->currentIndex());
}

void OpekeTool::changeTypeCylinder()
{
	changeType(Brick::Cylinder);
	emit setCylOrientation(b_orientation_cyl->currentIndex());
}

void OpekeTool::changeTypeInvCyl()
{
	changeType(Brick::InvertedCylinder);
	emit setCylOrientation(b_orientation_cyl->currentIndex());
}

void OpekeTool::changeTypeSphere()
{
	changeType(Brick::Sphere);
}

void OpekeTool::changeOrientation(int changedOrientation)
{
	b_orientation->setCurrentIndex(changedOrientation);	
}

void OpekeTool::changeCylOrientation(int changedOrientation)
{
	b_orientation_cyl->setCurrentIndex(changedOrientation);
}

void OpekeTool::resizeEvent(QResizeEvent * event)
{
	if (event->size().width() > event->size().height())
	MainLayout->setDirection(QBoxLayout::LeftToRight);
	else
	MainLayout->setDirection(QBoxLayout::TopToBottom);
}

void OpekeTool::setupUi()
{
	setObjectName("Tool Options");
	if (layout()) delete layout();
	MainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	SizeLayout = new QVBoxLayout();
	
	sizeX = new KIntNumInput(this);
	sizeX->setValue(24);
	sizeX->setMinimum(1);
	
	sizeY = new KIntNumInput(this);
	sizeY->setValue(16);
	sizeY->setMinimum(1);
	SizeLayout->addWidget(sizeY);

	SizeLayout->addWidget(sizeX);
	sizeZ = new KIntNumInput(this);
	sizeZ->setValue(12);
	sizeZ->setMinimum(1);
	SizeLayout->addWidget(sizeZ);
	
	MainLayout->addLayout(SizeLayout);

	ColorLayout = new QVBoxLayout();
	
	color = new KColorCombo(this);
	ColorLayout->addWidget(color);

	planeZ = new KIntNumInput(this);
	ColorLayout->addWidget(planeZ);
	
	MainLayout->addLayout(ColorLayout);

	RotateLayout = new QVBoxLayout();
	
	b_rotateX = new KPushButton(this);
	RotateLayout->addWidget(b_rotateX);

	b_rotateY = new KPushButton(this);
	RotateLayout->addWidget(b_rotateY);

	b_rotateZ = new KPushButton(this);
	RotateLayout->addWidget(b_rotateZ);

	b_orientation = new KComboBox(this);
	RotateLayout->addWidget(b_orientation);

	b_orientation_cyl = new KComboBox(this);
	RotateLayout->addWidget(b_orientation_cyl);
	
	MainLayout->addLayout(RotateLayout);
	if (height() > width())
		MainLayout->setDirection(QBoxLayout::TopToBottom);
	else
		MainLayout->setDirection(QBoxLayout::LeftToRight);
}

void OpekeTool::changePlaneZ(int value)
{
	planeZ->setValue(value);
}

#include "opeketool.moc"
