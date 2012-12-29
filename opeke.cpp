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

#include <QtGui/QDropEvent>
#include <QtGui/QMainWindow>
#include <QtGui/QDockWidget>

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
	m_tool->setObjectName("Tool Options");
	m_dockWidget = new QDockWidget(this);
	m_dockWidget->setWidget(m_tool);
	m_dockWidget->setObjectName("Tool Dock");
	addDockWidget (Qt::LeftDockWidgetArea, m_dockWidget);
	m_tool->show();
	setCentralWidget (m_view);
	m_view->setFocus();

	setupActions();
	undoAct->setEnabled(false);
	redoAct->setEnabled(false);

	statusBar()->show();
	
	setupGUI();
	
	setWindowModified(false);
	setCaption(QString::null, false);
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

	// The action to start building Bricks in OGRE
	KAction *build = new KAction ( KIcon ( "build" ), i18n ( "Build" ), this );
	build->setShortcut(QKeySequence(Qt::Key_B));
	actionCollection()->addAction ( QLatin1String ( "build_action" ), build );
	connect ( build, SIGNAL ( triggered ( bool ) ), m_view, SLOT ( setBuildMode() ) );

	// Start select mode
	KAction *select = new KAction ( KIcon ( "select" ), i18n ( "S&elect" ), this );
	select->setShortcut(QKeySequence(Qt::Key_E));
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
	connect (block, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *roof = new KAction (KIcon ( "roof" ), i18n ("Build &Roof"), this);
	roof->setShortcut(QKeySequence(Qt::Key_R));
	actionCollection()->addAction(QLatin1String("roof_action"), roof);
	connect (roof, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeRoof()));
	connect (roof, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));


	KAction *cylinder = new KAction (KIcon ( "cylinder" ), i18n ("Build &Cylinder"), this);
	cylinder->setShortcut(QKeySequence(Qt::Key_C));
	actionCollection()->addAction(QLatin1String("cylinder_action"), cylinder);
	connect (cylinder, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeCylinder() ));
	connect (cylinder, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *invCyl = new KAction (KIcon ("invCyl"), i18n ("Build &Inverted Cylinder"), this);
	invCyl->setShortcut(QKeySequence(Qt::Key_I));
	actionCollection()->addAction(QLatin1String("invCyl_action"), invCyl);
	connect (invCyl, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeInvCyl() ));
	connect (invCyl, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *sphere = new KAction (KIcon ("sphere"), i18n ("Build &Sphere"), this);
	sphere->setShortcut(QKeySequence(Qt::Key_S));
	actionCollection()->addAction(QLatin1String("sphere_action"), sphere);
	connect (sphere, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeSphere() ));
	connect (sphere, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *cone = new KAction (KIcon ( "cone" ), i18n ("Build Co&ne"), this);
	cone->setShortcut(QKeySequence(Qt::Key_N));
	actionCollection()->addAction(QLatin1String("cone_action"), cone);
	connect (cone, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeCone() ));
	connect (cone, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *corner = new KAction (KIcon ( "corner" ), i18n ("Build Roof C&orner"), this);
	corner->setShortcut(QKeySequence(Qt::Key_O));
	actionCollection()->addAction(QLatin1String("corner_action"), corner);
	connect (corner, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeCorner() ));
	connect (corner, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *invrcor = new KAction (KIcon ( "invrcor" ), i18n ("Build In&verted Roof Corner"), this);
	invrcor->setShortcut(QKeySequence(Qt::Key_V));
	actionCollection()->addAction(QLatin1String("invrcor_action"), invrcor);
	connect (invrcor, SIGNAL (triggered (bool)), m_view, SLOT (changeTypeInvCorner() ));
	connect (invrcor, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *pyramid = new KAction (KIcon ( "pyramid" ), i18n ("Build &Pyramid"), this);
	pyramid->setShortcut(QKeySequence(Qt::Key_P));
	actionCollection()->addAction(QLatin1String("pyramid_action"), pyramid);
	connect (pyramid, SIGNAL (triggered (bool)), m_view, SLOT (changeTypePyramid() ));
	connect (pyramid, SIGNAL (triggered (bool)), m_view, SLOT (setBuildMode() ));
	
	KAction *snapshot = new KAction (KIcon ("ksnapshot"), i18n ("&Take Screenshot"), this);
	snapshot->setShortcut(QKeySequence("Ctrl+T"));
	actionCollection()->addAction(QLatin1String("snapshot_action"), snapshot);
	connect (snapshot, SIGNAL(triggered(bool)), this, SLOT (saveScreen()));

	// Reload the field after loading a new file
	connect ( this, SIGNAL ( reload() ), m_view, SLOT ( update() ) );
	connect ( this, SIGNAL ( clear() ), m_view, SLOT ( newScene() ) );

	connect ( m_tool, SIGNAL ( colorViewed ( QColor ) ), m_view, SLOT ( viewColor ( QColor ) ) );
	connect ( m_tool, SIGNAL ( colorChanged ( QColor ) ), m_view, SLOT ( setColor ( QColor ) ) );

	connect ( m_tool, SIGNAL ( planeChanged ( int ) ), m_view, SLOT ( setPlaneZ ( int ) ) );

	connect ( m_tool, SIGNAL ( sizeXChanged ( int ) ), m_view, SLOT ( setSizeX ( int ) ) );
	connect ( m_tool, SIGNAL ( sizeYChanged ( int ) ), m_view, SLOT ( setSizeY ( int ) ) );
	connect ( m_tool, SIGNAL ( sizeZChanged ( int ) ), m_view, SLOT ( setSizeZ ( int ) ) );
	connect ( m_view, SIGNAL(planeChanged(int)), m_tool, SLOT(changePlaneZ(int)));

	connect ( m_view, SIGNAL(undoEmpty(bool)), this, SLOT(undoEnable(bool)));
	connect ( m_view, SIGNAL(redoEmpty(bool)), this, SLOT(redoEnable(bool)));
	connect ( m_view, SIGNAL(delEnable(bool)), this, SLOT(removeEnable(bool)));
	connect ( m_view, SIGNAL(modified()), this, SLOT(fileModified()));
	
	connect (m_tool, SIGNAL(rotX()), m_view, SLOT(rotateX()));
	connect (m_tool, SIGNAL(rotY()), m_view, SLOT(rotateY()));
	connect (m_tool, SIGNAL(rotZ()), m_view, SLOT(rotateZ()));
}

void Opeke::fileNew()
{
	if (maybeSave())
	{	
		emit clear();
		fileName.clear();
		setWindowModified(false);
		setCaption(fileName, false);
	}
}

void Opeke::saveFileAs ( const QString &outputFileName )
{
	KSaveFile *file  = new KSaveFile(outputFileName);
	file->open();
	m_view->saveBricks(file);
	
	file->finalize();
	file->close();
	fileName = KUrl(outputFileName).fileName();
	setCaption(fileName, false);
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
	QString tmpFile;
	if ( KIO::NetAccess::download (inputFileName, tmpFile, this ) )
	{
		emit clear();
		fileName = inputFileName;
		QFile* file  = new QFile (tmpFile);
		file->open ( QIODevice::ReadOnly );
		m_view->openBricks(file);
		file->close();
		setWindowModified(false);
		setCaption(fileName, false);
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
	QWidget *buildSettingsDlg = new QWidget;
	ui_prefs_base.setupUi (generalSettingsDlg);
	ui_prefs_build.setupUi(buildSettingsDlg);
	

	dialog->addPage(generalSettingsDlg, i18n("Display"));
	dialog->addPage(buildSettingsDlg, i18n("Building"));
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
	setWindowModified(true);
	setCaption(fileName, true);
}

void Opeke::closeEvent(QCloseEvent * event)
{
	if (maybeSave()) 
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

bool Opeke::maybeSave()
{
	if (!windowModified) return true;
	switch (KMessageBox::questionYesNoCancel(this, i18n ("The document has been modified.\n" "Do you want to save your changes?"), i18n ("Unsaved changes")))
	{
		case KMessageBox::Yes:
			saveFile();
		case KMessageBox::No:
			return true;
		default:
			return false;
	}
}

void Opeke::setWindowModified(bool mode)
{
	windowModified = mode;
}

void Opeke::saveScreen()
{
	QPixmap screen = QPixmap::grabWindow(m_view->winId());
	QString shotName = KFileDialog::getSaveFileName((KUrl)QString::null, "image/png image/jpeg image/bmp", (QWidget*)0, QString::null);
	if (!shotName.isEmpty())
		screen.save(shotName, "PNG", -1);
}

#include "opeke.moc"
