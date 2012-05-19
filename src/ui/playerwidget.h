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


#ifndef __PLAYERWIDGET_H__
#define __PLAYERWIDGET_H__

#include <xmmsclient/xmmsclient++.h>

#include <QMainWindow>
#include <QMouseEvent>
#include <QAction>

#include "fancyplaylistview.h"
#include "progressframe.h"
#include "playerbutton.h"
#include "volumebar.h"
#include "systemtray.h"
#include "minimode.h"
#include "shortcutmanager.h"
#include "esperanza_plugin.h"

class PlayerWidget : public QMainWindow
{
	Q_OBJECT
	public:
		PlayerWidget (QWidget *parent, XClient *client);

		void resizeEvent (QResizeEvent *);
		void moveEvent (QMoveEvent *);
		void closeEvent (QCloseEvent *);
		
		void toggle_mini () const;

		bool mini_isactive () const
		{
			return m_mini->isActiveWindow ();
		};

		bool mini_isvisible () const
		{
			return m_mini->isVisible ();
		};

		void setWindowFlags ();
		uint32_t currentID () { return m_current_id; };

	public slots:
		void play_pressed ();
		void playstop_pressed ();
		void fwd_pressed ();
		void back_pressed ();
		void shuffle_pressed ();
		void snett_pressed (QMouseEvent *);
		void add_local_file ();
		void add_local_dir ();
		void jump_back_ten ();
		void jump_forward_ten ();
		void jump_pos ();
		void check_hide ();
		void toggle_hide ();
		void playlist_create ();
		void playlist_load ();
		void playlist_load(QString name);
		void playlist_delete();
		
	private slots:
		void plus_pressed (QMouseEvent *);
		void info_pressed (QMouseEvent *);
		void minus_pressed (QMouseEvent *);
		void got_connection (XClient *);
		void add_remote_file ();
		void add_url ();
		void min_pressed ();
		void update_album_art(uint32_t);
		void remove_selected ();
		void remove_all ();

		void changed_settings ();
		void open_dialog ();
		
		void handle_selected_id (uint32_t id) {
			m_current_id = id;
			emit selectedID (id);
		};

		void quit ();
		
	signals:
		void selectedID (uint32_t);

	private:
		XClient *m_client;
		Xmms::Playback::Status m_status;
		FancyPlaylistView *m_playlist;
		PlayerButton *m_playbutt;
		PlayerButton *m_playstop;

		ShortcutManager *m_sm;

		QLabel *m_albumArt;

		QMenu *m_fileMenu;
		QMenu *m_playlistMenu;
		//    QLabel *m_currentPlaylist;

		QLabel *m_playlistLabel;
		QComboBox *m_currentPlaylist;
		bool handle_status (const Xmms::Playback::Status &);
		void handle_disconnect ();
		void process_dialog_plugin ();
		bool playlist_load_dialog(const Xmms::List<std::string> &playlists);
		bool populate_playlists(const Xmms::List<std::string> &playlists);
		bool handle_playlist_load(const std::string name);
		bool handle_playlist_name(const std::string name);
		bool handle_playlist_delete(const Xmms::List<std::string> &playlists);
		bool on_collection_changed(Xmms::Dict &d );
		uint32_t m_current_id;


		ProgressFrame *m_pf;
		VolumeBar *m_volbar;
		SystemTray *m_systray;
		MiniMode *m_mini;
		
		#ifdef Q_WS_MAC
		QMenuBar macMenuBar;
		#endif
		QMenu m_infomenu;
		QMenu m_playlistmenu;
		QMenu m_settingsmenu;
		QMap<int32_t, EsperanzaMain::EsperanzaDialog *> m_plugin_map;
};

#endif

