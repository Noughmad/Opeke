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

#include "brick.h"
#include <math.h>
#include <QColor>
#include <KDebug>

#include <Ogre.h>

Brick::Brick ( Ogre::SceneNode* node, Ogre::Entity* entity )
{
	mNode = node;
	mEntity = entity;
	mMaterial = mEntity->getSubEntity ( 0 )->getMaterial().getPointer();
	mColor = mMaterial->getTechnique(0)->getPass(0)->getAmbient();
	mOrientation = 0;
	mHidden = false;
	mSelected = false;
}

Brick::~Brick()
{
	mNode->detachAllObjects();
	mNode->getCreator()->destroyEntity(mEntity);
	mNode->getCreator()->destroySceneNode(mNode->getName());
}

void Brick::setNode ( Ogre::SceneNode* n )
{
	mNode = n;
}

Ogre::SceneNode* Brick::node()
{
	return mNode;
}

void Brick::setType ( int t )
{
	mType = t;
}

int Brick::type()
{
	return mType;
}

void Brick::setOrientation ( int o )
{
	mOrientation = o;
}

int Brick::orientation()
{
	return mOrientation;
}

void Brick::setColor ( QColor c )
{
	setColor ( toOgreColour ( c ) );
}

void Brick::setColor ( Ogre::ColourValue c )
{
	mColor =  c;
	if ( !mMaterial ) mMaterial = mEntity->getSubEntity ( 0 )->getMaterial().getPointer();
	mMaterial->setAmbient ( c );
	mMaterial->setDiffuse ( Ogre::ColourValue(c.r, c.g, c.b, c.a*0.5  ));
	mMaterial->setSpecular(c);
}

QColor Brick::color()
{
	return toQColor(mColor);
}

Ogre::ColourValue Brick::getOgreColor()
{
	return mColor;
}

void Brick::setSizeX ( int x )
{
	mSize.x = x;
	mNode->setScale ( mSize );
}

void Brick::setSizeY ( int y )
{
	mSize.y = y;
	mNode->setScale ( mSize );
}

void Brick::setSizeZ ( int z )
{
	mSize.z = z;
	mNode->setScale ( mSize );
}

void Brick::setSize ( Ogre::Vector3 s )
{
	mSize = s;
	mNode->setScale ( mSize );
}

Ogre::Vector3 Brick::size()
{
	return mSize;
}

void Brick::setPosition ( Ogre::Vector3 pos )
{
	mNode->setPosition ( pos );
}

Ogre::Vector3 Brick::position()
{
	return mNode->getPosition();
}

void Brick::move ( float x, float y, float z )
{
	move ( Ogre::Vector3 ( x, y, z ) );
}


void Brick::move ( Ogre::Vector3 v )
{
	mNode->translate ( v );
}

void Brick::rotateX()
{

}

void Brick::rotateY()
{

}

void Brick::rotateZ()
{

}

void Brick::setEntity ( Ogre::Entity* entity )
{
	mNode->detachAllObjects();
	mNode->attachObject ( entity );
	mEntity = entity;
}

Ogre::Entity* Brick::entity()
{
	return mEntity;
}

Ogre::Material* Brick::material() const
{
	return mMaterial;
}


void Brick::setMaterial ( Ogre::Material* m )
{
	mMaterial = m;
}


bool Brick::isSelected() const
{
	return mSelected;
}


void Brick::setSelected ( bool s )
{
	if ( s ) mMaterial->setSceneBlending ( Ogre::SBT_TRANSPARENT_ALPHA );
	else mMaterial->setSceneBlending ( Ogre::SBT_REPLACE );
	mSelected = s;
}

Ogre::ColourValue Brick::toOgreColour ( QColor c )
{
	return Ogre::ColourValue ( (float)c.redF(), (float)c.greenF(), (float)c.blueF(), (float)c.alphaF() );
}

QColor Brick::toQColor ( Ogre::ColourValue c )
{
	return QColor ( c.r*255, c.g*255, c.b*255, c.a*255 );
}

void Brick::show()
{
	mNode->attachObject(mEntity);
	mHidden = false;
}

void Brick::hide()
{
	mNode->detachObject(mEntity);
	mHidden = true;
}


bool Brick::isHidden() const
{
	return mHidden;
}
