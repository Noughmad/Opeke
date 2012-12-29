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

	setupActions();
}

OpekeTool::~OpekeTool()
{

}

void OpekeTool::setupActions()
{
	connect (b_rotateX, SIGNAL(clicked()), this, SIGNAL(rotX()));
	connect (b_rotateY, SIGNAL(clicked()), this, SIGNAL(rotY()));
	connect (b_rotateZ, SIGNAL(clicked()), this, SIGNAL(rotZ()));
		
	connect ( color, SIGNAL ( highlighted ( QColor ) ), this, SIGNAL ( colorChanged( QColor ) ) );
	connect ( color, SIGNAL ( activated ( QColor ) ), this, SIGNAL ( colorViewed ( QColor ) ) );

	connect ( planeZ, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( planeChanged ( int ) ) );

	connect ( sizeX, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( sizeXChanged ( int ) ) );
	connect ( sizeY, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( sizeYChanged ( int ) ) );
	connect ( sizeZ, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( sizeZChanged ( int ) ) );
}

void OpekeTool::rotateX()
{
	emit rotX();
}

void OpekeTool::rotateY()
{
	int temp = sizeZ->value();
	sizeZ->setValue(sizeX->value());
	sizeX->setValue(temp);
	emit rotY();
}

void OpekeTool::rotateZ()
{
	int temp = sizeX->value();
	sizeX->setValue(sizeY->value());
	sizeY->setValue(temp);
	emit rotZ();
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
	sizeX->setValue(6);
	sizeX->setMinimum(1);
	
	sizeY = new KIntNumInput(this);
	sizeY->setValue(4);
	sizeY->setMinimum(1);
	SizeLayout->addWidget(sizeY);

	SizeLayout->addWidget(sizeX);
	sizeZ = new KIntNumInput(this);
	sizeZ->setValue(3);
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
	b_rotateX->setText(i18n("Rotate around Width"));
	RotateLayout->addWidget(b_rotateX);

	b_rotateY = new KPushButton(this);
	b_rotateY->setText(i18n("Rotate around Length"));
	RotateLayout->addWidget(b_rotateY);

	b_rotateZ = new KPushButton(this);
	b_rotateZ->setText(i18n ("Rotate around Height"));
	RotateLayout->addWidget(b_rotateZ);
	
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
