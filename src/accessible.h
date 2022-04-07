#pragma once

#include "accessibledefine.h"

#include "multitasking.h"

SET_FORM_ACCESSIBLE(DesktopThumbnailManager, "desktopThumbnailManager")


QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    USE_ACCESSIBLE(classname, DesktopThumbnailManager);

    return interface;
}
