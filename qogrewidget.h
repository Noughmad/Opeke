//
// C++ Interface: qogrewidget
//
// Description: 
//
//
// Author: Miha Čančula <miha@noughmad.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef QOGREWIDGET_H
#define QOGREWIDGET_H

#include <QWidget>
#include <Ogre.h>

/**
	@author Miha Čančula <miha@noughmad.org>
*/

class QOgreWidget : public QWidget
{
	Q_OBJECT
	public:
		QOgreWidget();
		~QOgreWidget();
		
		
		
	private:
		Ogre::Root* mRoot;
		Ogre::ResourceGroupManager* mResourceGroupManager;
		Ogre::SceneManager* mSceneManager;
		Ogre::RenderWindow* mWindow;
		Ogre::Camera* mCamera;
		Ogre::Viewport* mViewport;
		Ogre::Light* mLight;
		
		Ogre::String mPluginsFile, mOgreFile, mLogFile, mResourcesFile;
		
		virtual void setFiles();
		virtual void loadFiles();
		virtual void initializeOgre();
		
		virtual void paintEvent (QPaintEvent*);
		virtual void resizeEvent (QResizeEvent*);
		
	private slots:
		virtual void updateOgre();
    

};

QOgreWidget::QOgreWidget()
{
	mRoot = 0;
	mResourceGroupManager = 0;
	mSceneManager = 0;
	mWindow = 0;
	mCamera = 0;
	mViewport = 0;
	mLight = 0;
}

QOgreWidget::~QOgreWidget()
{
	
}

void QOgreWidget::setFiles()
{
	mPluginsFile = "plugins.cfg";
	mOgreFile = "ogre.cfg";
	mLogFile = "ogre.cfg";
	mResourcesFile = "resources.cfg";	
}

void QOgreWidget::loadFiles()
{
	setFiles();
	try
	{
		mRoot = new Ogre::Root(mPluginsFile, mOgreFile, mLogFile);
		if (!mRoot->restoreConfig())
		{
			if (!mRoot->showConfigDialog())
			{
				return;
			}
		}
		
		Ogre::ConfigFile cf;
		cf.load(mResourcesFile);    

		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
		while (seci.hasMoreElements())
		{
			Ogre::String secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
			for (Ogre::ConfigFile::SettingsMultiMap::iterator i = settings->begin();
						  i != settings->end(); ++i)
			{
				Ogre::String typeName = i->first;
				Ogre::String archName = i->second;
				mRoot->addResourceLocation(
						archName, typeName, secName);
			}
		}
	}
	catch (Ogre::Exception& e)
	{
		KMessageBox::error ( this, e.getDescription().c_str());
		return;
	}
}

void QOgreWidget::initializeOgre()
{
	try
	{
		loadFiles();
		
		QX11Info windowInfo = x11Info ();
		Ogre::String dspl = Ogre::StringConverter::toString((unsigned long)windowInfo.display());
		Ogre::String scrn = Ogre::StringConverter::toString((unsigned int)windowInfo.appScreen());
		Ogre::String winHandle = Ogre::StringConverter::toString((unsigned long)winId());
		Ogre::NameValuePairList miscParams;
		miscParams["parentWindowHandle"] = (dspl + ":" + scrn + ":" + winHandle);
		mRoot->initialise(false);
		mWindow = mRoot->createRenderWindow("Window",width(),height(),false,&miscParams);
		
		mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC,"sceneManager");
		mCamera = mSceneManager->createCamera("MainCamera");
		mViewport = mWindow->addViewport(mCamera);
		mLight = mSceneManager->createLight("MainLight");
	}
	catch (Ogre::Exception& e)
	{
		KMessageBox::error ( this, e.getDescription().c_str());
		return;
	}
}

void QOgreWidget::paintEvent(QPaintEvent*)
{
	updateOgre();
}

void QOgreWidget::resizeEvent(QResizeEvent*)
{
	if (mWindow) 
	{
		mWindow->resize(width(), height());
		mCamera->setAspectRatio(Ogre::Real(width()) / Ogre::Real(height()));
		mWindow->windowMovedOrResized();
	}
	
	updateOgre();
}

void QOgreWidget::updateOgre()
{
	if (!mWindow) initializeOgre();
	mWindow->update();
}

#endif
