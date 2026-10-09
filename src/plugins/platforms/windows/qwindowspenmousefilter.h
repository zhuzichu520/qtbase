// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWINDOWSPENMOUSEFILTER_H
#define QWINDOWSPENMOUSEFILTER_H

#include <QtCore/qnamespace.h>

QT_BEGIN_NAMESPACE

// Windows can regenerate WM_MOUSEMOVE without the source marker of the pen's
// compatibility mouse press. Keep its button state out of the mouse stream too.
class QWindowsPenMouseFilter
{
public:
    void recordSuppressedButton(Qt::MouseButton button, bool pressed)
    {
        if (pressed)
            m_suppressedButtons |= button;
        else
            m_suppressedButtons &= ~button;
    }

    void recordDeliveredPress(Qt::MouseButton button)
    {
        m_suppressedButtons &= ~button;
    }

    Qt::MouseButtons filteredButtons(Qt::MouseButtons buttons) const
    {
        return buttons & ~m_suppressedButtons;
    }

private:
    Qt::MouseButtons m_suppressedButtons;
};

QT_END_NAMESPACE

#endif // QWINDOWSPENMOUSEFILTER_H
