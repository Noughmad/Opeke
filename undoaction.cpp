//
// C++ Implementation: undoaction
//
// Description:
//
//
// Author: Miha Čančula <miha@noughmad.org>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "undoaction.h"
#include <KDebug>

UndoAction::UndoAction ( Brick* brick, int type )
{
	mBrick = brick;
	mType = type;
}

UndoAction::~ UndoAction()
{
}

int UndoAction::getType() const
{
	return mType;
}


void UndoAction::setType ( int value )
{
	mType = value;
}


int UndoAction::getBrickType() const
{
	return mBrickType;
}


void UndoAction::setBrickType ( int value )
{
	mBrickType = value;
}


Ogre::ColourValue UndoAction::getPrevColor() const
{
	return prevColor;
}


void UndoAction::setPrevColor ( const Ogre::ColourValue& value )
{
	prevColor = value;
}


Ogre::ColourValue UndoAction::getNowColor() const
{
	return nowColor;
}


void UndoAction::setNowColor ( const Ogre::ColourValue& value )
{
	nowColor = value;
}


Ogre::Vector3 UndoAction::getPrevPos() const
{
	return prevPos;
}


void UndoAction::setPrevPos ( const Ogre::Vector3& value )
{
	prevPos = value;
}


Ogre::Vector3 UndoAction::getNowPos() const
{
	return nowPos;
}


void UndoAction::setNowPos ( const Ogre::Vector3& value )
{
	nowPos = value;
}


Ogre::Vector3 UndoAction::getPrevSize() const
{
	return prevSize;
}


void UndoAction::setPrevSize ( const Ogre::Vector3& value )
{
	prevSize = value;
}


Ogre::Vector3 UndoAction::getNowSize() const
{
	return nowSize;
}


void UndoAction::setNowSize ( const Ogre::Vector3& value )
{
	nowSize = value;
}

void UndoAction::redo()
{
	switch (mType)
	{
		case Build:
			mBrick->show();
			break;
		case Move:
			mBrick->setPosition(nowPos);
			break;
		case Delete:
			mBrick->hide();
			break;
		case Resize:
			mBrick->setSize(nowSize);
			break;
		case Color:
			mBrick->setColor(nowColor);
			break;
		default:
			return;
			break;
	}
}

void UndoAction::undo()
{
	switch (mType)
	{
		case Build:
			mBrick->hide();
			break;
		case Move:
			mBrick->setPosition(prevPos);
			break;
		case Delete:
			mBrick->show();
			break;
		case Resize:
			mBrick->setSize(prevSize);
			break;
		case Color:
			mBrick->setColor(prevColor);
			break;
		default:
			return;
			break;
	}
}
