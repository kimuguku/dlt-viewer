/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file treemodel.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>
#include <qmessagebox.h>

#include "tablemodel.h"

char buffer[DLT_VIEWER_LIST_BUFFER_SIZE];

 TableModel::TableModel(const QString & /*data*/, QObject *parent)
     : QAbstractTableModel(parent)
 {

 }

 TableModel::~TableModel()
 {

 }

 int TableModel::columnCount(const QModelIndex & /*parent*/) const
 {
     return 12;
 }

 QVariant TableModel::data(const QModelIndex &index, int role) const
 {
     QDltMsg msg;
     QByteArray buf;

     if (!index.isValid())
         return QVariant();

     if (index.row() >= qfile->sizeFilter() && index.row()<0)
         return QVariant();

     if (role == Qt::DisplayRole)
     {
         /* get the message with the selected item id */
         qfile->getMsg(qfile->getMsgFilterPos(index.row()), msg); // getMsg can be better optimized than the two single calls
         for(int num = 0; num < project->plugin->topLevelItemCount (); num++)
         {
             PluginItem *item = (PluginItem*)project->plugin->topLevelItem(num);

             if(item->getMode() != item->ModeDisable && item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg,0))
             {
                 item->plugindecoderinterface->decodeMsg(msg,0);
                 break;
             }
         }

         switch(index.column())
         {
         case 0:
             /* display index */
             return QString("%1").arg(qfile->getMsgFilterPos(index.row()));
         case 1:
             if( project->settings->automaticTimeSettings == 0 )
                return QString("%1.%2").arg(msg.getGmTimeWithOffsetString(project->settings->utcOffset,project->settings->dst)).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
             else
                return QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
         case 2:
             return QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
         case 3:
             return QString("%1").arg(msg.getMessageCounter());
         case 4:
             return msg.getEcuid();
         case 5:
             switch(project->settings->showApIdDesc){
             case 0:
                 return msg.getApid();
                 break;
             case 1:
                   for(int num = 0; num < project->ecu->topLevelItemCount (); num++)
                    {
                     EcuItem *ecuitem = (EcuItem*)project->ecu->topLevelItem(num);
                     for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
                     {
                         ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);
                         if(appitem->id == msg.getApid() && !appitem->description.isEmpty())
                         {
                            return appitem->description;
                         }
                     }
                    }
                   return QString("Apid: %1 (No description)").arg(msg.getApid());
                 break;
              default:
                 return msg.getApid();
             }
         case 6:
             switch(project->settings->showCtIdDesc){
             case 0:
                 return msg.getCtid();
                 break;
             case 1:

                   for(int num = 0; num < project->ecu->topLevelItemCount (); num++)
                    {
                     EcuItem *ecuitem = (EcuItem*)project->ecu->topLevelItem(num);
                     for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
                     {
                         ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);
                         for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
                         {
                             ContextItem * conitem = (ContextItem *) appitem->child(numcontext);

                             if(appitem->id == msg.getApid() && conitem->id == msg.getCtid()
                                     && !conitem->description.isEmpty())
                             {
                                return conitem->description;
                             }
                         }
                     }
                    }
                   return  QString("Ctid: %1 (No description)").arg(msg.getCtid());
                 break;
              default:
                 return msg.getCtid();
             }
         case 7:
             return msg.getTypeString();
         case 8:
             return msg.getSubtypeString();
         case 9:
             return msg.getModeString();
         case 10:
             return QString("%1").arg(msg.getNumberOfArguments());
         case 11:
             /* display payload */
             return msg.toStringPayload();
         }
     }

     if ( role == Qt::ForegroundRole ) {
         qfile->getMsg(qfile->getMsgFilterPos(index.row()), msg); // getMsg can be better optimized than the two single calls
         if(project->settings->autoMarkFatalError && !qfile->checkMarker(msg).isValid() && ( msg.getSubtypeString() == "error" || msg.getSubtypeString() == "fatal")  ){
            return QVariant(QBrush(QColor(255,255,255)));
         } else {
            return QVariant(QBrush(QColor(0,0,0)));
         }
     }

     if ( role == Qt::BackgroundRole ) {
         qfile->getMsg(qfile->getMsgFilterPos(index.row()), msg); // getMsg can be better optimized than the two single calls
         for(int num = 0; num < project->plugin->topLevelItemCount (); num++)
         {
             PluginItem *item = (PluginItem*)project->plugin->topLevelItem(num);

             if(item->getMode() != item->ModeDisable && item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg,0))
             {
                 item->plugindecoderinterface->decodeMsg(msg,0);
                 break;
             }
         }

         QColor color = qfile->checkMarker(msg);
         if(color.isValid())
         {
            return QVariant(QBrush(color));
         }
         else
         {
             if(project->settings->autoMarkFatalError && ( msg.getSubtypeString() == "error" || msg.getSubtypeString() == "fatal") ){
                return QVariant(QBrush(QColor(255,0,0)));
             }
             if(project->settings->autoMarkWarn && msg.getSubtypeString() == "warn"){
                return QVariant(QBrush(QColor(255,255,0)));
             }

             return QVariant(QBrush(QColor(255,255,255)));
         }
     }

     if ( role == Qt::TextAlignmentRole ) {
        switch(index.column())
        {
            case 0:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case 1:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 2:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case 3:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 4:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 5:
                switch(project->settings->showApIdDesc){
                case 0:
                    return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
                    break;
                case 1:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                }
            case 6:
                switch(project->settings->showCtIdDesc){
                case 0:
                    return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
                    break;
                case 1:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                }
            case 7:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 8:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 9:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 10:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case 11:
                return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
        }
    }

     return QVariant();
 }

  QVariant TableModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
 {
      if (role != Qt::DisplayRole)
          return QVariant();

      if (orientation == Qt::Horizontal)
      {
          switch(section)
          {
          case 0:
              return QString("Index");
          case 1:
              return QString("Time");
          case 2:
              return QString("Timestamp");
          case 3:
              return QString("Count");
          case 4:
              return QString("Ecuid");
          case 5:
              switch(project->settings->showApIdDesc){
              case 0:
                   return QString("Apid");
              case 1:
                   return QString("Apid Desc");
              }
          case 6:
              switch(project->settings->showCtIdDesc){
              case 0:
                   return QString("Ctid");
              case 1:
                   return QString("Ctid Desc");
              }
          case 7:
              return QString("Type");
          case 8:
              return QString("Subtype");
          case 9:
              return QString("Mode");
          case 10:
              return QString("#Args");
          case 11:
              return QString("Payload");
          }
      }

      return QVariant();
  }

 int TableModel::rowCount(const QModelIndex & /*parent*/) const
 {
     return qfile->sizeFilter();
 }

 void TableModel::modelChanged()
 {
     QModelIndex lIndex = index(0, 1);
     QModelIndex lLeft = index(qfile->sizeFilter()-1, 0);
     QModelIndex lRight = index(qfile->sizeFilter()-1, columnCount() - 1);
     emit(layoutChanged());
 }
