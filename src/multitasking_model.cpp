/*
 * Copyright (C) 2020 Uniontech Technology Co., Ltd.
 *
 * Author:     Lei Su <axylp@qq.com>
 *
 * Maintainer: Lei Su <axylp@qq.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "multitasking_model.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QScreen>

const QString dbusDeepinDaemonDisplayService = "com.deepin.daemon.Display";
const QString dbusDeepinDaemonDisplayObj = "/com/deepin/daemon/Display";
const QString dbusDeepinDaemonDisplayIntf = "com.deepin.daemon.Display";

DesktopThumbnailItem::DesktopThumbnailItem()
{

}

MultitaskingModel::MultitaskingModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentIndex(0)
    , m_nCurrentSelectIndex(-1)
{
}

MultitaskingModel::~MultitaskingModel()
{
}

int MultitaskingModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_desktopThumbnailItemList.count();
}

QVariant MultitaskingModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_desktopThumbnailItemList.count()) {
       return QVariant();
    }

    switch (role) {
    case ThumbnailRole:
//        return data.thumbnail();
    default:
        break;
    }

    return QVariant();
}



QHash<int, QByteArray> MultitaskingModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ThumbnailRole] = "dmThumbnail";
    return roles;
}

void MultitaskingModel::setWindows(EffectScreen * screen, int desktop, const QVariantList &windows)
{
    m_windows[screen][desktop] = windows;
}

QVariantList MultitaskingModel::windows(EffectScreen * screen, int desktop) const
{
    return m_windows[screen][desktop];
}

bool MultitaskingModel::isCurrentScreenWindows(EffectScreen * screen, int desktop, QVariant wid)
{
    int flag =m_windows[screen][desktop].indexOf(wid);
    if (flag == -1) {
        return false;
    }
    return true;
}

void MultitaskingModel::moveToScreen(EffectScreen * screen, int desktop, QVariant wid)
{
    QPair<EffectScreen*, int> sd = getScreenDesktopByWinID(m_nCurrentSelectIndex);
    EffectScreen* scrn = sd.first;
    int desk = sd.second;
    m_windows[scrn][desk].removeOne(wid);
    m_windows[screen][desktop].push_back(wid);
    emit currentIndexChanged(m_currentIndex);
}

int MultitaskingModel::getCalculateRowCount(EffectScreen * screen, int desktop)
{
    int ClientCount = getDesktopClientCount(screen,desktop);
    int ColumnCount = getCalculateColumnsCount(screen,desktop);
    if (ColumnCount == 0) {
        return 0;
    }
    int RowCount = ClientCount/ColumnCount;
    if (ClientCount%ColumnCount > 0) {
        RowCount++;
    }
    return RowCount;
}

int MultitaskingModel::getCalculateColumnsCount(EffectScreen * screen, int desktop)
{
    int ClientCount = m_windows[screen][desktop].size();
    int ColumnCount  = sqrt(ClientCount);
    int surplusClientCount = ClientCount - ColumnCount*ColumnCount;
    int ColumnCountTemp = ColumnCount;
    while (surplusClientCount > 0) {
        ColumnCount++;
        surplusClientCount = surplusClientCount - ColumnCountTemp;
    }
    return ColumnCount;
}

int MultitaskingModel::getDesktopClientCount(EffectScreen * screen, int desktop)
{
    return m_windows[screen][desktop].size();
}


QList<EffectScreen*> MultitaskingModel::numScreens() const
{
    return effects->screens();
}

QRect MultitaskingModel::screenGeometry(EffectScreen * screen) const
{
    return effects->clientArea(FullScreenArea,screen,effects->currentDesktop());
}


void MultitaskingModel::setCurrentIndex(int index)
{
    if (m_currentIndex != index) {
        m_currentIndex = index;
        emit currentIndexChanged(m_currentIndex);
        emit currentDesktopChanged(m_currentIndex + 1);
    }
}

int MultitaskingModel::currentIndex() const
{
	return m_currentIndex;
}

void MultitaskingModel::load(int desktopCount)
{
	clear();
	int index = m_desktopThumbnailItemList.count();
	for (int i = 0; i < desktopCount; ++i) { 
		DesktopThumbnailItem data;
		emit beginInsertRows(QModelIndex(), index, index);
		m_desktopThumbnailItemList.append(data);
		emit endInsertRows();
	}
	emit countChanged(m_desktopThumbnailItemList.count());
}

void MultitaskingModel::append()
{
	int index = m_desktopThumbnailItemList.count();
    if (index < 0 || index >= 4) {
       return;
    }

	DesktopThumbnailItem data;
    emit beginInsertRows(QModelIndex(), index, index);
    m_desktopThumbnailItemList.append(data);
    emit endInsertRows();
    emit appendDesktop();
    emit countChanged(m_desktopThumbnailItemList.count());
}

void MultitaskingModel::remove(int index)
{
    if (index < 0 || index >= m_desktopThumbnailItemList.count()) {
        return;
    }

    if (m_desktopThumbnailItemList.count() == 1)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_desktopThumbnailItemList.removeAt(index);
    endRemoveRows();
	emit removeDesktop(index + 1);
    emit countChanged(m_desktopThumbnailItemList.count());

    if (index > m_currentIndex)
    {
        return;
    }

    if (index < m_currentIndex)
    {
        setCurrentIndex(m_currentIndex - 1);
        return;
    }

    if (index == m_currentIndex && index == 0)
    {
        m_currentIndex = 1;
        setCurrentIndex(0);
    } else {
        setCurrentIndex(m_currentIndex - 1);
    }
}

void MultitaskingModel::clear()
{
	emit beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
	m_desktopThumbnailItemList.clear();
	emit endRemoveRows();
	emit countChanged(0);
}

int MultitaskingModel::count() const
{
    return rowCount(QModelIndex());
}

void MultitaskingModel::move(int from, int to) 
{
    assert(from != to);

    // The best way is to save pixmap in m_desktopThumbnailItemList
    // and move pixmap in m_desktopThumbnailItemList in the middle of
    // beginMoveRows() and endMoveRows();
    // but let's refactor step by step
    int dest = to;
    if( to > from && to <= rowCount(QModelIndex()) - 1)
        dest += 1;
    if(!beginMoveRows(QModelIndex(), from, from, QModelIndex(), dest))
        return;
    endMoveRows();

    emit switchDesktop(to+1, from+1);
    if(from == m_currentIndex) {
        setCurrentIndex(to);
        return;
    }
    if(to <= m_currentIndex && m_currentIndex < from)
        setCurrentIndex(m_currentIndex + 1);
    if(from < m_currentIndex && m_currentIndex <= to)
        setCurrentIndex(m_currentIndex - 1);
}

void MultitaskingModel::setCurrentSelectIndex(int index)
{
    m_nCurrentSelectIndex = index;
    emit currentWindowThumbnailChanged();
}

int MultitaskingModel::currentSelectIndex() const
{
    return  m_nCurrentSelectIndex;
}

void MultitaskingModel::updateWindowDestop( int nDesktop )
{
    emit currentIndexChanged( nDesktop - 1 );
}

QPair<KWin::EffectScreen*, int> MultitaskingModel::getScreenDesktopByWinID(int winid)
{
    QPair<KWin::EffectScreen*,int> scrnDesk;
    EffectWindow *ew = effects->findWindow(winid);
    scrnDesk.first = effects->screenAt(ew->pos());
    const int desktopCount = effects->numberOfDesktops();
    if (ew->isOnAllDesktops()) {
        scrnDesk.second = m_currentIndex+1;
    } else {
        for (int d = 1; d <= desktopCount; ++d) {
            if (ew->isOnDesktop(d)) {
                scrnDesk.second = d;
                break;
            }
        }
    }
    return scrnDesk;
}

void MultitaskingModel::selectNextWindow()
{
    if (m_nCurrentSelectIndex == -1 || m_nCurrentSelectIndex == 0) {
        return;
    }
    int winid = getNextWindowID();
    setCurrentSelectIndex(winid);
}
void MultitaskingModel::selectPrevWindow()
{    
    if (m_nCurrentSelectIndex == -1 || m_nCurrentSelectIndex == 0) {
        return;
    }
    int winid = getPrevWindowID();
    setCurrentSelectIndex(winid);
}
void MultitaskingModel::selectNextWindowVert(int dir)
{
    if (m_nCurrentSelectIndex == -1 || m_nCurrentSelectIndex == 0) {
        return;
    }

    QPair<KWin::EffectScreen*,int> sd = getScreenDesktopByWinID(m_nCurrentSelectIndex);
    KWin::EffectScreen* scrn = sd.first;
    int desk = sd.second;
    
    int rows = getCalculateRowCount(scrn, desk);
    if (rows <= 1) return;
    int columns = getCalculateColumnsCount(scrn, desk);

    int fromIndex = m_windows[scrn][desk].indexOf(m_nCurrentSelectIndex);
    int toIndex = fromIndex + dir * columns;
    QVariantList winlist = m_windows[scrn][desk];

    if (dir == 1 && toIndex < winlist.size() ) {
        setCurrentSelectIndex(winlist[toIndex].toInt());
    }
    if (dir == -1 && toIndex >= 0) {
        setCurrentSelectIndex(winlist[toIndex].toInt());
    }
}

int MultitaskingModel::getNextWindowID()
{
    QPair<KWin::EffectScreen*,int> sd = getScreenDesktopByWinID(m_nCurrentSelectIndex);
    KWin::EffectScreen* scrn = sd.first;
    int desk = sd.second;

    int winindex = m_windows[scrn][desk].indexOf(m_nCurrentSelectIndex);
    assert(winindex >= 0);
    if (winindex == m_windows[scrn][desk].size() - 1) // at the end of current screen win list
    {
        if (scrn == effects->screens().last()) // at the last screen
        {
            if (m_windows[0][desk].size() == 0) { // if first screen has no winthumb
                return m_windows[scrn][desk].first().toInt();
            } else {
                return m_windows[0][desk].first().toInt();
            }
        } else {
            if (m_windows[scrn+1][desk].size() == 0) { // if next screen has no winthumb
                return m_windows[scrn][desk].first().toInt();
            } else {
                return m_windows[scrn+1][desk].first().toInt();
            }
        }
    } else {
        return m_windows[scrn][desk][winindex+1].toInt();
    }
}

int MultitaskingModel::getPrevWindowID()
{
    QPair<KWin::EffectScreen*,int> sd = getScreenDesktopByWinID(m_nCurrentSelectIndex);
    KWin::EffectScreen* scrn = sd.first;
    int desk = sd.second;

    int winindex = m_windows[scrn][desk].indexOf(m_nCurrentSelectIndex);

    assert(winindex >= 0);
    
    if (winindex == 0) // at the first of current screen win list
    {
        if (scrn == 0) // at the first screen
        {
            if (m_windows[numScreens().last()][desk].size() == 0) { // if end screen has no winthumb
                return m_windows[scrn][desk].last().toInt();
            } else {
                return m_windows[numScreens().last()][desk].last().toInt();
            }
        } else {
            if (m_windows[scrn-1][desk].size() == 0) { // if previous screen has no winthumb
                return m_windows[scrn][desk].last().toInt();
            } else {
                return m_windows[scrn-1][desk].last().toInt();
            }
        }
    } else {
        return m_windows[scrn][desk][winindex-1].toInt();
    }
}

bool MultitaskingModel::isAllScreensEmpty()
{
    for (auto screen : effects->screens()) {
        for (int desk = 1; desk <= m_desktopThumbnailItemList.size(); ++desk)
        {
            if (m_windows[screen][desk].size() > 0)
            {
                return false;
            }
        }
    }

    return true;
}

void MultitaskingModel::windowSelected( QVariant winId )
{
    emit windowSelectedSignal( winId );
}

QPixmap MultitaskingModel::getWindowIcon( QVariant winId )
{
    EffectWindow *ew = effects->findWindow(winId.toULongLong());
    QPixmap pixmap;
    if (ew)
    {
        pixmap = ew->icon().pixmap( QSize(Constants::ICON_SIZE, Constants::ICON_SIZE) );
    }
    return pixmap;

}

bool MultitaskingModel::isCurrentScreensEmpty()
{
    for (auto screen : effects->screens()) {
        if (m_windows[screen][m_currentIndex + 1].size()) {
            return false;
        }
    }

    return true;
}

int MultitaskingModel::getWindowHeight(QVariant winId)
{
    EffectWindow *ew = effects->findWindow(winId.toULongLong());
    if (!ew) {
       return -1; //ERROR code
    }
    return ew->height();
}

int MultitaskingModel::getWindowWidth(QVariant winId)
{
    EffectWindow *ew = effects->findWindow(winId.toULongLong());
    if (!ew) {
       return -1; //ERROR code
    }
    return ew->width();
}

bool MultitaskingModel::getWindowKeepAbove(QVariant winId)
{
    EffectWindow *ew = effects->findWindow(winId.toULongLong());
    return ew->keepAbove();
}

void MultitaskingModel::setWindowKeepAbove(QVariant winId)
{
    EffectWindow *ew = effects->findWindow(winId.toULongLong());
    WId keepAboveWId = 0;
    for (auto wid: KWindowSystem::self()->windows()) {
        if (effects->findWindow(wid) == ew) {
            keepAboveWId = wid;
            break;
        }
    }

    if (keepAboveWId == 0) {
        return;
    }

    if (ew->keepAbove())
    {
        KWindowSystem::self()->clearState(keepAboveWId, NET::KeepAbove);
    } else {
        KWindowSystem::self()->setState(keepAboveWId, NET::KeepAbove);
    }
}

void MultitaskingModel::forceResetModel()
{
    beginResetModel();
    endResetModel();
}

void MultitaskingModel::selectNextSametypeWindow()
{
    if (m_nCurrentSelectIndex == -1 || m_nCurrentSelectIndex == 0) {
        return;
    }
    int winid = getNextSametypeWindowID();
    setCurrentSelectIndex(winid);
}

void MultitaskingModel::selectPrevSametypeWindow()
{
    if (m_nCurrentSelectIndex == -1 || m_nCurrentSelectIndex == 0) {
        return;
    }
    int winid = getPrevSametypeWindowID();
    setCurrentSelectIndex(winid);
}

int MultitaskingModel::getNextSametypeWindowID()
{
    QPair<EffectScreen*,int> sd = getScreenDesktopByWinID(m_nCurrentSelectIndex);
    QMap<EffectScreen*, QMap<int, QVariantList> > windowsClass;
    EffectScreen* scrn = sd.first;
    int desk = sd.second;
    int winindex = m_windows[scrn][desk].indexOf(m_nCurrentSelectIndex);
    EffectWindow *ew = effects->findWindow(m_windows[scrn][desk][winindex].toULongLong());

    for (auto screen : effects->screens()) {
        for (int j=0; j < m_windows[screen][desk].size(); j++) {
            if (ew->windowClass() == effects->findWindow(m_windows[screen][desk][j].toULongLong())->windowClass())
            {
                windowsClass[screen][desk].push_back(m_windows[screen][desk][j]);
            }
        }
    }

    int winClassIndex = windowsClass[scrn][desk].indexOf(m_nCurrentSelectIndex);
    if (winClassIndex == windowsClass[scrn][desk].size() - 1) // at the end of current screen win list
    {
        if (scrn == effects->screens().last())  // at the last screen
        {
            if (windowsClass[0][desk].size() == 0) { // if first screen has no winthumb
                return windowsClass[scrn][desk].first().toInt();
            } else {
                return windowsClass[0][desk].first().toInt();
            }
        } else {
            if (windowsClass[scrn+1][desk].size() == 0) { // if next screen has no winthumb
                return windowsClass[scrn][desk].first().toInt();
            } else {
                return windowsClass[scrn+1][desk].first().toInt();
            }
        }
    } else {
        return windowsClass[scrn][desk][winClassIndex+1].toInt();
    }
}

int MultitaskingModel::getPrevSametypeWindowID()
{
    QPair<EffectScreen*,int> sd = getScreenDesktopByWinID(m_nCurrentSelectIndex);
    QMap<EffectScreen*, QMap<int, QVariantList> > windowsClass;
    EffectScreen* scrn = sd.first;
    int desk = sd.second;
    int winindex = m_windows[scrn][desk].indexOf(m_nCurrentSelectIndex);
    EffectWindow *ew = effects->findWindow(m_windows[scrn][desk][winindex].toULongLong());

    for (auto screen : effects->screens()) {
        for (int j=0; j < m_windows[screen][desk].size(); j++) {
           if (ew->windowClass() == effects->findWindow(m_windows[screen][desk][j].toULongLong())->windowClass())
           {
              windowsClass[screen][desk].append(m_windows[screen][desk][j]);
           }
        }
    }

    int winClassIndex = windowsClass[scrn][desk].indexOf(m_nCurrentSelectIndex);

    if (winClassIndex == 0) // at the first of current screen win list
    {
        if (scrn == 0) // at the first screen
        {
            if (windowsClass[numScreens().last()][desk].size() == 0) { // if end screen has no winthumb
                return windowsClass[scrn][desk].last().toInt();
            } else {
                return windowsClass[numScreens().last()][desk].last().toInt();
            }
        } else {
            if (windowsClass[scrn-1][desk].size() == 0) { // if previous screen has no winthumb
                return windowsClass[scrn][desk].last().toInt();
            } else {
                return windowsClass[scrn-1][desk].last().toInt();
            }
        }
    } else {
        return windowsClass[scrn][desk][winClassIndex-1].toInt();
    }
}

int MultitaskingModel::displayMode() const
{
    QDBusInterface wm(dbusDeepinDaemonDisplayService, dbusDeepinDaemonDisplayObj, dbusDeepinDaemonDisplayIntf);
    auto displayMode = wm.property("DisplayMode");
    return displayMode.toInt();
}

bool MultitaskingModel::isExtensionMode() const
{
   QDesktopWidget desktop;
   for (int i = 0; i < desktop.numScreens(); ++i) {
       if (effects->virtualScreenGeometry() == desktop.screenGeometry(i)) {
           return false;
       }
   }
   return true;
}

QString MultitaskingModel::screenName(int x,int y)
{
    QScreen *pScreen = QGuiApplication::screenAt(QPoint(x,y));
    return pScreen->name();
}

int MultitaskingModel::currentDesktop()
{
    return m_currentIndex + 1;
}

int MultitaskingModel::lastNoEmptyScreen()
{
    int screenNumber = numScreens().length();
    for (int i = screenNumber; i > 0; i--)
    {
        if(!m_windows[numScreens()[i-1]][currentDesktop()].empty())
            return i-1;
    }
    return -1;
}

int MultitaskingModel::firstNoEmptyScreen()
{
    int screenNumber = numScreens().length();
    for (int i = 0; i < screenNumber; i++)
    {
        if(!m_windows[numScreens()[i]][currentDesktop()].empty())
            return i;
    }
    return -1;
}

void MultitaskingModel::selectFirstWindow()
{
    if (m_windows.empty()|| // no screen
        m_windows.first().empty()) // no workspace
        return;
    int firstScreen = firstNoEmptyScreen();
    if(firstScreen < 0)
        return;
    setCurrentSelectIndex(m_windows[numScreens()[firstScreen]][currentDesktop()].first().toInt());
}

void MultitaskingModel::selectLastWindow()
{
    if (m_windows.empty()|| // no screen
        m_windows.last().empty()) // no workspace
        return;

    int lastScreen = lastNoEmptyScreen();
    if(lastScreen <0)
        return;
    setCurrentSelectIndex(m_windows[numScreens()[lastNoEmptyScreen()]][currentDesktop()].last().toInt());
}
