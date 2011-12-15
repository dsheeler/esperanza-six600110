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


#ifndef __XCLIENT_H__
#define __XCLIENT_H__

#include <xmmsclient/xmmsclient++.h>

class XClient;

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QDir>
#include <QWidget>

#include "xclientcache.h"

class XSettings : public QObject
{
	Q_OBJECT
	public:
		XSettings (QObject *);
		void change_settings ();

	signals:
		void settingsChanged ();
};

class XClient : public QObject {
    Q_OBJECT
public:
    XClient (QObject *, const std::string &);

    void disconnect ();
    bool connect (const char *path = NULL, const bool &sync = false, QWidget* parent = NULL);
    static void propDictToQHash (const std::string &key,
                                 const Xmms::Dict::Variant &value,
                                 const std::string &source,
                                 QHash<QString, QVariant> &hash);
									 
    static void dictToQHash (const std::string &key,
                             const Xmms::Dict::Variant &value,
                             QHash<QString, QVariant> &hash);

    static QHash<QString, QVariant> convert_propdict (const Xmms::PropDict &);
    static QHash<QString, QVariant> convert_dict (const Xmms::Dict &);

    XClientCache *cache () const {
        return m_cache;
    };

    XSettings *settings () const {
        return m_settings;
    };

    const Xmms::Client *sync () const {
        return &m_sync;
    };

    static QString stdToQ (const std::string &);
    static std::string qToStd (const QString &);

    bool isConnected () const {
        return m_isconnected;
    };
        
    static QDir esperanza_dir ();
    void setDisconnectCallback (const Xmms::DisconnectCallback::value_type &lslot) { m_client->setDisconnectCallback (lslot); };
    const Xmms::Collection* collection () { if (m_client && m_client->isConnected ()) return &m_client->collection; else return NULL; }
    const Xmms::Playlist* playlist () { if (m_client && m_client->isConnected ()) return &m_client->playlist; else return NULL; }
    const Xmms::Playback* playback () { if (m_client && m_client->isConnected ()) return &m_client->playback; else return NULL; }
    const Xmms::Medialib* medialib () { if (m_client && m_client->isConnected ()) return &m_client->medialib; else return NULL; }
    const Xmms::Bindata* bindata () { if (m_client && m_client->isConnected ()) return &m_client->bindata; else return NULL; }
    const Xmms::Config* config () { if (m_client && m_client->isConnected ()) return &m_client->config; else return NULL; }
    const Xmms::Stats* stats () { if (m_client && m_client->isConnected ()) return &m_client->stats; else return NULL; }

	signals:
		void gotConnection (XClient *);

	private:
		std::string m_name;
		Xmms::Client *m_client;
		XClientCache *m_cache;
		XSettings *m_settings;
        bool m_isconnected;

		Xmms::Client m_sync;
};


#endif
