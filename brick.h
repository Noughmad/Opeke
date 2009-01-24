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

#ifndef BRICK_H
#define BRICK_H

#include <QtGui/QColor>
#include <Ogre.h>
#include <math.h>


class Brick
{
	public:
		
		Brick(Ogre::SceneNode* node, Ogre::Entity* entity);
		virtual ~Brick();
		
		/**
		 * Always make sure to only append to this list, not insert into it, for compatibility
		 */
		
		enum BrickType {Block, Roof, Cylinder, InvertedCylinder, Sphere, Cone, CornerRoof, Pyramid, InvertedRoofCorner};
		
		void setNode(Ogre::SceneNode* n);
		Ogre::SceneNode* node();
		
		void setEntity(Ogre::Entity* e);
		Ogre::Entity* entity();	

		void setType(int t);
		int type();
		
		void setOrientation(Ogre::Quaternion o);
		Ogre::Quaternion orientation();
		
		void setColor(QColor c);
		void setColor(Ogre::ColourValue c);
		QColor color();
		Ogre::ColourValue getOgreColor();
		
		void setSizeX(int x);
		void setSizeY(int y);
		void setSizeZ(int z);
		void setSize(Ogre::Vector3);
		Ogre::Vector3 size();
		
		void setPosition(Ogre::Vector3 pos);
		Ogre::Vector3 position();
				
		void move(float x, float y, float z);
		void move(Ogre::Vector3 m);
		void center(Ogre::Vector3 c);
		
		void rotateX();
		void rotateY();
		void rotateZ();

		void setMaterial ( Ogre::Material* m );
		Ogre::Material* material() const;

		void setSelected ( bool s );
		bool isSelected() const;
		
		static Ogre::ColourValue toOgreColour(QColor c);
		static QColor toQColor(Ogre::ColourValue c);
		
		void hide();
		void show();

	bool isHidden() const;
	
	
	
	protected:
		
		Ogre::Entity* mEntity;
		Ogre::SceneNode* mNode;
		Ogre::Material* mMaterial;
		int mType;
		Ogre::Quaternion mOrientation;
		Ogre::Vector3 mSize;
		Ogre::ColourValue mColor;
		bool mSelected;
		bool mHidden;
};

#endif
