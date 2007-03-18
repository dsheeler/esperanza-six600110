/** 
 *  This file is a part of Esperanza, an XMMS2 Client.
 *
 *  Copyright (C) 2005-2006 XMMS2 Team
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

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QErrorMessage>
#include <QSettings>

#include "xclient.h"
#include "xmmsqt4.h"

XSettings::XSettings (QObject *parent) : QObject (parent)
{
	/* dummy */
}

void
XSettings::change_settings ()
{
	emit settingsChanged ();
}

bool XClient::log ()
{
	return false;
}

bool
XClient::dummy_uint (const uint32_t &)
{
	return false;
}

QString
XClient::stdToQ (const std::string &str)
{
	return QString::fromUtf8 (str.c_str ());
}

std::string
XClient::qToStd (const QString &str)
{
	return std::string (str.toUtf8 ().data ());
}


XClient::XClient (QObject *parent, const std::string &name) : QObject (parent), Xmms::Client (name), m_sync (name + "-sync")
{
	m_cache = new XClientCache (this, this);
	m_settings = new XSettings (this);
}

bool
XClient::connect (const char *ipcpath)
{
	bool tried_once = false;

try_again:

	try {
		Xmms::Client::connect (ipcpath);
	}
	catch (Xmms::connection_error& e) {
		if (ipcpath == NULL && !tried_once) {
			QSettings s;
			if (s.value ("core/autostart", true).toBool ()) {
				if (!system ("xmms2-launcher")) {
					tried_once = true;
					goto try_again;
				}
			}
		}

		QErrorMessage *err = new QErrorMessage ();
		err->showMessage ("Couldn't connect to XMMS2, please try again.");
		err->exec ();
		delete err;
		return false;
	}

	setMainloop (new XmmsQT4 (getConnection ()));

	try {
		m_sync.connect (ipcpath);
	}
	catch (Xmms::connection_error &e) {
		qWarning ("Couldn't establish sync connection!");
	}

	emit gotConnection (this);

	return true;
}

void
XClient::propDictToQHash (const std::string &key,
						  const Xmms::Dict::Variant &value,
						  const std::string &source,
						  QHash<QString, QVariant> &hash)
{
	if (value.type () == typeid (int32_t)) {
		hash.insert (QString::fromLatin1 (key.c_str ()),
		             QVariant (boost::get< int32_t > (value)));
	} else if (value.type () == typeid (uint32_t)) {
		hash.insert (QString::fromLatin1 (key.c_str ()),
		             QVariant (boost::get< uint32_t > (value)));
	} else {
		QString val;
		if (key == "url") {
			/* This really is wrong ...*/
			char *c = const_cast<char *>(xmmsc_result_decode_url (NULL, boost::get< std::string >(value).c_str ()));
			val = QString::fromUtf8 (c);
			val = val.mid (val.lastIndexOf ("/") + 1);
			if (val.isEmpty ()) {
				val = QString::fromUtf8 (c);
			}
			free (c);
		} else {
			val = QString::fromUtf8 (boost::get< std::string > (value).c_str ());
		}

		hash.insert (QString::fromLatin1 (key.c_str ()),
		             QVariant (val));
	}
}

QHash<QString, QVariant>
XClient::convert_propdict (const Xmms::PropDict &dict)
{
	QHash<QString, QVariant> hash;
	dict.each (boost::bind (&XClient::propDictToQHash,
							_1, _2, _3, boost::ref (hash)));

	return hash;
}


