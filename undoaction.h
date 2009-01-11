//
// C++ Interface: undoaction
//
// Description:
//
//
// Author: Miha Čančula <miha@noughmad.org>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UNDOACTION_H
#define UNDOACTION_H

#include "brick.h"

/**
	@author Miha Čančula <miha@noughmad.org>
*/

class UndoAction
{
	public:
		UndoAction ( Brick* brick, int type );
		~UndoAction();
		
		enum Type {Build, Move, Delete, Resize, Color};
		
		void undo();
		void redo();

		void setType ( int value );
		int getType() const;
		void setBrickType ( int value );
		int getBrickType() const;
		void setPrevColor ( const Ogre::ColourValue& value );
		Ogre::ColourValue getPrevColor() const;
		void setNowColor ( const Ogre::ColourValue& value );
		Ogre::ColourValue getNowColor() const;
		void setPrevPos ( const Ogre::Vector3& value );
		Ogre::Vector3 getPrevPos() const;
		void setNowPos ( const Ogre::Vector3& value );
		Ogre::Vector3 getNowPos() const;
		void setPrevSize ( const Ogre::Vector3& value );
		Ogre::Vector3 getPrevSize() const;
		void setNowSize ( const Ogre::Vector3& value );
		Ogre::Vector3 getNowSize() const;

	private:
		int mType;
		Brick* mBrick;
		int mBrickType;
		Ogre::ColourValue prevColor, nowColor;
		Ogre::Vector3 prevPos, nowPos;
		Ogre::Vector3 prevSize, nowSize;
};

#endif
