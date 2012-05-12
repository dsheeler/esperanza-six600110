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

#include "collectioninfomodel.h"
#include <QDebug>

CollectionInfoModel::CollectionInfoModel (QObject *parent, XClient *client, const bool &all_field)
{
    m_client = client;
    m_all_field = all_field;
}

std::list<std::string>
CollectionInfoModel::str_list_to_std (const QList<QString> &list)
{
    std::list<std::string> ret;
    for (int i = 0; i < list.size (); i ++) {
        ret.push_back (XClient::qToStd (list.value (i)));
    }

    return ret;
}

void
CollectionInfoModel::set_collection (const Xmms::Coll::Coll &ref,
                                     const QList<QString> &fetch,
                                     const QList<QString> &order,
                                     const QList<QString> &group) 
{
	m_client->collection ()->queryInfos (ref, str_list_to_std (fetch), str_list_to_std (order), 0, 0, str_list_to_std (group))
	        (Xmms::bind (&CollectionInfoModel::info_callback, this));
}

bool
CollectionInfoModel::info_callback (const Xmms::List<Xmms::Dict> &list)
{
    /* convert the first entry in the list to get the headers */
	QList < QHash < QString, QVariant > > l;
	Xmms::List<Xmms::Dict>::const_iterator it_end = list.end();
	for (Xmms::List<Xmms::Dict>::const_iterator it = list.begin();
	     it != it_end; ++it) {
		QHash<QString, QVariant> h = XClient::convert_dict (*it);
		if (!h.isEmpty ())
			l.append (h);
	}
	
	set_data (l);

	return true;
}

void
CollectionInfoModel::set_data (const QList < QHash < QString, QVariant > > &data)
{
	/* Clear the model */
	beginRemoveRows (QModelIndex (), 0, rowCount ());
	clear ();
	endRemoveRows ();

	if (data.size() == 0)
	{
		// FIXME: This should not happen, but it does
		qDebug() << "FIXME:" << __FILE__ << " line: " << __LINE__;
		return;
	}

    /* take the headers and add the columns */
    QStringList s = data[0].keys ();
    setColumnCount (s.size ());
    
    for (int i = 0; i < s.size (); i ++) {
        /* capitilize this string later */
        QStandardItem *m = new QStandardItem (s.value (i));
        m->setEditable (false);
        setHorizontalHeaderItem (i, m);
    }
    
    /* run through them */
    if (m_all_field) {
        QStandardItem *item = new QStandardItem (tr ("[ All ]"));
        item->setEditable (false);
        appendRow (item);
    }

	for (int i = 0; i < data.size (); i ++) {
        QList<QStandardItem *> l;

		QHash<QString, QVariant> hash = data[i];
        for (int i = 0; i < s.size (); i ++) {
            QStandardItem *item = new QStandardItem (hash[s.value (i)].toString ());
            item->setEditable (false);
            l.append (item);
        }
        appendRow (l);
	}
	
}

