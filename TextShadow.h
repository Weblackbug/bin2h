////////////////////////////////////////////////////////
///            TEXT CUSTOM CONTROL   Vers. 1.0       ///
///--------------------------------------------------///
///         Sergi Serrano 2025  Open Source          ///
///   Control de texto con sombra y outline para     ///
///      Aplicaciones de Escritorio Windows C++      ///
///                  TextShadow.h                    ///
////////////////////////////////////////////////////////

#pragma once

#ifdef TEXTSHADOW_EXPORTS
#define TEXTSHADOW_API __declspec(dllexport)
#else
#define TEXTSHADOW_API __declspec(dllimport)
#endif

#include <string>
#include <windows.h>

class TEXTSHADOW_API TextControl {
public:
    TextControl();
    ~TextControl();
    void Attach(HWND hWnd);
    void Create(HWND parent, int x, int y, int width, int height, int id, HINSTANCE hInstance);
    void SetText(const std::wstring& text);
    void SetFontSize(int size);
    void SetFontName(const std::wstring& fontName);
    void SetTextColorValue(COLORREF color);
    void SetShadowColor(COLORREF color);
    void SetOutlineColor(COLORREF color);
    void SetOutlineWidth(int width);
    HWND GetHwnd() const;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
    HWND m_hWnd;
    std::wstring m_text;
    int m_fontSize;
    std::wstring m_fontName;
    COLORREF m_textColor;
    COLORREF m_shadowColor;
    COLORREF m_outlineColor;
    int m_outlineWidth;
    HFONT m_hFont;
    void UpdateFont();
};