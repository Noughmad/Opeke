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

OpekeView::OpekeView ( QWidget * )
	: fileName ( QString() )

{	
	selectedBrick = " ";
	ui_opekeview_base.setupUi ( this );

	setMouseTracking(true);
	
	mBrickType = Brick::Block;
	activeBrick = 0;
	currentBrick = 0;
	activeNode = 0;
	
	mRoot = 0;
	mResourceGroupManager = 0;
	mSceneManager = 0;
	mWindow = 0;
	mCamera = 0;
	mViewport = 0;
	
	mode = 1;
	
	maxHeight = 25;
	planeZ = 0;
	mColor = Ogre::ColourValue(1.0, 0.0, 0.0, 1.0);
	
	Bricks.clear();
	uList.clear();
	rList.clear();
	
	mSize = (Ogre::Vector3(24, 16, 12));
	
	readConfig();
	settingsChanged();
}

OpekeView::~OpekeView()
{
}

void OpekeView::setupOgre()
{
	try
	{
		mRoot = new Ogre::Root("../share/apps/opeke/plugins.cfg","ogre.cfg","ogre.log");
		if (!mRoot->restoreConfig())
		{
			if (!mRoot->showConfigDialog())
			{
				return;
			}
		}
		
		Ogre::ConfigFile cf;
		cf.load("../share/apps/opeke/resources.cfg");    

		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
		while (seci.hasMoreElements())
		{
			Ogre::String secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
			for (Ogre::ConfigFile::SettingsMultiMap::iterator i = settings->begin(); i != settings->end(); ++i)
			{
				Ogre::String typeName = i->first;
				Ogre::String archName = i->second;
				mRoot->addResourceLocation(archName, typeName, secName);
			}
		}
		
		QX11Info windowInfo = x11Info ();
				
		Ogre::String dspl = Ogre::StringConverter::toString((unsigned long)windowInfo.display());
		Ogre::String scrn = Ogre::StringConverter::toString((unsigned int)windowInfo.appScreen());
		Ogre::String winHandle = Ogre::StringConverter::toString((unsigned long)winId());
		
		Ogre::NameValuePairList miscParams;
		
		miscParams["parentWindowHandle"] = (dspl + ":" + scrn + ":" + winHandle);
		
		mRoot->initialise(false);
		mWindow = mRoot->createRenderWindow("Window",width(),height(),false,&miscParams);
		
		mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC,"sceneManager");
		mCamera = mSceneManager->createCamera("Camera");
		mViewport = mWindow->addViewport(mCamera);
		mLight = mSceneManager->createLight("MainLight");
	}
	catch (Ogre::Exception& e)
	{
		KMessageBox::error ( this, e.getDescription().c_str());
		return;
	}
}

void OpekeView::setupScene()
{
	mCamera->setPosition(Ogre::Vector3(0.0f,0.0f,500.0f));
	mCamera->lookAt(Ogre::Vector3(0.0f,0.0f,0.0f));
	mCamera->setNearClipDistance(5.0f);
	mCamera->setFarClipDistance(5000.0f);
	mViewport->setBackgroundColour(Ogre::ColourValue(0.0f,0.0f,0.0f));
	mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth())/Ogre::Real(mViewport->getActualHeight()));
	mCamera->setAutoAspectRatio(true);
	mCamera->setFixedYawAxis(false);
		
	mLight->setPosition(0,100,500);
	mLight->setDiffuseColour(1.0, 1.0, 1.0);
	mLight->setSpecularColour(0.0, 0.0, 0.0);
	
	nodeCount = 1;
	mMaterial = ((Ogre::MaterialPtr)Ogre::MaterialManager::getSingleton().create("Material" + Ogre::StringConverter::toString(nodeCount), "General")).getPointer();
	mMaterial->createTechnique();
	mMaterial->setAmbient(0.5, 0.0, 0.0);
	mMaterial->setDiffuse(1.0, 0.0, 0.0, 0.5);
	mMaterial->setSpecular(1.0, 1.0, 1.0, 0.5);
		
	activeBrick = newBrick();
	activeBrick->setSize(mSize);
}

void OpekeView::update()
{
	if(mWindow) mWindow->update();
	//if(mRoot) mRoot->renderOneFrame();
}

void OpekeView::resizeEvent ( QResizeEvent* e)
{
	if(!mWindow)
	{
		setupOgre();
		setupScene();
	}
	mWindow->resize(width(), height());
	mWindow->windowMovedOrResized();
	mViewport->_updateDimensions(); //RenderWindow should do this, but it doesn't. Perhaps it's only a bug in Ogre 1.4.9

	update();
}

void OpekeView::readConfig()
{
}

void OpekeView::saveSettings()
{
	Settings::self()->writeConfig();
}

void OpekeView::paintEvent( QPaintEvent * )
{
	if(!mWindow)
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
	
	switch (event->buttons())	
	{
		case (Qt::RightButton):
			xpos = event->x();
			ypos = event->y();
			break;
		case (Qt::LeftButton):
			if (!mode)
			{
				if (activeBrick) activeBrick->setSelected(false);
				activeBrick = setSelect ( event->x(), event->y() );
				moving = transform( event->x(), event->y() );
				if (selected >= 0)
				{
					if (activeBrick)
					{
						delete activeBrick;
						activeBrick = 0;
					}					
					activeBrick = Bricks[selected];
				}
			}
			break;
		case (Qt::XButton1):
			zoom +=1.0;
			break;
		case (Qt::XButton2):
			zoom -=1.0;
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
	if (!hasFocus()) setFocus();
	if (event->buttons() == Qt::RightButton)
	{
		mCamera->moveRelative(Ogre::Vector3( (xpos - event->x())/2, (event->y() - ypos)/2, 0));
		xpos = event->x();
		ypos = event->y();
	}
	else if ( mode == 1)
	{
		Ogre::Vector3 mouse = transform ( event->x(), event->y() );
		if (!activeBrick) activeBrick = newBrick();
		activeBrick->setPosition(mouse);
	}
	else if (activeBrick && event->buttons() == Qt::LeftButton)
	{
		mouseDown = 1;
		Ogre::Vector3 current = transform(event->x(), event->y());
		activeBrick->move(current.x - moving.x, current.y - moving.y, current.z - moving.z);
		moving = current;
	}
	update();
}

void OpekeView::mouseReleaseEvent(QMouseEvent *event)
{
	if ( event->button() == Qt::LeftButton )
	{
		if ( mode == 1 )	//if we're in build mode, add the active Brick to the list
		{
			
			
			if (activeBrick)
			{
				undoAction u;
				u.type = 0;
				u.uNum = Bricks.count();
				
				Bricks.append(activeBrick);
				activeBrick = newBrick();
				
				Ogre::Vector3 mouse = transform ( event->x(), event->y() );
				activeBrick->setPosition(Ogre::Vector3(mouse));
				
				if (uList.isEmpty()) emit undoEmpty(false);
				uList.append(u);
				rList.clear();
				emit redoEmpty(true);
				emit modified();
			}	
		}
		else if (currentBrick && selected >= 0 && mouseDown)
		{
			undoAction u;
			u.type = 2;
			u.move.x = Bricks[selected]->position().x - uCenter.x;
			u.move.y = Bricks[selected]->position().y - uCenter.y;
			u.move.z = Bricks[selected]->position().z - uCenter.z;
			u.uNum = selected;
			if (uList.isEmpty()) emit undoEmpty(false);
			uList.append(u);
			rList.clear();
			emit redoEmpty(true);
			emit modified();
		}
	}
	
	mouseDown = false;
	update();

}	

void OpekeView::keyPressEvent ( QKeyEvent* event )
{
	float mod = 10.0;
	if (event->isAutoRepeat()) mod *= 2;
	
	if (event->modifiers() == Qt::ControlModifier)
	{
		switch ( event->key() )
		{
			case ( Qt::Key_Up ) :
				mCamera->pitch(Ogre::Radian(-mod/400));
				xrot += mod;
				break;
	
			case ( Qt::Key_Down ) :
				mCamera->pitch(Ogre::Radian(mod/400));
				break;			
	
			case ( Qt::Key_Left ) :
				mCamera->yaw(Ogre::Radian(-mod/400));

				break;		
	
			case ( Qt::Key_Right ) :
				mCamera->yaw(Ogre::Radian(mod/400));
				break;
			
			default:
				event->ignore();
				break;
		}
	}
	else if (event->modifiers() == Qt::ShiftModifier)
	{
		switch (event->key())
		{
			case ( Qt::Key_Up ) :
				mCamera->moveRelative( Ogre::Vector3(0, 0, mod*2));
				break;
	
			case ( Qt::Key_Down ) :
				mCamera->moveRelative( Ogre::Vector3(0, 0, -mod*2));
				break;			
	
			case ( Qt::Key_Left ) :
				mCamera->roll(Ogre::Radian(-mod/400));
				break;		
	
			case ( Qt::Key_Right ) :
				mCamera->roll(Ogre::Radian(+mod/400));
				break;
			default:
				event->ignore();
				break;
		}
	}
	else
	{
		if (mode || selected < 0)
		{
			switch ( event->key() )
			{
				case ( Qt::Key_Up ) :
					mCamera->moveRelative( Ogre::Vector3(0, -mod, 0));
					break;
	
				case ( Qt::Key_Down ) :
					mCamera->moveRelative( Ogre::Vector3(0, mod, 0));
					break;			
	
				case ( Qt::Key_Left ) :
					mCamera->moveRelative( Ogre::Vector3(mod, 0, 0));
					break;		
	
				case ( Qt::Key_Right ) :
					mCamera->moveRelative( Ogre::Vector3(-mod, 0, 0));
					break;
	
				case ( Qt::Key_2 ) :
					zoom += mod/2;
					break;

				case ( Qt::Key_8 ) :
					zoom -= mod/2;
					break;
	
				case ( Qt::Key_4 ) :
					zrot += mod;
					break;
	
				case ( Qt::Key_6 ) :
					zrot -= mod;
					break;
				default: 
					event->ignore();
					break;
			}
		}
		else if (currentBrick)
		{
			switch (event->key())
			{
				case (Qt::Key_Up):
					currentNode->translate(0, 1.0, 0);
					uMove.y = 1.0;
					break;
				case (Qt::Key_Down):
					currentNode->translate(0, -1.0, 0);
					uMove.y = -1.0;
					break;
				case (Qt::Key_Left):
					currentNode->translate(-1.0, 0, 0);
					uMove.x = -1.0;
					break;
				case (Qt::Key_Right):
					currentNode->translate(1.0, 0, 0);
					uMove.x = 1.0;
					break;
				case (Qt::Key_8):
					currentNode->translate(0, 0, 1.0);
					uMove.z = 1.0;
					emit planeChanged(planeZ + 1);
					break;
				case (Qt::Key_2):
					currentNode->translate(0, 0, -1.0);
					uMove.z = -1.0;
					emit planeChanged(planeZ - 1);
					break;
				default: 
					event->ignore();
					break;
			}
			uCenter = Bricks[selected]->position();
			Bricks[selected] = currentBrick;
		}	
	}
	update();
}

void OpekeView::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();
	if (!mode && selected >= 0 && (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right ||
			key == Qt::Key_8 || key == Qt::Key_2))
	{
		undoAction u;
		u.type = 2;
		u.uNum = selected;
		u.move = uMove;
		if (uList.isEmpty()) emit undoEmpty(false);
		uList.append(u);
		rList.clear();
		emit redoEmpty(true);
		emit modified();
	}
	update();
}

void OpekeView::wheelEvent(QWheelEvent *event)
{
	if (!mode && selected >= 0)
	{
		currentNode->translate(0,0, event->delta()/30);
		emit planeChanged (planeZ + event->delta()/30);
		emit modified();
		emit redoEmpty(true);
	}
	else
	{
		mCamera->moveRelative( Ogre::Vector3(0, 0, event->delta()/10));
	}
	update();
}

Brick* OpekeView::newBrick()
{
	nodeCount++;
	activeNode = mSceneManager->getRootSceneNode()->createChildSceneNode("Node" + Ogre::StringConverter::toString(nodeCount));
	
	switch (mBrickType)
	{
		case Brick::Block:
			activeEntity = mSceneManager->createEntity("Brick" + Ogre::StringConverter::toString(nodeCount) , "block.mesh" );
			break;
		case Brick::Sphere:
			activeEntity = mSceneManager->createEntity("Brick" + Ogre::StringConverter::toString(nodeCount) , "sphere.mesh" );
			break;
		case Brick::Roof:
			activeEntity = mSceneManager->createEntity("Brick" + Ogre::StringConverter::toString(nodeCount) , "roof.mesh" );
			break;
		case Brick::Cylinder:
			activeEntity = mSceneManager->createEntity("Brick" + Ogre::StringConverter::toString(nodeCount) , "cylinder.mesh" );
			break;
		case Brick::InvertedCylinder:
			activeEntity = mSceneManager->createEntity("Brick" + Ogre::StringConverter::toString(nodeCount) , "invcyl.mesh" );
			break;
		default:
			activeEntity = mSceneManager->createEntity("Brick" + Ogre::StringConverter::toString(nodeCount) , "block.mesh" );
			break;
	}
			
	mMaterial = ((Ogre::MaterialPtr)Ogre::MaterialManager::getSingleton().create("Material" + Ogre::StringConverter::toString(nodeCount), "General")).getPointer(); 
	activeEntity->setMaterialName("Material" + Ogre::StringConverter::toString(nodeCount));
	activeNode->attachObject(activeEntity);
	Brick* mBrick = new Brick(activeNode, activeEntity);
	mBrick->setColor(mColor);
	mBrick->setSize(mSize);
	return mBrick;
}


Ogre::Vector3 OpekeView::transform ( int x, int y )
{
	Ogre::Real xf = (Ogre::Real)x/Ogre::Real(mViewport->getActualWidth());
	Ogre::Real yf = (Ogre::Real)y/Ogre::Real(mViewport->getActualHeight());
	
	Ogre::Ray mRay = mCamera->getCameraToViewportRay(xf, yf);
	Ogre::Vector3 mPoint;

	mPlane = Ogre::Plane(Ogre::Vector3(0.0,0.0,1.0),Ogre::Real(planeZ));
	std::pair<bool, Ogre::Real> v = mRay.intersects(mPlane);
	if (v.first)
	{
		mPoint = mRay.getPoint(v.second);
	}
	
	return mPoint;
}

Brick* OpekeView::setSelect ( int x, int y )
{
	Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(Ogre::Real(x)/Ogre::Real(mViewport->getActualWidth()), Ogre::Real(y)/Ogre::Real(mViewport->getActualHeight()));
	mRaySceneQuery = mSceneManager->createRayQuery(mouseRay);
	mRaySceneQuery->setRay(mouseRay);
	mRaySceneQuery->setSortByDistance(true);
	Ogre::RaySceneQueryResult result = mRaySceneQuery->execute();

	Ogre::MovableObject *closestObject = 0;
	Ogre::Real closestDistance = 100000;

	Ogre::RaySceneQueryResult::iterator rayIterator;

	for(rayIterator = result.begin(); rayIterator != result.end(); rayIterator++ ) 
	{
		if ((*rayIterator).movable && closestDistance > (*rayIterator).distance)
		{
			closestObject = ( *rayIterator ).movable;
			closestDistance = ( *rayIterator ).distance;
		}
	}

	mRaySceneQuery->clearResults();
//	mSceneManager->destroyQuery(mRaySceneQuery);
	Brick* sBrick;
	
	if ( closestObject )
	{
		emit delEnable(true);
		sBrick = new Brick(closestObject->getParentSceneNode(), (Ogre::Entity*)closestObject);
		sBrick->setSelected(true);
		return sBrick;
	}
	else
	{
		emit delEnable(false);
		return 0;
	}
}

void OpekeView::newScene()
{
	setBuildMode();
	Bricks.clear();
	xrot = -45.0;
	yrot = 0;
	zrot = 0;
	zoom = 5;
	uList.clear();
	rList.clear();
	update();
	emit undoEmpty(true);
	emit redoEmpty(true);
	emit modified();
}

void OpekeView::setBuildMode()
{
	mode = 1;
	selected = -1;
	update();
	emit delEnable(false);
}

void OpekeView::setSelectMode()
{
	delete activeBrick;
	activeBrick = 0;
	mode = 0;
	selected = -1;
	update();
	emit delEnable(false);
}

void OpekeView::settingsChanged()
{
	maxHeight = Settings::val_mahx();
	bgColor = Settings::col_background();
	
	glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(), 0.0);

	update();
	emit signalChangeStatusbar ( i18n("Settings changed") );
}

void OpekeView::setColor ( QColor signaled_color )
{
	if (activeBrick) activeBrick->setColor(signaled_color);
	if (!mode && selected >= 0)
	{
		undoAction u;
		u.type = 3;
		u.uNum = selected;
		u.color = Bricks[selected]->color();
		Bricks[selected]->setColor(signaled_color);
		if (uList.isEmpty()) emit undoEmpty(false);
		uList.append(u);
	}
	mColor = Ogre::ColourValue(signaled_color.redF(), signaled_color.greenF(), signaled_color.blueF(), signaled_color.alphaF());
	update();
}

void OpekeView::setSizeX ( int x )
{
	if (activeBrick) activeBrick->setSizeX(x);
	mSize.x = x;
	update();
}

void OpekeView::setSizeY ( int y )
{
	if (activeBrick) activeBrick->setSizeY(y);
	mSize.y = y;
	update();
}

void OpekeView::setSizeZ ( int z )
{
	if (activeBrick) activeBrick->setSizeZ(z);
	mSize.z = z;
	update();
}

void OpekeView::setPlaneZ ( int plane )
{
	if (activeBrick) activeBrick->move(Ogre::Vector3(0, 0, plane - planeZ));
	planeZ = plane;
	update();
}

void OpekeView::delBrick()
{
	if (mode == 0 && activeBrick)
	{
		delete activeBrick;
		activeBrick = 0;
		/*
		undoAction u;
		u.type = 1;
		u.uBrick = Bricks[selected];
		u.uNum = selected;
		
		currentNode->detachAllObjects();
		Bricks.removeAt(selected);
		
		if (uList.isEmpty()) emit undoEmpty(false);
		uList.append(u);
		rList.clear();
		emit redoEmpty(true);
		*/
		emit modified();
	}
	if (selected >= Bricks.count())
	{
		selected = -1;
		emit delEnable(false);
	}
	update();
}

void OpekeView::undo()
{
	if (uList.isEmpty())
	{
		emit undoEmpty(true);
		return;
	}
	if (rList.isEmpty()) emit redoEmpty(false);
	undoAction u = uList.last();
	QColor temp;
	switch (u.type)
	{
		case (0):
			u.type = 1;
			u.uBrick = Bricks[u.uNum];
			Bricks.removeAt(u.uNum);
			rList.append(u);
			break;
		case (1):
			if (u.uNum > Bricks.count())
			{
				u.uNum = Bricks.count();
			}
			Bricks.insert(u.uNum, u.uBrick);
			u.type = 0;
			rList.append(u);			
			break;
		case (2):
			u.move.x = -u.move.x;
			u.move.y = -u.move.y;
			u.move.z = -u.move.z;
			currentNode = mSceneManager->getSceneNode("Node" + u.uNum);
			currentNode->translate(u.move.x, u.move.y, u.move.z);
			rList.append(u);
			break;
		case (3):
			temp = Bricks[u.uNum]->color();
			Bricks[u.uNum]->setColor(u.color);
			u.color = temp;
			rList.append(u);
		default:
			break;
	}
	uList.removeLast();
	if (uList.isEmpty()) emit undoEmpty(true);
	emit modified();
	update();
}

void OpekeView::redo()
{
	if (rList.isEmpty()) 
	{
		emit redoEmpty(true);
		return;
	}
	if (uList.isEmpty()) emit undoEmpty(false);
	undoAction u = rList.last();
	QColor temp;
	switch (u.type)
	{
		case (0):
			u.type = 1;
			u.uBrick = Bricks[u.uNum];
			Bricks.removeAt(u.uNum);
			if (uList.isEmpty()) emit undoEmpty(false);
			uList.append(u);
			break;
		case (1):
			if (u.uNum > Bricks.count())
			{
				u.uNum = Bricks.count();
			}
			Bricks.insert(u.uNum, u.uBrick);
			u.type = 0;
			if (uList.isEmpty()) emit undoEmpty(false);
			uList.append(u);		
			break;
		case (2):
			u.move.x = -u.move.x;
			u.move.y = -u.move.y;
			u.move.z = -u.move.z;
			
			currentNode = mSceneManager->getSceneNode("Node" + u.uNum);
			currentNode->translate(u.move.x, u.move.y, u.move.z);
			if (uList.isEmpty()) emit undoEmpty(false);
			uList.append(u);
			break;
		case (3):
			temp = Bricks[u.uNum]->color();
			Bricks[u.uNum]->setColor(u.color);
			u.color = temp;
			uList.append(u);
		default:
			break;
	}
	rList.removeLast();
	if (rList.isEmpty()) emit redoEmpty(true);
	emit modified();
	update();
}

Ogre::SceneNode* OpekeView::workingNode()
{
	if (mode) return activeNode;
	else if (selected >= 0) return mSceneManager->getSceneNode("Node" + selected);
	else return 0;
}

void OpekeView::changeType(int changedType)
{
	mBrickType = changedType;
	if (activeBrick)
	{
		delete activeBrick;
		activeBrick = newBrick();
	}
}

void OpekeView::changeTypeBlock()
{
	changeType(Brick::Block);
}

void OpekeView::changeTypeRoof()
{
	changeType(Brick::Roof);
}

void OpekeView::changeTypeCylinder()
{
	changeType(Brick::Cylinder);
}

void OpekeView::changeTypeInvCyl()
{
	changeType(Brick::InvertedCylinder);
}

void OpekeView::changeTypeSphere()
{
	changeType(Brick::Sphere);
}

void OpekeView::changeOrientation(int changedOrientation)
{
	if(workingNode()) workingNode()->setOrientation(changedOrientation);
}

void OpekeView::rotateX()
{
	if(workingNode()) workingNode()->pitch(Ogre::Radian(3/2));
}

void OpekeView::rotateY()
{
	if(workingNode()) workingNode()->yaw(Ogre::Radian(3/2));
}

void OpekeView::rotateZ()
{
	if(workingNode()) workingNode()->roll(Ogre::Radian(3/2));
}

void OpekeView::sendOrientation()
{
	Ogre::SceneNode *w = workingNode();
	if (w) emit signalOrientation(w->getOrientation());
}

void OpekeView::openBricks(QList<Brick*> inBricks)
{
	// kDebug() << "Opened a file";
	int c = inBricks.count();
	Bricks.clear();
	for (int i = 0; i < c; i++)
	{
		Brick* b = newBrick();
		*b = *inBricks[i];
		// kDebug() << b->color;
		// kDebug() << b->position().x;
		// kDebug() << b->size[1]; 
		Bricks.append(b);
	}
}

#include "opekeview.moc"
