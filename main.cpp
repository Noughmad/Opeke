/***************************************************************************
 *   Copyright (C) 2008 by Miha Čančula 												*
 *   miha@noughmad.org   																	*
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
#include <iostream>

#include "opeke.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KDE/KLocale>
#include <KUrl>
#include <KDebug>

static const char description[] =
    I18N_NOOP ( "A KDE 4 application for building virtual bricks" );

static const char version[] = "0.4";

int main ( int argc, char **argv )
{
	KAboutData about ( "opeke", 0, ki18n ( "Opeke" ), version, ki18n ( description ),
	                   KAboutData::License_GPL, ki18n ( "(C) 2008 Miha Čančula" ), KLocalizedString(), 0, "miha@noughmad.org" );
	about.addAuthor ( ki18n ( "Miha Čančula" ), KLocalizedString(), "miha@noughmad.org" );
	KCmdLineArgs::init ( argc, argv, &about );

	KCmdLineOptions options;
	options.add ( "+[URL]", ki18n ( "Document to open" ) );
	KCmdLineArgs::addCmdLineOptions ( options );
	KApplication app;

	Opeke *mainwidget = new Opeke; // Main screen turn on

	// see if we are starting with session management
	if ( app.isSessionRestored() )
	{
		RESTORE ( Opeke );
	}
	else
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		if ( args->count() == 0 )
		{
			mainwidget->show();
		}
		else
		{
			mainwidget->show();
			mainwidget->openFile(args->url(0).url());
		}
		args->clear();
	}

	return app.exec();
}
