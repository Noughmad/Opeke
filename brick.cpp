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
#define PI 3.141592


Brick::Brick ( Ogre::SceneNode* sizeNode, Ogre::SceneNode* rotNode, Ogre::Entity* entity )
{
	mSizeNode = sizeNode;
	mRotateNode = rotNode;
	mEntity = entity;
	mMaterial = mEntity->getSubEntity ( 0 )->getMaterial().getPointer();
	mColor = mMaterial->getTechnique(0)->getPass(0)->getAmbient();
	mOrientation = mRotateNode->getOrientation();
	mHidden = false;
	mSelected = false;
}

Brick::~Brick()
{
	mRotateNode->detachAllObjects();
	mSizeNode->detachAllObjects();
	mSizeNode->getCreator()->destroyEntity(mEntity);
	mRotateNode->getCreator()->destroySceneNode(mSizeNode->getName());
	mRotateNode->getCreator()->destroySceneNode(mRotateNode->getName());
}


void Brick::setType ( int t )
{
	mType = t;
}

int Brick::type()
{
	return mType;
}

void Brick::setOrientation ( Ogre::Quaternion o )
{
	mRotateNode->setOrientation(o);
}

Ogre::Quaternion Brick::orientation()
{
	return mRotateNode->getOrientation();
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
	if (mSelected) mMaterial->setDiffuse(c/2);
	else mMaterial->setDiffuse(c);
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
	mSizeNode->setScale ( mSize );
}

void Brick::setSizeY ( int y )
{
	mSize.y = y;
	mSizeNode->setScale ( mSize );
}

void Brick::setSizeZ ( int z )
{
	mSize.z = z;
	mSizeNode->setScale ( mSize );
}

void Brick::setSize ( Ogre::Vector3 s )
{
	mSize = s;
	mSizeNode->setScale ( mSize );
}

Ogre::Vector3 Brick::size()
{
	return mSize;
}

void Brick::setPosition ( Ogre::Vector3 pos )
{
	mSizeNode->setPosition ( pos );
}

Ogre::Vector3 Brick::position()
{
	return mSizeNode->getPosition();
}

void Brick::move ( float x, float y, float z )
{
	move ( Ogre::Vector3 ( x, y, z ) );
}


void Brick::move ( Ogre::Vector3 v )
{
	mSizeNode->translate ( v );
}

void Brick::rotateX()
{
	mRotateNode->yaw(Ogre::Radian ( PI/2 ) );
}

void Brick::rotateY()
{
	mRotateNode->pitch(Ogre::Radian ( PI/2 )); 
}

void Brick::rotateZ()
{
	mRotateNode->roll(Ogre::Radian ( PI/2 ) );
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
	if ( s ) 
	{
		mMaterial->setDiffuse(mColor/2);
		mMaterial->setSceneBlending ( Ogre::SBT_TRANSPARENT_ALPHA );
	}
	else
	{
		mMaterial->setDiffuse(mColor);
		mMaterial->setSceneBlending ( Ogre::SBT_REPLACE );
	}
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
	mRotateNode->attachObject(mEntity);
	mHidden = false;
}

void Brick::hide()
{
	mRotateNode->detachObject(mEntity);
	mHidden = true;
}


bool Brick::isHidden() const
{
	return mHidden;
}
