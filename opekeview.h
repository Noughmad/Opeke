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

#ifndef OPEKE_VIEW_H
#define OPEKE_VIEW_H

#include <QtGui/QWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>

#include <Ogre.h>

#include "brick.h"
#include "undoaction.h"
#include "ui_opekeview_base.h"

class QPainter;
class KUrl;

/**
 * This is the main view class for Opeke.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author Miha Čančula <miha@noughmad.org>
 * @version 0.4
 */

class OpekeView : public QWidget, public Ui::opekeview_base
{
		Q_OBJECT
	public:
		/**
		 * Default constructor
		 */
		OpekeView ( QWidget *parent );

		/**
		 * Destructor
		 */
		virtual ~OpekeView();

		
		void update();
		virtual void setupOgre();
		virtual void setupScene();
		
		Brick *newBrick();

	void setBricks ( const QList< Brick * >& value );
	

	QList< Brick * > getBricks() const;
	
		
	protected:

		void paintEvent( QPaintEvent * );
		void resizeEvent( QResizeEvent* );
		
	//	virtual void setupResources();
		
		void readConfig();
		void saveSettings();

		void mousePressEvent ( QMouseEvent *event );
		void mouseMoveEvent ( QMouseEvent *event );
		void mouseReleaseEvent(QMouseEvent *event);
		void keyPressEvent ( QKeyEvent *event );
		void keyReleaseEvent ( QKeyEvent *event );
		void wheelEvent (QWheelEvent *event);
		
		void showGrid(Brick* toShow);
		void showNoGrid(Brick* toShow);
		
		Ogre::Root* mRoot;
		Ogre::ResourceGroupManager* mResourceGroupManager;
		Ogre::SceneManager* mSceneManager;
		Ogre::MaterialManager* mMaterialManager;
		Ogre::RenderWindow* mWindow;
		Ogre::Camera* mCamera;
		Ogre::Viewport* mViewport;
		Ogre::RaySceneQuery* mRaySceneQuery;
		Ogre::Light* mLight;
		
	//	Ogre::SceneNode* currentNode;
	//	Ogre::SceneNode* activeNode;
	//	Ogre::Entity* activeEntity;
		Ogre::Entity* mEntity;
		Ogre::Plane mPlane;
		Ogre::Material* mMaterial;
		
	private:
		
		QList<UndoAction*> undoList;
		QList<UndoAction*> redoList;
		Ogre::Vector3 uCenter, uMove, mSize;
		Ogre::Vector3 preMovePos;
		unsigned int nodeCount, materialCount;
		Ogre::ColourValue mColor, mBgColor;
		//QString selectedBrick;
		
		QList<Brick*> Bricks;
		QList<Brick*> selectedBricks;

		//int sizeX, sizeY, sizeZ;
		int planeZ, maxHeight;
		int mBrickType, type;

		Brick *activeBrick, *currentBrick;
		int selected;

		Ogre::Vector3 transform ( int x, int y );
		Ogre::Vector3 moving, current;
		bool mouseDown;
		QString fileName;
		QColor bgColor, gridColor;
		Ui::opekeview_base ui_opekeview_base;

		bool mode;
		/**
		 * 1 = build mode
		 *	0 = select mode
		 */
		int orientation;

		bool checkCollision(Brick *toCheck);
		Brick* setSelect ( int x, int y );
		Brick* newBrick(int newType);
		void changeType(int changedType);

	signals:
		/**
		 * Use this signal to change the content of the statusbar
		 */
		void signalChangeStatusbar ( const QString& text );
		void undoEmpty(bool empty);
		void redoEmpty(bool empty);
		void delEnable(bool enable);
		void planeChanged(int z);
		void modified();
		void getOrientationFromTool();
		void signalOrientation(Ogre::Quaternion signaledOrientation);
		
	private slots:
		void newScene();
		void settingsChanged();
		void setBuildMode();
		void setSelectMode();

		void setColor ( QColor );
		void setSizeX ( int );
		void setSizeY ( int );
		void setSizeZ ( int );

		void setPlaneZ ( int );
		void delBrick();
		
		void undo();
		void redo();
		
		void changeTypeBlock();
		void changeTypeRoof();
		void changeTypeCylinder();
		void changeTypeInvCyl();
		void changeTypeSphere();
		void changeOrientation(int changedOrientation);
		void rotateX();
		void rotateY();
		void rotateZ();
		void sendOrientation();

		void openBricks(QList<Brick*> inBricks);
};

#endif // OpekeVIEW_H
