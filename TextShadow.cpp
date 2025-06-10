////////////////////////////////////////////////////////
///            TEXT CUSTOM CONTROL   Vers. 1.0       ///
///--------------------------------------------------///
///         Sergi Serrano 2025  Open Source          ///
///   Control de texto con sombra y outline para     ///
///      Aplicaciones de Escritorio Windows C++      ///
///                  TextShadow.cpp                  ///
////////////////////////////////////////////////////////
/*

Llamadas al control...

g_textControl->SetText(L"Texto sombreado de ejemplo");//Texto
g_textControl->SetFontSize(32);/Tamaño Fuente
g_textControl->SetFontName(L"Consolas"); // Cambia el tipo de fuente aquí
g_textControl->SetTextColorValue(RGB(0, 0, 255));//Color Texto
g_textControl->SetShadowColor(RGB(128, 128, 128));//Color Sombra
g_textControl->SetOutlineColor(RGB(255, 255, 255));//Color Borde del Texto
g_textControl->SetOutlineWidth(3);Tamaño del Borde

*/


#include "TextShadow.h"
#include <mutex>

#define TEXTCONTROL_CLASSNAME L"CustomTextControl"

namespace {
    std::once_flag registrationFlag;
}

TextControl::TextControl()
    : m_hWnd(nullptr), m_fontSize(24), m_fontName(L"Segoe UI"), // por defecto
    m_textColor(RGB(0, 0, 0)), m_shadowColor(RGB(128, 128, 128)),
    m_outlineColor(RGB(255, 255, 255)), m_outlineWidth(2), m_hFont(nullptr) {
}

TextControl::~TextControl() {
    if (m_hFont) {
        DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
}

void TextControl::Attach(HWND hWnd) {
    m_hWnd = hWnd;
    SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    UpdateFont();
    InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::Create(HWND parent, int x, int y, int width, int height, int id, HINSTANCE hInstance) {
    std::call_once(registrationFlag, [hInstance]() {
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = TextControl::WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = TEXTCONTROL_CLASSNAME;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

        if (!RegisterClassW(&wc)) {
            MessageBoxW(nullptr, L"Error al registrar la clase", L"Error", MB_ICONERROR);
        }
        });

    m_hWnd = CreateWindowExW(
        0,
        TEXTCONTROL_CLASSNAME,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        x, y, width, height,
        parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        hInstance,
        this
    );

    if (!m_hWnd) {
        MessageBoxW(nullptr, L"Error al crear el control", L"Error", MB_ICONERROR);
    }

    UpdateFont();
}

void TextControl::SetText(const std::wstring& text) {
    m_text = text;
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::SetFontSize(int size) {
    m_fontSize = size;
    UpdateFont();
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::SetFontName(const std::wstring& fontName) {
    m_fontName = fontName;
    UpdateFont();
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::SetTextColorValue(COLORREF color) {
    m_textColor = color;
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::SetShadowColor(COLORREF color) {
    m_shadowColor = color;
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::SetOutlineColor(COLORREF color) {
    m_outlineColor = color;
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::SetOutlineWidth(int width) {
    m_outlineWidth = (width < 0) ? 0 : (width > 5) ? 5 : width;
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

void TextControl::UpdateFont() {
    if (m_hFont) {
        DeleteObject(m_hFont);
        m_hFont = nullptr;
    }

    if (!m_hWnd) return;

    HDC hdc = GetDC(m_hWnd);
    LOGFONTW lf = { 0 };
    lf.lfHeight = -MulDiv(m_fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    lf.lfWeight = FW_BOLD;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcsncpy_s(lf.lfFaceName, m_fontName.c_str(), LF_FACESIZE - 1);
    ReleaseDC(m_hWnd, hdc);

    m_hFont = CreateFontIndirectW(&lf);
    if (!m_hFont) {
        MessageBoxW(m_hWnd, L"Error al crear la fuente", L"Error", MB_ICONERROR);
    }
}

LRESULT CALLBACK TextControl::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TextControl* pThis = reinterpret_cast<TextControl*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<TextControl*>(cs->lpCreateParams);
        if (pThis) {
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->m_hWnd = hWnd;
        }
    }

    if (!pThis)
        return DefWindowProcW(hWnd, msg, wParam, lParam);

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);

        // Doble buffering
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hbmMem);

        // Fondo transparente: copiar fondo del padre
        HWND hParent = GetParent(hWnd);
        HDC hdcParent = GetDC(hParent);
        POINT pt = { 0, 0 };
        MapWindowPoints(hWnd, hParent, &pt, 1);
        BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, hdcParent, pt.x, pt.y, SRCCOPY);
        ReleaseDC(hParent, hdcParent);

        SetBkMode(hdcMem, TRANSPARENT);
        HFONT oldFont = static_cast<HFONT>(SelectObject(hdcMem, pThis->m_hFont));

        // Sombra
        SetTextColor(hdcMem, pThis->m_shadowColor);
        TextOutW(hdcMem,
            pThis->m_outlineWidth + 2,
            pThis->m_outlineWidth + 2,
            pThis->m_text.c_str(),
            static_cast<int>(pThis->m_text.length()));

        // Contorno
        SetTextColor(hdcMem, pThis->m_outlineColor);
        for (int dx = -pThis->m_outlineWidth; dx <= pThis->m_outlineWidth; ++dx) {
            for (int dy = -pThis->m_outlineWidth; dy <= pThis->m_outlineWidth; ++dy) {
                if (dx == 0 && dy == 0) continue;
                TextOutW(hdcMem,
                    pThis->m_outlineWidth + dx,
                    pThis->m_outlineWidth + dy,
                    pThis->m_text.c_str(),
                    static_cast<int>(pThis->m_text.length()));
            }
        }

        // Texto principal
        SetTextColor(hdcMem, pThis->m_textColor);
        TextOutW(hdcMem,
            pThis->m_outlineWidth,
            pThis->m_outlineWidth,
            pThis->m_text.c_str(),
            static_cast<int>(pThis->m_text.length()));

        // Copiar a pantalla
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

        // Limpieza
        SelectObject(hdcMem, oldFont);
        SelectObject(hdcMem, oldBmp);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        if (pThis->m_hFont) {
            DeleteObject(pThis->m_hFont);
            pThis->m_hFont = nullptr;
        }
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}