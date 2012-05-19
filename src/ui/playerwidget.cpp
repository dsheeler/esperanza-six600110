/**
 *  This file is a part of Esperanza, an XMMS2 Client.
 *
 *  Copyright (C) 2005-2007 XMMS2 Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */


#include <xmmsclient/xmmsclient++.h>

#include <QMainWindow>
#include <QMenu>
#include <QWidget>
#include <QGridLayout>
#include <QPixmap>
#include <QLabel>
#include <QPalette>
#include <QFont>
#include <QHBoxLayout>
#include <QMenu>
#include <QProgressBar>
#include <QProgressDialog>
#include <QErrorMessage>
#include <QTimer>
#include <QIcon>
#include <QPluginLoader>
#include <QAction>
#include <string>

#include "playerwidget.h"
#include "playerbutton.h"
#include "fancyplaylistview.h"
#include "progressframe.h"
#include "filedialog.h"
#include "volumebar.h"
#include "systemtray.h"
#include "minimode.h"
#include "shortcutmanager.h"
#include "esperanza_plugin.h"


xmmsc_connection_t *connection;
xmmsc_result_t *result;
xmmsv_t *return_value;
const char *err_buf;

PlayerWidget::PlayerWidget (QWidget *parent, XClient *client) : QMainWindow (parent)
#ifdef Q_WS_MAC
, macMenuBar(0)
#endif
{
	QSettings s;

	m_client = client;

	setWindowTitle ("Esperanza");
	setFocusPolicy (Qt::StrongFocus);

	QWidget *main_w = new QWidget (this);
	setCentralWidget (main_w);

	QWidget *dummy = new QWidget (main_w);

	QGridLayout *layout = new QGridLayout (main_w);

	m_fileMenu = QMainWindow::menuBar()->addMenu(tr("File"));
	m_fileMenu->addAction(tr("Quit"), this, SLOT(quit()));

	m_playlistMenu = QMainWindow::menuBar()->addMenu(tr("Playlist"));

	m_playlistMenu->addAction (tr ("Add local file"), this, SLOT (add_local_file ()));
	m_playlistMenu->addAction (tr ("Add local dir"), this, SLOT (add_local_dir ()));
	m_playlistMenu->addAction (tr ("Add URL"), this, SLOT (add_url ()));
	m_playlistMenu->addSeparator ();

	m_playlistMenu->addAction (tr ("Remove selected"), this, SLOT (remove_selected ()));
	m_playlistMenu->addAction (tr ("Clear"), this, SLOT (remove_all ()));
	m_playlistMenu->addSeparator ();
	m_playlistMenu->addAction (tr ("Shuffle"), this, SLOT(shuffle_pressed()));
	m_playlistMenu->addSeparator();
	m_playlistMenu->addAction (tr("Load"), this, SLOT(playlist_load()));
	m_playlistMenu->addAction (tr("Create"), this, SLOT(playlist_create()));
	m_playlistMenu->addAction (tr("Delete"), this, SLOT(playlist_delete()));




	m_albumArt = new QLabel(this);
	m_albumArt->setScaledContents(false);
	layout->addWidget(m_albumArt, 0,1,1, 1);

	m_playlist = new FancyPlaylistView (this, client);
	connect (m_playlist, SIGNAL (selectedID (uint32_t)), this, SLOT (handle_selected_id (uint32_t)));
	connect (m_playlist, SIGNAL (selectedID (uint32_t)), this, SLOT (update_album_art (uint32_t)));

	layout->addWidget (m_playlist, 0, 0, 1, 1);

	QHBoxLayout *pflay = new QHBoxLayout ();
	pflay->setMargin (0);
	
	PlayerButton *back = new PlayerButton (dummy, ":images/back.png");
	connect (back, SIGNAL (clicked (QMouseEvent *)),
	         this, SLOT (back_pressed ()));
	
	PlayerButton *fwd = new PlayerButton (dummy, ":images/forward.png");
	connect (fwd, SIGNAL (clicked (QMouseEvent *)),
	         this, SLOT (fwd_pressed ()));
	m_playstop = new PlayerButton (dummy, ":images/playstop.png");
	connect (m_playstop, SIGNAL (clicked (QMouseEvent *)),
	         this, SLOT (playstop_pressed ()));

	m_currentPlaylist = new QComboBox(this);
	m_currentPlaylist->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_playlistLabel = new QLabel(tr("Playlist: "),this);

	connect(m_currentPlaylist, SIGNAL(activated(QString)), this, SLOT(playlist_load(QString)));
	pflay->addWidget(m_playlistLabel);
	pflay->addWidget(m_currentPlaylist);
	pflay->addWidget (back);

	pflay->addWidget (m_playstop);
	if (!s.value ("ui/showstop").toBool ())
		m_playstop->hide ();

	m_playbutt = new PlayerButton (dummy, ":images/play.png");
	connect (m_playbutt, SIGNAL (clicked (QMouseEvent *)),
	         this, SLOT (play_pressed ()));

	pflay->addWidget (m_playbutt);
	pflay->addWidget (fwd);

	m_pf = new ProgressFrame (this, client);
	pflay->addWidget (m_pf);

	PlayerButton *min = new PlayerButton (dummy, ":images/minmax.png");
	connect (min, SIGNAL (clicked (QMouseEvent *)),
	         this, SLOT (min_pressed ()));
	pflay->addWidget (min);

	layout->addLayout (pflay, 1, 0, 1, 2);

	//	dummy = new QWidget (main_w);
	//	QHBoxLayout *hbox = new QHBoxLayout (dummy);
	//
	//	PlayerButton *plus = new PlayerButton (dummy, ":images/plus.png");
	//	connect (plus, SIGNAL (clicked (QMouseEvent *)),
	//			 this, SLOT (plus_pressed (QMouseEvent *)));
	//
	//	PlayerButton *minus = new PlayerButton (dummy, ":images/minus.png");
	//	connect (minus, SIGNAL (clicked (QMouseEvent *)),
	//			 this, SLOT (minus_pressed (QMouseEvent *)));
	//
	//
	//
	//	PlayerButton *sett = new PlayerButton (dummy, ":images/settings.png");
	//	connect (sett, SIGNAL (clicked (QMouseEvent *)),
	//			 this, SLOT (snett_pressed (QMouseEvent *)));
	//
	//	PlayerButton *info = new PlayerButton (dummy, ":images/info.png");
	//	connect (info, SIGNAL (clicked (QMouseEvent *)),
	//			 this, SLOT (info_pressed (QMouseEvent *)));

	//VolumeButton *volume = new VolumeButton (dummy, m_client);

	//        hbox->addStretch (1);
	//
	//        //hbox->addWidget (volume);
	//	hbox->addWidget (plus);
	//	hbox->addWidget (minus);
	//	hbox->addWidget (info);
	//	hbox->addWidget (sett);
	//
	//        layout->addWidget (dummy, 2, 0, 1, 1);



	layout->setRowStretch (0,1);
	layout->setRowStretch (1, 0);
	
	//layout->setColumnStretch (0, 1);
	//layout->setColumnStretch (2, 1);
	//layout->setColumnStretch (0, 0);
	layout->setMargin (5);
	
	m_status = Xmms::Playback::STOPPED;
	
	m_current_id = 0;
	connect (client, SIGNAL (gotConnection (XClient *)),
	         this, SLOT (got_connection (XClient *)));

	resize (s.value ("player/windowsize", QSize (550, 350)).toSize());
	if (s.contains ("player/position"))
		move (s.value ("player/position").toPoint ());
	m_sm = ShortcutManager::instance ();

	connect (m_client->settings (), SIGNAL (settingsChanged ()),
	         this, SLOT (changed_settings ()));

	// System Tray setup
	if (QSystemTrayIcon::isSystemTrayAvailable ()) {
		m_systray = new SystemTray (this, m_client);
	} else {
		m_systray = NULL;
	}

	m_mini = new MiniMode (this, m_client);

	/* run it once first time */
	// changed_settings ();


	m_sm->connect (this, "shortcuts/remove", "Del", SLOT (remove_selected ()));
	m_sm->connect (this, "shortcuts/remove2", "Backspace", SLOT (remove_selected ()));
	m_sm->connect (this, "shortcuts/shuffle", "S", SLOT (shuffle_pressed ()));
	m_sm->connect (this, "shortcuts/addfile", "A", SLOT (add_local_file ()));
	m_sm->connect (this, "shortcuts/adddir", "D", SLOT (add_local_dir ()));
	m_sm->connect (this, "shortcuts/removeall", "C", SLOT (remove_all ()));
	m_sm->connect (this, "shortcuts/play", "Space", SLOT (play_pressed ()));
	m_sm->connect (this, "shortcuts/forward", "B", SLOT (fwd_pressed ()));
	m_sm->connect (this, "shortcuts/back", "V", SLOT (back_pressed ()));
	m_sm->connect (this, "shortcuts/hide", "Esc", SLOT (check_hide ()));
	m_sm->connect (this, "shortcuts/jumppos", "Return", SLOT (jump_pos ()));
	m_sm->connect (this, "shortcuts/minmax", "Ctrl+M", SLOT (min_pressed ()));
#ifndef Q_WS_MACX
	m_sm->connect (this, "shortcuts/quit", "Ctrl+Q", SLOT (quit ()));
	m_sm->connect (this, "shortcuts/close", "Ctrl+W", SLOT (close ()));
#endif

	m_sm->connect (this, "globalshortcuts/play", "Ctrl+Shift+C", SLOT (play_pressed ()), true);
	m_sm->connect (this, "globalshortcuts/forward", "Ctrl+Shift+V", SLOT (fwd_pressed ()), true);
	m_sm->connect (this, "globalshortcuts/back", "Ctrl+Shift+X", SLOT (back_pressed ()), true);
	m_sm->connect (this, "globalshortcuts/stop", "Ctrl+Shift+B", SLOT (playstop_pressed ()), true);
	m_sm->connect (this, "globalshortcuts/show_hide", "Ctrl+Shift+M", SLOT (toggle_hide ()), true);
	m_sm->connect (this, "globalshortcuts/jump_begin", "Ctrl+Shift+-", SLOT (jump_back_ten ()), true);
	m_sm->connect (this, "globalshortcuts/jump_end", "Ctrl+Shift++", SLOT (jump_forward_ten ()), true);

	/* Process the plugins ... */
	process_dialog_plugin ();

#ifdef Q_WS_MAC
	m_infomenu.setTitle(tr("Info"));
	// m_playlistmenu.setTitle(tr("Playlist"));
	m_settingsmenu.setTitle(tr("Extras"));
	
	macMenuBar.addMenu(&m_infomenu);
	// macMenuBar.addMenu(&m_playlistmenu);
	macMenuBar.addMenu(&m_settingsmenu);
#endif

	m_infomenu.setTitle(tr("Tools"));
	QMainWindow::menuBar()->addMenu(&(this->m_infomenu));

	m_settingsmenu.setTitle(tr("Settings"));
	QMainWindow::menuBar()->addMenu(&(this->m_settingsmenu));

	connect(client->cache(), SIGNAL(entryChanged(uint32_t)), this, SLOT(update_album_art(uint32_t)));

	fprintf(stderr, "hi, at the end of constructor\n");
}

void PlayerWidget::update_album_art(uint32_t id) 
{
	if (this->m_current_id == id) {
		QPixmap p = m_client->cache()->get_pixmap(id);
		m_albumArt->setPixmap(p);
	}
}

bool PlayerWidget::populate_playlists(const Xmms::List<std::string> &playlists) 
{
	QStringList items;
	std::string str;
	this->m_currentPlaylist->clear();
	foreach(str, playlists) {
		if (str[0] != '_' && str[0] != ' ') {
			this->m_currentPlaylist->addItem(QString::fromStdString(str));
		}
	}
	m_client->playlist ()->currentActive() (Xmms::bind (&PlayerWidget::handle_playlist_name, this));
	return true;
}

void
PlayerWidget::process_dialog_plugin ()
{
	int i = 0;
	/* this uses the QPlugin, see esperanza_plugin.h for the interface */
	QObjectList l = QPluginLoader::staticInstances ();
	foreach (QObject *o, l) {
		EsperanzaMain::EsperanzaDialog *dialog = qobject_cast<EsperanzaMain::EsperanzaDialog *> (o);
		if (!dialog)
			continue;
			
		m_plugin_map[i] = dialog;
		dialog->init (this, m_client);
		
		QAction *action;
		switch (dialog->item ()) {
			case EsperanzaMain::DialogInfo:
				action = m_infomenu.addAction (dialog->label (), this, SLOT (open_dialog ()));
				break;
			case EsperanzaMain::DialogPlaylist:
				action = m_playlistmenu.addAction (dialog->label (), this, SLOT (open_dialog ()));
				break;
			case EsperanzaMain::DialogSettings:
				action = m_settingsmenu.addAction (dialog->label (), this, SLOT (open_dialog ()));
				break;
			default:
				action = NULL;
				break;
		}
		action->setData (QVariant (i));
		m_sm->connect (this, QString("shortcuts/dialog_%1").arg (dialog->label ()), dialog->shortcut (), SLOT (showit ()), false, dialog);
		i++;
	}
}

void
PlayerWidget::open_dialog ()
{
	QAction *a = qobject_cast<QAction *> (sender ());
	EsperanzaMain::EsperanzaDialog *dialog = m_plugin_map[a->data ().toInt ()];
	dialog->showit ();
}

void
PlayerWidget::toggle_mini () const
{
	QSettings s;

	if (m_mini->isVisible () &&
			(
				!s.value("ui/alwaysontop").toBool () && m_mini->isActiveWindow () ||
				s.value("ui/alwaysontop").toBool ()
			)
		) {
		m_mini->hide ();
	} else {
		m_mini->show ();
		m_mini->activateWindow ();
	}
}

void
PlayerWidget::closeEvent (QCloseEvent *ev)
{
	QSettings s;
	if( s.value("ui/hideOnClose").toBool () && s.value("core/systray").toBool () ) {
		hide ();
		ev->ignore ();
	}
}

void
PlayerWidget::min_pressed ()
{
	QSettings s;
	s.setValue ("ui/minimode", true);
	m_mini->show ();
	m_mini->activateWindow ();
	hide ();
}

void
PlayerWidget::changed_settings ()
{
	QSettings s;

	if (!s.value ("ui/showstop").toBool ())
		m_playstop->hide ();
	else
		m_playstop->show ();

	if (s.value ("ui/reverseplaytime").toBool ())
		m_pf->setReverse (true);

	if (m_systray) {
		if ( s.value ("core/systray").toBool ())
			m_systray->show();
		else
			m_systray->hide();
	}

	setWindowFlags ();
	update ();
}

void
PlayerWidget::moveEvent (QMoveEvent *ev)
{
	QSettings s;
	s.setValue ("player/position", pos ());
}

void
PlayerWidget::resizeEvent (QResizeEvent *ev)
{
	QSettings s;
	s.setValue ("player/windowsize", ev->size ());
}

void
PlayerWidget::check_hide ()
{
	if (isHidden ())
		return;

	QSettings s;

	if (m_systray && s.value ("core/systray").toBool ()) {
		hide ();
	}
}

void PlayerWidget::jump_back_ten() {
    m_client->playlist()->setNextRel(-10) ();
    m_client->playback()->tickle() ();
}

void PlayerWidget::jump_forward_ten() {
//    m_client->playlist()->setNext(m_playlist->rowCount()-1) ();
    m_client->playlist()->setNextRel(10);
    m_client->playback()->tickle() ();
}

void
PlayerWidget::jump_pos ()
{
        m_playlist->jump_pos (QModelIndex ());
}

void
PlayerWidget::play_pressed ()
{
	if (m_status == Xmms::Playback::PLAYING)
		m_client->playback ()->pause () ();
	else
		m_client->playback ()->start () ();
}

void
PlayerWidget::fwd_pressed ()
{
	m_client->playlist ()->setNextRel (1) ();
	m_client->playback ()->tickle () ();
}

void
PlayerWidget::back_pressed ()
{
	m_client->playlist ()->setNextRel (-1) ();
	m_client->playback ()->tickle () ();
}

void
PlayerWidget::plus_pressed (QMouseEvent *ev)
{
	QMenu m;

	m.addAction (tr ("Add local file"), this, SLOT (add_local_file ()));
	m.addAction (tr ("Add local dir"), this, SLOT (add_local_dir ()));
	m.addSeparator ();
	/*
	m.addAction (tr ("Add remote file"), this, SLOT (add_remote_file ()));
	*/
	m.addAction (tr ("Add URL"), this, SLOT (add_url ()));

	m.exec (ev->globalPos ());
}

void
PlayerWidget::info_pressed (QMouseEvent *ev)
{
	m_infomenu.exec (ev->globalPos ());
}

void
PlayerWidget::snett_pressed (QMouseEvent *ev)
{
	m_settingsmenu.exec (ev->globalPos ());
}

void
PlayerWidget::shuffle_pressed ()
{
	m_client->playlist ()->shuffle () ();
}

void
PlayerWidget::add_url ()
{
	bool ok;
	QString url = QInputDialog::getText (this, tr ("URL dialog"), tr ("Enter URL:"),
										 QLineEdit::Normal, "http://", &ok);
	if (ok && !url.isEmpty ()) {
		m_client->playlist ()->addUrl (XClient::qToStd (url)) ();
	}
}

void
PlayerWidget::add_remote_file ()
{
	FileDialog fd (this, "remote_file", true);
	QStringList files = fd.getFiles ();

	for (int i = 0; i < files.count(); i++) {
		m_client->playlist ()->addUrlEncoded (files.value (i).toStdString ()) ();
	}

}

void
PlayerWidget::add_local_dir ()
{
	FileDialog fd (this, "playlist_add_dir");
	QString dir = fd.getDirectory ();
	if (!dir.isNull ())
		m_client->playlist ()->addRecursive (XClient::qToStd ("file://" + dir)) ();
}

void
PlayerWidget::add_local_file ()
{
	QStringList files;
	FileDialog fd (this, "playlist_add_file");

	files = fd.getFiles ();

	for (int i = 0; i < files.count(); i++) {
		QString s = "file://" + files.at (i);
		m_client->playlist ()->addUrl (XClient::qToStd (s)) ();
	}

}

void
PlayerWidget::minus_pressed (QMouseEvent *ev)
{
	QMenu m;

	m.addAction (tr ("Remove selected files"), this, SLOT (remove_selected ()));
	m.addAction (tr ("Clear playlist"), this, SLOT (remove_all ()));

	m.exec (ev->globalPos ());
}

void
PlayerWidget::remove_selected ()
{
	QModelIndexList itm = m_playlist->getSelection ();
	QList<uint32_t> idlist;

	for (int i = 0; i < itm.size (); i++) {
		QModelIndex idx = itm.at (i);
		if (idx.column () != 0)
			continue;

		idlist.append (idx.row ());
	}

	qSort (idlist);
	for (int i = idlist.size () - 1; i > -1; i --) {
		m_client->playlist ()->removeEntry (idlist.at (i)) ();
	}
}

void
PlayerWidget::playstop_pressed ()
{
	m_client->playback ()->stop () ();
}

void
PlayerWidget::remove_all ()
{
	m_client->playlist ()->clear () ();
}

void
PlayerWidget::got_connection (XClient *client)
{
	m_client = client;

	client->playback ()->getStatus () (Xmms::bind (&PlayerWidget::handle_status, this));
	client->playback ()->broadcastStatus () (Xmms::bind (&PlayerWidget::handle_status, this));
	client->playlist ()->list() (Xmms::bind (&PlayerWidget::populate_playlists, this));
	client->playlist ()->broadcastLoaded() (Xmms::bind (&PlayerWidget::handle_playlist_load, this ));
	client->setDisconnectCallback (boost::bind (&PlayerWidget::handle_disconnect, this));
	client->playlist ()->broadcastLoaded() (Xmms::bind (&PlayerWidget::handle_playlist_name, this ));
	client->playlist ()->currentActive() (Xmms::bind (&PlayerWidget::handle_playlist_name, this));
	client->collection ()->broadcastCollectionChanged() (Xmms::bind (&PlayerWidget::on_collection_changed, this));
	client->playback ()->currentID () (Xmms::bind (&PlayerWidget::on_current_id, this));
	client->playback ()->broadcastCurrentID () (Xmms::bind (&PlayerWidget::on_current_id, this));


}

bool PlayerWidget::on_current_id(uint32_t id)
{
	this->m_current_id = id;
	update_album_art (id);
	return true;
}

bool PlayerWidget::on_collection_changed(Xmms::Dict &d) 
{
	m_client->playlist()->list() (Xmms::bind (&PlayerWidget::populate_playlists, this));
	return true;
}

bool PlayerWidget::handle_playlist_name(std::string name) {
	this->m_currentPlaylist->setCurrentIndex(m_currentPlaylist->findText(QString::fromStdString(name)));

	return true;
}

void
PlayerWidget::handle_disconnect ()
{
	QErrorMessage *err = new QErrorMessage (this);
	err->showMessage (tr ("Server died. Esperanza will close."));
	err->exec ();
	QApplication::quit ();
}

bool
PlayerWidget::handle_status (const Xmms::Playback::Status &st)
{
	m_status = st;

	if (st == Xmms::Playback::STOPPED ||
		st == Xmms::Playback::PAUSED) {
		m_playbutt->setPx (":images/play.png");
		m_mini->update_playbutton (":images/play.png");
	} else {
		m_playbutt->setPx (":images/pause.png");
		m_mini->update_playbutton (":images/pause.png");
	}

	return true;
}

void
PlayerWidget::setWindowFlags()
{
	QSettings s;
	Qt::WindowFlags f = 0;

	if(s.value ("ui/toolwindow").toBool () && s.value ("core/systray").toBool ())
		f |= Qt::Tool;
	else
		f |= Qt::Window;

	if(s.value("ui/alwaysontop").toBool ())
		f |= Qt::WindowStaysOnTopHint;

	QMainWindow::setWindowFlags (f);

	if(!s.value("ui/minimode").toBool ())
	{
		show ();
		activateWindow ();
	}
}

void
PlayerWidget::quit ()
{
	qApp->quit ();
}

void
PlayerWidget::toggle_hide ()
{
	QSettings s;
	if (s.value ("ui/minimode", false).toBool ()) {
		toggle_mini ();
	} else {
		if (isVisible () &&
			(
				!s.value("ui/alwaysontop").toBool () && isActiveWindow () ||
				s.value("ui/alwaysontop").toBool ()
			)
		) {
			hide ();
		} else {
			show ();
			activateWindow ();
		}
	}
}

void PlayerWidget::playlist_create() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Create New Playlist"), tr("New Playlist Name:"), QLineEdit::Normal,tr(""), &ok);

    m_client->playlist()->create(name.toAscii().data());
    m_client->playlist()->list() (Xmms::bind (&PlayerWidget::populate_playlists, this));
}

void PlayerWidget::playlist_delete() {
     m_client->playlist()->list() (Xmms::bind (&PlayerWidget::handle_playlist_delete, this));
}

bool PlayerWidget::handle_playlist_delete(const Xmms::List<std::string> &playlists) {
	QStringList items;
	std::string str;
	foreach(str, playlists) {
		if (str[0] != '_' && str[0] != ' ') {
			items.append(QString::fromStdString(str));
		}
	}
	
	bool ok;
	QString item = QInputDialog::getItem(this, tr("Delete Playlist"),
	                                     tr("Playlist:"), items, 0, false, &ok);

	if (ok) {
		this->m_client->playlist()->remove(item.toStdString());
	}

	m_client->playlist()->list() (Xmms::bind (&PlayerWidget::populate_playlists, this));

	return true;
}

bool PlayerWidget::handle_playlist_load(std::string name) {
	this->m_currentPlaylist->setCurrentIndex(m_currentPlaylist->findText(QString::fromStdString(name)));
	//this->m_currentPlaylist->setText(tr("Current Playlist: '%1'").arg(QString::fromStdString(name)));
	return true;
}

void PlayerWidget::playlist_load() {

    m_client->playlist()->list() (Xmms::bind (&PlayerWidget::playlist_load_dialog, this));
}

void PlayerWidget::playlist_load(QString name) {
    m_client->playlist()->load(name.toAscii().data());
}

bool PlayerWidget::playlist_load_dialog(const Xmms::List<std::string> &playlists) 
{
	QStringList items;
	std::string str;
	foreach(str, playlists) {
		if (str[0] != '_' && str[0] != ' ') {
			items.append(QString::fromStdString(str));
		}
	}
	
	bool ok;
	QString item = QInputDialog::getItem(this, tr("Load Playlist"),
	                                     tr("Playlist:"), items, 0, false, &ok);
	
	if (ok) {
		this->m_client->playlist()->load(item.toStdString());
	}
	
	return true;
	
}

