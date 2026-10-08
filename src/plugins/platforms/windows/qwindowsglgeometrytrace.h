// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWINDOWSGLGEOMETRYTRACE_H
#define QWINDOWSGLGEOMETRYTRACE_H

#include <QtCore/qdebug.h>
#include <QtCore/qrect.h>
#include <QtCore/qt_windows.h>

QT_BEGIN_NAMESPACE

inline bool glGeometryTraceEnabled()
{
    static const bool enabled =
        qEnvironmentVariable("QT_WINDOWS_GL_GEOMETRY_TRACE") != QLatin1String("0");
    return enabled;
}

struct QWindowsGLGeometrySnapshot
{
    QRect frame;
    QRect client;
    LONG_PTR style = 0;
    LONG_PTR exStyle = 0;
    UINT dpi = 0;
    bool valid = false;
    bool iconic = false;
    bool zoomed = false;
    bool visible = false;

    static QWindowsGLGeometrySnapshot capture(HWND hwnd)
    {
        const DWORD savedError = GetLastError();
        QWindowsGLGeometrySnapshot s;
        RECT frame{}, client{};
        const bool frameValid = GetWindowRect(hwnd, &frame);
        const bool clientValid = GetClientRect(hwnd, &client);
        SetLastError(ERROR_SUCCESS);
        const bool mapped = MapWindowPoints(hwnd, nullptr, reinterpret_cast<POINT *>(&client), 2)
            || GetLastError() == ERROR_SUCCESS;
        s.valid = frameValid && clientValid && mapped;
        s.frame = QRect(frame.left, frame.top, frame.right - frame.left, frame.bottom - frame.top);
        s.client = QRect(client.left, client.top, client.right - client.left, client.bottom - client.top);
        s.style = GetWindowLongPtrW(hwnd, GWL_STYLE);
        s.exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        s.dpi = GetDpiForWindow(hwnd);
        s.iconic = IsIconic(hwnd);
        s.zoomed = IsZoomed(hwnd);
        s.visible = IsWindowVisible(hwnd);
        SetLastError(savedError);
        return s;
    }

    bool operator==(const QWindowsGLGeometrySnapshot &other) const
    {
        return frame == other.frame && client == other.client && style == other.style
            && exStyle == other.exStyle && dpi == other.dpi && valid == other.valid
            && iconic == other.iconic && zoomed == other.zoomed && visible == other.visible;
    }
};

inline QDebug operator<<(QDebug debug, const QWindowsGLGeometrySnapshot &s)
{
    QDebugStateSaver saver(debug);
    debug << "nativeValid=" << s.valid << "frame=" << s.frame << "clientScreen=" << s.client
          << "dpi=" << s.dpi << "iconic=" << s.iconic << "zoomed=" << s.zoomed
          << "visible=" << s.visible << "style=" << Qt::hex << s.style
          << "exStyle=" << s.exStyle << Qt::dec;
    return debug;
}

QT_END_NAMESPACE
#endif
