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

#define BUFSIZE 64
#define GRIDSIZE 20.0
#define CV 36

#include "opekeview.h"
#include "settings.h"

#include <KMessageBox>
#include <KIO/NetAccess>

#include <math.h>
#include <klocale.h>
#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QX11Info>

#include <Ogre.h>

#include <kdebug.h>

#ifndef PI
#define PI 3.141592
#endif

OpekeView::OpekeView ( QWidget * )
		: fileName ( QString() )

{
	ui_opekeview_base.setupUi ( this );

	setMouseTracking ( true );

	mBrickType = Brick::Block;
	activeBrick = 0;

	mRoot = 0;
	mResourceGroupManager = 0;
	mSceneManager = 0;
	mWindow = 0;
	mCamera = 0;
	mViewport = 0;

	mode = 1;

	maxHeight = 25;
	planeZ = 0;
	mColor = Ogre::ColourValue ( 1.0, 0.0, 0.0, 1.0 );

	Bricks.clear();
	undoList.clear();
	redoList.clear();

	mSize = ( Ogre::Vector3 ( 24, 16, 12 ) );
}

OpekeView::~OpekeView()
{
}

void OpekeView::setupOgre()
{
	try
	{
		mRoot = new Ogre::Root ( "../share/apps/opeke/plugins.cfg","ogre.cfg","ogre.log" );
		if ( !mRoot->restoreConfig() )
		{
			if ( !mRoot->showConfigDialog() )
			{
				return;
			}
		}

		Ogre::ConfigFile cf;
		cf.load ( "../share/apps/opeke/resources.cfg" );

		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
		while ( seci.hasMoreElements() )
		{
			Ogre::String secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
			for ( Ogre::ConfigFile::SettingsMultiMap::iterator i = settings->begin(); i != settings->end(); ++i )
			{
				Ogre::String typeName = i->first;
				Ogre::String archName = i->second;
				mRoot->addResourceLocation ( archName, typeName, secName );
			}
		}

		QX11Info windowInfo = x11Info ();

		Ogre::String dspl = Ogre::StringConverter::toString ( ( unsigned long ) windowInfo.display() );
		Ogre::String scrn = Ogre::StringConverter::toString ( ( unsigned int ) windowInfo.appScreen() );
		Ogre::String winHandle = Ogre::StringConverter::toString ( ( unsigned long ) winId() );

		Ogre::NameValuePairList miscParams;

		miscParams["parentWindowHandle"] = ( dspl + ":" + scrn + ":" + winHandle );

		mRoot->initialise ( false );
		mWindow = mRoot->createRenderWindow ( "Window",width(),height(),false,&miscParams );

		mSceneManager = mRoot->createSceneManager ( Ogre::ST_GENERIC,"SceneManager" );
		mCamera = mSceneManager->createCamera ( "Camera" );
		mViewport = mWindow->addViewport ( mCamera );
		mLight = mSceneManager->createLight ( "MainLight" );
	}
	catch ( Ogre::Exception& e )
	{
		KMessageBox::error ( this, e.getDescription().c_str() );
		return;
	}
}

void OpekeView::setupScene()
{
	mCamera->setPosition ( Ogre::Vector3 ( 0.0f, 0.0f, -500.0f ) );
	mCamera->lookAt ( Ogre::Vector3 ( 0.0f,1.0f,1.0f ) );
	mCamera->setNearClipDistance ( 5.0f );
	mCamera->setFarClipDistance ( 5000.0f );
	mViewport->setBackgroundColour ( Ogre::ColourValue ( 0.0f,0.0f,0.0f ) );
	mCamera->setAspectRatio ( Ogre::Real ( mViewport->getActualWidth() ) /Ogre::Real ( mViewport->getActualHeight() ) );
	mCamera->setAutoAspectRatio ( true );
	mCamera->setFixedYawAxis ( false );

	mLight->setPosition ( 0,100,500 );
	mLight->setDiffuseColour ( 1.0, 1.0, 1.0 );
	mLight->setSpecularColour ( 0.0, 0.0, 0.0 );

	nodeCount = 1;
	mMaterial = ( ( Ogre::MaterialPtr ) Ogre::MaterialManager::getSingleton().create ( "Material" + Ogre::StringConverter::toString ( nodeCount ), "General" ) ).getPointer();
	mMaterial->createTechnique();
	mMaterial->setAmbient ( 0.5, 0.0, 0.0 );
	mMaterial->setDiffuse ( 1.0, 0.0, 0.0, 0.5 );
	mMaterial->setSpecular ( 1.0, 1.0, 1.0, 0.5 );

	activeBrick = newBrick();
	activeBrick->setSize ( mSize );
	activeBrick->setColor ( mColor );
}

void OpekeView::update()
{
	if ( mWindow ) mWindow->update();
	//if(mRoot) mRoot->renderOneFrame();
}

void OpekeView::resizeEvent ( QResizeEvent* e )
{
	if ( !mWindow )
	{
		setupOgre();
		setupScene();
	}
	mWindow->resize ( width(), height() );
	mWindow->windowMovedOrResized();
	mViewport->_updateDimensions(); //RenderWindow should do this, but it doesn't. Perhaps it's only a bug in Ogre 1.4.9

	update();
}

void OpekeView::paintEvent ( QPaintEvent * )
{
	if ( !mWindow )
	{
		setupOgre();
		setupScene();
	}
	update();
}

void OpekeView::mousePressEvent ( QMouseEvent *event )
{

	if ( !hasFocus() )
	{
		setFocus();
		return;
	}

	switch ( event->buttons() )
	{
		case ( Qt::RightButton ) :
						moving = Ogre::Vector3 ( event->x(), -event->y(), 0 );
			break;
		case ( Qt::LeftButton ) :
						if ( !mode )
				{
					/**
					 * Select
					 */

					if ( activeBrick ) activeBrick->setSelected ( false );
					activeBrick = setSelect ( event->x(), event->y() );
					moving = transform ( event->x(), event->y() );
					if ( activeBrick )
					{
						preMovePos = activeBrick->position();
						preChangeColor = activeBrick->getOgreColor();
						kDebug() << preChangeColor.r << preChangeColor.g << preChangeColor.b;
					}
				}
			break;
		default:
			event->ignore();
			break;
	}
	mouseDown = true;
	update();
}

void OpekeView::mouseMoveEvent ( QMouseEvent *event )
{
	if ( !hasFocus() ) setFocus();
	if ( event->buttons() == Qt::RightButton )
	{
		current = Ogre::Vector3 ( event->x(), event->y(), 0 );
		mCamera->moveRelative ( ( current - moving ) /2 );
		moving = current;
	}
	else if ( mode == 1 )
	{
		if ( !activeBrick ) activeBrick = newBrick();
		activeBrick->setPosition ( transform ( event->x(), event->y() ) );
	}
	else if ( activeBrick && event->buttons() == Qt::LeftButton )
	{
		mouseDown = 1;
		current = transform ( event->x(), event->y() );
		activeBrick->move ( current - moving );
		moving = current;
	}
	update();
}

void OpekeView::mouseReleaseEvent ( QMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton )
	{
		if ( mode == 1 )	//if we're in build mode, add the active Brick to the list
		{
			if ( activeBrick )
			{
				UndoAction *u = new UndoAction ( activeBrick, UndoAction::Build );
				Bricks.append ( activeBrick );
				activeBrick = newBrick();
				activeBrick->setPosition ( transform ( event->x(), event->y() ) );

				if ( undoList.isEmpty() ) emit undoEmpty ( false );
				undoList.append ( u );
				redoList.clear();
				emit redoEmpty ( true );
				emit modified();
			}
		}
		else if ( activeBrick && preMovePos != activeBrick->position() )
		{
			UndoAction *u = new UndoAction ( activeBrick, UndoAction::Move );
			u->setPrevPos ( preMovePos );
			u->setNowPos ( activeBrick->position() );

			if ( undoList.isEmpty() ) emit undoEmpty ( false );
			undoList.append ( u );
			redoList.clear();
			emit redoEmpty ( true );
			emit modified();
		}
	}

	mouseDown = false;
	update();

}

void OpekeView::keyPressEvent ( QKeyEvent* event )
{
	float mod = 10.0;
	if ( event->isAutoRepeat() ) mod *= 2;
	else preMovePos = activeBrick->position();

	if ( event->modifiers() == Qt::ControlModifier )
	{
		switch ( event->key() )
		{
			case ( Qt::Key_Up ) :
							mCamera->pitch ( Ogre::Radian ( -mod/400 ) );
				break;

			case ( Qt::Key_Down ) :
							mCamera->pitch ( Ogre::Radian ( mod/400 ) );
				break;

			case ( Qt::Key_Left ) :
							mCamera->yaw ( Ogre::Radian ( -mod/400 ) );
				break;

			case ( Qt::Key_Right ) :
							mCamera->yaw ( Ogre::Radian ( mod/400 ) );
				break;

			default:
				event->ignore();
				break;
		}
	}
	else if ( event->modifiers() == Qt::ShiftModifier )
{
		switch ( event->key() )
		{
			case ( Qt::Key_Up ) :
							mCamera->moveRelative ( Ogre::Vector3 ( 0, 0, mod*2 ) );
				break;

			case ( Qt::Key_Down ) :
							mCamera->moveRelative ( Ogre::Vector3 ( 0, 0, -mod*2 ) );
				break;

			case ( Qt::Key_Left ) :
							mCamera->roll ( Ogre::Radian ( -mod/400 ) );
				break;

			case ( Qt::Key_Right ) :
							mCamera->roll ( Ogre::Radian ( +mod/400 ) );
				break;
			default:
				event->ignore();
				break;
		}
	}
	else
{
		if ( mode || selected < 0 )
		{
			switch ( event->key() )
			{
				case ( Qt::Key_Up ) :
								mCamera->moveRelative ( Ogre::Vector3 ( 0, -mod, 0 ) );
					break;

				case ( Qt::Key_Down ) :
								mCamera->moveRelative ( Ogre::Vector3 ( 0, mod, 0 ) );
					break;

				case ( Qt::Key_Left ) :
								mCamera->moveRelative ( Ogre::Vector3 ( mod, 0, 0 ) );
					break;

				case ( Qt::Key_Right ) :
								mCamera->moveRelative ( Ogre::Vector3 ( -mod, 0, 0 ) );
					break;

				case ( Qt::Key_2 ) :
								mCamera->moveRelative ( Ogre::Vector3 ( 0, 0, -mod*2 ) );
					break;

				case ( Qt::Key_8 ) :
								mCamera->moveRelative ( Ogre::Vector3 ( 0, 0, mod*1 ) );
					break;

				case ( Qt::Key_4 ) :
								mCamera->roll ( Ogre::Radian ( -mod/400 ) );
					break;

				case ( Qt::Key_6 ) :
								mCamera->roll ( Ogre::Radian ( mod/400 ) );
					break;
				default:
					event->ignore();
					break;
			}
		}
		else if ( currentBrick )
{
			switch ( event->key() )
			{
				case ( Qt::Key_Up ) :

								activeBrick->move ( 0, 1.0, 0 );
					uMove.y = 1.0;
					break;
				case ( Qt::Key_Down ) :
								activeBrick->move ( 0, -1.0, 0 );
					uMove.y = -1.0;
					break;
				case ( Qt::Key_Left ) :
								activeBrick->move ( -1.0, 0, 0 );
					uMove.x = -1.0;
					break;
				case ( Qt::Key_Right ) :
								activeBrick->move ( 1.0, 0, 0 );
					uMove.x = 1.0;
					break;
				case ( Qt::Key_8 ) :
								activeBrick->move ( 0, 0, 1.0 );
					uMove.z = 1.0;
					emit planeChanged ( planeZ + 1 );
					break;
				case ( Qt::Key_2 ) :
								activeBrick->move ( 0, 0, -1.0 );
					uMove.z = -1.0;
					emit planeChanged ( planeZ - 1 );
					break;
				default:
					event->ignore();
					break;
			}
		}
	}
	update();
}

void OpekeView::keyReleaseEvent ( QKeyEvent* event )
{
	int key = event->key();
	if ( !mode && selected >= 0 && ( key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right ||
	                                 key == Qt::Key_8 || key == Qt::Key_2 ) )
	{
		UndoAction *u = new UndoAction ( activeBrick, UndoAction::Move );
		u->setPrevPos ( preMovePos );
		u->setNowPos ( activeBrick->position() );
		if ( undoList.isEmpty() ) emit undoEmpty ( false );
		undoList.append ( u );
		redoList.clear();
		emit redoEmpty ( true );
		emit modified();
	}
	update();
}

void OpekeView::wheelEvent ( QWheelEvent *event )
{
	if ( !mode && selected >= 0 )
	{
		activeBrick->move ( 0,0, event->delta() /30 );
		emit planeChanged ( planeZ + event->delta() /30 );
		emit modified();
		emit redoEmpty ( true );
	}
	else
	{
		mCamera->moveRelative ( Ogre::Vector3 ( 0, 0, event->delta() /10 ) );
	}
	update();
}

Brick* OpekeView::newBrick(int type)
{
	nodeCount++;
	Ogre::SceneNode* activeNode = mSceneManager->getRootSceneNode()->createChildSceneNode ( "Node" + Ogre::StringConverter::toString ( nodeCount ) );
	Ogre::Entity* activeEntity = 0;

	switch ( type )
	{
		case Brick::Block:
			activeEntity = mSceneManager->createEntity ( "Brick" + Ogre::StringConverter::toString ( nodeCount ) , "block.mesh" );
			break;
		case Brick::Sphere:
			activeEntity = mSceneManager->createEntity ( "Brick" + Ogre::StringConverter::toString ( nodeCount ) , "sphere.mesh" );
			break;
		case Brick::Roof:
			activeEntity = mSceneManager->createEntity ( "Brick" + Ogre::StringConverter::toString ( nodeCount ) , "roof.mesh" );
			break;
		case Brick::Cylinder:
			activeEntity = mSceneManager->createEntity ( "Brick" + Ogre::StringConverter::toString ( nodeCount ) , "cylinder.mesh" );
			break;
		case Brick::InvertedCylinder:
			activeEntity = mSceneManager->createEntity ( "Brick" + Ogre::StringConverter::toString ( nodeCount ) , "invcyl.mesh" );
			break;
		default:
			activeEntity = mSceneManager->createEntity ( "Brick" + Ogre::StringConverter::toString ( nodeCount ) , "block.mesh" );
			break;
	}

	mMaterial = ( ( Ogre::MaterialPtr ) Ogre::MaterialManager::getSingleton().create ( "Material" + Ogre::StringConverter::toString ( nodeCount ), "General" ) ).getPointer();
	activeEntity->setMaterialName ( "Material" + Ogre::StringConverter::toString ( nodeCount ) );
	activeNode->attachObject ( activeEntity );
	Brick* mBrick = new Brick ( activeNode, activeEntity );
	mBrick->setColor ( mColor );
	mBrick->setSize ( mSize );
	return mBrick;
}

Brick* OpekeView::newBrick()
{
	return newBrick(mBrickType);
}


Ogre::Vector3 OpekeView::transform ( int x, int y )
{
	Ogre::Real xf = ( Ogre::Real ) x/Ogre::Real ( mViewport->getActualWidth() );
	Ogre::Real yf = ( Ogre::Real ) y/Ogre::Real ( mViewport->getActualHeight() );

	Ogre::Ray mRay = mCamera->getCameraToViewportRay ( xf, yf );
	Ogre::Vector3 mPoint;

	mPlane = Ogre::Plane ( Ogre::Vector3 ( 0.0,0.0,1.0 ),Ogre::Real ( planeZ ) );
	std::pair<bool, Ogre::Real> v = mRay.intersects ( mPlane );
	if ( v.first )
	{
		mPoint = mRay.getPoint ( v.second );
	}

	return mPoint;
}

Brick* OpekeView::setSelect ( int x, int y )
{
	Ogre::Ray mouseRay = mCamera->getCameraToViewportRay ( Ogre::Real ( x ) /Ogre::Real ( mViewport->getActualWidth() ), Ogre::Real ( y ) /Ogre::Real ( mViewport->getActualHeight() ) );
	mRaySceneQuery = mSceneManager->createRayQuery ( mouseRay );
	mRaySceneQuery->setSortByDistance ( true );
	Ogre::RaySceneQueryResult result = mRaySceneQuery->execute();

	Ogre::MovableObject *closestObject = 0;
	Ogre::Real closestDistance = 100000;

	Ogre::RaySceneQueryResult::iterator rayIterator;

	for ( rayIterator = result.begin(); rayIterator != result.end(); rayIterator++ )
	{
		if ( ( *rayIterator ).movable && closestDistance > ( *rayIterator ).distance )
		{
			closestObject = ( *rayIterator ).movable;
			closestDistance = ( *rayIterator ).distance;
		}
	}
//	mSceneManager->destroyQuery(mRaySceneQuery);
	Brick* sBrick;

	if ( closestObject )
	{
		sBrick = new Brick ( closestObject->getParentSceneNode(), ( Ogre::Entity* ) closestObject );
		sBrick->setSelected ( true );
		mRaySceneQuery->clearResults();
		emit delEnable ( true );
		return sBrick;
	}
	else
	{
		mRaySceneQuery->clearResults();
		emit delEnable ( false );
		return 0;
	}
}

void OpekeView::newScene()
{
	foreach ( Brick *b, Bricks ) delete b;
	Bricks.clear();
	mSceneManager->clearScene();
	setBuildMode();
	undoList.clear();
	redoList.clear();
	update();
	emit undoEmpty ( true );
	emit redoEmpty ( true );
	emit modified();
}

void OpekeView::setBuildMode()
{
	mode = 1;
	selected = -1;
	update();
	emit delEnable ( false );
}

void OpekeView::setSelectMode()
{
	delete activeBrick;
	activeBrick = 0;
	mode = 0;
	selected = -1;
	update();
	emit delEnable ( false );
}

void OpekeView::settingsChanged()
{
	maxHeight = Settings::val_maxh();
	bgColor = Settings::col_background();

	mViewport->setBackgroundColour ( Brick::toOgreColour ( bgColor ) );

	update();
	emit signalChangeStatusbar ( i18n ( "Settings changed" ) );
}

void OpekeView::viewColor ( QColor signaled_color )
{
	if ( activeBrick )
	{
		activeBrick->setColor ( signaled_color );
	}
	update();
}

void OpekeView::setColor ( QColor signaled_color )
{
	if ( activeBrick )
	{
		if ( !mode )
		{
			UndoAction *u = new UndoAction ( activeBrick, UndoAction::Color );
			redoList.clear();
			emit redoEmpty ( true );

			u->setPrevColor ( preChangeColor );
			u->setNowColor ( Brick::toOgreColour ( signaled_color ) );
			if ( undoList.isEmpty() ) emit undoEmpty ( false );
			undoList.append ( u );
		}
		activeBrick->setColor ( signaled_color );
	}
	mColor = Brick::toOgreColour ( signaled_color );
	update();
}

void OpekeView::setSizeX ( int x )
{
	if ( activeBrick ) activeBrick->setSizeX ( x );
	mSize.x = x;
	update();
}

void OpekeView::setSizeY ( int y )
{
	if ( activeBrick ) activeBrick->setSizeY ( y );
	mSize.y = y;
	update();
}

void OpekeView::setSizeZ ( int z )
{
	if ( activeBrick ) activeBrick->setSizeZ ( z );
	mSize.z = z;
	update();
}

void OpekeView::setPlaneZ ( int plane )
{
	if ( activeBrick ) activeBrick->move ( Ogre::Vector3 ( 0, 0, plane - planeZ ) );
	planeZ = plane;
	update();
}

void OpekeView::delBrick()
{
	if ( mode == 0 && activeBrick )
	{
		activeBrick->hide();

		UndoAction *u = new UndoAction ( activeBrick, UndoAction::Delete );
		undoList.append ( u );

		activeBrick = 0;

		emit modified();
	}
	if ( selected >= Bricks.count() )
	{
		selected = -1;
		emit delEnable ( false );
	}
	update();
}

void OpekeView::undo()
{
	if ( undoList.isEmpty() )
	{
		emit undoEmpty ( true );
		return;
	}
	if ( redoList.isEmpty() ) emit redoEmpty ( false );
	undoList.last()->undo();
	redoList.append ( undoList.takeLast() );

	if ( undoList.isEmpty() ) emit undoEmpty ( true );
	emit modified();
	update();
}

void OpekeView::redo()
{
	if ( redoList.isEmpty() )
	{
		emit redoEmpty ( true );
		return;
	}
	if ( undoList.isEmpty() ) emit undoEmpty ( false );

	redoList.last()->redo();
	undoList.append ( redoList.takeLast() );

	if ( redoList.isEmpty() ) emit redoEmpty ( true );
	emit modified();
	update();
}

void OpekeView::changeType ( int changedType )
{
	kDebug ( "OMG" );
	mBrickType = changedType;
	if ( mode == 1 && activeBrick )
	{
		delete activeBrick;
		activeBrick = newBrick();
	}
}

void OpekeView::changeTypeBlock()
{
	changeType ( Brick::Block );
}

void OpekeView::changeTypeRoof()
{
	changeType ( Brick::Roof );
}

void OpekeView::changeTypeCylinder()
{
	changeType ( Brick::Cylinder );
}

void OpekeView::changeTypeInvCyl()
{
	changeType ( Brick::InvertedCylinder );
}

void OpekeView::changeTypeSphere()
{
	changeType ( Brick::Sphere );
}

void OpekeView::changeOrientation ( int changedOrientation )
{
	if ( activeBrick ) activeBrick->setOrientation ( changedOrientation );
}

void OpekeView::rotateX()
{
	if ( activeBrick ) activeBrick->node()->pitch ( Ogre::Radian ( PI/2 ) );
}

void OpekeView::rotateY()
{
	if ( activeBrick ) activeBrick->node()->yaw ( Ogre::Radian ( PI/2 ) );
}

void OpekeView::rotateZ()
{
	if ( activeBrick ) activeBrick->node()->roll ( Ogre::Radian ( PI/2 ) );
}

void OpekeView::sendOrientation()
{
	if ( activeBrick ) emit signalOrientation ( activeBrick->orientation() );
}

void OpekeView::openBricks ( QFile* file )
{
	/**ž
	 * TODO: Fix the crashing
	 */
	
	newScene();
	QList<Brick*> inBricks;
	QDataStream in ( file );
	QString fileVersion, program;
	in>>program>>fileVersion;
	if ( program != "Opeke" )
	{
		KMessageBox::error ( this, i18n ( "The file you're trying to open is not a recongniseable Opeke file." ) );
		return;
	}

	if ( fileVersion == "0.1" )
	{
		while ( !in.atEnd() )
		{
			Brick* br = newBrick();
			br->setType ( Brick::Block );
			Ogre::Vector3 inPos[8], tPos = Ogre::Vector3 ( 0, 0, 0 );
			for ( int m = 0; m < 8; m++ )
			{
				in >> inPos[m].x;
				in >> inPos[m].y;
				in >> inPos[m].z;
				tPos += inPos[m];
			}
			br->setPosition ( tPos/8 );
			QColor tColor;
			in>>tColor;
			br->setColor ( tColor );
			br->setSize( inPos[2] - inPos[4] );
			inBricks.append ( br );
		}
	}
	else if ( fileVersion == "0.2" )
	{
		while ( !in.atEnd() )
		{
			Brick* br = newBrick();
			int type;
			in >> type;
			br->setType ( Brick::Block );

			int size[3];
			in >> size[0] >> size[1] >> size[2];
			br->setSize ( Ogre::Vector3 ( size[1], size[2], size[3] ));

			               Ogre::Vector3 inPos, tPos = Ogre::Vector3 ( 0, 0, 0 );
			               for ( int m = 0; m < 8; m++ )
		{
			in >> inPos.x;
			in >> inPos.y;
			in >> inPos.z;
			tPos += inPos;
		}
		br->setPosition ( tPos/8 );

		QColor tColor;
		in>>tColor;
		br->setColor ( tColor );

		inBricks.append ( br );
	}
}
else if ( fileVersion == "0.3" )
	{
		while ( !in.atEnd() )
		{
			int brickType;
			float x, y, z;
			in>>brickType;
			Brick *br = newBrick ( brickType );
			in>>x>>y>>z;
			br->setPosition ( Ogre::Vector3 ( x, y, z ) );

			int size[3];
			in>>size[0];
			in>>size[1];
			in>>size[2];

			int tOrientation;
			in >> tOrientation;
			br->setOrientation ( tOrientation );

			QColor color;
			in>>color;
			br->setColor ( color );

			inBricks.append ( br );
		}
	}
	else
	{
		KMessageBox::error ( this, i18n ( "This file was made by an unknown Opeke version." ) );
		return;
	}
	newScene();
	Bricks = inBricks;
}

void OpekeView::saveBricks ( KSaveFile* file )
{
	/**
	 * TODO: Save Bricks into the file
	 */	
}

#include "opekeview.moc"
