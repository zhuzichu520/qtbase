// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWINDOWSWINTABMOUSEFILTER_H
#define QWINDOWSWINTABMOUSEFILTER_H

#include <QtCore/qpoint.h>
#include <QtCore/qnamespace.h>
#include <array>

QT_BEGIN_NAMESPACE

// Some WinTab drivers inject a second mouse stream without MI_WP_SIGNATURE.
// Match only injected button presses to tablet transitions actually delivered to Qt.
class QWindowsWinTabMouseFilter
{
public:
    void recordTabletEvent(quint32 time, const QPoint &position, Qt::MouseButtons buttons,
                           quint64 now)
    {
        const auto pressed = buttons & ~m_tabletButtons;
        m_tabletButtons = buttons;
        if (pressed != Qt::NoButton) {
            m_presses[m_next] = {time, position, pressed, now};
            m_next = (m_next + 1) % m_presses.size();
        }
    }

    void leaveProximity() { m_tabletButtons = Qt::NoButton; }

    bool suppressMouseMove(bool injected) const
    {
        // The duplicate stream must not acquire a Qt Quick grab between its
        // suppressed press and release. Tablet movement is delivered separately.
        return injected && m_suppressedButtons != Qt::NoButton;
    }

    bool suppressMouseEvent(quint32 time, const QPoint &position, Qt::MouseButton button,
                            bool pressed, bool injected, quint64 now)
    {
        if (button == Qt::NoButton)
            return false;
        if (!injected) {
            // A real mouse press starts its own gesture, even while the pen hovers.
            if (pressed)
                m_suppressedButtons &= ~button;
            return false;
        }
        if (!pressed) {
            // Pair releases with suppressed presses, even after proximity loss or
            // a long stroke. Never swallow an unmatched injected release.
            const bool suppress = m_suppressedButtons.testFlag(button);
            m_suppressedButtons &= ~button;
            return suppress;
        }

        m_suppressedButtons &= ~button;
        for (auto &press : m_presses) {
            // DWORD subtraction also handles the Windows message clock wrapping.
            if (press.buttons.testFlag(button) && quint32(time - press.time) <= 50
                && now - press.receivedAt <= 500
                && (position - press.position).manhattanLength() <= 4) {
                press.buttons &= ~button;
                m_suppressedButtons |= button;
                return true;
            }
        }
        return false;
    }

private:
    struct Press
    {
        quint32 time = 0;
        QPoint position;
        Qt::MouseButtons buttons;
        quint64 receivedAt = 0;
    };
    // Retain transitions across packet batches; movement must not evict a press.
    std::array<Press, 16> m_presses{};
    size_t m_next = 0;
    Qt::MouseButtons m_tabletButtons;
    Qt::MouseButtons m_suppressedButtons;
};

QT_END_NAMESPACE

#endif // QWINDOWSWINTABMOUSEFILTER_H
