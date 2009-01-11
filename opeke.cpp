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
#include "opeke.h"
#include "opeketool.h"
#include "settings.h"
#include <iostream>

#include <QtGui/QDropEvent>
#include <QtGui/QMainWindow>

#include <KConfigDialog>
#include <kstatusbar.h>

#include <kactioncollection.h>
#include <kstandardaction.h>

#include <KFileDialog>
#include <KMessageBox>
#include <KIO/NetAccess>
#include <KSaveFile>

#include <KDE/KLocale>

#define VERSION "0.4"


Opeke::Opeke()
		: KXmlGuiWindow(),
		m_printer ( 0 ),
		fileName ( QString() ),
		Bricks ( QList<Brick>() )
{
	// accept dnd
	setAcceptDrops ( true );

	// tell the KXmlGuiWindow that this is indeed the main widget
	m_view = new OpekeView ( this );
	m_tool = new OpekeTool ( this );

	m_tool->setWindowTitle ( i18n ("Tool Options") );

	addDockWidget (Qt::LeftDockWidgetArea, m_tool);
	setCentralWidget (m_view);
	m_view->setFocus();

	// then, setup our actions
	setupActions();
	undoAct->setEnabled(false);
	redoAct->setEnabled(false);

	// add a status bar
	statusBar()->show();

	// a call to KXmlGuiWindow::setupGUI() populates the GUI
	// with actions, using KXMLGUI.
	// It also applies the saved mainwindow settings, if any, and ask the
	// mainwindow to automatically save settings if changed: window size,
	// toolbar position, icon size, etc.
	
	setupGUI();
	
	
	setWindowTitle("Opeke [*]");
}

Opeke::~Opeke()
{
	delete m_view;
	delete m_tool;
}

void Opeke::setupActions()
{
	KStandardAction::openNew ( this, SLOT ( fileNew() ), actionCollection() );
	KStandardAction::quit ( qApp, SLOT ( quit() ), actionCollection() );
	KStandardAction::save ( this, SLOT ( saveFile() ), actionCollection() );
	KStandardAction::open ( this, SLOT ( openFile() ), actionCollection() );
	KStandardAction::saveAs ( this, SLOT ( saveFileAs() ), actionCollection() );

	KStandardAction::preferences ( this, SLOT ( optionsPreferences() ), actionCollection() );	
	undoAct = KStandardAction::undo (m_view, SLOT(undo()), actionCollection() );
	redoAct = KStandardAction::redo (m_view, SLOT(redo()), actionCollection() );
	
	removeAct = KStandardAction::cut (m_view, SLOT(delBrick()), actionCollection());
	removeAct->setShortcut(QKeySequence(Qt::Key_Delete));
	removeAct->setText( i18n ("Delete"));
	removeAct->setEnabled(false);

	// The action to start building Bricks in OpenGL
	KAction *build = new KAction ( KIcon ( "build" ), i18n ( "Build" ), this );
	build->setShortcut(QKeySequence(Qt::Key_B));
	actionCollection()->addAction ( QLatin1String ( "build_action" ), build );
	connect ( build, SIGNAL ( triggered ( bool ) ), m_view, SLOT ( setBuildMode() ) );

	// Start select mode
	KAction *select = new KAction ( KIcon ( "select" ), i18n ( "Sele&ct" ), this );
	select->setShortcut(QKeySequence(Qt::Key_C));
	actionCollection()->addAction ( QLatin1String ( "select_action" ), select );
	connect ( select, SIGNAL ( triggered ( bool ) ), m_view, SLOT ( setSelectMode() ) );
	
	// Enable or disable grid
	KAction *grid = new KAction (KIcon ( "grid" ), i18n ("Toggle &Grid"), this);
	grid->setShortcut(QKeySequence(Qt::Key_G));
	actionCollection()->addAction(QLatin1String("grid_action"), grid);
	connect (grid, SIGNAL (triggered (bool)), m_view, SLOT (flipGridEnabled() ));
	
	// Actions to choose what kind of Brick to build
	KAction *block = new KAction (KIcon ( "block" ), i18n ("Build B&lock"), this);
	block->setShortcut(QKeySequence(Qt::Key_L));
	actionCollection()->addAction(QLatin1String("block_action"), block);
	connect (block, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeBlock() ));
	connect (block, SIGNAL (triggered (bool)), m_tool, SLOT (changeTypeBlock() ));
	connect (block, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *roof = new KAction (KIcon ( "roof" ), i18n ("Build &Roof"), this);
	roof->setShortcut(QKeySequence(Qt::Key_R));
	actionCollection()->addAction(QLatin1String("roof_action"), roof);
	connect (roof, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeRoof()));
	connect (roof, SIGNAL (triggered (bool)), m_tool, SLOT (changeTypeRoof()));
	connect (roof, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));


	KAction *cylinder = new KAction (KIcon ( "cylinder" ), i18n ("Build &Cylinder"), this);
	cylinder->setShortcut(QKeySequence(Qt::Key_C));
	actionCollection()->addAction(QLatin1String("cylinder_action"), cylinder);
	connect (cylinder, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeCylinder() ));
	connect (cylinder, SIGNAL (triggered (bool)), m_tool, SLOT (changeTypeCylinder() ));
	connect (cylinder, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *invCyl = new KAction (KIcon ("invCyl"), i18n ("Build &Inverted Cylinder"), this);
	invCyl->setShortcut(QKeySequence(Qt::Key_I));
	actionCollection()->addAction(QLatin1String("invCyl_action"), invCyl);
	connect (invCyl, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeInvCyl() ));
	connect (invCyl, SIGNAL (triggered (bool)), m_tool, SLOT (changeTypeInvCyl() ));
	connect (invCyl, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *sphere = new KAction (KIcon ("sphere"), i18n ("Build &Sphere"), this);
	sphere->setShortcut(QKeySequence(Qt::Key_S));
	actionCollection()->addAction(QLatin1String("sphere_action"), sphere);
	connect (sphere, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeSphere() ));
	connect (sphere, SIGNAL (triggered (bool)), m_tool, SLOT (changeTypeSphere() ));
	connect (sphere, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
		
	connect (m_tool, SIGNAL(setOrientation(int)), m_view, SLOT (changeOrientation(int)));
	connect (m_tool, SIGNAL(setCylOrientation(int)), m_view, SLOT(changeOrientation(int)));

	// Reload the field after loading a new file
	connect ( this, SIGNAL ( reload() ), m_view, SLOT ( update() ) );
	connect ( this, SIGNAL ( clear() ), m_view, SLOT ( newScene() ) );

	connect ( m_tool->ui_opeketool_base.color, SIGNAL ( highlighted ( QColor ) ), m_view, SLOT ( viewColor ( QColor ) ) );
	connect ( m_tool->ui_opeketool_base.color, SIGNAL ( activated ( QColor ) ), m_view, SLOT ( setColor ( QColor ) ) );

	connect ( m_tool->ui_opeketool_base.planeZ, SIGNAL ( valueChanged ( int ) ), m_view, SLOT ( setPlaneZ ( int ) ) );

	connect ( m_tool->ui_opeketool_base.sizeX, SIGNAL ( valueChanged ( int ) ), m_view, SLOT ( setSizeX ( int ) ) );
	connect ( m_tool->ui_opeketool_base.sizeY, SIGNAL ( valueChanged ( int ) ), m_view, SLOT ( setSizeY ( int ) ) );
	connect ( m_tool->ui_opeketool_base.sizeZ, SIGNAL ( valueChanged ( int ) ), m_view, SLOT ( setSizeZ ( int ) ) );
	connect ( m_view, SIGNAL(planeChanged(int)), m_tool->ui_opeketool_base.planeZ, SLOT(setValue(int)));

	connect ( m_view, SIGNAL(undoEmpty(bool)), this, SLOT(undoEnable(bool)));
	connect ( m_view, SIGNAL(redoEmpty(bool)), this, SLOT(redoEnable(bool)));
	connect ( m_view, SIGNAL(delEnable(bool)), this, SLOT(removeEnable(bool)));
	connect ( m_view, SIGNAL(modified()), this, SLOT(fileModified()));
	
	connect (m_tool, SIGNAL(rotX()), m_view, SLOT(rotateX()));
	connect (m_tool, SIGNAL(rotY()), m_view, SLOT(rotateY()));
	connect (m_tool, SIGNAL(rotZ()), m_view, SLOT(rotateZ()));

	connect (m_tool, SIGNAL(getCylOrientation()), m_view, SLOT(sendOrientation()));
	connect (m_view, SIGNAL(signalOrientation(int)), m_tool, SLOT(changeCylOrientation(int)));
	
	connect (this, SIGNAL(BricksLoaded(QList<Brick*>)), m_view, SLOT(openBricks(QList<Brick*>)));
}

void Opeke::fileNew()
{
	// this slot is called whenever the File->New menu is selected,
	// the New shortcut is pressed (CTRL+N) or the New toolbar
	// button is clicked
	fileName.clear();
	setWindowTitle(fileName + " - Opeke [*]");
	setWindowModified(false);
	emit clear();
}

void Opeke::saveFileAs ( const QString &outputFileName )
{
	KSaveFile *file  = new KSaveFile(outputFileName);
	file->open();
	QDataStream output ( file );
	QString program = "Opeke", fileVersion = VERSION;
	output << program << fileVersion;
	m_view->saveBricks(file);
	
	file->finalize();
	file->close();
	fileName = outputFileName;
	setWindowTitle(fileName + " Opeke [*]");
	setWindowModified(false);
}

void Opeke::saveFileAs()
{
	saveFileAs ( KFileDialog::getSaveFileName((KUrl)QString::null, "*.opeke|" + i18n("Opeke files"), (QWidget*)0, QString::null) );
}

void Opeke::saveFile()
{
	if ( !fileName.isEmpty() )
	{
		saveFileAs ( fileName );
	}
	else
	{
		saveFileAs();
	}
}

void Opeke::openFile()
{
	QString fileNameFromDialog = KFileDialog::getOpenFileName((KUrl)QString::null, "*.opeke|" + i18n("Opeke files"), (QWidget*)0, QString::null);
	openFile(fileNameFromDialog);
}

void Opeke::openFile(const QString &inputFileName)
{
	/**
	 * TODO: Rewrite this function to implement Entities and Nodes
	 * Rewrite only the actual loading of Bricks, the stucture is OK
	 */
	
	QString tmpFile;
	if ( KIO::NetAccess::download (inputFileName, tmpFile, this ) )
	{
		fileName = inputFileName;
		QFile* file  = new QFile (tmpFile);
		file->open ( QIODevice::ReadOnly );
		m_view->openBricks(file);
		file->close();
		setWindowTitle(fileName + " - Opeke [*]");
		setWindowModified(false);
	}
	else
	{
		KMessageBox::error ( this, KIO::NetAccess::lastErrorString() );
	}
}



void Opeke::optionsPreferences()
{
	// The preference dialog is derived from prefs_base.ui
	//
	// compare the names of the widgets in the .ui file
	// to the names of the variables in the .kcfg file
	if ( KConfigDialog::showDialog ( "settings" ) )
	{
		return;
	}
	KConfigDialog *dialog = new KConfigDialog ( this, "settings", Settings::self() );
	QWidget *generalSettingsDlg = new QWidget;
//	QWidget *mouseSettingsDlg = new QWidget;
	ui_prefs_base.setupUi (generalSettingsDlg);
	
	/**
	 * Here we setup the Combobox options for mouse configuration.
	 * It's currently not implemented and I'm not sure whether it ever will be.
	 * If you want more configureation options, mail me, or do it yourself.
	 */
	
//	ui_prefs_mouse.setupUi (mouseSettingsDlg);

	dialog->addPage(generalSettingsDlg, i18n("Display"));
//	dialog->addPage(mouseSettingsDlg, i18n("Mouse"));
	connect ( dialog, SIGNAL(settingsChanged(const QString&)), m_view, SLOT ( settingsChanged() ) );
	dialog->setAttribute ( Qt::WA_DeleteOnClose );
	dialog->show();
}

void Opeke::undoEnable(bool empty)
{
	undoAct->setEnabled(!empty);
}

void Opeke::redoEnable(bool empty)
{
	redoAct->setEnabled(!empty);
}

void Opeke::removeEnable(bool enable)
{
	removeAct->setEnabled(enable);
}

void Opeke::fileModified()
{
	if (fileName.isEmpty()) setWindowTitle(fileName + " - Opeke [*]");
	else setWindowTitle("Opeke [*]");
	setWindowModified(true);
}





#include "opeke.moc"
